import time
import math
import chess
from typing import Optional, Tuple, Dict, List

PIECE_VALUE = {
    chess.PAWN: 100,
    chess.KNIGHT: 320,
    chess.BISHOP: 330,
    chess.ROOK: 500,
    chess.QUEEN: 900,
    chess.KING: 20000,
}

def evaluate_capture_value(piece_type: int) -> int:
    return PIECE_VALUE.get(piece_type, 0)


from .evaluate import evaluate as evaluate_board


class TimeOut(Exception):
    pass

class StrongEngine:
    def __init__(self):
        self.tt: Dict[int, Tuple[int, int, int, Optional[str]]] = {}
        self.start_time = 0.0
        self.time_limit = 0.0
        self.nodes = 0
        self.killers: Dict[int, List[str]] = {}
        self.history: Dict[Tuple[int,int], int] = {}
        self.PV: List[str] = []

    def _time_check(self):
        if time.time() - self.start_time > self.time_limit:
            raise TimeOut()

    def _key(self, board: chess.Board) -> int:
        return hash(board._transposition_key())

    def get_best_move(self, fen: str, time_ms: int = 200) -> Optional[str]:
        board = chess.Board(fen)
        return self.search(board, time_ms)

    def search(self, board: chess.Board, time_ms: int = 200, max_depth=100) -> Optional[str]:
        self.start_time = time.time()
        self.time_limit = time_ms / 1000.0
        self.tt.clear()
        self.killers.clear()
        self.history.clear()
        self.nodes = 0
        self.PV = []
        self.best_score = 0
        best_move = None
        best_score = 0

        legal = list(board.legal_moves)
        if not legal:
            return None

        try:
            for depth in range(1, max_depth+1):
                window = 50
                alpha = -math.inf if depth == 1 else best_score - window
                beta = math.inf if depth == 1 else best_score + window

                try:
                    score, pv = self._negamax_root(board, depth, alpha, beta)
                except AssertionError:
                    break

                if score <= alpha or score >= beta:
                    score, pv = self._negamax_root(board, depth, -math.inf, math.inf)

                best_score = score
                self.best_score = best_score
                best_move = pv[0] if pv else legal[0].uci()
                self.PV = pv[:]

        except TimeOut:
            pass

        return best_move

    def _negamax_root(self, board: chess.Board, depth: int, alpha: float, beta: float) -> Tuple[int, List[str]]:
        self._time_check()
        self.nodes += 1

        key = self._key(board)
        tt = self.tt.get(key)
        moves = list(board.legal_moves)
        if not moves:
            return (0, [])

        move_scores = []
        tt_move = tt[3] if tt else None
        for mv in moves:
            score = 0
            if tt_move and mv.uci() == tt_move:
                score += 100000
            if board.is_capture(mv):
                captured = board.piece_type_at(mv.to_square)
                if captured:
                    score += 10000 + evaluate_capture_value(captured)
            if mv.promotion:
                score += 8000
            klist = self.killers.get(depth, [])
            if mv.uci() in klist:
                score += 5000
            key_hist = (mv.from_square, mv.to_square)
            score += self.history.get(key_hist, 0)
            move_scores.append((mv, score))

        move_scores.sort(key=lambda x: x[1], reverse=True)

        best_val = -1_000_000
        best_move = None
        best_pv = []

        for mv, _s in move_scores:
            self._time_check()
            board.push(mv)
            val, pv = self._negamax(board, depth-1, -beta, -alpha)
            val = -val
            board.pop()

            if val > best_val:
                best_val = val
                best_move = mv.uci()
                best_pv = [mv.uci()] + pv

            alpha = max(alpha, val)
            if alpha >= beta:
                self._record_killer(depth, mv.uci())
                self._update_history(mv, depth)
                break

        flag = 0
        if best_val <= alpha:
            flag = 1 
        elif best_val >= beta:
            flag = -1
        else:
            flag = 0
        self.tt[key] = (int(best_val), depth, flag, best_move)
        return int(best_val), best_pv

    def _negamax(self, board: chess.Board, depth: int, alpha: float, beta: float, null_ok=True) -> Tuple[int, List[str]]:
        self._time_check()
        self.nodes += 1

        key = self._key(board)
        tt = self.tt.get(key)
        if tt and tt[1] >= depth:
            val, d, flag, best_move = tt
            if flag == 0:
                return val, [best_move] if best_move else []
            if flag == -1 and val >= beta:
                return val, [best_move] if best_move else []
            if flag == 1 and val <= alpha:
                return val, [best_move] if best_move else []

        if depth <= 0 or board.is_game_over():
            q = self._quiescence(board, alpha, beta)
            return q, []

        if null_ok and depth >= 3 and not board.is_check():
            self._time_check()
            board.push(chess.Move.null())
            val, _ = self._negamax(board, depth-1-2, -beta, -beta+1, False)
            val = -val
            board.pop()
            if val >= beta:
                return val, [] 

        moves = list(board.legal_moves)
        mv_list = []
        tt_move = tt[3] if tt else None
        for mv in moves:
            score = 0
            if tt_move and mv.uci() == tt_move:
                score += 100000
            if board.is_capture(mv):
                cp = board.piece_type_at(mv.to_square)
                if cp:
                    score += 10000 + evaluate_capture_value(cp)
            if mv.promotion:
                score += 8000
            key_hist = (mv.from_square, mv.to_square)
            score += self.history.get(key_hist, 0)
            mv_list.append((mv, score))

        mv_list.sort(key=lambda x: x[1], reverse=True)

        best_val = -1_000_000
        best_move = None
        best_pv = []

        for mv, _sc in mv_list:
            self._time_check()
            board.push(mv)
            val, pv = self._negamax(board, depth-1, -beta, -alpha)
            val = -val
            board.pop()

            if val > best_val:
                best_val = val
                best_move = mv.uci()
                best_pv = [mv.uci()] + pv

            if val > alpha:
                alpha = val
            if alpha >= beta:
                self._record_killer(depth, mv.uci())
                self._update_history(mv, depth)
                break

        flag = 0
        if best_val <= alpha:
            flag = 1
        elif best_val >= beta:
            flag = -1
        else:
            flag = 0
        self.tt[key] = (int(best_val), depth, flag, best_move)
        return int(best_val), best_pv

    def _quiescence(self, board: chess.Board, alpha: float, beta: float) -> int:
        self._time_check()
        self.nodes += 1

        stand_pat = self._evaluate_leaf(board)
        if stand_pat >= beta:
            return beta
        if stand_pat > alpha:
            alpha = stand_pat

        moves = []
        for mv in board.legal_moves:
            if board.is_capture(mv) or mv.promotion:
                moves.append(mv)

        def mvv_lva(mv):
            captured = board.piece_type_at(mv.to_square)
            attacker = board.piece_type_at(mv.from_square)
            return (evaluate_capture_value(captured) if captured else 0) * 10 - (evaluate_capture_value(attacker) if attacker else 0)

        moves.sort(key=mvv_lva, reverse=True)

        best = stand_pat
        for mv in moves:
            self._time_check()
            captured = board.piece_type_at(mv.to_square)
            if captured and stand_pat + evaluate_capture_value(captured) + 50 < alpha:
                continue

            board.push(mv)
            val = -self._quiescence(board, -beta, -alpha)
            board.pop()

            if val >= beta:
                return beta
            if val > best:
                best = val
            if best > alpha:
                alpha = best
        return best

    def _evaluate_leaf(self, board: chess.Board) -> int:
        if board.is_checkmate():
            return -1000000
        if board.is_stalemate() or board.is_insufficient_material() or board.can_claim_draw():
            return 0
        val = evaluate_board(board)
        return val if board.turn == chess.WHITE else -val

    def _record_killer(self, depth: int, move_uci: str):
        lst = self.killers.setdefault(depth, [])
        if move_uci in lst:
            return
        lst.insert(0, move_uci)
        if len(lst) > 2:
            lst.pop()

    def _update_history(self, mv: chess.Move, depth: int):
        key = (mv.from_square, mv.to_square)
        self.history[key] = self.history.get(key, 0) + (1 << depth)

    def get_best_move_with_eval(self, fen: str, time_ms: int = 200) -> dict:
        board = chess.Board(fen)
        move = self.search(board, time_ms)
        if self.best_score <= -100000:
            return {"bestmove": move, "cp": None, "mate": -1}
        elif self.best_score >= 100000:
            return {"bestmove": move, "cp": None, "mate": 1}
        else:
            return {"bestmove": move, "cp": self.best_score, "mate": None}


_engine_singleton = StrongEngine()

""" def get_best_move(fen: str, time_ms: int = 200) -> Optional[str]:
    return _engine_singleton.get_best_move(fen, time_ms) """

def get_best_move(fen: str, time_ms: int = 200) -> dict:
    return _engine_singleton.get_best_move_with_eval(fen, time_ms)