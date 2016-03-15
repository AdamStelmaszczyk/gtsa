#include <boost/functional/hash.hpp>

#include "../gtsa.hpp"

const int WIDTH = 8;
const int HEIGHT = 7;
const char PLAYER_1 = '1';
const char PLAYER_2 = '2';
const char EMPTY = '_';

struct ConnectFourMove : public Move<ConnectFourMove> {
    unsigned x;

    ConnectFourMove() { }

    ConnectFourMove(unsigned x) : x(x) { }

    void read() override {
        cout << "Enter column of your move [0; 7]: ";
        unsigned x;
        cin >> x;
        this->x = x;
    }

    ostream &to_stream(ostream &os) const override {
        return os << x;
    }

    bool operator==(const ConnectFourMove &rhs) const override {
        return x == rhs.x;
    }

    size_t hash() const override {
        using boost::hash_value;
        using boost::hash_combine;
        size_t seed = 0;
        hash_combine(seed, hash_value(x));
        return seed;
    }
};

struct ConnectFourState : public State<ConnectFourState, ConnectFourMove> {

    vector<char> board;

    ConnectFourState() : State(PLAYER_1) { }

    ConnectFourState(const string &init_string) : State(PLAYER_1) {
        const unsigned long length = init_string.length();
        const unsigned long correct_length = WIDTH * HEIGHT;
        if (length != correct_length) {
            throw invalid_argument("Initialization string length must be " + to_string(correct_length));
        }
        for (int i = 0; i < length; i++) {
            const char c = init_string[i];
            if (c != PLAYER_1 && c != PLAYER_2 && c != EMPTY) {
                throw invalid_argument(string("Undefined symbol used: '") + c + "'");
            }
        }
        board = vector<char>(init_string.begin(), init_string.end());
    }

    ConnectFourState clone() const override {
        ConnectFourState clone = ConnectFourState();
        clone.board = board;
        clone.player_to_move = player_to_move;
        return clone;
    }

    int get_goodness() const override {
        auto enemy = get_enemy(player_to_move);
        if (is_winner(player_to_move)) return 10000;
        if (is_winner(enemy)) return -10000;
        int player_lines = 0;
        int enemy_lines = 0;
        // - horizontal
        for (int y = 0; y < HEIGHT; ++y) {
            int count = 0;
            for (int x = 0; x < WIDTH; ++x) {
                if (board[y * WIDTH + x] == player_to_move || board[y * WIDTH + x] == EMPTY) {
                    count++;
                    if (count == 4) {
                        player_lines++;
                        count = 0;
                    }
                } else {
                    count = 0;
                }
                if (board[y * WIDTH + x] == enemy || board[y * WIDTH + x] == EMPTY) {
                    count++;
                    if (count == 4) {
                        enemy_lines++;
                        count = 0;
                    }
                } else {
                    count = 0;
                }
            }
        }
        // | vertical
        for (int x = 0; x < WIDTH; ++x) {
            int count = 0;
            for (int y = 0; y < HEIGHT; ++y) {
                if (board[y * WIDTH + x] == player_to_move || board[y * WIDTH + x] == EMPTY) {
                    count++;
                    if (count == 4) {
                        player_lines++;
                        count = 0;
                    }
                } else {
                    count = 0;
                }
                if (board[y * WIDTH + x] == enemy || board[y * WIDTH + x] == EMPTY) {
                    count++;
                    if (count == 4) {
                        enemy_lines++;
                        count = 0;
                    }
                } else {
                    count = 0;
                }
            }
        }
        // \ diagonal
        for (int y = 0; y < HEIGHT - 3; ++y) {
            for (int x = 0; x < WIDTH - 3; ++x) {
                int count = 0;
                for (int i = 0; i < 4; ++i) {
                    if (board[(y + i) * WIDTH + x + i] == player_to_move || board[(y + i) * WIDTH + x + i] == EMPTY) {
                        count++;
                    } else {
                        break;
                    }
                }
                if (count == 4) {
                    player_lines++;
                }

                count = 0;
                for (int i = 0; i < 4; ++i) {
                    if (board[(y + i) * WIDTH + x + i] == enemy || board[(y + i) * WIDTH + x + i] == EMPTY) {
                        count++;
                    } else {
                        break;
                    }
                }
                if (count == 4) {
                    enemy_lines++;
                }
            }
        }
        // / diagonal
        for (int y = 3; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH - 3; ++x) {
                int count = 0;
                for (int i = 0; i < 4; ++i) {
                    if (board[(y - i) * WIDTH + x + i] == player_to_move || board[(y - i) * WIDTH + x + i] == EMPTY) {
                        count++;
                    } else {
                        break;
                    }
                }
                if (count == 4) {
                    player_lines++;
                }

                count = 0;
                for (int i = 0; i < 4; ++i) {
                    if (board[(y - i) * WIDTH + x + i] == enemy || board[(y - i) * WIDTH + x + i] == EMPTY) {
                        count++;
                    } else {
                        break;
                    }
                }
                if (count == 4) {
                    enemy_lines++;
                }
            }
        }
        return player_lines - enemy_lines;
    }

    vector<ConnectFourMove> get_legal_moves() const override {
        vector<ConnectFourMove> result;
        for (unsigned x = 0; x < WIDTH; ++x) {
            if (board[x] == EMPTY) {
                result.emplace_back(ConnectFourMove(x));
            }
        }
        return result;
    }

    char get_enemy(char player) const override {
        return (player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
    }

    bool is_terminal() const override {
        if (!has_empty_space()) {
            return true;
        }
        return is_winner(player_to_move) || is_winner(get_enemy(player_to_move));
    }

    bool is_winner(char player) const override {
        // - horizontal
        for (int y = 0; y < HEIGHT; ++y) {
            int count = 0;
            for (int x = 0; x < WIDTH; ++x) {
                if (board[y * WIDTH + x] == player) {
                    count++;
                    if (count == 4) {
                        return true;
                    }
                } else {
                    count = 0;
                }
            }
        }
        // | vertical
        for (int x = 0; x < WIDTH; ++x) {
            int count = 0;
            for (int y = 0; y < HEIGHT; ++y) {
                if (board[y * WIDTH + x] == player) {
                    count++;
                    if (count == 4) {
                        return true;
                    }
                } else {
                    count = 0;
                }
            }
        }
        // \ diagonal
        for (int y = 0; y < HEIGHT - 3; ++y) {
            for (int x = 0; x < WIDTH - 3; ++x) {
                int count = 0;
                for (int i = 0; i < 4; ++i) {
                    if (board[(y + i) * WIDTH + x + i] == player) {
                        count++;
                    } else {
                        break;
                    }
                }
                if (count == 4) {
                    return true;
                }
            }
        }
        // / diagonal
        for (int y = 3; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH - 3; ++x) {
                int count = 0;
                for (int i = 0; i < 4; ++i) {
                    if (board[(y - i) * WIDTH + x + i] == player) {
                        count++;
                    } else {
                        break;
                    }
                }
                if (count == 4) {
                    return true;
                }
            }
        }
        return false;
    }

    void make_move(const ConnectFourMove &move) override {
        for (int y = HEIGHT - 1; y >= 0; --y) {
            if (board[y * WIDTH + move.x] == EMPTY) {
                board[y * WIDTH + move.x] = player_to_move;
                break;
            }
        }
        player_to_move = get_enemy(player_to_move);
        this->move = move;
    }

    void undo_move(const ConnectFourMove &move) override {
        for (int y = 0; y < HEIGHT; ++y) {
            if (board[y * WIDTH + move.x] != EMPTY) {
                board[y * WIDTH + move.x] = EMPTY;
                break;
            }
        }
        player_to_move = get_enemy(player_to_move);
    }

    bool has_empty_space() const {
        for (unsigned x = 0; x < WIDTH; ++x) {
            if (board[x] == EMPTY) {
                return true;
            }
        }
        return false;
    }

    ostream &to_stream(ostream &os) const override {
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                os << board[y * WIDTH + x];
            }
            os << "\n";
        }
        return os;
    }

    bool operator==(const ConnectFourState &other) const {
        return board == other.board && parent == other.parent && player_to_move == other.player_to_move;
    }

    size_t hash() const {
        using boost::hash_value;
        using boost::hash_combine;
        size_t seed = 0;
        hash_combine(seed, hash_value(board));
        hash_combine(seed, hash_value(parent));
        hash_combine(seed, hash_value(player_to_move));
        return seed;
    }
};
