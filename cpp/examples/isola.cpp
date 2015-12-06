#include <boost/functional/hash.hpp>

#include "../gtsa.hpp"

const int SIDE = 3;
const char PLAYER_1 = '1';
const char PLAYER_2 = '2';
const char EMPTY = '_';
const char REMOVED = '#';

const int DX[] = {-1, 1, 1, -1, 0, 0, -1, 1 };
const int DY[] = {-1, 1, -1, 1, -1, 1, 0, 0 };

static char get_opposite_player(char player) {
    return (player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
}

static int get_score_for_cords(int x, int y) {
    return min(min(x + 1, SIDE - x), min(y + 1, SIDE - y));
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

    void read() override {
        cout << "Enter space separated from_x from_y step_x step_y remove_x remove_y: ";
        unsigned from_x, from_y, step_x, step_y, remove_x, remove_y;
        cin >> from_x >> from_y >> step_x >> step_y >> remove_x >> remove_y;
        this->from_x = from_x;
        this->from_y = from_y;
        this->step_x = step_x;
        this->step_y = step_y;
        this->remove_x = remove_x;
        this->remove_y = remove_y;
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

    IsolaState() : State(PLAYER_1) { }

    IsolaState(const string &init_string) : State(PLAYER_1) {
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

    int get_goodness() const override {
        cords player_cords = get_player_cords(player_to_move);
        const int player_score = get_score_for_legal_steps(player_cords, 2);
        if (player_score == 0) {
            return -1000;
        }
        const char enemy = get_opposite_player(player_to_move);
        cords enemy_cords = get_player_cords(enemy);
        const int enemy_score = get_score_for_legal_steps(enemy_cords, 2);
        if (enemy_score == 0) {
            return 1000;
        }
        return player_score - enemy_score;
    }

    vector<IsolaMove> get_legal_moves() const override {
        auto player_cords = get_player_cords(player_to_move);
        auto step_moves = get_legal_step_moves(player_cords.first, player_cords.second);
        auto remove_moves = get_legal_remove_moves();
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

    bool is_terminal() const override {
        cords player_cords = get_player_cords(player_to_move);
        return get_score_for_legal_steps(player_cords) == 0;
    }

    bool is_winner(char player) const override {
        const char enemy = get_opposite_player(player);
        cords player_cords = get_player_cords(enemy);
        return player_to_move == enemy && get_score_for_legal_steps(player_cords) == 0;
    }

    void make_move(const IsolaMove &move) override {
        board[move.from_y * SIDE + move.from_x] = EMPTY;
        board[move.step_y * SIDE + move.step_x] = player_to_move;
        board[move.remove_y * SIDE + move.remove_x] = REMOVED;
        set_player_cords(player_to_move, make_pair(move.step_x, move.step_y));
        player_to_move = get_opposite_player(player_to_move);
    }

    void undo_move(const IsolaMove &move) override {
        player_to_move = get_opposite_player(player_to_move);
        set_player_cords(player_to_move, make_pair(move.from_x, move.from_y));
        board[move.remove_y * SIDE + move.remove_x] = EMPTY;
        board[move.step_y * SIDE + move.step_x] = EMPTY;
        board[move.from_y * SIDE + move.from_x] = player_to_move;
    }

    vector<cords> get_legal_remove_moves() const {
        // Prioritize remove moves around the enemy + the field player is standing on
        const auto enemy_cords = get_player_cords(get_opposite_player(player_to_move));
        auto result = get_legal_step_moves(enemy_cords.first, enemy_cords.second);
        result.emplace_back(get_player_cords(player_to_move));
        return result;
    }

    vector<cords> get_legal_step_moves(int start_x, int start_y) const {
        vector<cords> result(8);
        unsigned moves_count = 0;
        for (int i = 0; i < 8; ++i) {
            const int x = start_x + DX[i];
            const int y = start_y + DY[i];
            if (x >= 0 && x < SIDE && y >= 0 && y < SIDE && board[y * SIDE + x] == EMPTY) {
                result[moves_count++] = make_pair(x, y);
            }
        }
        result.resize(moves_count);
        return result;
    }

    int get_score_for_legal_steps(cords player_cords, int depth = 1) const {
        int result = 0;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int x = player_cords.first + dx;
                const int y = player_cords.second + dy;
                if (x >= 0 && x < SIDE && y >= 0 && y < SIDE && board[y * SIDE + x] == EMPTY) {
                    if (depth <= 1) {
                        result += get_score_for_cords(x, y);
                    } else {
                        result += get_score_for_legal_steps(make_pair(x, y), depth - 1);
                    }
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
        os << player_to_move << "\n";
        return os;
    }

    bool operator==(const IsolaState &other) const {
        return board == other.board && player_to_move == other.player_to_move;
    }

    size_t operator()(const IsolaState& key) const {
        using boost::hash_value;
        using boost::hash_combine;
        std::size_t seed = 0;
        hash_combine(seed, hash_value(key.board));
        hash_combine(seed, hash_value(key.player_to_move));
        return seed;
    }
};