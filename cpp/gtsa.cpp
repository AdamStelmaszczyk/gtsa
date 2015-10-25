#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <memory>
#include <cmath>

using namespace std;

static const double EPSILON = 0.01;

struct Timer {
    const double start;

    Timer() : start(get_time()) { }

    double get_time() {
        timeval tv;
        gettimeofday(&tv, 0);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }

    double seconds_elapsed() {
        return get_time() - start;
    }
};

template<class M>
struct Move {
    virtual bool operator==(const M &rhs) const = 0;

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

    ~State() {
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
        return score / (visits + EPSILON) + M_SQRT2 * sqrt(log(parent->visits + 1) / (visits + EPSILON));
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

    char get_opposite_player(char player) const {
        return (player == our_symbol) ? enemy_symbol : our_symbol;
    }

    virtual M get_move(S *state) const = 0;
};

template<class M>
struct MoveReader {
    virtual M read() const = 0;
};

template<class S, class M>
struct Human : public Algorithm<S, M> {
    const MoveReader<M> &move_reader;

    Human(char our_symbol, char enemy_symbol, const MoveReader<M> &move_reader) :
            Algorithm<S, M>(our_symbol, enemy_symbol),
            move_reader(move_reader) { }

    M get_move(S *state) const override {
        auto legal_moves = state->get_legal_moves(this->our_symbol);
        if (legal_moves.empty()) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        while (true) {
            const M &move = move_reader.read();
            if (find(legal_moves.begin(), legal_moves.end(), move) != legal_moves.end()) {
                return move;
            } else {
                cout << "Move " << move << " is not legal" << endl;
            }
        }
    }
};

template<class S, class M>
struct Minimax : public Algorithm<S, M> {
    const double max_seconds;
    const int max_depth;
    const bool verbose;

    Minimax(char our_symbol,
            char enemy_symbol,
            double max_seconds = 10,
            int max_depth = 10,
            bool verbose = false) :
        Algorithm<S, M>(our_symbol, enemy_symbol),
        max_seconds(max_seconds),
        max_depth(max_depth),
        verbose(verbose) { }

    M get_move(S *state) const override {
        if (state->is_terminal(this->our_symbol)) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        return minimax(state, this->max_depth, (int) -INFINITY, (int) INFINITY, this->our_symbol).second;
    }

    pair<int, M> minimax(S *state, int depth, int alpha, int beta, char analyzed_player) const {
        if (depth <= 0 || state->is_terminal(analyzed_player)) {
            return make_pair(state->get_goodness(this->our_symbol), M()); // FIXME: M() is not elegant
        }
        int best_goodness;
        M best_move;
        const auto &legal_moves = state->get_legal_moves(analyzed_player);
        if (analyzed_player == this->our_symbol) {
            best_goodness = (int) -INFINITY;
            for (const auto& move : legal_moves) {
                state->make_move(move, analyzed_player);
                const int goodness = minimax(state, depth - 1, alpha, beta, this->enemy_symbol).first;
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
                const int goodness = minimax(state, depth - 1, alpha, beta, this->our_symbol).first;
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
};

template<class S, class M>
struct MonteCarloTreeSearch : public Algorithm<S, M> {
    const double max_seconds;
    const int max_simulations;
    const bool verbose;

    MonteCarloTreeSearch(char our_symbol,
                         char enemy_symbol,
                         double max_seconds = 10,
                         int max_simulations = 500,
                         bool verbose = false) :
        Algorithm<S, M>(our_symbol, enemy_symbol),
        max_seconds(max_seconds),
        max_simulations(max_simulations),
        verbose(verbose) { }

    M get_move(S *state) const override {
        if (state->is_terminal(this->our_symbol)) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        Timer timer;
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
};

template<class S, class M>
struct Tester {
    S *state = nullptr;
    const Algorithm<S, M> &algorithm_1;
    const char player_1;
    const Algorithm<S, M> &algorithm_2;
    const char player_2;

    Tester(S *s, const Algorithm<S, M> &algorithm_1, const Algorithm<S, M> &algorithm_2) :
            state(s),
            algorithm_1(algorithm_1),
            player_1(algorithm_1.our_symbol),
            algorithm_2(algorithm_2),
            player_2(algorithm_2.our_symbol) {
    }

    void start() {
        cout << *state << endl;
        while (true) {
            if (state->is_terminal(player_1)) {
                break;
            }
            auto move = algorithm_1.get_move(state);
            state->make_move(move, player_1);
            cout << *state << endl;

            if (state->is_terminal(player_2)) {
                break;
            }
            move = algorithm_2.get_move(state);
            state->make_move(move, player_2);
            cout << *state << endl;
        }
    }
};