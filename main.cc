#include "include/kriol/constants.hh"
#include "include/kriol/cli.hh"


int main(int argc, const char* const* argv) {
    try {
        kriol::cli::Compiler Comp(KR_STANDARD_COMPILER_NAME, KR_VERSION_STRING);
        Comp.Run(argc, argv);
        kriol::cli::Compiler::Cleanup();
        return 0;
    } catch (const kriol::cli::FatalError& e) {
        kriol::cli::Compiler::Cleanup();
        return e.exitCode();
    } catch (...) {
        kriol::cli::Compiler::Cleanup();
        throw;
    }
}