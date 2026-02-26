// engine_connect5_2.h
#ifndef ENGINE_CONNECT5_H
#define ENGINE_CONNECT5_H

#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <functional>

constexpr int SIZE = 15;
constexpr int BOARD_CELLS = SIZE * SIZE;
constexpr int MAX_DEPTH = 20;
constexpr int INF = 10000000;

inline int idx(int r, int c) { return r * SIZE + c; }

const int DR[4] = {0, 1, 1, 1};
const int DC[4] = {1, 0, 1, -1};

struct Position {
    uint8_t board[BOARD_CELLS];
    int side_to_move; // 1 = black (AI), 2 = white (human)
    int last_move = -1;

    Position() {
        std::fill(board, board + BOARD_CELLS, 0);
        side_to_move = 1;
    }

    Position(const std::string &s) {
        std::fill(board, board + BOARD_CELLS, 0);
        
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
    
    std::string to_string() const {
        std::string result;
        for (int i = 0; i < BOARD_CELLS; i++) {
            if (board[i] == 0) result += '.';
            else if (board[i] == 1) result += 'B';
            else result += 'W';
        }
        return result;
    }
};

inline bool has_five_fast(const Position &pos, int color) {
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

inline bool would_create_five(const Position &pos, int move_idx, int color) {
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

inline bool creates_double_threat(const Position &pos, int move_idx, int color) {
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

inline int find_winning_move(const Position &pos, int color) {
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

inline int find_double_threat_move(const Position &pos, int our_color) {
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

inline int evaluate_position(const Position &pos) {
    if (has_five_fast(pos, 1)) return 1000000;
    if (has_five_fast(pos, 2)) return -1000000;

    constexpr int PATTERN_FIVE = 1000000;
    constexpr int OPEN_FOUR = 50000;
    constexpr int CLOSED_FOUR = 8000;
    constexpr int OPEN_THREE = 3000;
    constexpr int CLOSED_THREE = 500;
    constexpr int OPEN_TWO = 200;

    int black_grid[BOARD_CELLS] = {0};
    int white_grid[BOARD_CELLS] = {0};

    auto eval_direction = [&](int dr, int dc) {
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                int rr = r, cc = c;
                std::vector<int> line;
                line.reserve(SIZE);
                std::vector<int> idxs;

                while (rr >= 0 && rr < SIZE && cc >= 0 && cc < SIZE) {
                    line.push_back(pos.board[idx(rr, cc)]);
                    idxs.push_back(idx(rr, cc));
                    rr += dr;
                    cc += dc;
                }

                int len = (int)line.size();
                for (int i = 0; i <= len - 5; ++i) {
                    int black = 0, white = 0, empty = 0;
                    for (int j = 0; j < 5; ++j) {
                        if (line[i + j] == 1) black++;
                        else if (line[i + j] == 2) white++;
                        else empty++;
                    }

                    if (black > 0 && white > 0) continue;

                    int left_empty = (i > 0 && line[i - 1] == 0);
                    int right_empty = (i + 5 < len && line[i + 5] == 0);
                    bool open_ends = left_empty && right_empty;

                    int strength = 0;
                    int color = 0;

                    if (black > 0) {
                        color = 1;
                        if (black == 5) strength = PATTERN_FIVE;
                        else if (black == 4 && open_ends) strength = OPEN_FOUR;
                        else if (black == 4) strength = CLOSED_FOUR;
                        else if (black == 3 && open_ends) strength = OPEN_THREE;
                        else if (black == 3) strength = CLOSED_THREE;
                        else if (black == 2 && open_ends) strength = OPEN_TWO;
                    } else if (white > 0) {
                        color = 2;
                        if (white == 5) strength = PATTERN_FIVE;
                        else if (white == 4 && open_ends) strength = OPEN_FOUR;
                        else if (white == 4) strength = CLOSED_FOUR;
                        else if (white == 3 && open_ends) strength = OPEN_THREE;
                        else if (white == 3) strength = CLOSED_THREE;
                        else if (white == 2 && open_ends) strength = OPEN_TWO;
                    }

                    if (strength > 0) {
                        for (int j = 0; j < 5; ++j) {
                            int id = idxs[i + j];
                            if (color == 1) black_grid[id] += strength / 5;
                            else white_grid[id] += strength / 5;
                        }
                    }
                }
            }
        }
    };

    eval_direction(0, 1);
    eval_direction(1, 0);
    eval_direction(1, 1);
    eval_direction(-1, 1);

    double black_score = 0.0, white_score = 0.0;
    for (int i = 0; i < BOARD_CELLS; ++i) {
        if (pos.board[i] == 0) continue;
        if (pos.board[i] == 1) black_score += black_grid[i];
        else if (pos.board[i] == 2) white_score += white_grid[i];
    }

    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            int cell = pos.board[idx(r, c)];
            if (cell == 0) continue;
            int dist = std::abs(r - SIZE / 2) + std::abs(c - SIZE / 2);
            int center_bonus = (SIZE - dist);
            if (cell == 1) black_score += center_bonus * 2;
            else white_score += center_bonus * 2;
        }
    }

    auto nonlinear = [](double base) -> double {
        if (base == 0) return 0;
        return base + 0.8 * (base * base / 5000.0);
    };

    int total_score = (int)(nonlinear(black_score) - nonlinear(white_score));
    return std::clamp(total_score, -999999, 999999);
}


inline std::vector<int> generate_moves(const Position &pos) {

    constexpr int TOP_K = 12; 
    struct ScoredMove { int idx; int score; };
    std::vector<ScoredMove> scored;

    bool has_stones = false;
    for (int i = 0; i < BOARD_CELLS; ++i) {
        if (pos.board[i] != 0) { has_stones = true; break; }
    }
    if (!has_stones) {
        return { idx(SIZE/2, SIZE/2) };
    }

    std::vector<char> candidate(BOARD_CELLS, 0);
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (pos.board[idx(r,c)] != 0) {
                for (int dr = -2; dr <= 2; ++dr) {
                    for (int dc = -2; dc <= 2; ++dc) {
                        int rr = r + dr, cc = c + dc;
                        if (rr >= 0 && rr < SIZE && cc >= 0 && cc < SIZE) {
                            int id = idx(rr, cc);
                            if (pos.board[id] == 0) candidate[id] = 1;
                        }
                    }
                }
            }
        }
    }

    auto line_potential = [&](int move_idx, int color) {
        int r = move_idx / SIZE, c = move_idx % SIZE;
        int potential = 0;
        for (int dir = 0; dir < 4; ++dir) {
            int dr = DR[dir], dc = DC[dir];
            for (int offset = -4; offset <= 0; ++offset) {
                int our = 0, opp = 0, empty = 0;
                bool valid = true;
                for (int k = 0; k < 5; ++k) {
                    int rr = r + (offset + k) * dr;
                    int cc = c + (offset + k) * dc;
                    if (rr < 0 || rr >= SIZE || cc < 0 || cc >= SIZE) { valid = false; break; }
                    int val = pos.board[idx(rr,cc)];
                    if (offset + k == -offset) { 
                    }
                    if (val == color) our++;
                    else if (val == 0) empty++;
                    else opp++;
                }
                if (!valid) continue;

                if (opp == 0) {
                    if (our == 4 && empty == 1) potential += 10000; // open/closed four
                    if (our == 3 && empty == 2) potential += 2000;  // open three-like
                    if (our == 2 && empty == 3) potential += 200;   // small potential
                } else {
                    if (our == 4 && opp == 0) potential += 10000;
                }
            }
        }
        return potential;
    };

    for (int i = 0; i < BOARD_CELLS; ++i) {
        if (!candidate[i]) continue;
        int move_score = 0;
        int r = i / SIZE, c = i % SIZE;

        int center_dist = std::abs(r - SIZE/2) + std::abs(c - SIZE/2);
        move_score += (SIZE * 10) - center_dist * 10;

        if (would_create_five(pos, i, pos.side_to_move)) {
            move_score += 1000000;
        }

        if (would_create_five(pos, i, 3 - pos.side_to_move)) {
            move_score += 500000;
        }

        if (creates_double_threat(pos, i, pos.side_to_move)) {
            move_score += 800000;
        }

        if (creates_double_threat(pos, i, 3 - pos.side_to_move)) {
            move_score += 600000;
        }

        move_score += line_potential(i, pos.side_to_move);
        move_score += line_potential(i, 3 - pos.side_to_move) / 2; 


        if (pos.last_move != -1) {
            int lr = pos.last_move / SIZE, lc = pos.last_move % SIZE;
            int d = std::abs(lr - r) + std::abs(lc - c);
            if (d <= 2) move_score += 50;
        }

        scored.push_back({i, move_score});
    }

    std::sort(scored.begin(), scored.end(), [](const ScoredMove &a, const ScoredMove &b) {
        return a.score > b.score;
    });

    std::vector<int> moves;
    int limit = std::min((int)scored.size(), TOP_K);
    for (int k = 0; k < limit; ++k) moves.push_back(scored[k].idx);

    if (moves.empty()) {
        for (int i = 0; i < BOARD_CELLS; ++i) if (pos.board[i] == 0) { moves.push_back(i); break; }
    }

    return moves;
}


#endif