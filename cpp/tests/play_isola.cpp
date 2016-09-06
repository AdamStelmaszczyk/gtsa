#include "../examples/isola.cpp"

vector<cords> get_remove_moves(const IsolaState *state, int how_many) {
    int moves_count = SIDE * SIDE;
    if (how_many > moves_count) {
        how_many = moves_count;
    }

    vector<cords> result(how_many);

    int size = 0;
    auto enemy = state->get_enemy(state->player_to_move);
    auto enemy_cords = state->get_player_cords(enemy);
    int dx_order = 1;
    if (enemy_cords.first < SIDE / 2) {
        dx_order = -1;
    }
    int dy_order = 1;
    if (enemy_cords.second < SIDE / 2) {
        dy_order = -1;
    }
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            const int x = enemy_cords.first + dx * dx_order;
            const int y = enemy_cords.second + dy * dy_order;
            if (x >= 0 && x < SIDE && y >= 0 && y < SIDE && state->is_empty(x, y)) {
                result[size++] = make_pair(x, y);
                if (size >= how_many - 1) {
                    result[size++] = state->get_player_cords(state->player_to_move);
                    return result;
                }
            }
        }
    }

    result[size++] = state->get_player_cords(state->player_to_move);
    result.resize(size);
    return result;
}

vector<IsolaMove> get_legal_moves(const IsolaState *state, int how_many = INF) {
    auto player_cords = state->get_player_cords(state->player_to_move);
    auto step_moves = state->get_moves_around(player_cords.first, player_cords.second);
    assert(!step_moves.empty());

    int how_many_removes = ceil((double) how_many / step_moves.size());
    auto remove_moves = get_remove_moves(state, how_many_removes);

    int moves_count = step_moves.size() * remove_moves.size();
    if (how_many > moves_count) {
        how_many = moves_count;
    }
    vector<IsolaMove> moves(how_many);

    int size = 0;
    for (auto step_move : step_moves) {
        for (auto remove_move : remove_moves) {
            if (step_move == remove_move) {
                continue;
            }
            moves[size++] = IsolaMove(
                    player_cords.first, player_cords.second,
                    step_move.first, step_move.second,
                    remove_move.first, remove_move.second
            );
            if (size >= how_many) {
                return moves;
            }
        }
    }

    moves.resize(size);
    return moves;
}



int main() {
    IsolaState state = IsolaState("___2___"
                                          "_______"
                                          "_______"
                                          "_______"
                                          "_______"
                                          "_______"
                                          "___1___");

    Minimax<IsolaState, IsolaMove> a(60, 25);
    Human<IsolaState, IsolaMove> b;

//    Tester<IsolaState, IsolaMove> tester(&state, a, b, 1, true);
//    tester.start();

    a.get_move(&state);
    cout << a.read_log();

    return 0;
}