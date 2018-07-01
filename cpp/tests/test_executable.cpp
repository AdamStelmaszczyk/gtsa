#include "../examples/isola.cpp"

int main() {
    IsolaState state = IsolaState("___2___"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "_______"
                                  "___1___");

    vector<shared_ptr<Algorithm<IsolaState, IsolaMove>>> algorithms = {
            shared_ptr<Algorithm<IsolaState, IsolaMove>>(new Minimax<IsolaState, IsolaMove>(0.9, 30)),
            shared_ptr<Algorithm<IsolaState, IsolaMove>>(new Executable<IsolaState, IsolaMove>("./tests/marten.o")),
    };

    Tester<IsolaState, IsolaMove> tester(&state, algorithms);
    tester.start();

    return 0;
}