
import sys
print("Running with Python:", sys.version)

import time
import chess
import chess.pgn
from engine_strong import StrongEngine, get_best_move
from engine_strong2 import StrongEngine2, get_best_move
from engine_strong_cpp import get_best_move_sp as get_best
class StrongEngineCPP:
    def get_best_move(self, fen, time_ms):
        return get_best(fen, time_ms)


""" from engine import get_best_move as get_move_old
class StrongEngineOld:
    def get_best_move(self, fen, time_ms):
        return get_move_old(fen, time_ms) """

def play_game(engine_white: StrongEngine, engine_black: StrongEngine, time_ms_white=200, time_ms_black=200, max_moves=300):
    board = chess.Board()
    game = chess.pgn.Game()
    node = game

    nodes_total = 0
    times = {"white": 0.0, "black": 0.0}

    for ply in range(max_moves):
        if board.is_game_over():
            break
        color = 'white' if board.turn == chess.WHITE else 'black'
        engine = engine_white if board.turn == chess.WHITE else engine_black
        time_ms = time_ms_white if board.turn == chess.WHITE else time_ms_black

        start = time.time()
        move_uci = engine.get_best_move(board.fen(), time_ms)
        duration = time.time() - start
        times[color] += duration

        if not move_uci:
            break
        move = chess.Move.from_uci(move_uci)
        if move not in board.legal_moves:
            print("ILLEGAL MOVE FROM ENGINE:", move_uci)
            # fallback random legal move
            move = list(board.legal_moves)[0]
            move_uci = move.uci()

        board.push(move)
        node = node.add_variation(move)

    game.headers["Result"] = board.result(claim_draw=True)
    game.headers["WhiteTime"] = str(times["white"])
    game.headers["BlackTime"] = str(times["black"])
    return game, board.result(), times

def tournament(n_games=10, time_ms=200):
    e1 = StrongEngine()
    e2 = StrongEngine2()
    stats = {"1-0": 0, "0-1": 0, "1/2-1/2": 0}
    games = []

    print("\nTournament Results:")
    print("=" * 40)
    print(f"{'Game':<6} {'Result':<8} {'White Engine':<20} {'Black Engine':<20} {'White Time':<10} {'Black Time':<10}")
    print("-" * 40)

    for i in range(n_games):
        if i % 2 == 0:
            game, result, times = play_game(e1, e2, time_ms, time_ms)
            white_engine = "e1"
            black_engine = "e2"
        else:
            game, result, times = play_game(e2, e1, time_ms, time_ms)
            white_engine = "e2"
            black_engine = "e1"
            if result == "1-0":
                result = "0-1"
            elif result == "0-1":
                result = "1-0"

        stats[result] += 1
        games.append((game, result, times))

        print(f"{i+1:<6} {result:<8} {white_engine:<20} {black_engine:<20} {times['white']:<10.2f} {times['black']:<10.2f}")


    print("=" * 40)
    print("\nFinal Stats:")
    print(f"{'Engine':<20} {'Wins':<5} {'Losses':<5} {'Draws':<5}")
    print("-" * 40)
    print(f"{'e1':<20} {stats['1-0']:<5} {stats['0-1']:<5} {stats['1/2-1/2']:<5}")
    print(f"{'e2':<20} {stats['0-1']:<5} {stats['1-0']:<5} {stats['1/2-1/2']:<5}")

    with open("selfplay_results.pgn", "w") as f:
        for g, r, t in games:
            f.write(str(g))
            f.write("\n\n")

    return stats


if __name__ == "__main__":
    tournament(6, time_ms=200)
