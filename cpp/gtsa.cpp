#include <vector>

using namespace std;

struct Move {};

template <class S, class M>
struct State {
    unsigned visits = 0;
    unsigned score = 0;
    char player_who_moved;
    Move move;
    State *parent;
    vector<S> children;

    virtual int get_goodness(char current_player) = 0;
    virtual vector<M> get_legal_moves(char player) = 0;
    virtual void make_move(M &move, char player) = 0;
    virtual void undo_move(M &move, char player) = 0;
    virtual bool is_terminal(char player) = 0;
    virtual bool is_winner(char player) = 0;
};

