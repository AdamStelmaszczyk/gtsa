#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <limits>

#include "../gtsa.cpp"

using namespace std;

const int SIDE = 3;
const char PLAYER_1 = 'X';
const char PLAYER_2 = 'O';
const char EMPTY = '_';

char get_opposite_player(char player) {
    return (player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
}

struct TicTacToeMove : public Move {
    unsigned x;
    unsigned y;

    TicTacToeMove(unsigned x, unsigned y) : x(x), y(y) { }
};

ostream &operator<<(ostream &os, TicTacToeMove const &coord) {
    return os << coord.x << " " << coord.y;
}

const auto LINES = [] {
    vector<vector<TicTacToeMove>> lines;
    for (int y = 0; y < SIDE; ++y) {
        lines.emplace_back(); // add a new line to back()
        for (int x = 0; x < SIDE; ++x)
            lines.back().emplace_back(x, y); // add a new coord to that line
    }
    for (int x = 0; x < SIDE; ++x) {
        lines.emplace_back();
        for (int y = 0; y < SIDE; ++y)
            lines.back().emplace_back(x, y);
    }
    lines.emplace_back();
    for (int i = 0; i < SIDE; ++i) {
        lines.back().emplace_back(i, i);
    }
    lines.emplace_back();
    for (int i = 0; i < SIDE; ++i) {
        lines.back().emplace_back(SIDE - i - 1, i);
    }
    return lines;
}();

struct TicTacToeState : public State<TicTacToeState, TicTacToeMove> {

    const unsigned side;
    vector<char> board;

    TicTacToeState(unsigned side, const string &init_string = "") : side(side) {
        const int correct_length = side * side;
        if (init_string != "") {
            const unsigned long length = init_string.length();
            if (length != correct_length) {
                throw invalid_argument("Initialization string length must be " + std::to_string(correct_length));
            }
            for (int i = 0; i < length; i++) {
                const char c = init_string[i];
                if (c != PLAYER_1 && c != PLAYER_2 && c != EMPTY) {
                    string message = "Undefined symbol used: '";
                    message += c;
                    message += "'";
                    throw invalid_argument(message);
                }
            }
            board = vector<char>(init_string.begin(), init_string.end());
        }
    }

    unique_ptr<TicTacToeState> clone() const override {
        TicTacToeState *clone = new TicTacToeState(side);
        clone->board = board;
        return unique_ptr<TicTacToeState>(clone);
    }

    int get_goodness(char current_player) const override {
        int goodness = 0;
        auto counts = count_players_on_lines(current_player);
        for (const auto &count : counts) {
            if (count[0] == 3) {
                goodness += side * side;
            }
            else if (count[1] == side) {
                goodness -= side * side;
            }
            else if (count[0] == SIDE - 1 and count[1] == 0) {
                goodness += side;
            }
            else if (count[1] == SIDE - 1 and count[0] == 0) {
                goodness -= side;
            }
            else if (count[0] == SIDE - 2 and count[1] == 0) {
                ++goodness;
            }
            else if (count[1] == SIDE - 2 and count[0] == 0) {
                --goodness;
            }
        }
        return goodness;
    }

    vector<TicTacToeMove> get_legal_moves(char player) const override {
        vector<TicTacToeMove> result;
        for (unsigned y = 0; y < side; ++y) {
            for (unsigned x = 0; x < side; ++x) {
                if (board[y * side + x] == EMPTY) {
                    result.emplace_back(TicTacToeMove(x, y));
                }
            }
        }
        return result;
    }

    bool is_terminal(char player) const override {
        if (!has_empty_space()) {
            return true;
        }
        for (const auto &count : count_players_on_lines(player)) {
            if (count[0] == side || count[1] == side) {
                return true;
            }
        }
        return false;
    }

    bool is_winner(char player) const override {
        for (const auto &count : count_players_on_lines(player)) {
            if (count[0] == side) {
                return true;
            }
        }
        return false;
    }

    void make_move(TicTacToeMove &move, char player) override {
        board[move.y * side + move.x] = player;
        player_who_moved = player;
    }

    void undo_move(TicTacToeMove &move, char player) override {
        board[move.y * side + move.x] = EMPTY;
        player_who_moved = get_opposite_player(player);
    }

    bool has_empty_space() const {
        for (unsigned y = 0; y < side; ++y) {
            for (unsigned x = 0; x < side; ++x) {
                if (board[y * side + x] == EMPTY) {
                    return true;
                }
            }
        }
        return false;
    }

    vector<vector<int>> count_players_on_lines(char current_player) const {
        vector<vector<int>> counts;
        char next_player = get_opposite_player(current_player);
        for (const auto &line : LINES) {
            int player_places = 0;
            int enemy_places = 0;
            for (const TicTacToeMove &coord : line) {
                const int i = coord.y * side + coord.x;
                if (board[i] == current_player) {
                    ++player_places;
                }
                else if (board[i] == next_player) {
                    ++enemy_places;
                }
            }
            auto count = {player_places, enemy_places};
            counts.emplace_back(count);
        }
        return counts;
    }

};

ostream &operator<<(ostream &os, TicTacToeState const &state) {
    for (int y = 0; y < state.side; ++y) {
        for (int x = 0; x < state.side; ++x) {
            os << state.board[y * state.side + x];
        }
        os << "\n";
    }
    return os;
}

int main() {
    TicTacToeState state = TicTacToeState(3, "___"
                                             "_X_"
                                             "___");
    return 0;
}