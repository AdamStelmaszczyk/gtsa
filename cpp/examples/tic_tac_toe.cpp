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
    unsigned x;
    unsigned y;

    Coord(unsigned x, unsigned y): x(x), y(y) {}
};

const auto LINES = [] {
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

    const unsigned side;
    vector<char> board;

    TicTacToeState(unsigned side, const string& init_string = ""): side(side) {
        const unsigned correct_length = side * side;
        if (init_string != "") {
            const unsigned long length = init_string.length();
            if (length != correct_length) {
                throw invalid_argument("Initialization string length must be " + std::to_string(correct_length));
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
            board = vector<char>(init_string.begin(), init_string.end());
        }
    }

    TicTacToeState(const TicTacToeState& rhs): side(rhs.side) {
        board = rhs.board;
    }

    string to_string() {
        string result = "";
        for (int y = 0; y < side; y++) {
            for (int x = 0; x < side; x++) {
                result += board[y * side + x];
            }
            result += "\n";
        }
        return result;
    }

    int get_goodness(char current_player) {
        int goodness = 0;
        auto counts = count_players_on_lines(current_player);
        for (const auto& count : counts) {
            if (count[0] == 3) {
                goodness += side * side;
            }
            else if (count[1] == side) {
                goodness -= side * side;
            }
            else if (count[0] == SIDE - 1 and count[1] == 0) {
                goodness += side;
            }
            else if (count[1] == SIDE - 1 and count[0] == 0) {
                goodness -= side;
            }
            else if (count[0] == SIDE - 2 and count[1] == 0) {
                ++goodness;
            }
            else if (count[1] == SIDE - 2 and count[0] == 0) {
                --goodness;
            }
        }
        return goodness;
    }

    vector<vector<int>> count_players_on_lines(char current_player) {
        vector<vector<int>> counts;
        char next_player = get_opposite_player(current_player);
        for (const auto& line : LINES) {
            int player_places = 0;
            int enemy_places = 0;
            for (const Coord &coord : line) {
                const int i = coord.y * side + coord.x;
                if (board[i] == current_player) {
                    ++player_places;
                }
                else if (board[i] == next_player) {
                    ++enemy_places;
                }
            }
            auto count = {player_places, enemy_places};
            counts.emplace_back(count);
        }
        return counts;
    }

};

int main() {
    TicTacToeState state = TicTacToeState(3, "___"
                                             "_X_"
                                             "___");
    cout << state.get_goodness('X') << endl;
    return 0;
}