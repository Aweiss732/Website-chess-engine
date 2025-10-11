#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "board_adapter.h"
#include <chrono>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <iostream>

namespace py = pybind11;
using namespace board_adapter;

static bool virgo_initialized = false;

void initialize_virgo() {
    if (!virgo_initialized) {
        virgo::virgo_init();
        virgo_initialized = true;
        //std::cout << "Virgo initialized" << std::endl;
    }
}

// transposition table
struct TTEntry {
    int depth;
    int eval;
    uint16_t best_move;
    int node_type; // 0=exact, 1=upper bound, 2=lower bound
};

static std::unordered_map<uint64_t, TTEntry> transposition_table;

uint64_t simple_hash(Position &pos) {
    return pos.hash_position();
}

bool is_promotion(uint16_t move) {
    int move_type = MOVE_TYPE(move);
    return (move_type >= virgo::PQ_R && move_type <= virgo::PC_N);
}

int quiescence(Position &pos, int alpha, int beta) {
    int stand_pat = pos.evaluate();
    
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;
    
    auto moves = pos.get_legal_moves();
    // Only consider captures in quiescence
    std::vector<uint16_t> capture_moves;
    for (auto move : moves) {
        int to_square = MOVE_TO(move);
        auto piece = pos.board[to_square];
        if (piece.first != virgo::EMPTY) {
            capture_moves.push_back(move);
        }
    }
    
    std::sort(capture_moves.begin(), capture_moves.end(), [&](uint16_t a, uint16_t b) {
        int toA = MOVE_TO(a);
        int toB = MOVE_TO(b);
        auto victimA = pos.board[toA];
        auto victimB = pos.board[toB];
        int fromA = MOVE_FROM(a);
        int fromB = MOVE_FROM(b);
        auto attackerA = pos.board[fromA];
        auto attackerB = pos.board[fromB];
        
        const int piece_values[] = {0, 100, 500, 320, 330, 10000, 900};
        int scoreA = piece_values[victimA.first] - piece_values[attackerA.first]/10;
        int scoreB = piece_values[victimB.first] - piece_values[attackerB.first]/10;
        return scoreA > scoreB;
    });
    
    for (auto move : capture_moves) {
        pos.make_move(move);
        int score = -quiescence(pos, -beta, -alpha);
        pos.undo_move();
        
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    
    return alpha;
}

std::pair<int, uint16_t> negamax(Position &pos, int depth, int alpha, int beta, bool nullWindow = false) {
    if (pos.is_repetition_draw(2)) {
        return {0, 0};
    }

    if (depth == 0) {
        return {quiescence(pos, alpha, beta), 0};
    }
    
    auto moves = pos.get_legal_moves();
    if (moves.empty()) {
        // Checkmate or stalemate detection
        bool in_check = false;
        return {-100000 + depth, 0};
    }
    
    uint64_t key = simple_hash(pos);
    auto tt_it = transposition_table.find(key);
    if (tt_it != transposition_table.end() && tt_it->second.depth >= depth) {
        if (tt_it->second.node_type == 0) {
            return {tt_it->second.eval, tt_it->second.best_move};
        } else if (tt_it->second.node_type == 1) { 
            if (tt_it->second.eval <= alpha) return {tt_it->second.eval, tt_it->second.best_move};
            beta = std::min(beta, tt_it->second.eval);
        } else if (tt_it->second.node_type == 2) {
            if (tt_it->second.eval >= beta) return {tt_it->second.eval, tt_it->second.best_move};
            alpha = std::max(alpha, tt_it->second.eval);
        }
    }

    std::vector<std::pair<int, uint16_t>> move_scores;
    uint16_t tt_move = 0;
    if (tt_it != transposition_table.end()) {
        tt_move = tt_it->second.best_move;
    }
    
    for (auto move : moves) {
        int score = 0;

        if (move == tt_move) {
            score = 10000;
        }
        
        int to_square = MOVE_TO(move);
        int from_square = MOVE_FROM(move);
        auto captured_piece = pos.board[to_square];
        
        // Prioritize captures
        if (captured_piece.first != virgo::EMPTY) {
            const int piece_values[] = {0, 100, 500, 320, 330, 10000, 900};
            score += piece_values[captured_piece.first] * 10;
            auto attacker = pos.board[from_square];
            if (piece_values[attacker.first] < piece_values[captured_piece.first]) {
                score += 50;
            }
        }

        if (is_promotion(move)) {
            score += 800;
        }
        
        // Penalty for moves that cause repetition in winning positions
        int current_eval = pos.evaluate();
        if (abs(current_eval) > 100) { 
            if (pos.would_cause_repetition(move, 2)) {
                score -= 200;
            }
        }
        
        move_scores.push_back({score, move});
    }

    std::sort(move_scores.begin(), move_scores.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    int best_eval = -1000000;
    uint16_t best_move = move_scores.empty() ? 0 : move_scores[0].second;
    int original_alpha = alpha;
    
    for (const auto& [score, move] : move_scores) {
        pos.make_move(move);
        
        int eval;
        if (best_eval == -1000000) {
            auto [move_eval, _] = negamax(pos, depth - 1, -beta, -alpha);
            eval = -move_eval;
        } else {
            auto [move_eval, _] = negamax(pos, depth - 1, -alpha - 1, -alpha, true);
            eval = -move_eval;

            if (eval > alpha && eval < beta) {
                auto [re_eval, __] = negamax(pos, depth - 1, -beta, -alpha);
                eval = -re_eval;
            }
        }
        pos.undo_move();
        
        if (eval > best_eval) {
            best_eval = eval;
            best_move = move;
            alpha = std::max(alpha, eval);
        }
        
        if (alpha >= beta) {
            break;
        }

        alpha = std::max(alpha, eval);
    }

    TTEntry entry;
    entry.depth = depth;
    entry.eval = best_eval;
    entry.best_move = best_move;
    
    if (best_eval <= original_alpha) {
        entry.node_type = 1;
    } else if (best_eval >= beta) {
        entry.node_type = 2;
    } else {
        entry.node_type = 0;
    }
    transposition_table[key] = entry;
    
    return {best_eval, best_move};
}

py::dict get_best_move_cpp(const std::string &fen, int time_ms) {
    initialize_virgo();
    
    Position pos(fen);
    auto moves = pos.get_legal_moves();
    
    //std::cout << "Legal moves count: " << moves.size() << std::endl;
    
    if (moves.empty()) {
        py::dict result;
        result["bestmove"] = "";
        result["cp"] = 0;
        result["mate"] = 0;
        return result;
    }

    auto start = std::chrono::steady_clock::now();
    std::string best_move_uci;
    int best_eval = 0;
    uint16_t best_move = moves[0];

    transposition_table.clear();

    for (int depth = 1; depth <= 8; ++depth) {
        auto [current_eval, current_best_move] = negamax(pos, depth, -1000000, 1000000);

        if (current_best_move != 0) {
            best_eval = current_eval;
            best_move = current_best_move;
            best_move_uci = pos.move_to_uci(best_move);
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
            
        /* std::cout << "Depth " << depth << " completed in " << elapsed.count() 
                  << "ms. Best move: " << best_move_uci << " eval: " << best_eval << std::endl; */

        if (elapsed.count() > time_ms * 0.7) {
            //std::cout << "Breaking due to time constraints" << std::endl;
            break;
        }

        if (abs(best_eval) > 9000) {
            //std::cout << "Breaking due to mate found" << std::endl;
            break;
        }
    }

    py::dict result;
    result["bestmove"] = best_move_uci;
    result["cp"] = best_eval;
    result["mate"] = 0;
    return result;
}

PYBIND11_MODULE(engine_core, m) {
    m.def("get_best_move_cpp", &get_best_move_cpp, "Get best move using Virgo board logic");
}