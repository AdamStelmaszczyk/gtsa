#pragma once

/*
Game Tree Search Algorithms
Copyright (C) Adam Stelmaszczyk <stelmaszczyk.adam@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <boost/math/distributions/binomial.hpp>
#include <unordered_map>
#include <unordered_set>
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <random>
#include <vector>

using namespace std;

static const int MAX_SIMULATIONS = 10000000;
static const double UCT_C = sqrt(2);
static const double WIN_SCORE = 1;
static const double DRAW_SCORE = 0.5;
static const double LOSE_SCORE = 0;

static const int MAX_DEPTH = 20;
static const int INF = 2147483647;

struct Random {

    virtual ~Random() {}

    int uniform(int min, int max) const {
        mt19937 engine;
        uniform_int_distribution<int> distribution(min, max);
        return distribution(engine);
    }
};

struct Timer {
    double start_time;

    virtual ~Timer() {}

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

    virtual void read(istream &stream) = 0;

    virtual ostream &to_stream(ostream &os) const = 0;

    friend ostream &operator<<(ostream &os, const Move &move) {
        return move.to_stream(os);
    }

    virtual bool operator==(const M &rhs) const = 0;

    virtual size_t hash() const = 0;
};

enum TTEntryType { EXACT_VALUE, LOWER_BOUND, UPPER_BOUND };

template<class M>
struct TTEntry {
    M move;
    int depth;
    int value;
    TTEntryType value_type;

    TTEntry() {}

    virtual ~TTEntry() {}

    TTEntry(const M &move, int depth, int value, TTEntryType value_type) :
            move(move), depth(depth), value(value), value_type(value_type) {}

    ostream &to_stream(ostream &os) {
        return os << "move: " << move << " depth: " << depth << " value: " << value << " value_type: " << value_type;
    }

    friend ostream &operator<<(ostream &os, const TTEntry &entry) {
        return entry.to_stream(os);
    }
};

template<class S, class M>
struct State {
    unsigned visits = 0;
    double score = 0;
    char player_to_move = 0;
    S *parent = nullptr;
    unordered_map<size_t, shared_ptr<S>> children = unordered_map<size_t, shared_ptr<S>>();

    State(char player_to_move) : player_to_move(player_to_move) {}

    virtual ~State() {}

    void update_stats(double result) {
        score += result;
        ++visits;
    }

    double get_uct(double c) const {
        assert(visits > 0);
        double parent_visits = 0.0;
        if (parent != nullptr) {
            parent_visits = parent->visits;
        }
        return score / visits + c * sqrt(log(parent_visits) / visits);
    }

    shared_ptr<S> create_child(const M &move) {
        S child = clone();
        child.make_move(move);
        child.parent = (S*) this;
        return make_shared<S>(child);
    }

    S* add_child(const M &move) {
        const auto child = create_child(move);
        const auto key = move.hash();
        const auto pair = children.insert({key, child});
        const auto it = pair.first;
        return it->second.get();
    }

    S* get_child(const M &move) const {
        const auto key = move.hash();
        const auto it = children.find(key);
        if (it == children.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    virtual string to_executable_format() const {
        stringstream ss;
        ss << *this;
        return ss.str();
    }

    virtual void swap_players() {}

    virtual S clone() const = 0;

    virtual int get_goodness() const = 0;

    virtual vector<M> get_legal_moves(int max_moves) const = 0;

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

    stringstream log;

    Algorithm() { }

    Algorithm(const Algorithm& algorithm) {}

    virtual ~Algorithm() {}

    string read_log() {
        string result = log.str();
        log.str("");
        return result;
    }

    virtual void reset() {}

    virtual M get_move(const S *state) = 0;

    virtual string get_name() const = 0;

    friend ostream &operator<<(ostream &os, const Algorithm &algorithm) {
        os << algorithm.get_name();
        return os;
    }
};

template<class S, class M>
struct Human : public Algorithm<S, M> {

    Human() : Algorithm<S, M>() {}

    M get_move(const S *state) override {
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

template<class S, class M>
struct Executable : public Algorithm<S, M> {

    const string executable;

    Executable(string executable) : Algorithm<S, M>(), executable(executable) {}

    M get_move(const S *state) override {
        const vector<M> &legal_moves = state->get_legal_moves();
        if (legal_moves.empty()) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }

        stringstream cmd;
        cmd << "echo \"" << state->to_executable_format() << "\" | " << executable;

        const string output = run_cmd(cmd.str());
        stringstream stream_to_read(output);

        M move = M();
        move.read(stream_to_read);
        if (find(legal_moves.begin(), legal_moves.end(), move) != legal_moves.end()) {
            return move;
        } else {
            stringstream message;
            message << "Legal moves: ";
            for (auto move : legal_moves) {
                message << move << ", ";
            }
            message << endl;
            message << "Move " << move << " is not legal, state:" << endl;
            message << *state;
            throw runtime_error(message.str());
        }
    }

    string run_cmd(string cmd) {
        stringstream result;
        const int BUFFER_SIZE = 128;
        char buffer[BUFFER_SIZE];
        shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            throw runtime_error("popen() failed");
        }
        while (!feof(pipe.get())) {
            if (fgets(buffer, BUFFER_SIZE, pipe.get()) != NULL) {
                result << buffer;
            }
        }
        return result.str();
    }

    string get_name() const {
        return "Executable " + executable;
    }
};

template<class M>
struct MinimaxResult {
    int goodness;
    M best_move;
    bool completed;
};

template<class S, class M>
struct Minimax : public Algorithm<S, M> {
    unordered_map<size_t, TTEntry<M>> transposition_table;
    const double MAX_SECONDS;
    const int MAX_MOVES;
    function<vector<M>(const S*, int)> get_legal_moves;
    function<int(const S*)> get_goodness;
    Timer timer;
    int scout_cuts;
    int beta_cuts, cut_bf_sum;
    int tt_hits, tt_exacts, tt_cuts;
    int nodes, leafs;

    Minimax(double max_seconds = 1, int max_moves = INF, function<vector<M>(const S*, int)> get_legal_moves = nullptr, function<int(const S*)> get_goodness = nullptr) :
            Algorithm<S, M>(),
            transposition_table(unordered_map<size_t, TTEntry<M>>(1000000)),
            MAX_SECONDS(max_seconds),
            MAX_MOVES(max_moves),
            get_legal_moves(get_legal_moves),
            get_goodness(get_goodness),
            timer(Timer()) {}

    void reset() {
        transposition_table.clear();
    }

    M get_move(const S *state) override {
        if (state->is_terminal()) {
            stringstream stream;
            state->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        if (get_legal_moves == nullptr) {
            get_legal_moves = &State<S,M>::get_legal_moves;
        }
        if (get_goodness == nullptr) {
            get_goodness = &State<S,M>::get_goodness;
        }
        timer.start();

        const auto moves = get_legal_moves(state, MAX_MOVES);
        this->log << "moves: " << moves.size() << endl;
        for (const auto move : moves) {
            this->log << move << ", ";
        }
        this->log << endl;

        M best_move;
        for (int max_depth = 1; max_depth <= MAX_DEPTH; ++max_depth) {
            scout_cuts = 0;
            beta_cuts = 0;
            cut_bf_sum = 0;
            tt_hits = 0;
            tt_exacts = 0;
            tt_cuts = 0;
            nodes = 0;
            leafs = 0;
            S clone = state->clone();
            auto result = minimax(&clone, max_depth, -INF, INF);
            if (result.completed) {
                best_move = result.best_move;
                this->log << "goodness: " << result.goodness
                << " time: " << timer
                << " move: " << best_move
                << " nodes: " << nodes
                << " leafs: " << leafs
                << " scout_cuts: " << scout_cuts
                << " beta_cuts: " << beta_cuts
                << " cutBF: " << (double) cut_bf_sum / beta_cuts
                << " tt_hits: " << tt_hits
                << " tt_exacts: " << tt_exacts
                << " tt_cuts: " << tt_cuts
                << " tt_size: " << transposition_table.size()
                << " max_depth: " << max_depth << endl;
            }
            if (timer.exceeded(MAX_SECONDS)) {
                break;
            }
        }
        return best_move;
    }

    // Find Minimax value of the given tree,
    // Minimax value lies within a range of [alpha; beta] window.
    // Whenever alpha >= beta, further checks of children in a node can be pruned.
    MinimaxResult<M> minimax(S *state, int depth, int alpha, int beta) {
        ++nodes;
        const int alpha_original = alpha;

        M best_move;
        if (depth == 0 || state->is_terminal()) {
            ++leafs;
            return {get_goodness(state), best_move, false};
        }

        TTEntry<M> entry;
        const bool entry_found = get_tt_entry(state, entry);
        if (entry_found && entry.depth >= depth) {
            ++tt_hits;
            if (entry.value_type == TTEntryType::EXACT_VALUE) {
                ++tt_exacts;
                return {entry.value, entry.move, true};
            }
            if (entry.value_type == TTEntryType::LOWER_BOUND && alpha < entry.value) {
                alpha = entry.value;
            }
            if (entry.value_type == TTEntryType::UPPER_BOUND && beta > entry.value) {
                beta = entry.value;
            }
            if (alpha >= beta) {
                ++tt_cuts;
                return {entry.value, entry.move, true};
            }
        }

        int max_goodness = -INF;

        bool completed = true;
        const auto legal_moves = get_legal_moves(state, MAX_MOVES);
        assert(legal_moves.size() > 0);
        for (int i = 0; i < legal_moves.size(); i++) {
            const auto move = legal_moves[i];
            state->make_move(move);
            int goodness;
            if (i > 0) {
                // null window search
                goodness = -minimax(
                    state,
                    depth - 1,
                    -alpha - 1,
                    -alpha
                ).goodness;
                if (alpha < goodness && goodness < beta) {
                    // failed high, do a full re-search
                    goodness = -minimax(
                        state,
                        depth - 1,
                        -beta,
                        -goodness
                    ).goodness;
                } else {
                    scout_cuts++;
                }
            }
            else {
                goodness = -minimax(
                    state,
                    depth - 1,
                    -beta,
                    -alpha
                ).goodness;
            }
            state->undo_move(move);
            if (timer.exceeded(MAX_SECONDS)) {
                completed = false;
                break;
            }
            if (max_goodness < goodness) {
                max_goodness = goodness;
                best_move = move;
                if (max_goodness >= beta) {
                    ++beta_cuts;
                    cut_bf_sum += i + 1;
                    break;
                }
            }
            if (alpha < max_goodness) {
                alpha = max_goodness;
            }
        }

        if (completed) {
            update_tt(state, alpha_original, beta, max_goodness, best_move, depth);
        }

        return {max_goodness, best_move, completed};
    }

    bool get_tt_entry(const S *state, TTEntry<M> &entry) const {
        const auto key = state->hash();
        const auto it = transposition_table.find(key);
        if (it == transposition_table.end()) {
            return false;
        }
        entry = it->second;
        return true;
    }

    void add_tt_entry(const S *state, const TTEntry<M> &entry) {
        const auto key = state->hash();
        transposition_table.insert({key, entry});
    }

    void update_tt(const S *state, int alpha, int beta, int max_goodness, const M &best_move, int depth) {
        TTEntryType value_type;
        if (max_goodness <= alpha) {
            value_type = TTEntryType::UPPER_BOUND;
        }
        else if (max_goodness >= beta) {
            value_type = TTEntryType::LOWER_BOUND;
        }
        else {
            value_type = TTEntryType::EXACT_VALUE;
        }
        const TTEntry<M> entry = {best_move, depth, max_goodness, value_type};
        add_tt_entry(state, entry);
    }

    string get_name() const {
        return "Minimax";
    }
};

template<class S, class M>
struct MonteCarloTreeSearch : public Algorithm<S, M> {
    const double max_seconds;
    const int max_simulations;
    const bool block;
    const Random random;

    MonteCarloTreeSearch(double max_seconds = 1,
                         int max_simulations = MAX_SIMULATIONS,
                         bool block = false) :
        Algorithm<S, M>(),
        max_seconds(max_seconds),
        block(block),
        max_simulations(max_simulations) {}

    M get_move(const S *root) override {
        if (root->is_terminal()) {
            stringstream stream;
            root->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        Timer timer;
        timer.start();
        int simulation = 0;
        S clone = root->clone();
        while (simulation < max_simulations && !timer.exceeded(max_seconds)) {
            monte_carlo_tree_search(&clone);
            ++simulation;
        }
        this->log << "ratio: " << root->score / root->visits << endl;
        this->log << "simulations: " << simulation << endl;
        const auto legal_moves = root->get_legal_moves();
        this->log << "moves: " << legal_moves.size() << endl;
        for (const auto move : legal_moves) {
            this->log << "move: " << move;
            const auto child = root->get_child(move);
            if (child != nullptr) {
                this->log << " score: " << child->score
                << " visits: " << child->visits
                << " UCT: " << child->get_uct(UCT_C);
            }
            this->log << endl;
        }
        return get_most_visited_move(&clone);
    }

    void monte_carlo_tree_search(S *root) const {
        S *current = tree_policy(root, root);
        const auto result = rollout(current, root);
        propagate_up(current, result);
    }

    void propagate_up(S *current, double result) const {
        current->update_stats(result);
        if (current->parent) {
            propagate_up(current->parent, result);
        }
    }

    S* tree_policy(S *state, const S *root) const {
        if (state->is_terminal()) {
            return state;
        }
        const M move = get_tree_policy_move(state, root);
        const auto child = state->get_child(move);
        if (child == nullptr) {
            return state->add_child(move);
        }
        return tree_policy(child, root);
    }

    M get_most_visited_move(const S *state) const {
        const auto legal_moves = state->get_legal_moves();
        assert(legal_moves.size() > 0);
        M best_move;
        double max_visits = -INF;
        for (const auto move : legal_moves) {
            const auto child = state->get_child(move);
            if (child != nullptr) {
                const auto visits = child->visits;
                if (max_visits < visits) {
                    max_visits = visits;
                    best_move = move;
                }
            }
        }
        assert(max_visits != -INF);
        return best_move;
    }

    M get_best_move(S *state, const S *root) const {
        const auto legal_moves = state->get_legal_moves();
        assert(legal_moves.size() > 0);
        M best_move;
        if (state->player_to_move == root->player_to_move) {
            // maximize
            double best_uct = -INF;
            for (const auto move : legal_moves) {
                const auto child = state->get_child(move);
                if (child != nullptr) {
                    const auto uct = child->get_uct(UCT_C);
                    if (best_uct < uct) {
                        best_uct = uct;
                        best_move = move;
                    }
                } else {
                    return move;
                }
            }
        }
        else {
            // minimize
            double best_uct = INF;
            for (const auto move : legal_moves) {
                const auto child = state->get_child(move);
                if (child != nullptr) {
                    const auto uct = child->get_uct(-UCT_C);
                    if (best_uct > uct) {
                        best_uct = uct;
                        best_move = move;
                    }
                } else {
                    return move;
                }
            }
        }
        return best_move;
    }

    M get_random_move(const S *state) const {
        const auto legal_moves = state->get_legal_moves();
        assert(legal_moves.size() > 0);
        const int index = random.uniform(0, legal_moves.size() - 1);
        return legal_moves[index];
    }

    shared_ptr<M> get_winning_move(const S *state) const {
        const auto current_player = state->player_to_move;
        const auto legal_moves = state->get_legal_moves();
        S clone = state->clone();
        assert(legal_moves.size() > 0);
        for (const M &move : legal_moves) {
            clone.make_move(move);
            if (clone.is_winner(current_player)) {
                return make_shared<M>(move);
            }
            clone.undo_move(move);
        }
        return nullptr;
    }

    shared_ptr<M> get_blocking_move(const S *state) const {
        const auto current_player = state->player_to_move;
        const auto enemy = state->get_enemy(current_player);
        S enemy_state = state->clone();
        enemy_state.player_to_move = enemy;
        return get_winning_move(&enemy_state);
    }

    M get_tree_policy_move(S *state, const S *root) const {
        // If player has a winning move he makes it.
        auto move_ptr = get_winning_move(state);
        if (move_ptr != nullptr) {
            return *move_ptr;
        }
        if (block) {
            // If player has a blocking move he makes it.
            move_ptr = get_blocking_move(state);
            if (move_ptr != nullptr) {
                return *move_ptr;
            }
        }
        return get_best_move(state, root);
    }

    M get_default_policy_move(const S *state) const {
        // If player has a winning move he makes it.
        auto move_ptr = get_winning_move(state);
        if (move_ptr != nullptr) {
            return *move_ptr;
        }
        // If player has a blocking move he makes it.
        move_ptr = get_blocking_move(state);
        if (move_ptr != nullptr) {
            return *move_ptr;
        }
        return get_random_move(state);
    }

    double rollout(S *current, const S *root) const {
        if (current->is_terminal()) {
            if (current->is_winner(root->player_to_move)) {
                return WIN_SCORE;
            }
            if (current->is_winner(root->get_enemy(root->player_to_move))) {
                return LOSE_SCORE;
            }
            return DRAW_SCORE;
        }
        M move = get_default_policy_move(current);
        current->make_move(move);
        auto result = rollout(current, root);
        current->undo_move(move);
        return result;
    }

    string get_name() const {
        return "MonteCarloTreeSearch";
    }

};

struct OutcomeCounts {
    int wins = 0;
    int draws = 0;
    int loses = 0;
};

template<class S, class M>
struct Tester {
    S *root = nullptr;
    Algorithm<S, M> &algorithm_1;
    Algorithm<S, M> &algorithm_2;
    const int MATCHES;
    const bool VERBOSE;
    const bool SAVE;
    const double SIGNIFICANCE_LEVEL = 0.005; // two sided 99% confidence interval

    Tester(S *state,
           Algorithm<S, M> &algorithm_1,
           Algorithm<S, M> &algorithm_2,
           int matches = INF,
           bool verbose = false,
           bool save = false
    ) : root(state), algorithm_1(algorithm_1), algorithm_2(algorithm_2), MATCHES(matches), VERBOSE(verbose), SAVE(save) {}

    virtual ~Tester() {}

    OutcomeCounts start() {
        Timer all_timer;
        all_timer.start();
        OutcomeCounts outcome_counts;
        unordered_set<int> unique_game_hashes;
        const char enemy = root->get_enemy(root->player_to_move);
        for (int i = 1; i <= MATCHES; ++i) {
            int move_number = 1;
            auto current = root->clone();
            if (i % 4 == 0 || i % 4 == 2) {
                current.player_to_move = current.get_enemy(current.player_to_move);
            }
            if (i % 4 == 0 || i % 4 == 3) {
                current.swap_players();
            }
            if (VERBOSE) {
                cout << current << endl;
            }
            if (SAVE) {
                save_file(move_number, current);
            }
            int game_hash = current.hash();
            while (!current.is_terminal()) {
                auto &algorithm = (current.player_to_move == root->player_to_move) ? algorithm_1 : algorithm_2;
                if (VERBOSE) {
                    cout << current.player_to_move << " " << algorithm << endl;
                }
                algorithm.reset();
                Timer timer;
                timer.start();
                auto move = algorithm.get_move(&current);
                if (VERBOSE) {
                    cout << algorithm.read_log();
                    cout << timer << endl;
                }
                current.make_move(move);
                ++move_number;
                if (VERBOSE) {
                    cout << current << endl;
                }
                if (SAVE) {
                    save_file(move_number, current);
                }
                game_hash ^= current.hash();
            }
            cout << "Game " << i << ": ";
            auto insert = unique_game_hashes.insert(game_hash);
            if (!insert.second) {
                cout << "Not unique, not counting" << endl << endl;
                continue;
            }
            if (current.is_winner(root->player_to_move)) {
                ++outcome_counts.wins;
                cout << root->player_to_move << " " << algorithm_1 << " won" << endl;
            } else if (current.is_winner(enemy)) {
                ++outcome_counts.loses;
                cout << enemy << " " << algorithm_2 << " won" << endl;
            } else {
                ++outcome_counts.draws;
                cout << "draw" << endl;
            }
            const int unique_games_count = unique_game_hashes.size();
            cout << "Unique games: " << unique_games_count << endl;
            cout << root->player_to_move << " " << algorithm_1 << " wins: " << outcome_counts.wins << endl;
            cout << enemy << " " << algorithm_2 << " wins: " << outcome_counts.loses << endl;
            cout << "Draws: " << outcome_counts.draws << endl;
            const double successes = outcome_counts.wins + 0.5 * outcome_counts.draws;
            const double ratio = successes / unique_games_count;
            cout << "Ratio: " << ratio << endl;
            const double lower = boost::math::binomial_distribution<>::find_lower_bound_on_p(unique_games_count, successes, SIGNIFICANCE_LEVEL);
            const double upper = boost::math::binomial_distribution<>::find_upper_bound_on_p(unique_games_count, successes, SIGNIFICANCE_LEVEL);
            cout << "Lower confidence bound: " << lower << endl;
            cout << "Upper confidence bound: " << upper << endl;
            cout << endl;
            if (upper < 0.5 || lower > 0.5) {
                cout << "Total time: " << all_timer << endl;
                break;
            }
        }
        if (SAVE) {
            shell("convert -delay 100 -loop 0 $(ls -v *.gif) game.gif");
            shell("rm [0-9]*.gif");
        }
        return outcome_counts;
    }

    void save_file(int move_number, const S &state) const {
        const int FONT_SIZE = 16;
        stringstream ss;
        ss << state;
        vector<string> lines;
        string line;
        while (getline(ss, line)) {
            lines.push_back(line);
        }
        assert(lines.size() > 0);
        const int width = lines[0].length() * FONT_SIZE;
        const int height = lines.size() * FONT_SIZE;
        stringstream command;
        command << "convert -size " << width << "x" << height;
        command << " xc:black -font square.ttf -pointsize " << FONT_SIZE << " -fill white -draw \"";
        for (int y = 0; y < lines.size(); y++) {
            command << "text 0," << (y + 1) * FONT_SIZE << " '" << lines[y] << "' ";
        }
        command << "\" " << move_number << ".gif";
        shell(command.str());
    }

    void shell(const string &command) const {
        const int return_code = system(command.c_str());
        if (return_code != 0) {
            cout << "Command " << command << " returned " << return_code;
        }
    }
};
