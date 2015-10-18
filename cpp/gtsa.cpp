#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

static const double EPSILON = 0.01;

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
    char player_who_moved;
    unique_ptr<Move<M>> move;
    const S *parent;
    vector<S> children;

    void expand(char player) {
        vector<S> current_children = vector<S>();
        for (const auto &move : get_legal_moves(player)) {
            unique_ptr<S> child = create_child(move, player);
            if (child->is_winner(player)) {
                // If player has a winning move he makes it.
                children.emplace_back(*child);
                return;
            }
            current_children.emplace_back(*child);
        }
        children.swap(current_children);
    }

    unique_ptr<S> create_child(M move, char player_who_moved) const {
        unique_ptr<S> child = clone();
        child->visits = 0;
        child->score = 0;
        child->player_who_moved = player_who_moved;
        child->move = unique_ptr<Move<M>>(move);
        child->parent = this;
        child->children.clear();
        child->make_move(move, player_who_moved);
        return child;
    }

    void remove_children() {
        children.clear();
    }

    void update_stats(double result) {
        score += result;
        ++visits;
    }

    bool is_leaf() const {
        return children.empty();
    }

    const S *select_child_by_ratio() const {
        const S *best_child = nullptr;
        double max_ratio = -INFINITY;
        int max_visits = -INFINITY;
        for (const auto &child : children) {
            double child_ratio = get_win_ratio();
            if (max_ratio < child_ratio || (max_ratio == child_ratio && max_visits < child.visits)) {
                max_ratio = child_ratio;
                max_visits = child.visits;
                best_child = &child;
            }
        }
        return best_child;
    }

    const S *select_child_by_uct() const {
        const S *best_child = nullptr;
        double max_uct = -INFINITY;
        for (const auto &child : children) {
            double child_uct = child.get_uct_value();
            if (max_uct < child_uct) {
                max_uct = child_uct;
                best_child = &child;
            }
        }
        return best_child;
    }

    double get_uct_value() const {
        return score / (visits + EPSILON) + M_SQRT2 * sqrt(log(parent->visits + 1) / (visits + EPSILON))
               + ((double) rand() / RAND_MAX) * EPSILON;
    }

    double get_win_ratio() const {
        return score / (visits + EPSILON);
    }

    virtual unique_ptr<S> clone() const = 0;

    virtual int get_goodness(char current_player) const = 0;

    virtual vector<M> get_legal_moves(char player) const = 0;

    virtual bool is_terminal(char player) const = 0;

    virtual bool is_winner(char player) const = 0;

    virtual void make_move(M &move, char player) = 0;

    virtual void undo_move(M &move, char player) = 0;

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

    virtual M get_move(const S &state) const = 0;
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

    M get_move(const S &state) const override {
        auto legal_moves = state.get_legal_moves(this->our_symbol);
        if (legal_moves.empty()) {
            throw invalid_argument("Given state is terminal");
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
struct Tester {
    S &state;
    const Algorithm<S, M> &algorithm_1;
    const char player_1;
    const Algorithm<S, M> &algorithm_2;
    const char player_2;

    Tester(S &state, const Algorithm<S, M> &algorithm_1, const Algorithm<S, M> &algorithm_2) :
            state(state),
            algorithm_1(algorithm_1),
            player_1(algorithm_1.our_symbol),
            algorithm_2(algorithm_2),
            player_2(algorithm_2.our_symbol) { }

    void start() {
        cout << state << endl;
        while (true) {
            if (state.is_terminal(player_1)) {
                break;
            }
            auto move = algorithm_1.get_move(state);
            state.make_move(move, player_1);
            cout << state << endl;

            if (state.is_terminal(player_2)) {
                break;
            }
            move = algorithm_2.get_move(state);
            state.make_move(move, player_2);
            cout << state << endl;
        }
    }
};