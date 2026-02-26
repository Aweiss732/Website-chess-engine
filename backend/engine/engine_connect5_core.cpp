// engine_connect5_core.cpp
#include "engine_connect5.h"
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

struct SearchResult {
    int move;
    int score;
    int depth;
};

bool time_limit_exceeded(Clock::time_point start_time, int time_ms) {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_time);
    return elapsed.count() >= time_ms;
}

int minimax(Position &pos, int depth, int alpha, int beta, bool maximizing_player, 
            Clock::time_point start_time, int time_ms, int &nodes_searched) {
    nodes_searched++;

    if ((nodes_searched & 511) == 0 && time_limit_exceeded(start_time, time_ms)) {
        return maximizing_player ? -INF : INF;
    }


    if (has_five_fast(pos, 1)) return 1000000 - depth;
    if (has_five_fast(pos, 2)) return -1000000 + depth;

    if (depth == 0) {
        return evaluate_position(pos);
    }

    std::vector<int> moves = generate_moves(pos);
    if (moves.empty()) return 0;

    if (maximizing_player) {
        int value = -INF;
        for (size_t i = 0; i < moves.size(); ++i) {
            if (time_limit_exceeded(start_time, time_ms)) break;

            int mv = moves[i];
            pos.make_move(mv, 1);

            if (has_five_fast(pos, 1)) {
                pos.undo_move(mv);
                return 1000000 - (MAX_DEPTH - depth);
            }
            int score = minimax(pos, depth - 1, alpha, beta, false, start_time, time_ms, nodes_searched);
            pos.undo_move(mv);

            if (score == -INF || score == INF) return score; 

            if (score > value) value = score;
            if (value > alpha) alpha = value;
            if (alpha >= beta) {

                break;
            }
        }
        return value;
    } else {
        int value = INF;
        for (size_t i = 0; i < moves.size(); ++i) {
            if (time_limit_exceeded(start_time, time_ms)) break;

            int mv = moves[i];
            pos.make_move(mv, 2);
            if (has_five_fast(pos, 2)) {
                pos.undo_move(mv);
                return -1000000 + (MAX_DEPTH - depth);
            }
            int score = minimax(pos, depth - 1, alpha, beta, true, start_time, time_ms, nodes_searched);
            pos.undo_move(mv);

            if (score == -INF || score == INF) return score;

            if (score < value) value = score;
            if (value < beta) beta = value;
            if (alpha >= beta) {
                break;
            }
        }
        return value;
    }
}


SearchResult find_best_move(Position &pos, int max_depth, Clock::time_point start_time, int time_ms) {

    if (!time_limit_exceeded(start_time, time_ms)) {
        int winning_move = find_winning_move(pos, pos.side_to_move);
        if (winning_move != -1) {
            return {winning_move, (pos.side_to_move == 1) ? 1000000 : -1000000, max_depth};
        }
    }

    if (!time_limit_exceeded(start_time, time_ms)) {
        int blocking_move = find_winning_move(pos, 3 - pos.side_to_move);
        if (blocking_move != -1) {
            return {blocking_move, (pos.side_to_move == 1) ? 900000 : -900000, max_depth};
        }
    }

    if (!time_limit_exceeded(start_time, time_ms)) {
        int double_threat_move = find_double_threat_move(pos, pos.side_to_move);
        if (double_threat_move != -1) {
            return {double_threat_move, (pos.side_to_move == 1) ? 800000 : -800000, max_depth};
        }
    }
    
    std::vector<int> moves = generate_moves(pos);
    if (moves.empty()) return {-1, 0, 0};
    
    int best_move = moves[0];
    int best_score = (pos.side_to_move == 1) ? -INF : INF;
    int nodes_searched = 0;
    
    for (int move : moves) {
        if (time_limit_exceeded(start_time, time_ms)) break;
        
        pos.make_move(move, pos.side_to_move);
        
        int score;
        if (pos.side_to_move == 1) {
            score = minimax(pos, max_depth - 1, -INF, INF, true, start_time, time_ms, nodes_searched);
        } else { 
            score = minimax(pos, max_depth - 1, -INF, INF, false, start_time, time_ms, nodes_searched);
        }
        
        pos.undo_move(move);
        
        if (score == -INF || score == INF) {

            break;
        }
        
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
    
    return {best_move, best_score, max_depth};
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
        if (time_limit_exceeded(start_time, time_ms)) {
            break;
        }
        
        auto result = find_best_move(pos, depth, start_time, time_ms);
        
        if (result.move != -1) {
            best_move = result.move;
            best_eval = result.score;
            depth_reached = result.depth;
        }

        if (abs(best_eval) > 900000) break;

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_time);
        if (elapsed.count() > time_ms * 0.7) break;
    }
    
    if (best_move != -1) {
        out["row"] = best_move / SIZE;
        out["col"] = best_move % SIZE;
        out["eval"] = best_eval;
        out["depth"] = depth_reached;
    } else {
        std::vector<int> moves = generate_moves(pos);
        if (!moves.empty()) {
            best_move = moves[0];
            out["row"] = best_move / SIZE;
            out["col"] = best_move % SIZE;
            out["eval"] = 0;
            out["depth"] = 0;
        } else {
            out["row"] = -1;
            out["col"] = -1;
            out["eval"] = 0;
            out["depth"] = 0;
        }
    }
    
    return out;
}

PYBIND11_MODULE(engine_connect5_core, m) {
    m.def("get_best_move_cpp", &get_best_move_cpp, "Get best move for Connect-Five (v2)");
}