#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

using namespace std;

static const double EPSILON = 0.01;

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

template<class S, class M>
struct State {
    unsigned visits = 0;
    double score = 0;
    char player_who_moved = 0;
    M move;
    S *parent = nullptr;
    S *children = nullptr;
    unsigned long children_size = 0;

    State() { }

    virtual ~State() {
        remove_children();
    }

    void expand(char player) {
        vector<M> legal_moves = get_legal_moves(player);
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

    S create_child(M move, char player_who_moved) {
        S child = clone();
        child.player_who_moved = player_who_moved;
        child.move = move;
        child.parent = (S *) this;
        child.make_move(move, player_who_moved);
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
        double max_ratio = -INFINITY;
        int max_visits = -INFINITY;
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
        double max_uct = -INFINITY;
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

    virtual S clone() const = 0;

    virtual int get_goodness(char current_player) const = 0;

    virtual vector<M> get_legal_moves(char player) const = 0;

    virtual bool is_terminal(char player) const = 0;

    virtual bool is_winner(char player) const = 0;

    virtual void make_move(const M &move, char player) = 0;

    virtual void undo_move(const M &move, char player) = 0;

    virtual ostream &to_stream(ostream &os) const = 0;

    friend ostream &operator<<(std::ostream &os, State const &state) {
        return state.to_stream(os);
    }
};

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
        const vector<M> &legal_moves = state->get_legal_moves(this->our_symbol);
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
    int max_depth;
    Timer *timer = new Timer();

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
        if (state->is_terminal(this->our_symbol)) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        timer->start();
        max_depth = 1;
        int best_goodness = (int) -INFINITY;
        M best_move;
        int best_at_depth = 1;
        while (timer->seconds_elapsed() < MAX_SECONDS && max_depth < 100) {
            auto pair = minimax(state, 0, (int) -INFINITY, (int) INFINITY, this->our_symbol);
            if (VERBOSE) {
                cout << "goodness: " << pair.first << " at max_depth: " << max_depth << endl;
            }
            if (best_goodness <= pair.first) {
                best_goodness = pair.first;
                best_move = pair.second;
                best_at_depth = max_depth;
            }
            max_depth += 2;
        }
        if (VERBOSE) {
            cout << "best_goodness: " << best_goodness << " at max_depth: " << best_at_depth << endl;
        }
        return best_move;
    }

    pair<int, M> minimax(S *state, int depth, int alpha, int beta, char analyzed_player) const {
        if (depth >= max_depth
            || state->is_terminal(analyzed_player)
            || timer->seconds_elapsed() > MAX_SECONDS) {
            return make_pair(state->get_goodness(this->our_symbol), M()); // FIXME: M() is not elegant
        }
        int best_goodness;
        M best_move;
        const auto &legal_moves = state->get_legal_moves(analyzed_player);
        if (analyzed_player == this->our_symbol) {
            best_goodness = (int) -INFINITY;
            for (const auto& move : legal_moves) {
                state->make_move(move, analyzed_player);
                const int goodness = minimax(state, depth + 1, alpha, beta, this->enemy_symbol).first;
                state->undo_move(move, analyzed_player);
                if (best_goodness < goodness) {
                    best_goodness = goodness;
                    best_move = move;
                }
                alpha = max(alpha, best_goodness);
                if (beta <= alpha) {
                    break;
                }
            }
        } else {
            best_goodness = (int) INFINITY;
            for (const auto& move : legal_moves) {
                state->make_move(move, analyzed_player);
                const int goodness = minimax(state, depth + 1, alpha, beta, this->our_symbol).first;
                state->undo_move(move, analyzed_player);
                if (best_goodness > goodness) {
                    best_goodness = goodness;
                    best_move = move;
                }
                beta = min(beta, best_goodness);
                if (beta <= alpha) {
                    break;
                }
            }
        }
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
                         int max_simulations = 10000) :
        Algorithm<S, M>(our_symbol, enemy_symbol),
        max_seconds(max_seconds),
        verbose(verbose),
        max_simulations(max_simulations) { }

    M get_move(S *state) override {
        if (state->is_terminal(this->our_symbol)) {
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
        while (current->has_children() && !current->is_terminal(analyzed_player)) {
            current = current->select_child_by_uct();
            analyzed_player = this->get_opposite_player(analyzed_player);
        }

        // 2. Expansion
        if (!current->is_terminal(analyzed_player)) {
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
        if (state->is_terminal(analyzed_player) || state->is_terminal(opponent)) {
            if (state->is_winner(analyzed_player)) {
                return (analyzed_player == this->our_symbol) ? 1 : 0;
            }
            if (state->is_winner(opponent)) {
                return (opponent == this->our_symbol) ? 1 : 0;
            }
            return 0.5;
        }

        // If player has a winning move he makes it.
        const auto &legal_moves = state->get_legal_moves(analyzed_player);
        for (const M &move : legal_moves) {
            state->make_move(move, analyzed_player);
            if (state->is_winner(analyzed_player)) {
                state->undo_move(move, analyzed_player);
                return (analyzed_player == this->our_symbol) ? 1 : 0;
            }
            state->undo_move(move, analyzed_player);
        }

        // Otherwise random move.
        M move = legal_moves[rand() % legal_moves.size()]; // not ideally uniform but should be fine
        state->make_move(move, analyzed_player);
        const double result = simulate(state, opponent);
        state->undo_move(move, analyzed_player);
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

    Tester(S *s, Algorithm<S, M> &algorithm_1, Algorithm<S, M> &algorithm_2) :
            state(s),
            algorithm_1(algorithm_1),
            player_1(algorithm_1.our_symbol),
            algorithm_2(algorithm_2),
            player_2(algorithm_2.our_symbol) {
    }

    void start() {
        cout << *state << endl;
        Timer timer;
        while (true) {
            if (state->is_terminal(player_1)) {
                break;
            }
            cout << algorithm_1.our_symbol << " " << algorithm_1.get_name() << endl;
            timer.start();
            auto move = algorithm_1.get_move(state);
            timer.print_seconds_elapsed();
            state->make_move(move, player_1);
            cout << *state << endl;

            if (state->is_terminal(player_2)) {
                break;
            }
            cout << algorithm_2.our_symbol << " " << algorithm_2.get_name() << endl;
            timer.start();
            move = algorithm_2.get_move(state);
            timer.print_seconds_elapsed();
            state->make_move(move, player_2);
            cout << *state << endl;
        }
    }
};