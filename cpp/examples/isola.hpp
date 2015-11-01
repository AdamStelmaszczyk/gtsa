#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <limits>

#include "../gtsa.hpp"

using namespace std;

const int SIDE = 3;
const char PLAYER_1 = '1';
const char PLAYER_2 = '2';
const char EMPTY = '_';
const char REMOVED = '#';

static char get_opposite_player(char player) {
    return (player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
}

static int get_score(int options) {
    if (options == 0) {
        return -50;
    } else if (options == 1) {
        return -10;
    } else if (options == 2) {
        return 0;
    } else {
        return options + 2;
    }
}

struct IsolaMove : public Move<IsolaMove> {
    unsigned from_x;
    unsigned from_y;
    unsigned step_x;
    unsigned step_y;
    unsigned remove_x;
    unsigned remove_y;

    IsolaMove() { }

    IsolaMove(
        unsigned from_x,
        unsigned from_y,
        unsigned step_x,
        unsigned step_y,
        unsigned remove_x,
        unsigned remove_y
    ): from_x(from_x), from_y(from_y), step_x(step_x), step_y(step_y), remove_x(remove_x), remove_y(remove_y) { }

    bool operator==(const IsolaMove &rhs) const override {
        return from_x == rhs.from_x && from_y == rhs.from_y &&
               step_x == rhs.step_x && step_y == rhs.step_y &&
               remove_x == rhs.remove_x && remove_y == rhs.remove_y;
    }

    ostream &to_stream(ostream &os) const override {
        return os << from_x << " " << from_y << " " << step_x << " " << step_y << " " << remove_x << " " << remove_y;
    }
};

typedef pair<unsigned, unsigned> cords;

struct IsolaState : public State<IsolaState, IsolaMove> {

    vector<char> board;
    cords player_1_cords;
    cords player_2_cords;

    IsolaState() { }

    IsolaState(const string &init_string) {
        const unsigned long length = init_string.length();
        const unsigned long correct_length = SIDE * SIDE;
        if (length != correct_length) {
            throw invalid_argument("Initialization string length must be " + std::to_string(correct_length));
        }
        for (int i = 0; i < length; i++) {
            const char c = init_string[i];
            if (c != PLAYER_1 && c != PLAYER_2 && c != EMPTY && c != REMOVED) {
                throw invalid_argument(string("Undefined symbol used: '") + c + "'");
            }
        }
        board = vector<char>(init_string.begin(), init_string.end());
        player_1_cords = find_player_cords(PLAYER_1);
        player_2_cords = find_player_cords(PLAYER_2);
    }

    IsolaState clone() const override {
        IsolaState clone = IsolaState();
        clone.board = board;
        clone.player_1_cords = player_1_cords;
        clone.player_2_cords = player_2_cords;
        return clone;
    }

    int get_goodness(char player) const override {
        if (is_winner(player)) {
            return 100;
        }
        char enemy = get_opposite_player(player);
        if (is_winner(enemy)) {
            return -100;
        }
        const int current_player_options = get_number_of_legal_steps(player);
        const int current_player_score = get_score(current_player_options);
        const int next_player_options = get_number_of_legal_steps(enemy);
        const int next_player_score = get_score(next_player_options);
        return current_player_score - next_player_score;
    }

    vector<IsolaMove> get_legal_moves(char player) const override {
        auto player_cords = get_player_cords(player);
        auto step_moves = get_legal_step_moves(player_cords.first, player_cords.second);
        auto remove_moves = get_legal_remove_moves(player);
        vector<IsolaMove> legal_moves(step_moves.size() * remove_moves.size());
        unsigned legal_moves_count = 0;
        for (const auto &step_move : step_moves) {
            for (const auto &remove_move : remove_moves) {
                if (step_move != remove_move) {
                    legal_moves[legal_moves_count] = IsolaMove(
                            player_cords.first, player_cords.second,
                            step_move.first, step_move.second,
                            remove_move.first, remove_move.second
                    );
                    ++legal_moves_count;
                }
            }
        }
        legal_moves.resize(legal_moves_count);
        return legal_moves;
    }

    bool is_terminal(char player) const override {
        return get_number_of_legal_steps(player) == 0;
    }

    bool is_winner(char player) const override {
        return this->player_who_moved == player && get_number_of_legal_steps(get_opposite_player(player)) == 0;
    }

    void make_move(const IsolaMove &move, char player) override {
        board[move.from_y * SIDE + move.from_x] = EMPTY;
        board[move.step_y * SIDE + move.step_x] = player;
        board[move.remove_y * SIDE + move.remove_x] = REMOVED;
        set_player_cords(player, make_pair(move.step_x, move.step_y));
        player_who_moved = player;
    }

    void undo_move(const IsolaMove &move, char player) override {
        board[move.remove_y * SIDE + move.remove_x] = EMPTY;
        board[move.from_y * SIDE + move.from_x] = player;
        board[move.step_y * SIDE + move.step_x] = EMPTY;
        set_player_cords(player, make_pair(move.from_x, move.from_y));
        player_who_moved = get_opposite_player(player);
    }

    vector<cords> get_legal_remove_moves(char player) const {
        vector<cords> result(SIDE * SIDE);
        unsigned moves_count = 0;
        for (unsigned y = 0; y < SIDE; ++y) {
            for (unsigned x = 0; x < SIDE; ++x) {
                const char symbol = board[y * SIDE + x];
                if (symbol == EMPTY || symbol == player) {
                    result[moves_count] = make_pair(x, y);
                    ++moves_count;
                }
            }
        }
        result.resize(moves_count);
        return result;
    }

    vector<cords> get_legal_step_moves(int start_x, int start_y) const {
        vector<cords> result(8);
        unsigned moves_count = 0;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int x = start_x + dx;
                const int y = start_y + dy;
                if (x >= 0 && x < SIDE && y >= 0 && y < SIDE && board[y * SIDE + x] == EMPTY) {
                    result[moves_count] = make_pair(x, y);
                    ++moves_count;
                }
            }
        }
        result.resize(moves_count);
        return result;
    }

    int get_number_of_legal_steps(char player) const {
        int result = 0;
        auto player_cords = get_player_cords(player);
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int x = player_cords.first + dx;
                const int y = player_cords.second + dy;
                if (x >= 0 && x < SIDE && y >= 0 && y < SIDE && board[y * SIDE + x] == EMPTY) {
                    ++result;
                }
            }
        }
        return result;
    }

    cords get_player_cords(char player) const {
        return (player == PLAYER_1) ? player_1_cords : player_2_cords;
    }

    cords find_player_cords(char player) const {
        for (int y = 0; y < SIDE; ++y) {
            for (int x = 0; x < SIDE; ++x) {
                if (board[y * SIDE + x] == player) {
                    return make_pair(x, y);
                }
            }
        }
        throw invalid_argument(string("No ") + player + " on the board");
    };

    void set_player_cords(char player, cords player_cords) {
        (player == PLAYER_1) ? player_1_cords = player_cords : player_2_cords = player_cords;
    }

    ostream &to_stream(ostream &os) const override {
        for (int y = 0; y < SIDE; ++y) {
            for (int x = 0; x < SIDE; ++x) {
                os << board[y * SIDE + x];
            }
            os << "\n";
        }
        return os;
    }

};

struct IsolaMoveReader : public MoveReader<IsolaMove> {
    IsolaMove read() const {
        // TODO: implement
    }
};