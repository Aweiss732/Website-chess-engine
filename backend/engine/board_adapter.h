// board_adapter.h
#pragma once
#include "virgo/virgo.h"
#include <string>
#include <vector>
#include <cmath>
#include <unordered_set>

namespace board_adapter {

struct Position {
    virgo::Chessboard board;
    bool whiteToMove;
    std::vector<uint64_t> positionHistory; // track for repetitions

    Position(const std::string &fen) {
        board = virgo::position_from_fen(fen.c_str());
        whiteToMove = board.get_next_to_move() == virgo::WHITE;
        positionHistory.push_back(hash_position());
    }

    virgo::Player get_next_to_move() {
        return board.get_next_to_move();
    }

    std::vector<uint16_t> get_legal_moves() {
        std::vector<uint16_t> moves;
        if (board.get_next_to_move() == virgo::WHITE) {
            virgo::get_legal_moves<virgo::WHITE>(board, moves);
        } else {
            virgo::get_legal_moves<virgo::BLACK>(board, moves);
        }
        return moves;
    }

    void make_move(uint16_t move) {
        if (board.get_next_to_move() == virgo::WHITE) {
            virgo::make_move<virgo::WHITE>(move, board);
        } else {
            virgo::make_move<virgo::BLACK>(move, board);
        }
        whiteToMove = !whiteToMove;
        positionHistory.push_back(hash_position());
    }

    void undo_move() {
        if (board.get_next_to_move() == virgo::BLACK) {
            virgo::take_move<virgo::WHITE>(board);
        } else {
            virgo::take_move<virgo::BLACK>(board);
        }
        whiteToMove = !whiteToMove;
        if (!positionHistory.empty()) {
            positionHistory.pop_back();
        }
    }

    uint64_t hash_position() {
        uint64_t hash = 0;

        for (int square = 0; square < 64; square++) {
            auto piece = board[square];
            if (piece.first != virgo::EMPTY) {
                uint64_t piece_hash = (static_cast<uint64_t>(piece.first) << 8) | 
                                    (static_cast<uint64_t>(piece.second) << 4) | 
                                    square;
                hash ^= piece_hash * 0x9e3779b97f4a7c15ull;
            }
        }

        hash ^= (board.get_next_to_move() == virgo::WHITE) ? 0x123456789abcdef0ull : 0x0fedcba987654321ull;
        
        return hash;
    }

    bool is_repetition_draw(int max_repetitions = 3) {
        if (positionHistory.empty()) return false;
        
        uint64_t current_hash = positionHistory.back();
        int count = 0;

        for (auto hash : positionHistory) {
            if (hash == current_hash) {
                count++;
                if (count >= max_repetitions) {
                    return true;
                }
            }
        }
        
        return false;
    }

    bool would_cause_repetition(uint16_t move, int max_repetitions = 2) {
        make_move(move);
        bool causes_repetition = is_repetition_draw(max_repetitions);
        undo_move();
        return causes_repetition;
    }

    bool is_game_over() {
        std::vector<uint16_t> moves = get_legal_moves();
        return moves.empty() || is_repetition_draw();
    }

    static int get_piece_value(virgo::Piece piece) {
        switch (piece) {
            case virgo::PAWN: return 100;
            case virgo::KNIGHT: return 320;
            case virgo::BISHOP: return 330;
            case virgo::ROOK: return 500;
            case virgo::QUEEN: return 900;
            case virgo::KING: return 10000;
            default: return 0;
        }
    }

    // Piece square tables:
    // Took these values from this site: https://adamberent.com/piece-square-table/
    static int get_pawn_table_value(int square) {
        static const int table[64] = {
             0,   0,   0,   0,   0,   0,   0,   0,
            50,  50,  50,  50,  50,  50,  50,  50,
            10,  10,  20,  30,  30,  20,  10,  10,
             5,   5,  10,  25,  25,  10,   5,   5,
             0,   0,   0,  20,  20,   0,   0,   0,
             5,  -5, -10,   0,   0, -10,  -5,   5,
             5,  10,  10, -20, -20,  10,  10,   5,
             0,   0,   0,   0,   0,   0,   0,   0
        };
        return table[square];
    }

    static int get_knight_table_value(int square) {
        static const int table[64] = {
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20,   0,   0,   0,   0, -20, -40,
            -30,   0,  10,  15,  15,  10,   0, -30,
            -30,   5,  15,  20,  20,  15,   5, -30,
            -30,   0,  15,  20,  20,  15,   0, -30,
            -30,   5,  10,  15,  15,  10,   5, -30,
            -40, -20,   0,   5,   5,   0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50
        };
        return table[square];
    }

    static int get_bishop_table_value(int square) {
        static const int table[64] = {
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -10,   0,   5,  10,  10,   5,   0, -10,
            -10,   5,   5,  10,  10,   5,   5, -10,
            -10,   0,  10,  10,  10,  10,   0, -10,
            -10,  10,  10,  10,  10,  10,  10, -10,
            -10,   5,   0,   0,   0,   0,   5, -10,
            -20, -10, -10, -10, -10, -10, -10, -20
        };
        return table[square];
    }

    static int get_rook_table_value(int square) {
        static const int table[64] = {
             0,   0,   0,   0,   0,   0,   0,   0,
             5,  10,  10,  10,  10,  10,  10,   5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
             0,   0,   0,   5,   5,   0,   0,   0
        };
        return table[square];
    }

    static int get_queen_table_value(int square) {
        static const int table[64] = {
            -20, -10, -10,  -5,  -5, -10, -10, -20,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -10,   0,   5,   5,   5,   5,   0, -10,
             -5,   0,   5,   5,   5,   5,   0,  -5,
              0,   0,   5,   5,   5,   5,   0,  -5,
            -10,   5,   5,   5,   5,   5,   0, -10,
            -10,   0,   5,   0,   0,   0,   0, -10,
            -20, -10, -10,  -5,  -5, -10, -10, -20
        };
        return table[square];
    }

    static int get_king_table_value(int square) {
        static const int table[64] = {
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -20, -30, -30, -40, -40, -30, -30, -20,
            -10, -20, -20, -20, -20, -20, -20, -10,
             20,  20,   0,   0,   0,   0,  20,  20,
             20,  30,  10,   0,   0,  10,  30,  20
        };
        return table[square];
    }

    static int get_king_endgame_table_value(int square) {
        static const int table[64] = {
            -50, -40, -30, -20, -20, -30, -40, -50,
            -30, -20, -10,   0,   0, -10, -20, -30,
            -30, -10,  20,  30,  30,  20, -10, -30,
            -30, -10,  30,  40,  40,  30, -10, -30,
            -30, -10,  30,  40,  40,  30, -10, -30,
            -30, -10,  20,  30,  30,  20, -10, -30,
            -30, -30,   0,   0,   0,   0, -30, -30,
            -50, -30, -30, -30, -30, -30, -30, -50
        };
        return table[square];
    }

    int get_piece_square_value(virgo::Piece piece, int square, bool endgame) {
        int actual_square = square;
        if (board[square].second == virgo::BLACK) {
            actual_square = 63 - square;
        }
        
        switch (piece) {
            case virgo::PAWN: return get_pawn_table_value(actual_square);
            case virgo::KNIGHT: return get_knight_table_value(actual_square);
            case virgo::BISHOP: return get_bishop_table_value(actual_square);
            case virgo::ROOK: return get_rook_table_value(actual_square);
            case virgo::QUEEN: return get_queen_table_value(actual_square);
            case virgo::KING: 
                return endgame ? get_king_endgame_table_value(actual_square) : get_king_table_value(actual_square);
            default: return 0;
        }
    }

    bool is_endgame() {
        int piece_count = 0;
        int queen_count = 0;
        
        for (int square = 0; square < 64; square++) {
            auto piece = board[square];
            if (piece.first != virgo::EMPTY && piece.first != virgo::KING) {
                piece_count++;
                if (piece.first == virgo::QUEEN) {
                    queen_count++;
                }
            }
        }
        
        return piece_count <= 8 || queen_count == 0 || (queen_count <= 1 && piece_count <= 10);
    }

    int evaluate_mobility() {
        int white_moves = 0;
        int black_moves = 0;

        auto current_next = board.get_next_to_move();

        if (current_next == virgo::WHITE) {
            white_moves = get_legal_moves().size();

            std::vector<uint16_t> black_moves_list;
            virgo::get_legal_moves<virgo::BLACK>(board, black_moves_list);
            black_moves = black_moves_list.size();
        } else {
            black_moves = get_legal_moves().size();

            std::vector<uint16_t> white_moves_list;
            virgo::get_legal_moves<virgo::WHITE>(board, white_moves_list);
            white_moves = white_moves_list.size();
        }

        return (white_moves - black_moves);
    }

    int evaluate_pawn_structure() {
        int score = 0;
        
        // Doubled pawns, isolated pawns, passed pawns
        uint64_t white_pawns = board.get_bitboard<virgo::WHITE>(virgo::PAWN);
        uint64_t black_pawns = board.get_bitboard<virgo::BLACK>(virgo::PAWN);

        for (int file = 0; file < 8; file++) {
            int white_pawns_in_file = 0;
            int black_pawns_in_file = 0;
            
            for (int rank = 0; rank < 8; rank++) {
                int square = rank * 8 + file;
                if (white_pawns & (1ULL << square)) white_pawns_in_file++;
                if (black_pawns & (1ULL << square)) black_pawns_in_file++;
            }

            if (white_pawns_in_file > 1) score -= 10 * (white_pawns_in_file - 1);
            if (black_pawns_in_file > 1) score += 10 * (black_pawns_in_file - 1);
        }
        
        return score;
    }

    int evaluate_bishop_pair() {
        int score = 0;

        uint64_t white_bishops = board.get_bitboard<virgo::WHITE>(virgo::BISHOP);
        uint64_t black_bishops = board.get_bitboard<virgo::BLACK>(virgo::BISHOP);
        
        int white_bishop_count = 0;
        int black_bishop_count = 0;
        
        while (white_bishops) {
            white_bishop_count++;
            white_bishops &= white_bishops - 1;
        }
        
        while (black_bishops) {
            black_bishop_count++;
            black_bishops &= black_bishops - 1;
        }
        
        if (white_bishop_count >= 2) score += 30;
        if (black_bishop_count >= 2) score -= 30;
        
        return score;
    }

    bool is_in_check() {
        virgo::Player current = board.get_next_to_move();

        virgo::Chessboard temp_board = board;

        int king_square = (current == virgo::WHITE) ? 
            temp_board.king_square<virgo::WHITE>() : 
            temp_board.king_square<virgo::BLACK>();

        std::vector<uint16_t> opponent_moves;
        if (current == virgo::WHITE) {
            virgo::get_legal_moves<virgo::BLACK>(temp_board, opponent_moves);
        } else {
            virgo::get_legal_moves<virgo::WHITE>(temp_board, opponent_moves);
        }
        
        for (auto move : opponent_moves) {
            int to_square = (move >> 6) & 0x3F;
            if (to_square == king_square) {
                return true;
            }
        }
        
        return false;
    }

    int evaluate() {
        bool endgame = is_endgame();
        int score = 0;

        for (int square = 0; square < 64; square++) {
            auto piece = board[square];
            if (piece.first != virgo::EMPTY) {
                int material_value = get_piece_value(piece.first);
                int positional_value = get_piece_square_value(piece.first, square, endgame);
                int total_value = material_value + positional_value;
                
                if (piece.second == virgo::WHITE) {
                    score += total_value;
                } else {
                    score -= total_value;
                }
            }
        }

        score += evaluate_mobility();
        score += evaluate_pawn_structure();
        score += evaluate_bishop_pair();
        
        // Tempo bonus (small bonus for side to move)
        if (board.get_next_to_move() == virgo::WHITE) {
            score += 10;
        } else {
            score -= 10;
        }
        
        return (board.get_next_to_move() == virgo::WHITE) ? score : -score;
    }

    std::string move_to_uci(uint16_t move) {
        return virgo::string::move_to_string(move);
    }
};

}  // namespace board_adapter