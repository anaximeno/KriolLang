#include "include/kriol/constants.hh"
#include "include/kriol/cli.hh"

int main(int argc, const char* const* argv) {
    kriol::cli::Compiler Comp(KL_STANDARD_COMPILER_NAME, KL_VERSION_STRING);

    Comp.Run(argc, argv);

    return 0;
}