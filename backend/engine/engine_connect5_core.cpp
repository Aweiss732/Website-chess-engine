// engine_connect5_core.cpp
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <string>
#include <iostream>
#include <cstdint>
#include <chrono>

namespace py = pybind11;
using Clock = std::chrono::steady_clock;

constexpr int SIZE = 15;
constexpr int BOARD_CELLS = SIZE * SIZE;
constexpr int MAX_DEPTH = 10;
constexpr int INF = 10000000;

inline int idx(int r, int c) { return r * SIZE + c; }


const int DR[4] = {0, 1, 1, 1};
const int DC[4] = {1, 0, 1, -1};

struct Position {
    uint8_t board[BOARD_CELLS];
    int side_to_move; // 1 = black (AI), 2 = white (human)
    int last_move = -1;  // Track last move for faster win checking

    Position() {
        std::fill(board, board + BOARD_CELLS, 0);
        side_to_move = 1;
    }

    Position(const std::string &s) {
        std::fill(board, board + BOARD_CELLS, 0);
        
        // Count stones to determine whose turn it is
        int black_count = 0, white_count = 0;
        
        for (int i = 0; i < (int)std::min((int)s.size(), BOARD_CELLS); ++i) {
            char ch = s[i];
            if (ch == 'B' || ch == 'b') {
                board[i] = 1;
                black_count++;
                last_move = i;
            } else if (ch == 'W' || ch == 'w') {
                board[i] = 2;
                white_count++;
                last_move = i;
            } else {
                board[i] = 0;
            }
        }
        
        if (black_count == white_count) {
            side_to_move = 1;
        } else if (black_count == white_count + 1) {
            side_to_move = 2;  
        } else {
            side_to_move = 1;
        }
    }

    void make_move(int move_idx, int color) {
        board[move_idx] = (uint8_t)color;
        side_to_move = 3 - color;
        last_move = move_idx;
    }

    void undo_move(int move_idx) {
        int color = board[move_idx];
        board[move_idx] = 0;
        side_to_move = color;
    }

    bool is_empty(int r, int c) const {
        if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) return false;
        return board[idx(r,c)] == 0;
    }
    
    bool has_stone(int r, int c, int color) const {
        if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) return false;
        return board[idx(r,c)] == color;
    }
};


bool has_five_fast(const Position &pos, int color) {
    if (pos.last_move != -1) {
        int r = pos.last_move / SIZE, c = pos.last_move % SIZE;
        if (pos.board[pos.last_move] == color) {
            for (int dir = 0; dir < 4; ++dir) {
                int count = 1;
                int dr = DR[dir], dc = DC[dir];

                for (int k = 1; k <= 4; k++) {
                    int rr = r + k * dr, cc = c + k * dc;
                    if (pos.has_stone(rr, cc, color)) count++;
                    else break;
                }
 
                for (int k = 1; k <= 4; k++) {
                    int rr = r - k * dr, cc = c - k * dc;
                    if (pos.has_stone(rr, cc, color)) count++;
                    else break;
                }
                
                if (count >= 5) return true;
            }
        }
    }

    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (!pos.has_stone(r, c, color)) continue;
            
            for (int dir = 0; dir < 4; ++dir) {
                int count = 1;
                int dr = DR[dir], dc = DC[dir];
                
                for (int k = 1; k <= 4; k++) {
                    int rr = r + k * dr, cc = c + k * dc;
                    if (pos.has_stone(rr, cc, color)) count++;
                    else break;
                }

                for (int k = 1; k <= 4; k++) {
                    int rr = r - k * dr, cc = c - k * dc;
                    if (pos.has_stone(rr, cc, color)) count++;
                    else break;
                }
                
                if (count >= 5) return true;
            }
        }
    }
    return false;
}

bool would_create_five(const Position &pos, int move_idx, int color) {
    int r = move_idx / SIZE, c = move_idx % SIZE;

    for (int dir = 0; dir < 4; ++dir) {
        int dr = DR[dir], dc = DC[dir];

        int count = 1;

        for (int k = 1; k <= 4; k++) {
            int rr = r + k * dr, cc = c + k * dc;
            if (rr < 0 || rr >= SIZE || cc < 0 || cc >= SIZE) break;
            if (pos.board[idx(rr,cc)] == color) count++;
            else break;
        }
 
        for (int k = 1; k <= 4; k++) {
            int rr = r - k * dr, cc = c - k * dc;
            if (rr < 0 || rr >= SIZE || cc < 0 || cc >= SIZE) break;
            if (pos.board[idx(rr,cc)] == color) count++;
            else break;
        }
        
        if (count >= 5) return true;
    }
    return false;
}

bool creates_double_threat(const Position &pos, int move_idx, int color) {
    int r = move_idx / SIZE, c = move_idx % SIZE;
    int threat_count = 0;

    for (int dir = 0; dir < 4 && threat_count < 2; ++dir) {
        int dr = DR[dir], dc = DC[dir];

        for (int offset = -4; offset <= 0 && threat_count < 2; ++offset) {
            int start_r = r + offset * dr;
            int start_c = c + offset * dc;

            bool valid = true;
            int line[6];
            for (int i = 0; i < 6; ++i) {
                int rr = start_r + i * dr;
                int cc = start_c + i * dc;
                if (rr < 0 || rr >= SIZE || cc < 0 || cc >= SIZE) {
                    valid = false;
                    break;
                }
                line[i] = pos.board[idx(rr, cc)];
            }
            if (!valid) continue;

            int simulated_line[6];
            std::copy(line, line + 6, simulated_line);
            int move_pos_in_line = -offset;
            if (move_pos_in_line >= 0 && move_pos_in_line < 6) {
                simulated_line[move_pos_in_line] = color;
            }

            for (int i = 0; i <= 1 && threat_count < 2; ++i) {
                int our_stones = 0;
                int empty_cells = 0;
                bool pattern_valid = true;
                
                for (int j = 0; j < 5; ++j) {
                    int cell = simulated_line[i + j];
                    if (cell == color) our_stones++;
                    else if (cell == 0) empty_cells++;
                    else {
                        pattern_valid = false;
                        break;
                    }
                }
                
                if (pattern_valid && our_stones == 4 && empty_cells == 1) {
                    bool left_open = (i > 0 && simulated_line[i-1] == 0);
                    bool right_open = (i + 5 < 6 && simulated_line[i+5] == 0);
                    
                    if (left_open && right_open) {
                        threat_count++;
                    }
                }
            }
        }
    }
    
    return threat_count >= 2;
}

int find_winning_move(const Position &pos, int color) {
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (pos.board[idx(r,c)] != 0) { 
                for (int dr = -2; dr <= 2; dr++) {
                    for (int dc = -2; dc <= 2; dc++) {
                        int rr = r + dr, cc = c + dc;
                        if (rr >= 0 && rr < SIZE && cc >= 0 && cc < SIZE && 
                            pos.is_empty(rr, cc)) {
                            int move_idx = idx(rr, cc);
                            if (would_create_five(pos, move_idx, color)) {
                                return move_idx;
                            }
                        }
                    }
                }
            }
        }
    }
    return -1;
}

int find_double_threat_move(const Position &pos, int our_color) {
    int opponent_color = 3 - our_color;
    
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (pos.is_empty(r, c)) {
                int move_idx = idx(r, c);
                if (creates_double_threat(pos, move_idx, opponent_color)) {
                    return move_idx;
                }
            }
        }
    }
    return -1;
}

int evaluate_position(const Position &pos) {
    if (has_five_fast(pos, 1)) return 1000000;
    if (has_five_fast(pos, 2)) return -1000000;
    
    int score = 0;

    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            int cell_idx = idx(r, c);
            if (pos.board[cell_idx] == 1) {
                score += 10;
                int center_dist = std::abs(r - SIZE/2) + std::abs(c - SIZE/2);
                score += (SIZE - center_dist);

                if (would_create_five(pos, cell_idx, 1)) {
                    score += 1000; 
                }
            } else if (pos.board[cell_idx] == 2) {
                score -= 10;

                int center_dist = std::abs(r - SIZE/2) + std::abs(c - SIZE/2);
                score -= (SIZE - center_dist);

                if (would_create_five(pos, cell_idx, 2)) {
                    score -= 1000;
                }
            }
        }
    }
    
    return score;
}

std::vector<int> generate_moves(const Position &pos) {
    std::vector<int> moves;
    std::vector<bool> candidate(BOARD_CELLS, false);
    std::vector<int> scored_moves;
    
    bool has_stones = false;
    for (int i = 0; i < BOARD_CELLS; i++) {
        if (pos.board[i] != 0) {
            has_stones = true;
            break;
        }
    }
    
    if (!has_stones) {
        moves.push_back(idx(SIZE/2, SIZE/2));
        return moves;
    }

    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (pos.board[idx(r,c)] != 0) {
                for (int dr = -2; dr <= 2; dr++) {
                    for (int dc = -2; dc <= 2; dc++) {
                        int rr = r + dr, cc = c + dc;
                        if (rr >= 0 && rr < SIZE && cc >= 0 && cc < SIZE && 
                            pos.board[idx(rr,cc)] == 0) {
                            candidate[idx(rr,cc)] = true;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < BOARD_CELLS; i++) {
        if (candidate[i]) {
            int r = i / SIZE, c = i % SIZE;
            int score = 0;

            int center_dist = std::abs(r - SIZE/2) + std::abs(c - SIZE/2);
            score += (SIZE - center_dist) * 10;

            if (would_create_five(pos, i, pos.side_to_move)) {
                score += 10000;
            }
            if (would_create_five(pos, i, 3 - pos.side_to_move)) {
                score += 5000; 
            }
            
            scored_moves.push_back(i);
            scored_moves.push_back(score);
        }
    }

    for (size_t i = 0; i < scored_moves.size(); i += 2) {
        for (size_t j = i + 2; j < scored_moves.size(); j += 2) {
            if (scored_moves[j + 1] > scored_moves[i + 1]) {
                std::swap(scored_moves[i], scored_moves[j]);
                std::swap(scored_moves[i + 1], scored_moves[j + 1]);
            }
        }
    }

    for (size_t i = 0; i < scored_moves.size(); i += 2) {
        moves.push_back(scored_moves[i]);
    }
    
    return moves;
}

int minimax(Position &pos, int depth, int alpha, int beta, bool maximizing_player) {
    if (has_five_fast(pos, 1)) return 1000000 - depth;
    if (has_five_fast(pos, 2)) return -1000000 + depth;
    
    if (depth == 0) {
        return evaluate_position(pos);
    }
    
    std::vector<int> moves = generate_moves(pos);
    if (moves.empty()) return 0;
    
    if (maximizing_player) {
        int max_eval = -INF;
        for (int move : moves) {
            pos.make_move(move, 1);
            int eval = minimax(pos, depth - 1, alpha, beta, false);
            pos.undo_move(move);
            
            max_eval = std::max(max_eval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return max_eval;
    } else {
        int min_eval = INF;
        for (int move : moves) {
            pos.make_move(move, 2);
            int eval = minimax(pos, depth - 1, alpha, beta, true);
            pos.undo_move(move);
            
            min_eval = std::min(min_eval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return min_eval;
    }
}

std::pair<int, int> find_best_move(Position &pos, int max_depth) {
    int winning_move = find_winning_move(pos, pos.side_to_move);
    if (winning_move != -1) {
        return {winning_move, (pos.side_to_move == 1) ? 1000000 : -1000000};
    }

    int blocking_move = find_winning_move(pos, 3 - pos.side_to_move);
    if (blocking_move != -1) {
        return {blocking_move, (pos.side_to_move == 1) ? 900000 : -900000};
    }

    int double_threat_move = find_double_threat_move(pos, pos.side_to_move);
    if (double_threat_move != -1) {
        return {double_threat_move, (pos.side_to_move == 1) ? 800000 : -800000};
    }
    
    std::vector<int> moves = generate_moves(pos);
    if (moves.empty()) return {-1, 0};
    
    int best_move = moves[0];
    int best_score = (pos.side_to_move == 1) ? -INF : INF;
    
    for (int move : moves) {
        pos.make_move(move, pos.side_to_move);
        
        int score;
        if (pos.side_to_move == 1) {
            score = minimax(pos, max_depth - 1, -INF, INF, true);
        } else { 
            score = minimax(pos, max_depth - 1, -INF, INF, false);
        }
        
        pos.undo_move(move);
        
        if (pos.side_to_move == 1) {
            if (score > best_score) {
                best_score = score;
                best_move = move;
            }
        } else {
            if (score < best_score) {
                best_score = score;
                best_move = move;
            }
        }
    }
    
    return {best_move, best_score};
}

py::dict get_best_move_cpp(const std::string &board_str, int time_ms) {
    Clock::time_point start_time = Clock::now();
    
    Position pos(board_str);

    bool has_empty = false;
    for (int i = 0; i < BOARD_CELLS; ++i) {
        if (pos.board[i] == 0) {
            has_empty = true;
            break;
        }
    }
    
    py::dict out;
    if (!has_empty) {
        out["row"] = -1;
        out["col"] = -1;
        out["eval"] = 0;
        out["depth"] = 0;
        return out;
    }

    int best_move = -1;
    int best_eval = 0;
    int depth_reached = 0;
    
    for (int depth = 1; depth <= MAX_DEPTH; depth++) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_time);
        if (elapsed.count() > time_ms) {
            break;
        }
        
        auto [move, eval] = find_best_move(pos, depth);
        
        if (move != -1) {
            best_move = move;
            best_eval = eval;
            depth_reached = depth;
        }

        if (abs(best_eval) > 900000) break;

        if (elapsed.count() > time_ms * 0.8) break;
    }
    
    if (best_move != -1) {
        out["row"] = best_move / SIZE;
        out["col"] = best_move % SIZE;
        out["eval"] = best_eval;
        out["depth"] = depth_reached;
    } else {
        out["row"] = -1;
        out["col"] = -1;
        out["eval"] = 0;
        out["depth"] = 0;
    }
    
    return out;
}

PYBIND11_MODULE(engine_connect5_core, m) {
    m.def("get_best_move_cpp", &get_best_move_cpp, "Get best move for Connect-Five");
}