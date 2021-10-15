#include "../gtsa.hpp"

using std::unordered_set;

const int SIDE = 5;
const char PLAYER_1 = '1';
const char PLAYER_2 = '2';
const char EMPTY = '_';

struct GoMove : public Move<GoMove> {
    int x;
    int y;

    GoMove() { }

    GoMove(int x, int y) : x(x), y(y) { }

    void read(istream &stream = cin) override {
        if (&stream == &cin) {
            cout << "Enter space separated X and Y of your move (X = -1 is a pass): ";
        }
        stream >> x >> y;
    }

    ostream &to_stream(ostream &os) const override {
        return os << x << " " << y;
    }

    bool operator==(const GoMove &rhs) const override {
        return x == rhs.x && y == rhs.y;
    }

    size_t hash() const override {
        using boost::hash_value;
        using boost::hash_combine;
        size_t seed = 0;
        hash_combine(seed, hash_value(x));
        hash_combine(seed, hash_value(y));
        return seed;
    }
};

struct cords {
    int x;
    int y;

    friend ostream &operator<<(ostream &os, const cords &c) {
        return os << c.x << " " << c.y;
    }
};

struct ReachResult {
    vector<cords> area;
    bool closed;

    friend ostream &operator<<(ostream &os, const ReachResult &r) {
        os << "area:" << endl;
        for (const auto a : r.area) {
            os << a << endl;
        }
        os << "closed: " << r.closed;
        return os;
    }
};

struct GoState : public State<GoState, GoMove> {

    vector<char> board;
    vector<char> prev_board;
    unordered_set<size_t> board_history;
    vector<bool> pass;

    GoState() : State({0, 1}) {
        board_history.insert(hash());
    }

    GoState(const string &init_string) : State({0, 1}) {
        const unsigned long length = init_string.length();
        const unsigned long correct_length = SIDE * SIDE;
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
        pass = vector<bool>(teams.size());
        board_history.insert(hash());
    }

    GoState clone() const override {
        GoState clone = GoState();
        clone.board = board;
        clone.prev_board = prev_board;
        clone.pass = pass;
        clone.board_history = board_history;
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

    vector<GoMove> get_legal_moves(int max_moves = INF) const override {
        // A turn is either a pass; or a move that doesn't repeat an earlier grid coloring.
        int available_moves = SIDE * SIDE + 1;
        if (max_moves > available_moves) {
            max_moves = available_moves;
        }
        vector<GoMove> moves(max_moves);
        auto copy = clone();
        int i = 0;
        for (int y = 0; y < SIDE; ++y) {
            for (int x = 0; x < SIDE; ++x) {
                if (board[y * SIDE + x] == EMPTY) {
                    GoMove move = {x, y};
                    copy.make_move(move);
                    auto hash = copy.hash();
                    copy.undo_move(move);
                    if (board_history.find(hash) == board_history.end()) { // positional superko
                        moves[i++] = move;
                        if (i >= max_moves) {
                            return moves;
                        }
                    }
                }
            }
        }
        moves[i++] = GoMove(-1, 0); // pass
        moves.resize(i);
        return moves;
    }

    bool is_terminal() const override {
        return all_of(pass.begin(), pass.end(), [](bool v) { return v; });
    }

    bool is_winner(int player) const override {
        return get_score(player) > get_score(get_next_player(player));
    }

    int get_score(int player) const {
        return get_stones(player) + get_area(player);
    }

    int get_stones(int player) const {
        return count(board.begin(), board.end(), player_index_to_char(player));
    }

    int get_area(int player) const { // area = number of empty points that reach only player's stones
        int area = 0;
        vector<bool> seen(board.size());
        const char player_char = player_index_to_char(player);
        for (int x = 0; x < SIDE; ++x) {
            for (int y = 0; y < SIDE; ++y) {
                if (!seen[y * SIDE + x]) {
                    const auto result = reach(x, y, EMPTY, player_char, seen);
                    if (result.closed) {
                        area += result.area.size();
                    }
                }
            }
        }
        return area;
    }

    ReachResult reach(const int x, const int y, const char from, const char to, vector<bool> &seen) const {
        if (x < 0 || y < 0 || x >= SIDE || y >= SIDE) {
            return {{}, true};
        }
        const int i = y * SIDE + x;
        if (seen[i] && board[i] == from) {
            return {{}, true};
        }
        seen[i] = true;
        if (board[i] == to) {
            return {{}, true};
        }
        if (board[i] != from) {
            return {{}, false};
        }
        const auto n = reach(x, y - 1, from, to, seen);
        const auto e = reach(x + 1, y, from, to, seen);
        const auto w = reach(x - 1, y, from, to, seen);
        const auto s = reach(x, y + 1, from, to, seen);
        vector<cords> area = {{x, y}};
        area.insert(area.end(), n.area.begin(), n.area.end());
        area.insert(area.end(), e.area.begin(), e.area.end());
        area.insert(area.end(), w.area.begin(), w.area.end());
        area.insert(area.end(), s.area.begin(), s.area.end());
        return {area, n.closed && e.closed && w.closed && s.closed};
    }

    void clear(const int x, const int y, const int player) { // empty all player's stones that reach only enemy
        vector<bool> seen(board.size());
        auto result = reach(x, y, player_index_to_char(player), player_index_to_char(get_next_player(player)), seen);
        if (result.closed) {
            for (const cords c : result.area) {
                board[c.y * SIDE + c.x] = EMPTY;
            }
        }
    }

    void make_move(const GoMove &move) override {
        if (move.x == -1) {
            pass[player_to_move] = true;
            player_to_move = get_next_player(player_to_move);
            return;
        }
        pass[player_to_move] = false;
        prev_board = board;
        board[move.y * SIDE + move.x] = player_index_to_char(player_to_move);
        const int enemy = get_next_player(player_to_move);
        clear(move.x, move.y - 1, enemy);
        clear(move.x + 1, move.y, enemy);
        clear(move.x - 1, move.y, enemy);
        clear(move.x, move.y + 1, enemy);
        clear(move.x, move.y, player_to_move); // suicide
        player_to_move = get_next_player(player_to_move);
        board_history.insert(hash());
    }

    void undo_move(const GoMove &move) override {
        board_history.erase(hash());
        player_to_move = get_prev_player(player_to_move);
        board = prev_board;
        pass[player_to_move] = false;
    }

    int player_char_to_index(char player) const override {
        return (player == PLAYER_1) ? 0 : 1;
    }

    char player_index_to_char(int index) const override {
        return (index == 0) ? PLAYER_1 : PLAYER_2;
    }

    ostream &to_stream(ostream &os) const override {
        for (int y = 0; y < SIDE; ++y) {
            for (int x = 0; x < SIDE; ++x) {
                os << board[y * SIDE + x];
            }
            os << endl;
        }
        os << player_index_to_char(player_to_move) << endl;
        for (auto p : pass) {
            os << p << " ";
        }
        os << endl;
        return os;
    }

    bool operator==(const GoState &other) const override {
        return board == other.board;
    }

    size_t hash() const override {
        using boost::hash_value;
        using boost::hash_combine;
        size_t seed = 0;
        hash_combine(seed, hash_value(board));
        return seed;
    }
};
