#include <unordered_map>
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

using namespace std;

static const int MAX_DEPTH = 100;
static const double EPSILON = 0.01;
static const int INF = 2147483647;

struct Timer {
    double start_time;

    void start() {
        start_time = get_time();
    }

    double get_time() {
        timeval tv;
        gettimeofday(&tv, 0);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }

    double seconds_elapsed() {
        return get_time() - start_time;
    }

    void print_seconds_elapsed() {
        cout << setprecision(1) << fixed << seconds_elapsed() << "s" << endl;
    }
};

template<class M>
struct Move {
    virtual ~Move() {}

    virtual bool operator==(const M &rhs) const = 0;

    virtual void read() = 0;

    virtual ostream &to_stream(ostream &os) const = 0;

    friend ostream &operator<<(std::ostream &os, Move const &move) {
        return move.to_stream(os);
    }
};

enum EntryType { EXACT_VALUE, LOWER_BOUND, UPPER_BOUND };

template<class M>
struct Entry {
    M move;
    int depth;
    int value;
    EntryType value_type;

    Entry() {}

    Entry(M move, int depth, int value, EntryType value_type) :
            move(move), depth(depth), value(value), value_type(value_type) {}

    ostream &to_stream(ostream &os) {
        os << "move: " << move << " depth: " << depth << " value: " << value << " value_type: " << value_type;
    }

    friend ostream &operator<<(std::ostream &os, Entry const &entry) {
        return entry.to_stream(os);
    }
};

template<class S, class M>
struct State {
    static unordered_map<S, Entry<M>, S>* TRANSPOSITION_TABLE;
    unsigned visits = 0;
    double score = 0;
    char player_to_move = 0;
    M move;
    S *parent = nullptr;
    S *children = nullptr;
    unsigned long children_size = 0;

    State(char player_to_move) : player_to_move(player_to_move) {}

    virtual ~State() {
        remove_children();
    }

    void expand(char player) {
        vector<M> legal_moves = get_legal_moves();
        children_size = legal_moves.size();
        if (children_size == 0) {
            return;
        }
        children = new S[children_size];
        for (int i = 0; i < children_size; ++i) {
            S child = create_child(legal_moves[i], player);
            if (child.is_winner(player)) {
                // If player has a winning move he makes it.
                delete[] children;
                children_size = 1;
                children = new S[children_size];
                children[0] = child;
                return;
            }
            children[i] = child;
        }
    }

    S create_child(M move, char player_to_move) {
        S child = clone();
        child.player_to_move = player_to_move;
        child.move = move;
        child.parent = (S *) this;
        child.make_move(move);
        return child;
    }

    void remove_children() {
        delete[] children;
        children_size = 0;
    }

    void update_stats(double result) {
        score += result;
        ++visits;
    }

    bool has_children() const {
        return children_size != 0;
    }

    S *select_child_by_ratio() const {
        if (children_size == 0) {
            return nullptr;
        }
        int best_index = -1;
        double max_ratio = -INF;
        int max_visits = -INF;
        for (int i = 0; i < children_size; ++i) {
            double child_ratio = get_win_ratio();
            if (max_ratio < child_ratio || (max_ratio == child_ratio && max_visits < children[i].visits)) {
                max_ratio = child_ratio;
                max_visits = children[i].visits;
                best_index = i;
            }
        }
        return &children[best_index];
    }

    S *select_child_by_uct() const {
        if (children_size == 0) {
            return nullptr;
        }
        int best_index = -1;
        double max_uct = -INF;
        for (int i = 0; i < children_size; ++i) {
            double child_uct = children[i].get_uct_value();
            if (max_uct < child_uct) {
                max_uct = child_uct;
                best_index = i;
            }
        }
        return &children[best_index];
    }

    double get_uct_value() const {
        return score / (visits + EPSILON) + M_SQRT2 * sqrt(log(parent->visits + 1) / (visits + EPSILON))
               + ((double) rand() / RAND_MAX) * EPSILON;
    }

    double get_win_ratio() const {
        return score / (visits + EPSILON);
    }

    static bool get_entry(S *state, Entry<M> &entry) {
        auto it = TRANSPOSITION_TABLE->find(*state);
        if (it == TRANSPOSITION_TABLE->end()) {
            return false;
        }
        entry = it->second;
        return true;
    }

    static void add_entry(S *state, Entry<M> &entry) {
        TRANSPOSITION_TABLE->operator[](*state) = entry;
    }

    virtual S clone() const = 0;

    virtual int get_goodness() const = 0;

    virtual vector<M> get_legal_moves() const = 0;

    virtual bool is_terminal() const = 0;

    virtual bool is_winner(char player) const = 0;

    virtual void make_move(const M &move) = 0;

    virtual void undo_move(const M &move) = 0;

    virtual ostream &to_stream(ostream &os) const = 0;

    friend ostream &operator<<(std::ostream &os, State const &state) {
        return state.to_stream(os);
    }

    virtual bool operator==(const S &other) const = 0;

    virtual size_t operator()(const S &key) const = 0;
};

template<class S, class M>
unordered_map<S, Entry<M>, S>* State<S, M>::TRANSPOSITION_TABLE = new unordered_map<S, Entry<M>, S>();

template<class S, class M>
struct Algorithm {
    const char our_symbol;
    const char enemy_symbol;

    Algorithm(char our_symbol, char enemy_symbol) : our_symbol(our_symbol), enemy_symbol(enemy_symbol) { }

    virtual ~Algorithm() {}

    char get_opposite_player(char player) const {
        return (player == our_symbol) ? enemy_symbol : our_symbol;
    }

    virtual M get_move(S *state) = 0;

    virtual string get_name() const = 0;
};

template<class S, class M>
struct Human : public Algorithm<S, M> {

    Human(char our_symbol, char enemy_symbol) :
        Algorithm<S, M>(our_symbol, enemy_symbol) { }

    M get_move(S *state) const override {
        const vector<M> &legal_moves = state->get_legal_moves();
        if (legal_moves.empty()) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        while (true) {
            M move = M();
            move.read();
            if (find(legal_moves.begin(), legal_moves.end(), move) != legal_moves.end()) {
                return move;
            } else {
                cout << "Move " << move << " is not legal" << endl;
            }
        }
    }

    string get_name() const {
        return "Human";
    }
};

template<class S, class M>
struct Minimax : public Algorithm<S, M> {
    const double MAX_SECONDS;
    const bool VERBOSE;
    Timer *timer = new Timer();
    int tt_hits;

    Minimax(char our_symbol,
            char enemy_symbol,
            double max_seconds = 1,
            bool verbose = false) :
        Algorithm<S, M>(our_symbol, enemy_symbol),
        MAX_SECONDS(max_seconds),
        VERBOSE(verbose) { }

    ~Minimax() {
        delete timer;
    }

    M get_move(S *state) override {
        if (state->is_terminal()) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        timer->start();
        int best_goodness = (int) -INF;
        M best_move;
        int best_at_depth = 1;
        int max_depth = 1;
        while (timer->seconds_elapsed() < MAX_SECONDS && max_depth < MAX_DEPTH) {
            tt_hits = 0;
            auto pair = minimax(state, max_depth, (int) -INF, (int) INF, this->our_symbol);
            if (VERBOSE) {
                cout << "goodness: " << pair.first
                     << " tt_hits: " << tt_hits
                     << " tt_size: " << State<S, M>::TRANSPOSITION_TABLE->size()
                     << " at max_depth: " << max_depth << endl;
            }
            if (best_goodness <= pair.first) {
                best_goodness = pair.first;
                best_move = pair.second;
                best_at_depth = max_depth;
            }
            ++max_depth;
        }
        if (VERBOSE) {
            cout << "best_goodness: " << best_goodness << " at max_depth: " << best_at_depth << endl;
        }
        return best_move;
    }

    pair<int, M> minimax(S *state, int depth, int alpha, int beta, char analyzed_player) {
        Entry<M> entry;
        bool found = State<S, M>::get_entry(state, entry);
        if (found && entry.depth >= depth) {
            ++tt_hits;
            if (entry.value_type == EntryType::EXACT_VALUE) {
                return make_pair(entry.value, entry.move);
            }
            if (entry.value_type == EntryType::LOWER_BOUND && entry.value > alpha) {
                alpha = entry.value;
            }
            if (entry.value_type == EntryType::UPPER_BOUND && entry.value < beta) {
                beta = entry.value;
            }
            if (alpha >= beta) {
                return make_pair(entry.value, entry.move);
            }
        }
        M best_move;
        if (depth == 0 || state->is_terminal() || timer->seconds_elapsed() > MAX_SECONDS) {
            return make_pair(state->get_goodness(), best_move);
        }
        int best_goodness = (int) -INF;
        const auto &legal_moves = state->get_legal_moves();
        for (const auto& move : legal_moves) {
            state->make_move(move);
            const int goodness = -minimax(
                    state,
                    depth - 1,
                    -beta,
                    -alpha,
                    this->get_opposite_player(analyzed_player)
            ).first;
            state->undo_move(move);
            if (best_goodness < goodness) {
                best_goodness = goodness;
                best_move = move;
            }
            alpha = max(alpha, best_goodness);
            if (best_goodness >= beta) {
                break;
            }
        }
        EntryType value_type;
        if (best_goodness <= alpha) {
            value_type = EntryType::LOWER_BOUND;
        }
        else if (best_goodness >= beta) {
            value_type = EntryType::UPPER_BOUND;
        }
        else {
            value_type = EntryType::EXACT_VALUE;
        }
        entry = {best_move, depth, best_goodness, value_type};
        State<S, M>::add_entry(state, entry);
        return make_pair(best_goodness, best_move);
    };

    string get_name() const {
        return "Minimax";
    }
};

template<class S, class M>
struct MonteCarloTreeSearch : public Algorithm<S, M> {
    const double max_seconds;
    const int max_simulations;
    const bool verbose;

    MonteCarloTreeSearch(char our_symbol,
                         char enemy_symbol,
                         double max_seconds = 1,
                         bool verbose = false,
                         int max_simulations = 1000000) :
        Algorithm<S, M>(our_symbol, enemy_symbol),
        max_seconds(max_seconds),
        verbose(verbose),
        max_simulations(max_simulations) { }

    M get_move(S *state) override {
        if (state->is_terminal()) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        Timer timer;
        timer.start();
        state->remove_children();
        int simulation = 0;
        while (simulation < max_simulations && timer.seconds_elapsed() < max_seconds) {
            monte_carlo_tree_search(state, this->our_symbol);
            ++simulation;
        }
        if (verbose) {
            cout << "simulations: " << simulation << " moves: " << state->children_size << endl;
            for (int i = 0; i < state->children_size; ++i) {
                const S *child = &state->children[i];
                cout << "move: " << child->move
                << " trials: " << child->visits
                << " ratio: " << setprecision(1) << fixed << 100 * child->get_win_ratio() << "%" << endl;
            }
        }
        return state->select_child_by_ratio()->move;
    }

    void monte_carlo_tree_search(S *state, char analyzed_player) const {
        // 1. Selection - find state without children (not expanded yet)
        S *current = state;
        while (current->has_children() && !current->is_terminal()) {
            current = current->select_child_by_uct();
            analyzed_player = this->get_opposite_player(analyzed_player);
        }

        // 2. Expansion
        if (!current->is_terminal()) {
            current->expand(analyzed_player);
            current = current->select_child_by_uct();
            analyzed_player = this->get_opposite_player(analyzed_player);
        }

        // 3. Simulation
        const double result = simulate(current, analyzed_player);

        // 4. Propagation
        while (current->parent) {
            current->update_stats(result);
            current = current->parent;
        }
        current->update_stats(result);
    }

    double simulate(S *state, char analyzed_player) const {
        const char opponent = this->get_opposite_player((analyzed_player));
        if (state->is_terminal()) {
            if (state->is_winner(analyzed_player)) {
                return (analyzed_player == this->our_symbol) ? 1 : 0;
            }
            if (state->is_winner(opponent)) {
                return (opponent == this->our_symbol) ? 1 : 0;
            }
            return 0.5;
        }

        // If player has a winning move he makes it.
        const auto &legal_moves = state->get_legal_moves();
        for (const M &move : legal_moves) {
            state->make_move(move);
            if (state->is_winner(analyzed_player)) {
                state->undo_move(move);
                return (analyzed_player == this->our_symbol) ? 1 : 0;
            }
            state->undo_move(move);
        }

        // Otherwise random move.
        M move = legal_moves[rand() % legal_moves.size()]; // not ideally uniform but should be fine
        state->make_move(move);
        const double result = simulate(state, opponent);
        state->undo_move(move);
        return result;
    }

    string get_name() const {
        return "MonteCarloTreeSearch";
    }
};

template<class S, class M>
struct Tester {
    S *state = nullptr;
    Algorithm<S, M> &algorithm_1;
    const char player_1;
    Algorithm<S, M> &algorithm_2;
    const char player_2;
    const int matches;
    const bool verbose;
    int algorithm_1_wins;

    Tester(S *s, Algorithm<S, M> &algorithm_1, Algorithm<S, M> &algorithm_2, int matches = 1, bool verbose = true) :
            state(s),
            algorithm_1(algorithm_1),
            player_1(algorithm_1.our_symbol),
            algorithm_2(algorithm_2),
            player_2(algorithm_2.our_symbol),
            matches(matches),
            verbose(verbose) { }

    bool handle_player(S &state, char player, Algorithm<S, M> &algorithm) {
        if (state.is_terminal()) {
            if (state.is_winner(player_1)) {
                ++algorithm_1_wins;
            }
            return true;
        }
        if (verbose) {
            cout << algorithm.our_symbol << " " << algorithm.get_name() << endl;
        }
        Timer timer;
        timer.start();
        auto move = algorithm.get_move(&state);
        if (verbose) {
            timer.print_seconds_elapsed();
        }
        state.make_move(move);
        if (verbose) {
            cout << state << endl;
        }
        return false;
    }

    void start() {
        algorithm_1_wins = 0;
        for (int i = 1; i <= matches; ++i) {
            cout << "Match " << i << "/" << matches << endl;
            auto current_state = state->clone();
            if (verbose) {
                cout << *state << endl;
            }
            Timer timer;
            while (true) {
                bool end = handle_player(current_state, player_1, algorithm_1);
                if (end) {
                    break;
                }
                end = handle_player(current_state, player_2, algorithm_2);
                if (end) {
                    break;
                }
            }
        }
        cout << player_1 << " " << algorithm_1.get_name() << " won " <<
                algorithm_1_wins << "/" << matches << " matches" << endl;
    }
};