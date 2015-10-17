#include <vector>

using namespace std;

struct Move {

};

struct State {
    unsigned visits = 0;
    unsigned score = 0;
    char player_who_moved;
    Move move;
    State* parent;
    vector<State> children;
};

