#include "include/kriol/cli.hh"

int main(int argc, const char* const* argv) {
    kriol::cli::Compiler Comp("kriol", "Kriol v1.2.2");

    Comp.Run(argc, argv);

    return 0;
}