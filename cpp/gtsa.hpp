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
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <unordered_set>
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <random>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::istream;
using std::ostream;
using std::function;
using std::to_string;
using std::shared_ptr;
using std::make_shared;
using std::stringstream;
using std::runtime_error;
using std::unordered_map;
using std::unordered_set;
using std::invalid_argument;

static const int MAX_SIMULATIONS = 10000000;
static const double UCT_C = sqrt(2);
static const double WIN_SCORE = 1;
static const double DRAW_SCORE = 0.5;
static const double LOSE_SCORE = 0;

static const int MAX_DEPTH = 20;
static const int INF = 2147483647;
static const int SEED = 42;

struct Random {
    std::mt19937 engine = std::mt19937(SEED);

    virtual ~Random() {}

    int uniform(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine);
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
        gettimeofday(&tv, nullptr);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }

    double seconds_elapsed() const {
        return get_time() - start_time;
    }

    bool exceeded(double seconds) const {
        return seconds_elapsed() > seconds;
    }

    friend ostream &operator<<(ostream &os, const Timer &timer) {
        return os << std::setprecision(2) << std::fixed << timer.seconds_elapsed() << "s";
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

    ostream &to_stream(ostream &os) const {
        return os << "move: " << move << " depth: " << depth << " value: " << value << " value_type: " << value_type;
    }

    friend ostream &operator<<(ostream &os, const TTEntry &entry) {
        return entry.to_stream(os);
    }
};

template<class S, class M>
struct State {
    unsigned visits = 5; // virtual visits
    double score = 0;
    int player_to_move = 0;
    S *parent = nullptr;
    unordered_map<size_t, shared_ptr<S>> children = unordered_map<size_t, shared_ptr<S>>();
    const vector<int> teams;

    State(const vector<int> &teams) : teams(teams) {}

    virtual ~State() {}

    void update_stats(double result) {
        score += result;
        ++visits;
    }

    double get_uct(const int player) const {
        assert(visits > 0);
        double parent_visits = 0.0;
        if (parent != nullptr) {
            parent_visits = parent->visits;
        }
        double ratio = score / visits;
        if (player != player_to_move) {
            ratio = (visits - score) / score;
        }
        return ratio + UCT_C * sqrt(log(parent_visits) / visits);
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

    int get_next_player(int player) const {
        return (player + 1) % teams.size();
    }

    int get_prev_player(int player) const {
        return (player > 0) ? (player - 1) : (teams.size() - 1);
    }

    bool is_team_mate(int index) const {
        return teams[player_to_move] == teams[index];
    }

    virtual int player_char_to_index(char player) const {
        return player - '0' - 1;
    }

    virtual char player_index_to_char(int index) const {
        return index + '0' + 1;
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

    virtual bool is_terminal() const = 0;

    virtual bool is_winner(int player) const = 0;

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
            if (std::find(legal_moves.begin(), legal_moves.end(), move) != legal_moves.end()) {
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
        if (std::find(legal_moves.begin(), legal_moves.end(), move) != legal_moves.end()) {
            return move;
        } else {
            stringstream message;
            message << "Legal moves: ";
            for (auto legal_move : legal_moves) {
                message << legal_move << ", ";
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
            if (fgets(buffer, BUFFER_SIZE, pipe.get()) != nullptr) {
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
    const int verbose;

    Minimax(double max_seconds = 1,
            int max_moves = INF,
            function<vector<M>(const S*, int)> get_legal_moves = nullptr,
            function<int(const S*)> get_goodness = nullptr,
            int verbose = 0) :
        Algorithm<S, M>(),
        transposition_table(1000000),
        MAX_SECONDS(max_seconds),
        MAX_MOVES(max_moves),
        get_legal_moves(get_legal_moves),
        get_goodness(get_goodness),
        verbose(verbose),
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
        if (verbose > 1) {
            for (const auto move : moves) {
                this->log << move << ", ";
            }
            this->log << endl;
        }

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
        assert(!legal_moves.empty());
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
    const int verbose;
    mutable Random random;
    mutable int policy_moves;
    mutable int rollout_moves;

    MonteCarloTreeSearch(double max_seconds = 1,
                         int max_simulations = MAX_SIMULATIONS,
                         int verbose = 1) :
        Algorithm<S, M>(),
        max_seconds(max_seconds),
        max_simulations(max_simulations),
        verbose(verbose) {}

    M get_move(const S *root) override {
        if (root->is_terminal()) {
            stringstream stream;
            root->to_stream(stream);
            throw invalid_argument("Given state is terminal:\n" + stream.str());
        }
        Timer timer;
        timer.start();
        S clone = root->clone();
        policy_moves = 0;
        rollout_moves = 0;
        while (clone.visits < max_simulations && !timer.exceeded(max_seconds)) {
            monte_carlo_tree_search(&clone);
        }
        this->log << "ratio: " << clone.score / clone.visits << endl;
        this->log << "simulations: " << clone.visits << endl;
        this->log << "policy moves: " << policy_moves << endl;
        this->log << "rollout moves: " << rollout_moves << endl;
        const auto legal_moves = clone.get_legal_moves();
        this->log << "moves: " << legal_moves.size() << endl;
        if (verbose >= 2) {
            for (const auto move : legal_moves) {
                this->log << "move: " << move;
                const auto child = clone.get_child(move);
                if (child != nullptr) {
                    this->log << " score: " << child->score
                              << " visits: " << child->visits
                              << " UCT: " << child->get_uct(UCT_C);
                }
                this->log << endl;
            }
        }
        return get_most_visited_move(&clone);
    }

    void monte_carlo_tree_search(S *root) const {
        S *current = tree_policy(root, root);
        const auto result = rollout(current, current->player_to_move);
        propagate_up(current, result);
    }

    void propagate_up(S *current, double result) const {
        current->update_stats(result);
        if (current->parent) {
            propagate_up(current->parent, 1 - result);
        }
    }

    S* tree_policy(S *state, const S *root) const {
        if (state->is_terminal()) {
            return state;
        }
        ++policy_moves;
        const M move = get_best_move(state);
        const auto child = state->get_child(move);
        if (child == nullptr) {
            return state->add_child(move);
        }
        return tree_policy(child, root);
    }

    M get_most_visited_move(const S *state) const {
        const auto legal_moves = state->get_legal_moves();
        assert(!legal_moves.empty());
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

    M get_best_move(S *state) const {
        const auto legal_moves = state->get_legal_moves();
        assert(!legal_moves.empty());
        M best_move;
        double best_uct = -INF;
        for (const auto move : legal_moves) {
            const auto child = state->get_child(move);
            if (child != nullptr) {
                const auto uct = child->get_uct(state->player_to_move);
                if (best_uct < uct) {
                    best_uct = uct;
                    best_move = move;
                }
            } else {
                return move;
            }
        }
        return best_move;
    }

    M get_random_move(const S *state) const {
        const auto legal_moves = state->get_legal_moves();
        assert(!legal_moves.empty());
        const int index = random.uniform(0, legal_moves.size() - 1);
        return legal_moves[index];
    }

    double rollout(S *current, const int rollout_player) const {
        if (current->is_terminal()) {
            if (current->is_winner(rollout_player)) {
                return WIN_SCORE;
            }
            if (current->is_winner(current->get_next_player(rollout_player))) {
                return LOSE_SCORE;
            }
            return DRAW_SCORE;
        }
        ++rollout_moves;
        M move = get_random_move(current);
        current->make_move(move);
        auto result = rollout(current, rollout_player);
        current->undo_move(move);
        return result;
    }

    string get_name() const {
        return "MCTS";
    }

};

struct OutcomeCounts {
    vector<int> wins;
    int draws = 0;

    OutcomeCounts(int players) : wins(players) {};
};

template<class S, class M>
struct Tester {
    S *root = nullptr;
    const vector<shared_ptr<Algorithm<S, M>>> algorithms;
    const int MATCHES;
    const int VERBOSE;
    const bool SAVE;
    const double P_VALUE = 0.005; // two sided 99% confidence interval
    const double draw_score = 0.5;

    Tester(S *state,
           const vector<shared_ptr<Algorithm<S, M>>> &algorithms,
           int matches = INF,
           int verbose = 0,
           bool save = false
    ) : root(state), algorithms(algorithms), MATCHES(matches), VERBOSE(verbose), SAVE(save) {
        if (algorithms.size() != state->teams.size()) {
            throw invalid_argument("State requires passing " + to_string(state->teams.size()) + " algorithms");
        }
    }

    virtual ~Tester() {}

    OutcomeCounts start() {
        Timer all_timer;
        all_timer.start();
        const int players = root->teams.size();
        OutcomeCounts outcome_counts = OutcomeCounts(players);
        unordered_set<int> unique_game_hashes;
        for (int i = 1; i <= MATCHES; ++i) {
            int move_number = 1;
            auto current = root->clone();
            if (i % 4 == 0 || i % 4 == 2) {
                current.player_to_move = current.get_next_player(current.player_to_move);
            }
            if (i % 4 == 0 || i % 4 == 3) {
                current.swap_players();
            }
            if (VERBOSE >= 1) {
                cout << current << endl;
            }
            if (SAVE) {
                save_file(move_number, current);
            }
            auto game_hash = current.hash();
            while (!current.is_terminal()) {
                const auto algorithm_ptr = algorithms[current.player_to_move];
                if (VERBOSE >= 1) {
                    cout << current.player_index_to_char(current.player_to_move) << " " << *algorithm_ptr << endl;
                }
                algorithm_ptr->reset();
                Timer timer;
                timer.start();
                auto move = algorithm_ptr->get_move(&current);
                if (VERBOSE >= 2) {
                    cout << algorithm_ptr->read_log();
                    cout << timer << endl;
                }
                current.make_move(move);
                ++move_number;
                if (VERBOSE >= 1) {
                    cout << current << endl;
                }
                if (SAVE) {
                    save_file(move_number, current);
                }
                boost::hash_combine(game_hash, current.hash());
            }
            cout << "Game " << i << endl;
            const auto insert = unique_game_hashes.insert(game_hash);
            if (!insert.second) {
                cout << "Not unique, not counting" << endl << endl;
                continue;
            }
            bool somebody_won = false;
            for (int j = 0; j < players; ++j) {
                if (current.is_winner(j)) {
                    ++outcome_counts.wins[j];
                    cout << current.player_index_to_char(j) << " " << *algorithms[j] << " won" << endl;
                    somebody_won = true;
                }
            }
            if (!somebody_won) {
                ++outcome_counts.draws;
                cout << "draw" << endl;
            }
            const auto unique_games_count = unique_game_hashes.size();
            cout << "Unique games: " << unique_games_count << endl;
            cout << "Draws: " << outcome_counts.draws << endl;
            vector<double> successes(players), ratio(players), lower(players), upper(players);
            bool done = false;
            for (int j = 0; j < players; ++j) {
                successes[j] = outcome_counts.wins[j] + draw_score * outcome_counts.draws;
                ratio[j] = successes[j] / unique_games_count;
                lower[j] = boost::math::binomial_distribution<>::find_lower_bound_on_p(unique_games_count, successes[j], P_VALUE);
                upper[j] = boost::math::binomial_distribution<>::find_upper_bound_on_p(unique_games_count, successes[j], P_VALUE);
                cout << current.player_index_to_char(j) << " " << *algorithms[j] << " wins: " << outcome_counts.wins[j];
                cout << " ratio: " << ratio[j] << " confidence bounds: " << lower[j] << ", " << upper[j] << endl;
                if (upper[j] < draw_score || lower[j] > draw_score) {
                    done = true;
                }
            }
            cout << endl;
            if (done) {
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
        assert(!lines.empty());
        const auto width = lines[0].length() * FONT_SIZE;
        const auto height = lines.size() * FONT_SIZE;
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
