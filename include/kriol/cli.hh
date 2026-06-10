#ifndef _KRIOL_CLI_HEADER
#define _KRIOL_CLI_HEADER

#include "ast.hh"

#include "../external/argparse.hpp"

#include <iostream>
#include <string>
#include <filesystem>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace ap = argparse;

namespace kriol
{
    namespace cli
    {
        /// Exception thrown instead of exit() so RAII cleanup runs
        class FatalError : public std::exception {
            std::string msg_;
            int code_;
        public:
            FatalError(std::string msg, int code) : msg_(std::move(msg)), code_(code) {}
            const char* what() const noexcept override { return msg_.c_str(); }
            int exitCode() const noexcept { return code_; }
        };

        /// Prints an error message to the stderr
        void PrintErr(std::string message);
        /// Prints an error message to the stderr and exits returning the `exitNum`
        void PrintErr(std::string message, int exitNum);
        /// Prints a source-located error message (file:line: msg) and optionally exits
        void PrintErr(const std::string& file, int line, const std::string& msg, int exitNum = -1);
        /// Sets the source filename used by PrintErr and yyerror for location context
        void SetSourceFile(const std::string& filename);
        /// Returns the currently active source filename
        const std::string& GetSourceFile();
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
                std::string target;
                bool emitIR;
                bool shouldCheckExtension;
            } Args;

        public:
            Compiler(std::string Name, std::string Ver)
                : Name(Name), Version(Ver), Parser(std::make_unique<ap::ArgumentParser>(Name, Ver)) {}
            /// Used to run the compiler
            void Run(const int argc, const char *const *argv);
            /// Cleans up left over resources
            static void Cleanup();

        private:
            /// Used to parse the command-line args
            void ParseArgs(const int argc, const char *const *argv);
            /// Used to define the arguments
            void DefineArgs(void);
            /// Save the transpiled code into a file
            void SaveCodeToFile(std::string Code, std::string Filename);
        };
    };
};

#endif // _KRIOL_CLI_HEADER
