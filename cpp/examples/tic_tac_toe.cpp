#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include "../gtsa.cpp"

using namespace std;

const int SIDE = 3;
const char PLAYER_1 = 'X';
const char PLAYER_2 = 'O';
const char EMPTY = '_';

char get_opposite_player(char player) {
    return (player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
}

struct Coord
{
    int x;
    int y;

    Coord(int x, int y): x(x), y(y) {}
};

const auto lines = [] {
    vector<vector<Coord>> lines;
    for (int y = 0; y < SIDE; ++y) {
        lines.emplace_back(); // add a new line to back()
        for (int x = 0; x < SIDE; ++x)
            lines.back().emplace_back(x, y); // add a new coord to that line
    }
    for (int x = 0; x < SIDE; ++x) {
        lines.emplace_back();
        for (int y = 0; y < SIDE; ++y)
            lines.back().emplace_back(x, y);
    }
    lines.emplace_back();
    for (int i = 0; i < SIDE; ++i) {
        lines.back().emplace_back(i, i);
    }
    lines.emplace_back();
    for (int i = 0; i < SIDE; ++i) {
        lines.back().emplace_back(SIDE - i - 1, i);
    }
    return lines;
}();

struct TicTacToeState : public State {

    const int side;
    const char* board;

    TicTacToeState(int side, const string& init_string = ""): side
    (side) {
        board = new char[side * side];
        if (init_string != "") {
            const int length = init_string.length();
            const int correct_length = side * side;
            if (length != correct_length) {
                throw invalid_argument("Initialization string length must be "
                    + to_string(correct_length));
            }
            for (int i = 0; i < length; i++) {
                const char c = init_string[i];
                if (c != PLAYER_1 && c != PLAYER_2 && c != EMPTY) {
                    string message = "Undefined symbol used: '";
                    message += c;
                    message += "'";
                    throw invalid_argument(message);
                }
            }
            board = init_string.c_str();
        }
    }

};

int main() {
    TicTacToeState state = TicTacToeState(3, "___"
                                             "___"
                                             "___");
    cout << state.board << endl;
    return 0;
}