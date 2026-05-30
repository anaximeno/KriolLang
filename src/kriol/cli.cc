#include "../../include/kriol/cli.hh"
#include "../../include/kriol/ast.hh"
#include "../../include/kriol/codegen.hh"
#include "../../include/kriol/sema.hh"
#include "../../include/kriol/constants.hh"

#include "../../include/external/argparse.hpp"

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <sstream>

namespace fs = std::filesystem;
namespace ap = argparse;

using namespace kriol;

extern FILE *yyin;

extern int yyparse(kriol::ast::BlockSttmt** Program);

void cli::PrintErr(std::string message)
{
    std::cerr << KL_STANDARD_NAME << ": Err: " << message << std::endl;
}

void cli::PrintErr(std::string message, int exitNum)
{
    cli::PrintErr(message);
    exit(exitNum);
}

void cli::ExecuteCommand(std::string command)
{
    int out = system(command.c_str());
    if (out != 0)
    {
        cli::PrintErr("Could not execute the following command '" + command + "'", out);
    }
}

std::string cli::ConvertToHex(std::string str)
{
    std::stringstream sstrm;

    for (const auto &ch : str)
    {
        sstrm << std::hex << int(ch);
    }

    return sstrm.str();
}

void cli::Compiler::DefineArgs(void)
{
    Parser->add_argument("file")
        .help("Name of the target compilation file.")
        .required()
        .nargs(1);

    Parser->add_argument("-o", "--output")
        .help("Option used to specify the name of the output file.")
        .nargs(1);

    Parser->add_argument("--emit-ir")
        .help("Emit LLVM IR text to stdout (or -o file) instead of compiling natively.")
        .default_value(false)
        .implicit_value(true);

    Parser->add_argument("-T", "--ignore-extension")
        .help("Without this flag, only files with extension '" + std::string(KL_STANDARD_FILE_EXTENSION) + "' will be allowed by the compiler.")
        .default_value(false)
        .implicit_value(true);
}

void cli::Compiler::ParseArgs(const int argc, const char *const *argv)
{
    this->DefineArgs();

    try
    {
        Parser->parse_args(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::stringstream sstrm;

        sstrm << *(Parser);

        cli::PrintErr(std::string(e.what()) + "\n\n" + sstrm.str(), 1);
    }

    try
    {
        Args.filename = Parser->get<std::string>("file");
    }
    catch (const std::exception &err)
    {
        Args.filename = "";
    }

    try
    {
        Args.outfile = Parser->get<std::string>("--output");
    }
    catch (const std::exception &err)
    {
        Args.outfile = "";
    }

    Args.emitIR               = Parser->get<bool>("--emit-ir");
    Args.shouldCheckExtension = !Parser->get<bool>("--ignore-extension");
}

ast::BlockSttmt *cli::KriolLangParserWrapper::ParseCode(std::string Content, bool isFile)
{
    ast::BlockSttmt* ProgramAST = nullptr;
    if (isFile)
    {
        ParseFile(Content, &ProgramAST);
    }
    else
    {
        ParseText(Content, &ProgramAST);
    }
    return ProgramAST;
}

void cli::Compiler::SaveCodeToFile(std::string Code, std::string Filename)
{
    FILE *file = fopen(Filename.c_str(), "w");

    if (file == NULL)
    {
        cli::PrintErr("Couldn't create file '" + Filename + "'!", 1);
    }

    fputs(Code.c_str(), file);

    fclose(file);
}

void cli::KriolLangParserWrapper::ParseFile(std::string filename, ast::BlockSttmt** Program)
{
    if (!fs::exists(filename))
    {
        cli::PrintErr("File '" + filename + "' was not found!", 1);
    }

    FILE *file = fopen(filename.c_str(), "r");

    if (file == NULL)
    {
        cli::PrintErr("Couldn't open the file '" + filename + "'!", 1);
    }

    yyin = file;
    yyparse(Program);

    fclose(file);
}

void cli::KriolLangParserWrapper::ParseText(std::string text, ast::BlockSttmt** Program)
{
    /// TODO: Implement this
    cli::PrintErr("cli::KriolLangParserWrapper::ParseText is not implemented yet!\n", -1);
}



void cli::Compiler::Run(const int argc, const char *const *argv)
{
    this->ParseArgs(argc, argv);

    auto matchAtEndOfFilename = [name = Args.filename](std::string endstr)
    {
        return name.find(endstr, name.size() - endstr.size());
    };

    if (Args.shouldCheckExtension && (matchAtEndOfFilename(KL_STANDARD_FILE_EXTENSION) == std::string::npos))
    {
        cli::PrintErr("File format not recognized. Filename should end with a '" + std::string(KL_STANDARD_FILE_EXTENSION) + "' file extension.", 1);
    }

    ast::BlockSttmt *ProgramAST = KriolLangParserWrapper::ParseCode(Args.filename, true);
    std::unique_ptr<ast::BlockSttmt> ProgramNode(ProgramAST);

    if (ProgramNode)
    {
        kriol::sema::SemanticAnalyzer sema;
        sema.Check(ProgramNode.get());
        if (sema.HasErrors())
        {
            for (const auto& err : sema.GetErrors())
                cli::PrintErr(err);
            exit(1);
        }
    }

    try
    {
        ast::CodeGenVisitor codegenVisitor(Args.filename);
        if (ProgramNode) ProgramNode->accept(codegenVisitor);

        if (Args.emitIR)
        {
            std::string ir = codegenVisitor.emitIR();
            if (Args.outfile != "")
                SaveCodeToFile(ir, Args.outfile);
            else
                std::cout << ir;
        }
        else
        {
            std::string outfile = Args.outfile != "" ? Args.outfile : "a.out";
            codegenVisitor.emitNative(outfile);
        }
    }
    catch (std::exception &err)
    {
        cli::PrintErr(err.what(), 1);
    }
}
