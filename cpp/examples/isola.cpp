#include <boost/functional/hash.hpp>
#include <bitset>

#include "../gtsa.hpp"

const int SIDE = 7;
const char PLAYER_1 = '1';
const char PLAYER_2 = '2';
const char EMPTY = '_';
const char REMOVED = '#';

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

    bool operator==(const IsolaMove &rhs) const override {
        return from_x == rhs.from_x && from_y == rhs.from_y &&
               step_x == rhs.step_x && step_y == rhs.step_y &&
               remove_x == rhs.remove_x && remove_y == rhs.remove_y;
    }

    size_t hash() const override {
        using boost::hash_value;
        using boost::hash_combine;
        size_t seed = 0;
        hash_combine(seed, hash_value(from_x));
        hash_combine(seed, hash_value(from_y));
        hash_combine(seed, hash_value(step_x));
        hash_combine(seed, hash_value(step_y));
        hash_combine(seed, hash_value(remove_x));
        hash_combine(seed, hash_value(remove_y));
        return seed;
    }
};

typedef pair<int, int> cords;

struct Board {
    bitset<SIDE * SIDE> board = bitset<SIDE * SIDE>();

    Board() {}

    Board(const Board& other) {
        board = other.board;
    }

    void set(int x, int y, bool value) {
        board.set(y * SIDE + x, value);
    }

    bool get(int x, int y) const {
        return board.test(y * SIDE + x);
    }

    bool operator==(const Board &other) const {
        return board == other.board;
    }
};

size_t hash_value(const Board &board) {
    hash<bitset<SIDE * SIDE>> hash_fn;
    return hash_fn(board.board);
}

struct IsolaState : public State<IsolaState, IsolaMove> {

    Board board;
    cords player_1_cords = {-1, -1};
    cords player_2_cords = {-1, -1};

    IsolaState() : State(PLAYER_1) { }

    IsolaState(const string &init_string, char player_to_move = PLAYER_1) : State(player_to_move) {
        const unsigned long length = init_string.length();
        const unsigned long correct_length = SIDE * SIDE;
        if (length != correct_length) {
            throw invalid_argument("Initialization string length must be " + to_string(correct_length));
        }
        for (int i = 0; i < length; i++) {
            int x = i % SIDE;
            int y = i / SIDE;
            const char c = init_string[i];
            if (c == PLAYER_1) {
                player_1_cords = make_pair(x, y);
            } else if (c == PLAYER_2) {
                player_2_cords = make_pair(x, y);
            } else if (c == EMPTY) {
                board.set(x, y, 0);
            } else if (c == REMOVED) {
                board.set(x, y, 1);
            } else {
                throw invalid_argument(string("Undefined symbol used: '") + c + "'");
            }
        }
        if (player_1_cords.first == -1 || player_2_cords.first == -1) {
            throw invalid_argument("Missing player symbols");
        }
    }

    void swap_players() {
        auto temp = player_1_cords;
        player_1_cords = player_2_cords;
        player_2_cords = temp;
    }

    IsolaState clone() const override {
        IsolaState clone = IsolaState();
        clone.board = Board(board);
        clone.player_1_cords = player_1_cords;
        clone.player_2_cords = player_2_cords;
        clone.player_to_move = player_to_move;
        return clone;
    }

    int center_score(cords player_cords) const {
        int CENTER = SIDE / 2;
        return - abs(player_cords.first - CENTER) - abs(player_cords.second - CENTER);
    }

    int mobility_score(int moves) const {
        if (moves == 1) {
            return -100;
        }
        return moves;
    }

    int get_goodness() const override {
        cords player_cords = get_player_cords(player_to_move);
        cords enemy_cords = get_player_cords(get_enemy(player_to_move));

        int player_moves = count_moves_around(player_cords);
        if (player_moves == 0) {
            return -10000;
        }

        int enemy_moves = count_moves_around(enemy_cords);
        if (enemy_moves == 0) {
            return 10000;
        }

        int mobility = mobility_score(player_moves) - mobility_score(enemy_moves);
        int center = center_score(player_cords) - center_score(enemy_cords);
        int noise = random() % 2;

        return 10 * mobility + center + noise;
    }

    vector<IsolaMove> get_legal_moves(int how_many = INF) const override {
        auto player_cords = get_player_cords(player_to_move);
        auto step_moves = get_moves_around(player_cords.first, player_cords.second);
        assert(!step_moves.empty());

        int how_many_removes = ceil((double) how_many / step_moves.size());
        auto remove_moves = get_remove_moves(how_many_removes);

        int moves_count = step_moves.size() * remove_moves.size();
        if (how_many > moves_count) {
            how_many = moves_count;
        }
        vector<IsolaMove> moves(how_many);

        int size = 0;
        for (auto step_move : step_moves) {
            for (auto remove_move : remove_moves) {
                if (step_move == remove_move) {
                    continue;
                }
                moves[size++] = IsolaMove(
                    player_cords.first, player_cords.second,
                    step_move.first, step_move.second,
                    remove_move.first, remove_move.second
                );
                if (size >= how_many) {
                    return moves;
                }
            }
        }

        moves.resize(size);
        return moves;
    }

    vector<cords> get_remove_moves(int how_many) const {
        int moves_count = SIDE * SIDE;
        if (how_many > moves_count) {
            how_many = moves_count;
        }

        vector<cords> result(how_many);

        int size = 0;
        auto enemy = get_enemy(player_to_move);
        auto enemy_cords = get_player_cords(enemy);
        int dx_order = 1;
        if (enemy_cords.first < SIDE / 2) {
            dx_order = -1;
        }
        int dy_order = 1;
        if (enemy_cords.second < SIDE / 2) {
            dy_order = -1;
        }
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int x = enemy_cords.first + dx * dx_order;
                const int y = enemy_cords.second + dy * dy_order;
                if (x >= 0 && x < SIDE && y >= 0 && y < SIDE && is_empty(x, y)) {
                    result[size++] = make_pair(x, y);
                    if (size >= how_many - 1) {
                        result[size++] = get_player_cords(player_to_move);
                        return result;
                    }
                }
            }
        }

        result[size++] = get_player_cords(player_to_move);
        result.resize(size);
        return result;
    }

    char get_enemy(char player) const override {
        return (player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
    }

    bool is_terminal() const override {
        cords player_cords = get_player_cords(player_to_move);
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int x = player_cords.first + dx;
                const int y = player_cords.second + dy;
                if (x >= 0 && x < SIDE && y >= 0 && y < SIDE && is_empty(x, y)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool is_winner(char player) const override {
        return player == get_enemy(player_to_move) && is_terminal();
    }

    void make_move(const IsolaMove &move) override {
        board.set(move.remove_x, move.remove_y, 1);
        set_player_cords(player_to_move, make_pair(move.step_x, move.step_y));
        player_to_move = get_enemy(player_to_move);
    }

    void undo_move(const IsolaMove &move) override {
        player_to_move = get_enemy(player_to_move);
        set_player_cords(player_to_move, make_pair(move.from_x, move.from_y));
        board.set(move.remove_x, move.remove_y, 0);
    }

    vector<cords> get_moves_around(int start_x, int start_y) const {
        // Closer to the center first
        vector<cords> result(8);
        int dx_order = 1;
        if (start_x < SIDE / 2) {
            dx_order = -1;
        }
        int dy_order = 1;
        if (start_y < SIDE / 2) {
            dy_order = -1;
        }
        unsigned moves_count = 0;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int x = start_x + dx * dx_order;
                const int y = start_y + dy * dy_order;
                if (x >= 0 && x < SIDE && y >= 0 && y < SIDE && is_empty(x, y)) {
                    result[moves_count++] = make_pair(x, y);
                }
            }
        }
        result.resize(moves_count);
        return result;
    }

    int count_moves_around(const cords &player_cords) const {
        int result = 0;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int x = player_cords.first + dx;
                const int y = player_cords.second + dy;
                if (x >= 0 && x < SIDE && y >= 0 && y < SIDE && is_empty(x, y)) {
                    ++result;
                }
            }
        }
        return result;
    }

    bool is_empty(int x, int y) const {
        const cords c = make_pair(x, y);
        return board.get(x, y) == 0 && c != player_1_cords && c != player_2_cords;
    }

    cords get_player_cords(char player) const {
        return (player == PLAYER_1) ? player_1_cords : player_2_cords;
    }

    void set_player_cords(char player, const cords &player_cords) {
        (player == PLAYER_1) ? player_1_cords = player_cords : player_2_cords = player_cords;
    }

    ostream &to_stream(ostream &os) const override {
        for (int y = 0; y < SIDE; ++y) {
            for (int x = 0; x < SIDE; ++x) {
                const cords c = make_pair(x, y);
                if (c == player_1_cords) {
                    os << PLAYER_1;
                } else if (c == player_2_cords) {
                    os << PLAYER_2;
                } else if (board.get(x, y) == 0) {
                    os << EMPTY;
                } else if (board.get(x, y) == 1) {
                    os << REMOVED;
                }
            }
            os << endl;
        }
        os << player_to_move << endl;
        return os;
    }

    bool operator==(const IsolaState &other) const {
        return board == other.board
               && player_1_cords == other.player_1_cords
               && player_2_cords == other.player_2_cords
               && player_to_move == other.player_to_move;
    }

    size_t hash() const {
        using boost::hash_value;
        using boost::hash_combine;
        size_t seed = 0;
        hash_combine(seed, hash_value(board));
        hash_combine(seed, hash_value(player_1_cords));
        hash_combine(seed, hash_value(player_2_cords));
        hash_combine(seed, hash_value(player_to_move));
        return seed;
    }
};
