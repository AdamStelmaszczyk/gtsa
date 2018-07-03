#include <boost/functional/hash.hpp>

#include "../gtsa.hpp"

using std::pair;
using std::make_pair;

const int WIDTH = 8;
const int HEIGHT = 7;
const char PLAYER_1 = '1';
const char PLAYER_2 = '2';
const char EMPTY = '_';

struct ConnectFourMove : public Move<ConnectFourMove> {
    unsigned x;

    ConnectFourMove() { }

    ConnectFourMove(unsigned x) : x(x) { }

    void read(istream &stream = cin) override {
        if (&stream == &cin) {
            cout << "Enter column of your move [0; 7]: ";
        }
        stream >> x;
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

typedef pair<int, int> cords;

struct Board {
    uint64_t board = 0;

    Board() {}

    Board(const Board& other) {
        board = other.board;
    }

    void set(int x, int y, uint64_t value) {
        uint64_t i = HEIGHT - y - 1 + x * WIDTH;
        board ^= (-value ^ board) & (1LL << i);
    }

    bool get(int x, int y) const {
        uint64_t i = HEIGHT - y - 1 + x * WIDTH;
        return (board >> i) & 1;
    }

    bool operator==(const Board &other) const {
        return board == other.board;
    }
};

size_t hash_value(const Board &board) {
    std::hash<uint64_t> hash_fn;
    return hash_fn(board.board);
}

struct ConnectFourState : public State<ConnectFourState, ConnectFourMove> {

    Board board_1, board_2;

    ConnectFourState() : State(2) { }

    ConnectFourState(const string &init_string) : State(2) {
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
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                const char c = init_string[y * WIDTH + x];
                if (c == PLAYER_1) {
                    board_1.set(x, y, 1);
                } else if (c == PLAYER_2) {
                    board_2.set(x, y, 1);
                }
            }
        }
    }

    ConnectFourState clone() const override {
        ConnectFourState clone = ConnectFourState();
        clone.board_1 = Board(board_1);
        clone.board_2 = Board(board_2);
        clone.player_to_move = player_to_move;
        return clone;
    }

    int get_goodness() const override {
        if (is_terminal()) {
            if (is_winner(player_to_move)) {
                return 10000;
            }
            if (is_winner(get_next_player(player_to_move))) {
                return -10000;
            }
            return 10;
        }
        return 0;
    }

    Board& get_board(int player) {
        return (player == 0) ? board_1 : board_2;
    }

    const Board& get_board(int player) const {
        return (player == 0) ? board_1 : board_2;
    }

    vector<ConnectFourMove> get_legal_moves(int max_moves = INF) const override {
        const auto &board = get_board(player_to_move);
        int available_moves = WIDTH;
        if (max_moves > available_moves) {
            max_moves = available_moves;
        }
        vector<ConnectFourMove> moves(max_moves);
        int i = 0;
        for (unsigned x = 0; x < WIDTH; ++x) {
            if (is_empty(x, 0)) {
                moves[i++] = ConnectFourMove(x);
                if (i >= max_moves) {
                    return moves;
                }
            }
        }
        moves.resize(i);
        return moves;
    }

    bool is_terminal() const override {
        if (!has_empty_space()) {
            return true;
        }
        return is_winner(player_to_move) || is_winner(get_next_player(player_to_move));
    }

    bool is_winner(int player) const override {
        uint64_t board = get_board(player).board;
        uint64_t y = board & (board >> 7LL);
        uint64_t z = board & (board >> 8LL);
        uint64_t w = board & (board >> 9LL);
        uint64_t x = board & (board >> 1LL);
        return (y & (y >> 2 * 7LL)) |
               (z & (z >> 2 * 8LL)) |
               (w & (w >> 2 * 9LL)) |
               (x & (x >> 2LL));
    }

    void make_move(const ConnectFourMove &move) override {
        for (int y = HEIGHT - 1; y >= 0; --y) {
            if (is_empty(move.x, y)) {
                auto &board = get_board(player_to_move);
                board.set(move.x, y, 1);
                break;
            }
        }
        player_to_move = get_next_player(player_to_move);
    }

    void undo_move(const ConnectFourMove &move) override {
        for (int y = 0; y < HEIGHT; ++y) {
            if (!is_empty(move.x, y)) {
                board_1.set(move.x, y, 0);
                board_2.set(move.x, y, 0);
                break;
            }
        }
        player_to_move = get_next_player(player_to_move);
    }

    bool has_empty_space() const {
        uint64_t board = board_1.board | board_2.board;
        // checks if top row has any empty space
        return (board & 4629771061636907072LL) != 4629771061636907072LL;
    }

    bool is_empty(int x, int y) const {
        return board_1.get(x, y) == 0 && board_2.get(x, y) == 0;
    }

    ostream &to_stream(ostream &os) const override {
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                const cords c = make_pair(x, y);
                if (board_1.get(x, y) == 1) {
                    os << PLAYER_1;
                } else if (board_2.get(x, y) == 1) {
                    os << PLAYER_2;
                } else {
                    os << EMPTY;
                }
            }
            os << endl;
        }
        os << player_index_to_char(player_to_move) << endl;
        return os;
    }

    bool operator==(const ConnectFourState &other) const override {
        return board_1 == other.board_1 &&
               board_2 == other.board_2 &&
               player_to_move == other.player_to_move;
    }

    size_t hash() const override {
        using boost::hash_value;
        using boost::hash_combine;
        size_t seed = 0;
        hash_combine(seed, hash_value(board_1));
        hash_combine(seed, hash_value(board_2));
        hash_combine(seed, hash_value(player_to_move));
        return seed;
    }
};
