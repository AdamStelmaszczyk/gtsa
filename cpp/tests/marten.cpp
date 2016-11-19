// Original author of this file: Marten Wiman
// https://www.hackerearth.com/submission/2539262/

#include <bits/stdc++.h>

using namespace std;

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)

typedef double fl;
typedef long long ll;
typedef pair<int, int> pii;
typedef vector<int> vi;
typedef vector<pii> vpi;

int dx[8] = {1, 1, 1, 0, -1, -1, -1, 0}, dy[8] = {1, 0, -1, -1, -1, 0, 1, 1};
int regionSz[2];
int w[10][7][7];

struct Bot {
    bool useNewHeuristic;
    bool useKiller;
    bool useAlphaBeta;
    bool useIterativeDeepening;
    bool preferCentre;
    bool useParallel;
    int killerDepth;
    int parallelAdd;
    int stepN;
    int removeDisFirst;
    int extraCostOpponent;
    float freeSquareVal[5];
    int regionSquareVal;
    string name;

    Bot() {}

    Bot(string _name) {
        useIterativeDeepening = true;
        useNewHeuristic = false;
        preferCentre = true;
        killerDepth = 4;
        useKiller = true;
        useAlphaBeta = true;
        useParallel = true;
        parallelAdd = 2;
        removeDisFirst = 1;
        stepN = 4;
        extraCostOpponent = 0;
        name = _name;
        freeSquareVal[1] = 4;
        freeSquareVal[2] = 1;
        freeSquareVal[3] = 0.33;
        freeSquareVal[4] = 0;
        regionSquareVal = 10;
    }
} bot;

struct Move {
    int mx, my;
    int rmx, rmy;

    Move() {
    }

    Move(int _mx, int _my, int _rmx, int _rmy) {
        mx = _mx;
        my = _my;
        rmx = _rmx;
        rmy = _rmy;
    }
};

struct State {
    char f[7][7];
    int turn;
    int px[2], py[2];

    bool okMove(int x, int y) {
        if (x < 0 || y < 0 || x > 6 || y > 6)
            return 0;
        if (f[x][y] != 0)
            return 0;
        return 1;
    }

    int evalPlayer(int p) {
        bool vis[7][7];
        int dis[7][7];
        int ways[7][7];
        rep(i, 0, 7)
            rep(j, 0, 7) {
                vis[i][j] = 0;
                ways[i][j] = 0;
            }
        float ret = 0;
        /*rep(i,0,8){
            int x=px[p]+dx[i];
            int y=py[p]+dy[i];
            if(okMove(x, y))
                ++ret;
        }*/
        int regionSize = 0;
        queue<pair<int, int> > q;
        q.push(make_pair(px[p], py[p]));
        dis[px[p]][py[p]] = 0;
        vis[px[p]][py[p]] = 1;
        ways[px[p]][py[p]] = 1;
        while (!q.empty()) {
            int xs = q.front().first;
            int ys = q.front().second;
            if (bot.extraCostOpponent && xs == px[!p] && ys == py[!p])
                ++dis[xs][ys];
            /*if(dis[xs][ys] == 1){
                ret += 4;
            }
            else if(dis[xs][ys] == 2){
                ret += 1+ways[xs][ys];
            }
            else if(dis[xs][ys] == 3){
                ret += ways[xs][ys]/3;
            }*/
            if (dis[xs][ys] <= 4)
                ret += bot.freeSquareVal[dis[xs][ys]];
            q.pop();
            ++regionSize;
            rep(i, 0, 8) {
                int x = xs + dx[i];
                int y = ys + dy[i];
                if (okMove(x, y)/* || (x == px[0] && y == py[0]) || (x == px[1] && y == py[1])*/) {
                    ways[x][y] += ways[xs][ys];
                    if (vis[x][y])
                        continue;
                    vis[x][y] = 1;
                    dis[x][y] = dis[xs][ys] + 1;
                    q.push(make_pair(x, y));
                }
            }
        }
        regionSz[p] = regionSize;
        return bot.regionSquareVal * regionSize + ret - abs(px[p] - 3) * bot.preferCentre -
               abs(py[p] - 3) * bot.preferCentre;
    }

    int eval2(int p) {
        float ret = 0;
        int xs = px[p];
        if (xs == 7)
            return 0;
        int ys = py[p];
        rep(k, 1, bot.stepN + 1) {
            ret += 1000 * (log(1 + w[k][xs][ys]));
        }
        if (w[1][xs][ys] == 0) {
            if (p == turn)
                ret -= 1000000;
            else
                ret -= 500000;
        }
        else if (w[1][xs][ys] == 1) {
            if (p == turn) {
                ret -= 50;
                if (abs(px[0] - px[1]) <= 1 && abs(py[0] - py[1]) <= 1)
                    ret -= 1000000;
            }
            else
                ret -= 200000;
        }
        else if (w[1][xs][ys] == 2) {
            int deg[2];
            int foundN = 0;
            rep(i, 0, 8) {
                int x = xs + dx[i];
                int y = ys + dy[i];
                if (!okMove(x, y))
                    continue;
                deg[foundN++] = w[1][x][y];
            }
            if (p == turn) {
                if (deg[0] <= 2 && deg[1] <= 2)
                    ret -= 50000;
                if (deg[0] == 1 && deg[1] == 1)
                    ret -= 100000;
            }
            else {
                if (deg[0] == 1 || deg[1] == 1)
                    ret -= 100000;
                else if (deg[0] <= 2 || deg[1] <= 2)
                    ret -= 30000;
            }
        }
        return ret;
    }

    int eval() {
        if (bot.useNewHeuristic) {
            rep(i, 0, 7)
                rep(j, 0, 7)
                    w[0][i][j] = 1;
            rep(k, 1, bot.stepN + 1) {
                rep(i, 0, 7)
                    rep(j, 0, 7) {
                        if (f[i][j] == -1)
                            continue;
                        w[k][i][j] = 0;
                        rep(l, 0, 8) {
                            int x = i + dx[l];
                            int y = j + dy[l];
                            if (!okMove(x, y))
                                continue;
                            w[k][i][j] += w[k - 1][x][y];
                        }
                    }
            }
            return (int) (eval2(turn) - eval2(!turn));
        }
        return evalPlayer(turn) - evalPlayer(!turn);
    }

    State performMove(Move move) {
        State ret = *this;
        if (move.mx != -1) {
            swap(ret.f[px[turn]][py[turn]], ret.f[move.mx][move.my]);
            ret.px[turn] = move.mx;
            ret.py[turn] = move.my;
        }
        if (move.rmx != -1)
            ret.f[move.rmx][move.rmy] = -1;
        ret.turn = !turn;
        return ret;
    }

    ll getLL() {
        ll ret = 0;
        rep(i, 0, 7)
            rep(j, 0, 7)
                if (f[i][j] == -1)
                    ret |= (1LL << (i * 7 + j));
        ret |= ((ll) px[0]) << 49;
        ret |= ((ll) px[1]) << 52;
        ret |= ((ll) py[0]) << 55;
        ret |= ((ll) py[1]) << 58;
        return ret;
    }

    bool operator<(const State &other) const {
        return 0;
    }

    void print() {
        rep(i, 0, 7) {
            rep(j, 0, 7) {
                if (f[i][j] == -1)printf("#");
                if (f[i][j] == 0)printf(".");
                if (f[i][j] == 1)printf("B");
                if (f[i][j] == 2)printf("R");
            }
            printf("\n");
        }
        printf("%d\n", turn);
        printf("\n");
    }
};

unordered_map<ll, int> scoreMap2[25][2];
unordered_map<ll, int> killerMap[25];
unordered_map<ll, int> scoreMap[25];
unordered_map<ll, int> lowerBound[25];

int statesDone;
bool aborted;
int checkTime;
clock_t startTime;
//char endgame[983050];

int killer(State *s, int d) {
    /*if(statesDone > MAXSTATES){
        aborted=1;
        return 0;
    }*/
    statesDone += 10;
    ll sLL = s->getLL();
    if (killerMap[d].count(sLL))
        return killerMap[d][sLL];
    if (d == 0) {
        return 0;
    }
    int sc = -1000000000;//+1000000*__builtin_popcount(sLL&());
    killerMap[d][sLL] = sc;
    int tmpx = s->px[s->turn];
    int tmpy = s->py[s->turn];
    rep(i, 0, 8) {
        int x = tmpx + dx[i];
        int y = tmpy + dy[i];
        if (!s->okMove(x, y))
            continue;
        rep(attempt, 0, 2) {
            if (attempt == 1) {
                rep(rmx, 0, 7)
                    rep(rmy, 0, 7) {
                        if (x == rmx && y == rmy)
                            continue;
                        if (rmx < 0 || rmy < 0 || rmx > 6 || rmy > 6 || s->f[rmx][rmy] != 0)
                            continue;
                        swap(s->f[tmpx][tmpy], s->f[x][y]);
                        s->px[s->turn] = x;
                        s->py[s->turn] = y;
                        s->f[rmx][rmy] = -1;
                        s->turn ^= 1;
                        sc = max(sc, -killer(s, d - 1));
                        s->turn ^= 1;
                        s->f[rmx][rmy] = 0;
                        swap(s->f[tmpx][tmpy], s->f[x][y]);
                        if (sc > 0) {
                            s->px[s->turn] = tmpx;
                            s->py[s->turn] = tmpy;
                            //s->print();
                            //printf("returned %d\n", sc);
                            return sc;
                        }
                    }
            }
            bool any = 0;
            rep(j, 0, 8) {
                int rmx = s->px[!s->turn] + dx[j];
                int rmy = s->py[!s->turn] + dy[j];
                if (x == rmx && y == rmy)
                    continue;
                if (rmx < 0 || rmy < 0 || rmx > 6 || rmy > 6 || s->f[rmx][rmy] == -1)
                    continue;
                /*if(sLL == 1956886071354727456LL && x == 5 && y == 6 && rmx == 2 && rmy == 5){
                    printf("Problem 2");
                }*/
                any = 1;
                swap(s->f[tmpx][tmpy], s->f[x][y]);
                s->px[s->turn] = x;
                s->py[s->turn] = y;
                s->f[rmx][rmy] = -1;
                s->turn ^= 1;
                sc = max(sc, -killer(s, d - 1));
                s->turn ^= 1;
                s->f[rmx][rmy] = 0;
                swap(s->f[tmpx][tmpy], s->f[x][y]);
                if (sc > 0) {
                    s->px[s->turn] = tmpx;
                    s->py[s->turn] = tmpy;
                    //s->print();
                    //printf("returned %d\n", sc);
                    return sc;
                }
            }
            if (any)
                break;
        }
    }
    s->px[s->turn] = tmpx;
    s->py[s->turn] = tmpy;
    //s->print();
    //printf("returned %d\n", sc);
    return sc;
}

int rec2(State s, int d, int hunted) {
    /*if(statesDone > MAXSTATES){
        aborted=1;
        return 0;
    }*/
    ll sLL = s.getLL();
    if (scoreMap2[d][hunted].count(sLL))
        return scoreMap2[d][hunted][sLL];
    if (d == 0) {
        statesDone += 200;
        int val = s.eval();
        scoreMap2[d][hunted][sLL] = val;
        return val;
    }
    statesDone += 40;
    /*if(statesDone > checkTime){
        checkTime += 10000;
        float elapsed=(float(clock()-startTime))/CLOCKS_PER_SEC;
        //printf("%f\n", elapsed);
        if((float(clock()-startTime))/CLOCKS_PER_SEC > 0.7){
            aborted=1;
            return 0;
        }
    }*/
    int sc = -500000000 - 1000000 * d;
    if (s.turn == hunted) {
        rep(i, 0, 8) {
            int x = s.px[s.turn] + dx[i];
            int y = s.py[s.turn] + dy[i];
            if (!s.okMove(x, y))
                continue;
            Move move(x, y, -1, -1);
            State newState = s.performMove(move);
            int score = -rec2(newState, d - 1, hunted);
            sc = max(sc, score);
        }
    }
    else {
        rep(attempt, 0, 2) {
            bool any = 0;
            rep(j, 0, 7)
                rep(k, 0, 7) {
                    if (s.f[j][k])
                        continue;
                    if (!attempt && (abs(j - s.px[!s.turn]) > 1 || abs(k - s.py[!s.turn]) > 1))
                        continue;
                    any = 1;
                    Move move(-1, -1, j, k);
                    State newState = s.performMove(move);
                    int score = -rec2(newState, d - 1, hunted);
                    sc = max(sc, score);
                }
            if (any)
                break;
            if (attempt == 1)
                sc = 500000000 + 1000000 * (d - 1);
        }
    }
    scoreMap2[d][hunted][sLL] = sc;
    return sc;
}

int alphabeta(State s, int d, int alpha, int beta) {
    if (aborted)
        return 0;
    if (statesDone > checkTime) {
        checkTime += 100000;
        if (/*statesDone > MAXSTATES && */(float(clock() - startTime)) / CLOCKS_PER_SEC > 0.9) {
            aborted = 1;
            return 0;
        }
    }
    ll sLL = s.getLL();
    //if(scoreMap[d].count(sLL))
    //	return scoreMap[d][sLL];
    if (d == 0) {
        if (scoreMap[0].count(sLL))
            return scoreMap[0][sLL];
        statesDone += 200;
        int val = s.eval();
        if (bot.useKiller) {
            if (regionSz[0] + regionSz[1] < 13) {
                //s.print();
                int k = killer(&s, bot.killerDepth);
                //if(k < 0){
                //	printf("k = %d\n", k);
                //}
                if (k) {
                    scoreMap[0][sLL] = k;
                    return k;
                }
            }
        }
        scoreMap[0][sLL] = val;
        //printf("%lld %d\n", sLL, val);
        return val;
    }
    int sc = -1000000000;
    if (scoreMap[d].count(sLL) && bot.useAlphaBeta) {
        sc = scoreMap[d][sLL];
        if (sc >= beta)
            return sc;
    }
    statesDone += 40;
    bool sameComp = 0;
    int playersDis;
    int b[2] = {-1, -1};
    vector<pair<int, int> > V[2];
    rep(p, 0, 2) {
        bool vis[7][7];
        int dis[7][7];
        rep(i, 0, 7)
            rep(j, 0, 7) {
                vis[i][j] = 0;
            }
        V[p].push_back(make_pair(s.px[p], s.py[p]));
        int minx = V[p][0].first, miny = V[p][0].second;
        vis[s.px[p]][s.py[p]] = 1;
        dis[s.px[p]][s.py[p]] = 0;
        int vf = 0;
        while (vf < V[p].size() && !sameComp) {
            int sx = V[p][vf].first;
            int sy = V[p][vf].second;
            ++vf;
            rep(i, 0, 8) {
                int x = sx + dx[i];
                int y = sy + dy[i];
                if (x == s.px[!p] && y == s.py[!p]) {
                    sameComp = 1;
                    playersDis = dis[sx][sy] + 1;
                    break;
                }
                if (s.okMove(x, y)/* || (x == px[0] && y == py[0]) || (x == px[1] && y == py[1])*/) {
                    if (vis[x][y])
                        continue;
                    vis[x][y] = 1;
                    dis[x][y] = dis[sx][sy] + 1;
                    V[p].push_back(make_pair(x, y));
                    minx = min(minx, x);
                    miny = min(miny, y);
                }
            }
        }
        if (sameComp)break;
        /*int bits=0;
        bool ok=1;
        rep(i,0,v.size()){
            int x=v[i].first-minx;
            int y=v[i].second-miny;
            if(x > 3 || y > 3 || (x == 3 && y == 3)){
                ok=0;
                break;
            }
            bits |= (2<<(x*4+y));
        }
        if(!ok)
            continue;
        bits |= ((s.px[p]-minx)*4+s.py[p]-miny)<<16;
        bits |= (p == s.turn);
        b[p]=bits;*/
    }
    if (bot.useParallel &&
        (!sameComp || playersDis >= bot.parallelAdd + d)/*min(abs(s.px[0]-s.px[1]),abs(s.py[0]-s.py[1])) >= 4*/) {
        if (!sameComp && bot.useNewHeuristic) {
            if (scoreMap[d].count(sLL) && bot.useAlphaBeta) {
                sc = scoreMap[d][sLL];
                return sc;
            }
            /*if(sLL == 1399671446368551936){
                printf("Problem\n");
            }*/
            int ret = s.eval();
            //s.print();
            rep(p, 0, 2) {
                State sp = s;
                sp.px[!p] = 7;
                sp.py[!p] = 7;
                rep(i, 0, 7)
                    rep(j, 0, 7)
                        if (sp.f[i][j] != p + 1)
                            sp.f[i][j] = -1;
                rep(i, 0, V[p].size()) {
                    int x = V[p][i].first;
                    int y = V[p][i].second;
                    if (sp.f[x][y] == -1)
                        sp.f[x][y] = 0;
                }
                //sp.print();
                ret += rec2(sp, d, p) - sp.eval();
            }
            //if(ret == 0){
            //}
            sc = ret;
            if (sc < -800000)
                sc = -1000000000;
            if (sc > 800000)
                sc = 1000000000;
            //printf("%d\n", sc);
            scoreMap[d][sLL] = sc;
            return sc;
        }
        sc = rec2(s, d, 0) + rec2(s, d, 1);
        scoreMap[d][sLL] = sc;
        return sc;
    }
    /*if(b[0] != -1 && b[1] != -1){
        if(endgame[b[s.turn]] > endgame[b[!s.turn]])
            return 1000000000;
        return -1000000000;
    }*/
    vector<pair<int, State> > v;
    rep(i, 0, 8) {
        int x = s.px[s.turn] + dx[i];
        int y = s.py[s.turn] + dy[i];
        if (!s.okMove(x, y))
            continue;
        rep(attempt, 0, 2) {
            bool any = 0;
            rep(j, 0, 7)
                rep(k, 0, 7) {
                    if (j == x && k == y)
                        continue;
                    if (!(j == s.px[s.turn] && k == s.py[s.turn]) && s.f[j][k])
                        continue;
                    if (!attempt &&
                        (abs(j - s.px[!s.turn]) > bot.removeDisFirst || abs(k - s.py[!s.turn]) > bot.removeDisFirst))
                        continue;
                    any = 1;
                    Move move(x, y, j, k);
                    State newState = s.performMove(move);
                    ll newLL = newState.getLL();
                    int V = scoreMap[d - 1].count(newLL) ? scoreMap[d - 1][newLL] : 0;
                    v.push_back(make_pair(-V, newState));
                    /*int score=-alphabeta(newState, d-1, -beta, -alpha);
                    alpha=max(alpha, score);
                    sc=max(sc, score);
                    if(sc >= beta){
                        //scoreMap[d][sLL]=sc;
                        return sc;
                    }*/
                }
            if (any)
                break;
        }
    }
    //s.print();
    if (bot.useIterativeDeepening)sort(v.begin(), v.end());
    rep(i, 0, v.size()) {
        /*if(d == 1){
            v[i].second.print();
        }*/
        int score = -alphabeta(v[i].second, d - 1, -beta, -alpha);
        alpha = max(alpha, score);
        /*if(d == 1){
            v[i].second.print();
            printf("score = %d\n", score);
        }*/
        sc = max(sc, score);
        if (sc >= beta && bot.useAlphaBeta) {
            return sc;
        }
    }
    //lowerBound[d][sLL]=sc;
    scoreMap[d][sLL] = sc;
    return sc;
}

Move findMove(State s) {
    //startTime=clock();
    /*rep(i,0,(1<<15)){
        rep(j,0,15){
            int sx=j/4;
            int sy=j%4;
            rep(turn,0,2){
                int bits=turn|(i<<1)|(j<<16);
                int best=turn?0:20;
                if(turn == 1){
                    rep(k,0,8){
                        int x=sx+dx[k];
                        int y=sy+dy[k];
                        if(i&(1<<(4*x+y)))
                            continue;
                        if(x < 0 || y < 0 || x > 3 || y > 3 || (x == 3 && y == 3))
                            continue;
                        best=max(best, endgame[(i<<1)|((4*x+y)<<16)]-'0'+1);
                    }
                }
                else{
                    rep(k,0,15){
                        if(i&(1<<k))
                            continue;
                        if(sx == k/4 && sy == k%4)
                            continue;
                        best=min(best, endgame[1+((i|(1<<k))<<1)|j]-'0'+1);
                    }
                }
                endgame[bits]=best+'0';
            }
        }
    }*/
    Move bestMove(-1, -1, -1, -1);
    Move lastBestMove(-1, -1, -1, -1);
    bool lastBestMoveExists = 0;
    int bestScore = -1000000001;
    bool extendSearchSpace = false;
    vector<int> scores;
    rep(d, 0, 20) {
        //printf("d = %d\n", d);
        bestScore = -1000000001;
        bestMove = lastBestMove;
        if (lastBestMove.mx != -1) {
            Move move = lastBestMove;
            State newState = s.performMove(move);
            int score = -alphabeta(newState, d, -1000000000, 1000000000);
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
        bool anyMove = 0;
        rep(i, 0, 8) {
            int x = s.px[s.turn] + dx[i];
            int y = s.py[s.turn] + dy[i];
            if (!s.okMove(x, y))
                continue;
            anyMove = 1;
            rep(attempt, 0, 2) {
                bool any = 0;
                rep(j, 0, 7)
                    rep(k, 0, 7) {
                        if (j == x && k == y)
                            continue;
                        if (!(j == s.px[s.turn] && k == s.py[s.turn]) && s.f[j][k])
                            continue;
                        if (!extendSearchSpace && !attempt &&
                            (abs(j - s.px[!s.turn]) > 1 || abs(k - s.py[!s.turn]) > 1))
                            continue;
                        any = 1;
                        Move move(x, y, j, k);
                        if (d && lastBestMove.mx == move.mx && lastBestMove.my == move.my &&
                            lastBestMove.rmx == move.rmx && lastBestMove.rmy == move.rmy)
                            continue;
                        /*if(d == 1 && x == 4 && y == 6 && j == 3 && k == 6){
                            printf("Problem\n");
                        }*/
                        State newState = s.performMove(move);
                        int score = -alphabeta(newState, d, -1000000000, -bestScore);
                        if (score > bestScore && !aborted) {
                            bestScore = score;
                            bestMove = move;
                        }
                    }
                if (any)
                    break;
            }
        }
        if (!anyMove)
            return Move(-2, -2, -2, -2);
        lastBestMove = bestMove;
        if (aborted) {
            break;
        }
        scores.push_back(bestScore);
        if (bestScore > 3000000) {
            //char* debug = new char[200000000];
            break;
        }
        if (bestScore < -3000000) {
            extendSearchSpace = true;
        }
        //char* debug = new char[10000000];
    }
//    printf("%d (%d)\n", scores.back(), scores.size());
    bestMove = lastBestMove;
    return bestMove;
}

int main() {
    startTime = clock();

    vector<Bot> bots;
    Bot bot1("bot1");
    bots.push_back(bot1);
    Bot bot2("noKiller");
    bot2.useKiller = false;
    bots.push_back(bot2);
    Bot bot3("noAB");
    //bot3.useAlphaBeta = false;
    bot3.useParallel = false;
    //bot3.useKiller = false;
    bots.push_back(bot3);
    Bot bot4("k2");
    bot4.killerDepth = 2;
    bots.push_back(bot4);
    Bot bot5("nKp1");
    bot5.useKiller = false;
    bot5.parallelAdd = 1;
    bots.push_back(bot5);
    Bot bot6("nKp3");
    bot6.useKiller = false;
    bot6.parallelAdd = 3;
    bots.push_back(bot6);
    Bot bot7("nKp6");
    bot7.useKiller = false;
    bot7.parallelAdd = 6;
    bots.push_back(bot7);
    Bot bot8("aC");
    bot8.preferCentre = 0;
    bots.push_back(bot8);
    Bot bot9("k6");
    bot9.killerDepth = 6;
    bots.push_back(bot9);
    Bot bot10("nH");
    bot10.useNewHeuristic = true;
    bot10.useKiller = false;
    bots.push_back(bot10);
    Bot bot11("nH2");
    bot11.useNewHeuristic = true;
    bot11.useKiller = false;
    bot11.stepN = 2;
    bots.push_back(bot11);
    Bot bot12("nH3");
    bot12.useNewHeuristic = true;
    bot12.useKiller = false;
    bot12.stepN = 3;
    bots.push_back(bot12);
    Bot bot13("aCrF2");
    bot13.preferCentre = 0;
    bot13.removeDisFirst = 2;
    bots.push_back(bot13);
    Bot bot14("aCeO");
    bot14.preferCentre = 0;
    bot14.extraCostOpponent = 1;
    bots.push_back(bot14);
    Bot bot15("aCv15v23v32v41");
    bot15.preferCentre = 0;
    bot15.freeSquareVal[1] = 5;
    bot15.freeSquareVal[2] = 3;
    bot15.freeSquareVal[3] = 2;
    bot15.freeSquareVal[4] = 1;
    bots.push_back(bot15);
    Bot bot16("nH2noIT");
    bot16.useNewHeuristic = true;
    bot16.useKiller = false;
    bot16.stepN = 2;
    bot16.useIterativeDeepening = false;
    bots.push_back(bot16);
    Bot bot17("aCv15v21eO");
    bot17.preferCentre = 0;
    bot17.freeSquareVal[1] = 5;
    bot17.freeSquareVal[2] = 1;
    bot17.freeSquareVal[3] = 0.2;
    bot17.freeSquareVal[4] = 0.1;
    bot17.extraCostOpponent = 1;
    bots.push_back(bot17);
    Bot bot18("aCv15v23v32eO");
    bot18.preferCentre = 0;
    bot18.freeSquareVal[1] = 5;
    bot18.freeSquareVal[2] = 3;
    bot18.freeSquareVal[3] = 2;
    bot18.freeSquareVal[4] = 1;
    bot18.extraCostOpponent = 1;
    bots.push_back(bot18);
    Bot bot19("aCv15v24v33v42noIt");
    bot19.preferCentre = 0;
    bot19.freeSquareVal[1] = 5;
    bot19.freeSquareVal[2] = 4;
    bot19.freeSquareVal[3] = 3;
    bot19.freeSquareVal[4] = 2;
    bot19.useIterativeDeepening = false;
    bots.push_back(bot19);
    Bot bot20("nH2p3");
    bot20.useNewHeuristic = true;
    bot20.useKiller = false;
    bot20.stepN = 2;
    bot20.parallelAdd = 3;
    bots.push_back(bot20);
    Bot bot21("nH2p6");
    bot21.useNewHeuristic = true;
    bot21.useKiller = false;
    bot21.stepN = 2;
    bot21.parallelAdd = 6;
    bots.push_back(bot21);
    Bot bot22("nH2p4");
    bot22.useNewHeuristic = true;
    bot22.useKiller = false;
    bot22.stepN = 2;
    bot22.parallelAdd = 20;
    bot22.useParallel = 0;
    bots.push_back(bot22);
    bot = bot22;

    State s;
    rep(i, 0, 7)
        rep(j, 0, 7) {
            scanf("%d", s.f[i] + j);
            if (s.f[i][j] == 1) {
                s.px[0] = i;
                s.py[0] = j;
            }
            if (s.f[i][j] == 2) {
                s.px[1] = i;
                s.py[1] = j;
            }
        }
    scanf("%d", &s.turn);
    --s.turn;
    Move bestMove = findMove(s);
    printf("%d %d %d %d %d %d\n", s.py[s.turn], s.px[s.turn], bestMove.my, bestMove.mx, bestMove.rmy, bestMove.rmx);
    return 0;
}