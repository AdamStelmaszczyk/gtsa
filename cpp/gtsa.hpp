#include <unordered_map>
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

using namespace std;

const static int MAX_SIMULATIONS = 10000;
static const double WIN_SCORE = 1;
static const double DRAW_SCORE = 0.9;
static const double LOSE_SCORE = 0;

static const int MAX_DEPTH = 20;
static const double EPSILON = 0.01;
static const int INF = 2147483647;

struct Random {
    mt19937 engine;

    int uniform(int min, int max) {
        return uniform_int_distribution<int>{min, max}(engine);
    }
};

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
    unsigned visits = 0;
    double score = 0;
    char player_to_move = 0;
    M move;
    S *parent = nullptr;

    State(char player_to_move) : player_to_move(player_to_move) {}

    void update_stats(double result) {
        score += result;
        ++visits;
    }

    double get_win_ratio() const {
        return score / (visits + EPSILON);
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
    unordered_map<size_t, Entry<M>> transposition_table;
    unordered_map<size_t, int> history_table;
    const double MAX_SECONDS;
    const bool VERBOSE;
    Timer timer;
    int beta_cuts;
    int tt_hits, tt_exacts, tt_cuts, tt_firsts;
    int nodes, leafs;

    Minimax(double max_seconds = 1, bool verbose = false) :
            Algorithm<S, M>(),
            transposition_table(unordered_map<size_t, Entry<M>>()),
            history_table(unordered_map<size_t, int>()),
            MAX_SECONDS(max_seconds),
            VERBOSE(verbose),
            timer(Timer()) { }

    M get_move(S *state) override {
        if (state->is_terminal()) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        timer.start();
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
                    << " time: " << timer
                    << " move: " << best_move
                    << " nodes: " << nodes
                    << " leafs: " << leafs
                    << " beta_cuts: " << beta_cuts
                    << " tt_hits: " << tt_hits
                    << " tt_exacts: " << tt_exacts
                    << " tt_cuts: " << tt_cuts
                    << " tt_firsts: " << tt_firsts
                    << " tt_size: " << transposition_table.size()
                    << " ht_size: " << history_table.size()
                    << " max_depth: " << max_depth << endl;
                }
            }
            if (timer.exceeded(MAX_SECONDS)) {
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
        bool entry_found = get_entry(state, entry);
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
            sort_by_history_heuristic(legal_moves);
            for (const auto& move : legal_moves) {
                state->make_move(move);
                const int goodness = -minimax(
                    state,
                    depth - 1,
                    -beta,
                    -alpha
                ).goodness;
                state->undo_move(move);
                if (timer.exceeded(MAX_SECONDS)) {
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
            update_tt(state, alpha_original, beta, best_goodness, best_move, depth);
            update_history(best_move, depth);
        }

        return {best_goodness, best_move, best_move_is_valid};
    }

    bool get_entry(S *state, Entry<M> &entry) {
        auto key = state->hash();
        auto it = transposition_table.find(key);
        if (it == transposition_table.end()) {
            return false;
        }
        entry = it->second;
        return true;
    }

    void add_entry(S *state, const Entry<M> &entry) {
        auto key = state->hash();
        transposition_table.insert({key, entry});
    }

    void update_tt(S *state, int alpha, int beta, int best_goodness, M &best_move, int depth) {
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

    void update_history(const M &move, int depth) {
        auto key = move.hash();
        history_table[key] += (1 << depth);
    }

    void sort_by_history_heuristic(vector<M> &moves) {
        stable_sort(moves.begin(), moves.end(), [&](const M &a, const M &b) {
            int a_score = history_table[a.hash()];
            int b_score = history_table[b.hash()];
            return a_score > b_score;
        });
    }

    string get_name() const {
        return "Minimax";
    }
};

template<class S, class M>
struct MonteCarloTreeSearch : public Algorithm<S, M> {
    unordered_map<size_t, S> tree_table;
    const double max_seconds;
    const int max_simulations;
    const bool verbose;
    Random random;

    MonteCarloTreeSearch(double max_seconds = 1, bool verbose = false, int max_simulations = MAX_SIMULATIONS) :
        Algorithm<S, M>(),
        tree_table(unordered_map<size_t, S>()),
        max_seconds(max_seconds),
        verbose(verbose),
        max_simulations(max_simulations) { }

    M get_move(S *root) override {
        if (root->is_terminal()) {
            stringstream stream;
            root->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        Timer timer;
        timer.start();
        remove_tree();
        int simulation = 0;
        while (simulation < max_simulations && !timer.exceeded(max_seconds)) {
            monte_carlo_tree_search(root);
            ++simulation;
        }
        if (verbose) {
            auto legal_moves = root->get_legal_moves();
            cout << "simulations: " << simulation << " moves: " << legal_moves.size()
            << " ratio: " << setprecision(1) << fixed << 100 * root->get_win_ratio() << "%" << endl;
            for (auto move : legal_moves) {
                auto child = get_child_from_hashmap(root, move);
                if (child != nullptr) {
                    cout << "move: " << move
                    << " trials: " << child->visits
                    << " ratio: " << setprecision(1) << fixed << 100 * child->get_win_ratio() << "%" << endl;
                }
            }
        }
        return select_best_move(root);
    }

    void remove_tree() {
        tree_table.clear();
    }

    void monte_carlo_tree_search(S *root) {
        S *current = tree_policy(root);
        auto result = rollout(current, root);
        propagate_up(current, result);
    }

    void propagate_up(S *current, double result) {
        current->update_stats(result);
        if (current->parent) {
            propagate_up(current->parent, result);
        }
    }

    S* tree_policy(S *root) {
        if (root->is_terminal()) {
            return root;
        }
        M move = select_tree_policy_move(root);
        auto child = get_child_from_hashmap(root, move);
        if (child == nullptr) {
            return add_child_to_hashmap(root, move);
        }
        return tree_policy(child);
    }

    M select_best_move(S *state) {
        auto legal_moves = state->get_legal_moves();
        assert(legal_moves.size() > 0);
        double best_ratio = -INF;
        M best_move;
        for (auto move : legal_moves) {
            auto child = get_child_from_hashmap(state, move);
            if (child != nullptr) {
                auto ratio = child->get_win_ratio();
                if (best_ratio < ratio) {
                    best_ratio = ratio;
                    best_move = move;
                }
            }
        }
        assert(best_ratio != -INF);
        return best_move;
    }

    M select_random_move(const S *state) {
        auto legal_moves = state->get_legal_moves();
        int index = random.uniform(0, legal_moves.size() - 1);
        return legal_moves[index];
    }

    M select_tree_policy_move(const S *state) {
        return select_random_move(state);
    }

    M select_default_policy_move(const S *state) {
        return select_random_move(state);
    }

    double rollout(S *current, S *root) {
        if (current->is_terminal()) {
            if (current->is_winner(root->player_to_move)) {
                return WIN_SCORE;
            }
            if (current->is_winner(root->get_enemy(root->player_to_move))) {
                return LOSE_SCORE;
            }
            return DRAW_SCORE;
        }

        // If player has a winning move he makes it.
        auto legal_moves = current->get_legal_moves();
        for (M &move : legal_moves) {
            current->make_move(move);
            if (current->is_winner(current->player_to_move)) {
                current->undo_move(move);
                return (current->player_to_move == root->player_to_move) ? WIN_SCORE : LOSE_SCORE;
            }
            current->undo_move(move);
        }

        // Otherwise follow default policy.
        M move = select_default_policy_move(current);
        current->make_move(move);
        auto result = rollout(current, root);
        current->undo_move(move);
        return result;
    }

    string get_name() const {
        return "MonteCarloTreeSearch";
    }

    S create_child(S *parent, M &move) {
        S child = parent->clone();
        child.make_move(move);
        child.parent = parent;
        return child;
    }

    S* add_child_to_hashmap(S *parent, M &move) {
        S child = create_child(parent, move);
        auto key = child.hash();
        auto pair = tree_table.insert({key, child});
        auto it = pair.first;
        return &it->second;
    }

    S* get_child_from_hashmap(S *parent, M &move) {
        auto child = create_child(parent, move);
        return get_state_from_hashmap(&child);
    }

    S* get_state_from_hashmap(S *state) {
        auto key = state->hash();
        auto it = tree_table.find(key);
        if (it == tree_table.end()) {
            return nullptr;
        }
        return &it->second;
    }
};

template<class S, class M>
struct Tester {
    S *root = nullptr;
    Algorithm<S, M> &algorithm_1;
    Algorithm<S, M> &algorithm_2;
    const int matches;
    const bool verbose;

    Tester(S *state, Algorithm<S, M> &algorithm_1, Algorithm<S, M> &algorithm_2, int matches = 1, bool verbose = true) :
            root(state), algorithm_1(algorithm_1), algorithm_2(algorithm_2), matches(matches), verbose(verbose) { }

    int start() {
        int draws = 0;
        int algorithm_1_wins = 0;
        int algorithm_2_wins = 0;
        char enemy = root->get_enemy(root->player_to_move);
        for (int i = 1; i <= matches; ++i) {
            if (verbose) {
                cout << *root << endl;
            }
            auto current = root->clone();
            while (!current.is_terminal()) {
                auto &algorithm = (current.player_to_move == root->player_to_move) ? algorithm_1 : algorithm_2;
                if (verbose) {
                    cout << current.player_to_move << " " << algorithm << endl;
                }
                Timer timer;
                timer.start();
                auto copy = current.clone();
                auto move = algorithm.get_move(&copy);
                if (verbose) {
                    cout << timer << endl;
                }
                current.make_move(move);
                if (verbose) {
                    cout << current << endl;
                }
            }
            cout << "Match " << i << "/" << matches << ": ";
            if (current.is_winner(root->player_to_move)) {
                ++algorithm_1_wins;
                cout << root->player_to_move << " " << algorithm_1 << endl;
            } else if (current.is_winner(enemy)) {
                ++algorithm_2_wins;
                cout << enemy << " " << algorithm_2 << endl;
            } else {
                ++draws;
                cout << "draw" << endl;
            }
        }
        cout << endl;
        cout << root->player_to_move << " " << algorithm_1 << " wins: " << algorithm_1_wins << endl;
        cout << enemy << " " << algorithm_2 << " wins: " << algorithm_2_wins << endl;
        cout << "Draws: " << draws << endl;
        return draws;
    }
};
