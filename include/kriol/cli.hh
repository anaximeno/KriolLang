#ifndef _KRIOL_CLI_HEADER
#define _KRIOL_CLI_HEADER

#include "ast.hh"

#include "../external/argparse.hpp"

#include <iostream>
#include <string>
#include <filesystem>
#include <memory>
#include <algorithm>

namespace ap = argparse;

namespace kriol
{
    namespace cli
    {
        /// Prints an error message to the stderr
        void PrintErr(std::string message);
        /// Prints an error message to the stderr and exits returning the `exitNum`
        void PrintErr(std::string message, int exitNum);
        /// Exectutes a command on the terminal
        void ExecuteCommand(std::string command);
        /// Encodes a string to hexadecimal string
        std::string ConvertToHex(std::string str);

        class KriolLangParserWrapper
        {
            /// Parses a code on a file
            static void ParseFile(std::string filename, ast::BlockSttmt** Program);
            /// Parses as text
            static void ParseText(std::string text, ast::BlockSttmt** Program);

        public:
            /// Used to parse the code
            static ast::BlockSttmt *ParseCode(std::string Content, bool isFile);
        };

        class Compiler
        {
            std::string Name;
            std::string Version;
            std::unique_ptr<ap::ArgumentParser> Parser;

            struct
            {
                std::string filename;
                std::string outfile;
                bool shouldBuildOutput;
                bool shouldFormatOutput;
                bool shouldCheckExtension;
            } Args;

        public:
            Compiler(std::string Name, std::string Ver)
                : Name(Name), Version(Ver), Parser(std::make_unique<ap::ArgumentParser>(Name, Ver)) {}
            /// Used to run the compiler
            void Run(const int argc, const char *const *argv);

        private:
            /// Used to parse the command-line args
            void ParseArgs(const int argc, const char *const *argv);
            /// Used to define the arguments
            void DefineArgs(void);
            /// Save the transpiled code into a file
            void SaveCodeToFile(std::string Code, std::string Filename);
            /// Used to build the resultant code
            void BuildCode(std::string Code);
        };
    };
};

#endif // _KRIOL_CLI_HEADER
