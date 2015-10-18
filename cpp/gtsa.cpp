#include <vector>
#include <memory>
#include <cmath>

using namespace std;

static const double EPSILON = 0.01;

struct Move {
};

template<class S, class M>
struct State {
    unsigned visits = 0;
    double score = 0;
    char player_who_moved;
    Move move;
    const State<S, M> *parent;
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
        child->move = move;
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

    virtual void make_move(M &move, char player) = 0;

    virtual void undo_move(M &move, char player) = 0;

    virtual bool is_terminal(char player) const = 0;

    virtual bool is_winner(char player) const = 0;
};

