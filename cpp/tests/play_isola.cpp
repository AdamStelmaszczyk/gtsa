#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___3___"
                                  "_______"
                                  "_______"
                                  "2_____4"
                                  "_______"
                                  "_______"
                                  "___1___", {0, 1, 0, 1});

    vector<shared_ptr<Algorithm<IsolaState, IsolaMove>>> algorithms = {
            shared_ptr<Algorithm<IsolaState, IsolaMove>>(new MonteCarloTreeSearch<IsolaState, IsolaMove>(0.1)),
            shared_ptr<Algorithm<IsolaState, IsolaMove>>(new Minimax<IsolaState, IsolaMove>(0.1)),
            shared_ptr<Algorithm<IsolaState, IsolaMove>>(new MonteCarloTreeSearch<IsolaState, IsolaMove>(0.1)),
            shared_ptr<Algorithm<IsolaState, IsolaMove>>(new Minimax<IsolaState, IsolaMove>(0.1)),
    };

    Tester<IsolaState, IsolaMove> tester(&state, algorithms);
    tester.start();

    return 0;
}