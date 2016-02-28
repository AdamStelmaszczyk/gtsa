#include <unordered_map>
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

using namespace std;

static const int MAX_DEPTH = 20;
static const double EPSILON = 0.01;
static const int INF = 2147483647;

struct Timer {
    double start_time;

    void start() {
        start_time = get_time();
    }

    double get_time() const {
        timeval tv;
        gettimeofday(&tv, 0);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }

    double seconds_elapsed() const {
        return get_time() - start_time;
    }

    bool exceeded(double seconds) const {
        return seconds_elapsed() > seconds;
    }

    friend ostream &operator<<(ostream &os, const Timer &timer) {
        return os << setprecision(2) << fixed << timer.seconds_elapsed() << "s";
    }
};

template<class M>
struct Move {
    virtual ~Move() {}

    virtual void read() = 0;

    virtual ostream &to_stream(ostream &os) const = 0;

    friend ostream &operator<<(ostream &os, const Move &move) {
        return move.to_stream(os);
    }

    virtual bool operator==(const M &rhs) const = 0;

    virtual size_t hash() const = 0;
};

enum EntryType { EXACT_VALUE, LOWER_BOUND, UPPER_BOUND };

template<class M>
struct Entry {
    M move;
    int depth;
    int value;
    EntryType value_type;

    Entry() {}

    Entry(const M &move, int depth, int value, EntryType value_type) :
            move(move), depth(depth), value(value), value_type(value_type) {}

    ostream &to_stream(ostream &os) {
        os << "move: " << move << " depth: " << depth << " value: " << value << " value_type: " << value_type;
    }

    friend ostream &operator<<(ostream &os, const Entry &entry) {
        return entry.to_stream(os);
    }
};

template<class S, class M>
struct State {
    static unordered_map<size_t, Entry<M>>* TRANSPOSITION_TABLE;
    static unordered_map<size_t, int>* HISTORY_TABLE;
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

    void expand() {
        vector<M> legal_moves = get_legal_moves();
        children_size = legal_moves.size();
        if (children_size == 0) {
            return;
        }
        children = new S[children_size];
        for (int i = 0; i < children_size; ++i) {
            S child = create_child(legal_moves[i]);
            if (child.is_winner(player_to_move)) {
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

    S create_child(const M &move) {
        S child = clone();
        child.player_to_move = player_to_move;
        child.move = move;
        child.parent = (S*) this;
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
        auto key = state->hash();
        auto it = TRANSPOSITION_TABLE->find(key);
        if (it == TRANSPOSITION_TABLE->end()) {
            return false;
        }
        entry = it->second;
        return true;
    }

    static void add_entry(S *state, const Entry<M> &entry) {
        auto key = state->hash();
        TRANSPOSITION_TABLE->operator[](key) = entry;
    }

    static void add_best_move(size_t key, M best_move) {
        Entry<M> entry = {best_move, MAX_DEPTH, INF, EntryType::EXACT_VALUE};
        TRANSPOSITION_TABLE->operator[](key) = entry;
    }

    static void update_tt(S *state, int alpha, int beta, int best_goodness, M &best_move, int depth) {
        EntryType value_type;
        if (best_goodness <= alpha) {
            value_type = EntryType::UPPER_BOUND;
        }
        else if (best_goodness >= beta) {
            value_type = EntryType::LOWER_BOUND;
        }
        else {
            value_type = EntryType::EXACT_VALUE;
        }
        Entry<M> entry = {best_move, depth, best_goodness, value_type};
        add_entry(state, entry);
    }

    static void update_history(const M &move, int depth) {
        auto key = move.hash();
        HISTORY_TABLE->operator[](key) += (1 << depth);
    }

    static void sort_by_history_heuristic(vector<M> &moves) {
        stable_sort(moves.begin(), moves.end(), [](const M &a, const M &b) {
            int a_score = HISTORY_TABLE->operator[](a.hash());
            int b_score = HISTORY_TABLE->operator[](b.hash());
            return a_score > b_score;
        });
    }

    virtual S clone() const = 0;

    virtual int get_goodness() const = 0;

    virtual vector<M> get_legal_moves() const = 0;

    virtual char get_enemy(char player) const = 0;

    virtual bool is_terminal() const = 0;

    virtual bool is_winner(char player) const = 0;

    virtual void make_move(const M &move) = 0;

    virtual void undo_move(const M &move) = 0;

    virtual ostream &to_stream(ostream &os) const = 0;

    friend ostream &operator<<(ostream &os, const State &state) {
        return state.to_stream(os);
    }

    virtual bool operator==(const S &other) const = 0;

    virtual size_t hash() const = 0;
};

template<class S, class M>
unordered_map<size_t, Entry<M>>* State<S, M>::TRANSPOSITION_TABLE = new unordered_map<size_t, Entry<M>>();

template<class S, class M>
unordered_map<size_t, int>* State<S, M>::HISTORY_TABLE = new unordered_map<size_t, int>();

template<class S, class M>
struct Algorithm {

    Algorithm() { }

    virtual ~Algorithm() { }

    virtual M get_move(S *state) = 0;

    virtual string get_name() const = 0;

    friend ostream &operator<<(ostream &os, const Algorithm &algorithm) {
        os << algorithm.get_name();
        return os;
    }
};

template<class S, class M>
struct Human : public Algorithm<S, M> {

    Human() : Algorithm<S, M>() { }

    M get_move(S *state) override {
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

template<class M>
struct Result {
    int goodness;
    M best_move;
    bool valid_move;
};

template<class S, class M>
struct Minimax : public Algorithm<S, M> {
    const double MAX_SECONDS;
    const bool VERBOSE;
    Timer *timer = new Timer();
    int beta_cuts;
    int tt_hits, tt_exacts, tt_cuts, tt_firsts;
    int nodes, leafs;

    Minimax(double max_seconds = 1, bool verbose = false) :
            Algorithm<S, M>(),
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
        M best_move;
        for (int max_depth = 1; max_depth <= MAX_DEPTH; ++max_depth) {
            beta_cuts = 0;
            tt_hits = 0;
            tt_exacts = 0;
            tt_cuts = 0;
            tt_firsts = 0;
            nodes = 0;
            leafs = 0;
            auto result = minimax(state, max_depth, -INF, INF);
            if (result.valid_move) {
                best_move = result.best_move;
                if (VERBOSE) {
                    cout << "goodness: " << result.goodness
                    << " time: " << *timer
                    << " move: " << best_move
                    << " nodes: " << nodes
                    << " leafs: " << leafs
                    << " beta_cuts: " << beta_cuts
                    << " tt_hits: " << tt_hits
                    << " tt_exacts: " << tt_exacts
                    << " tt_cuts: " << tt_cuts
                    << " tt_firsts: " << tt_firsts
                    << " tt_size: " << State<S, M>::TRANSPOSITION_TABLE->size()
                    << " ht_size: " << State<S, M>::HISTORY_TABLE->size()
                    << " max_depth: " << max_depth << endl;
                }
            }
            if (timer->exceeded(MAX_SECONDS)) {
                break;
            }
        }
        return best_move;
    }

    Result<M> minimax(S *state, int depth, int alpha, int beta) {
        ++nodes;
        const int alpha_original = alpha;

        M best_move;
        bool best_move_is_valid = false;
        if (depth == 0 || state->is_terminal()) {
            ++leafs;
            return {state->get_goodness(), best_move, best_move_is_valid};
        }

        Entry<M> entry;
        bool entry_found = State<S, M>::get_entry(state, entry);
        if (entry_found && entry.depth >= depth) {
            ++tt_hits;
            if (entry.value_type == EntryType::EXACT_VALUE) {
                ++tt_exacts;
                return {entry.value, entry.move, true};
            }
            if (entry.value_type == EntryType::LOWER_BOUND && alpha < entry.value) {
                alpha = entry.value;
            }
            if (entry.value_type == EntryType::UPPER_BOUND && beta > entry.value) {
                beta = entry.value;
            }
            if (alpha >= beta) {
                ++tt_cuts;
                return {entry.value, entry.move, true};
            }
        }

        bool generate_moves = true;
        int best_goodness = -INF;

        if (entry_found) {
            // If available, first try the best move from the table
            best_move = entry.move;
            best_move_is_valid = true;
            state->make_move(best_move);
            best_goodness = -minimax(
                state,
                depth - 1,
                -beta,
                -alpha
            ).goodness;
            state->undo_move(best_move);
            if (best_goodness >= beta) {
                ++tt_firsts;
                generate_moves = false;
            }
        }

        if (generate_moves) {
            auto legal_moves = state->get_legal_moves();
            State<S, M>::sort_by_history_heuristic(legal_moves);
            for (const auto& move : legal_moves) {
                state->make_move(move);
                const int goodness = -minimax(
                    state,
                    depth - 1,
                    -beta,
                    -alpha
                ).goodness;
                state->undo_move(move);
                if (timer->exceeded(MAX_SECONDS)) {
                    break;
                }
                if (best_goodness < goodness) {
                    best_goodness = goodness;
                    best_move = move;
                    best_move_is_valid = true;
                    if (best_goodness >= beta) {
                        ++beta_cuts;
                        break;
                    }
                }
                if (alpha < best_goodness) {
                    alpha = best_goodness;
                }
            }
        }

        if (best_move_is_valid) {
            State<S, M>::update_tt(state, alpha_original, beta, best_goodness, best_move, depth);
            State<S, M>::update_history(best_move, depth);
        }

        return {best_goodness, best_move, best_move_is_valid};
    }

    string get_name() const {
        return "Minimax";
    }
};

template<class S, class M>
struct MonteCarloTreeSearch : public Algorithm<S, M> {
    const double max_seconds;
    const int max_simulations;
    const bool verbose;

    MonteCarloTreeSearch(double max_seconds = 1, bool verbose = false, int max_simulations = 1000000) :
        Algorithm<S, M>(),
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
        while (simulation < max_simulations && !timer.exceeded(max_seconds)) {
            monte_carlo_tree_search(state);
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

    void monte_carlo_tree_search(S *root) const {
        // 1. Selection - find state without children (not expanded yet)
        S *current = root;
        while (current->has_children() && !current->is_terminal()) {
            current = current->select_child_by_uct();
        }

        // 2. Expansion
        if (!current->is_terminal()) {
            current->expand();
            current = current->select_child_by_uct();
        }

        // 3. Simulation
        const double result = simulate(current, root);

        // 4. Propagation
        while (current->parent) {
            current->update_stats(result);
            current = current->parent;
        }
        current->update_stats(result);
    }

    double simulate(S *state, S *root) const {
        char enemy = root->get_enemy(root->player_to_move);
        if (state->is_terminal()) {
            if (state->is_winner(root->player_to_move)) {
                return 1;
            }
            if (state->is_winner(root->get_enemy(root->player_to_move))) {
                return 0;
            }
            return 0.5;
        }

        // If player has a winning move he makes it.
        const auto &legal_moves = state->get_legal_moves();
        for (const M &move : legal_moves) {
            state->make_move(move);
            if (state->is_winner(state->player_to_move)) {
                state->undo_move(move);
                return 1;
            }
            state->undo_move(move);
        }

        // Otherwise random move.
        M move = legal_moves[rand() % legal_moves.size()]; // not ideally uniform but should be fine
        state->make_move(move);
        const double result = simulate(state, root);
        state->undo_move(move);
        return result;
    }

    string get_name() const {
        return "MonteCarloTreeSearch";
    }
};

template<class S, class M>
struct Tester {
    S *root = nullptr;
    Algorithm<S, M> &algorithm_1;
    Algorithm<S, M> &algorithm_2;
    const int matches;
    const bool verbose;
    int algorithm_1_wins;

    Tester(S *state, Algorithm<S, M> &algorithm_1, Algorithm<S, M> &algorithm_2, int matches = 1, bool verbose = true) :
            root(state), algorithm_1(algorithm_1), algorithm_2(algorithm_2), matches(matches), verbose(verbose) { }

    void start() {
        algorithm_1_wins = 0;
        for (int i = 1; i <= matches; ++i) {
            if (verbose) {
                cout << *root << endl;
            }
            auto current_state = root->clone();
            while (!current_state.is_terminal()) {
                auto &algorithm = (current_state.player_to_move == root->player_to_move) ? algorithm_1 : algorithm_2;
                if (verbose) {
                    cout << algorithm << endl;
                }
                Timer timer;
                timer.start();
                auto move = algorithm.get_move(&current_state);
                if (verbose) {
                    cout << timer << endl;
                }
                current_state.make_move(move);
                if (verbose) {
                    cout << current_state << endl;
                }
            }
            cout << "Match " << i << "/" << matches << ", winner: ";
            if (current_state.is_winner(root->player_to_move)) {
                ++algorithm_1_wins;
                cout << root->player_to_move << " " << algorithm_1 << endl;
            } else {
                cout << root->get_enemy(root->player_to_move) << " " << algorithm_2 << endl;
            }
        }
        cout << root->player_to_move << " " << algorithm_1 << " won " <<
                algorithm_1_wins << "/" << matches << " matches" << endl;
    }
};
