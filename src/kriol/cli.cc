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

#include <llvm/Support/ManagedStatic.h>

namespace fs = std::filesystem;
namespace ap = argparse;

using namespace kriol;

extern FILE *yyin;

extern int yyparse(kriol::ast::BlockSttmt** Program);
extern int yylex_destroy(void);
extern int yylineno;
struct yy_buffer_state;
typedef yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *yy_str);
extern void yy_delete_buffer(YY_BUFFER_STATE b);

static std::string g_source_file;

void cli::SetSourceFile(const std::string& filename) {
    g_source_file = filename;
}

const std::string& cli::GetSourceFile() {
    return g_source_file;
}

void cli::PrintErr(std::string message)
{
    std::cerr << KR_STANDARD_COMPILER_NAME << ": err: " << message << std::endl;
}

void cli::PrintErr(std::string message, int exitNum)
{
    cli::PrintErr(message);
    throw cli::FatalError(message, exitNum);
}

void cli::PrintErr(const std::string& file, int line, const std::string& msg, int exitNum) {
    std::string location = file.empty() ? "" : file + ":" + std::to_string(line) + ": ";
    std::cerr << KR_STANDARD_COMPILER_NAME << ": err: " << location << msg << std::endl;
    if (exitNum >= 0) throw cli::FatalError(location + msg, exitNum);
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
        .help("Emit LLVM IR text to stdout (or -o file) instead of compiling.")
        .default_value(false)
        .implicit_value(true);

    Parser->add_argument("--target")
        .help("Output target: 'native' or 'wasm32-wasi' (experimental).")
        .default_value(std::string("native"))
        .nargs(1)
        .choices("native", "wasm32-wasi");

    Parser->add_argument("-T", "--ignore-extension")
        .help("Without this flag, only files with extensions '." +
             std::string(KR_STANDARD_FILE_EXTENSION) + "' or '." +
             std::string(KR_ALTERNATIVE_FILE_EXTENSION) +
             "' will be allowed by the compiler.")
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

    Args.target               = Parser->get<std::string>("--target");
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
    cli::SetSourceFile(filename);
    yylineno = 1;
    try {
        yyparse(Program);
    } catch (...) {
        fclose(file);
        throw;
    }
    fclose(file);
}

void cli::KriolLangParserWrapper::ParseText(std::string text, ast::BlockSttmt** Program)
{
    YY_BUFFER_STATE buffer = yy_scan_string(text.c_str());
    if (!buffer)
        cli::PrintErr("Couldn't create scanner buffer for source text!", 1);

    yylineno = 1;
    try {
        yyparse(Program);
    } catch (...) {
        yy_delete_buffer(buffer);
        throw;
    }
    yy_delete_buffer(buffer);
}

cli::CompileResult cli::Compile(const cli::CompileOptions& options)
{
    CompileResult result;
    std::string sourceName = options.sourceName.empty()
        ? (options.inputKind == CompileInputKind::File ? options.input : "<memory>")
        : options.sourceName;
    cli::SetSourceFile(sourceName);

    ast::BlockSttmt *ProgramAST = KriolLangParserWrapper::ParseCode(
        options.input,
        options.inputKind == CompileInputKind::File
    );
    std::unique_ptr<ast::BlockSttmt> ProgramNode(ProgramAST);

    if (ProgramNode)
    {
        kriol::sema::SemanticAnalyzer sema;
        sema.SetSourceFile(sourceName);
        sema.Check(ProgramNode.get());
        if (sema.HasErrors())
        {
            result.diagnostics = sema.GetErrors();
            return result;
        }
    }

    ast::CodeGenVisitor codegenVisitor(sourceName);
    if (ProgramNode) ProgramNode->accept(codegenVisitor);

    if (options.emitIR)
    {
        result.ir = codegenVisitor.emitIR();
        if (options.outfile != "")
        {
            Compiler::SaveCodeToFile(result.ir, options.outfile);
            result.outputPath = options.outfile;
        }
        return result;
    }

    ast::EmitOptions emitOptions;
    emitOptions.Target = options.target == "wasm32-wasi"
        ? ast::CodegenTarget::Wasm32Wasi
        : ast::CodegenTarget::Native;

    std::string defaultOutfile = emitOptions.Target == ast::CodegenTarget::Wasm32Wasi
        ? "a.wasm"
        : "a.out";
    std::string outfile = options.outfile != "" ? options.outfile : defaultOutfile;
    codegenVisitor.emit(outfile, emitOptions);
    result.outputPath = outfile;
    return result;
}



void cli::Compiler::Run(const int argc, const char *const *argv)
{
    this->ParseArgs(argc, argv);

    auto matchAtEndOfFilename = [name = Args.filename](std::string endstr)
    {
        return name.find(endstr, name.size() - endstr.size());
    };

    if (Args.shouldCheckExtension &&
        (matchAtEndOfFilename(KR_STANDARD_FILE_EXTENSION) == std::string::npos) &&
        (matchAtEndOfFilename(KR_ALTERNATIVE_FILE_EXTENSION) == std::string::npos))
    {
        cli::PrintErr("File format not recognized. Filename should end with a '" +
            std::string(KR_STANDARD_FILE_EXTENSION) + "' or '" + std::string(KR_ALTERNATIVE_FILE_EXTENSION) + "' file extension.", 1);
    }

    try
    {
        CompileOptions options;
        options.inputKind = CompileInputKind::File;
        options.input = Args.filename;
        options.sourceName = Args.filename;
        options.outfile = Args.outfile;
        options.target = Args.target;
        options.emitIR = Args.emitIR;

        CompileResult result = Compile(options);
        if (!result.diagnostics.empty())
        {
            for (const auto& err : result.diagnostics)
                cli::PrintErr(err);
            throw cli::FatalError("semantic errors", 1);
        }

        if (Args.emitIR && Args.outfile == "")
            std::cout << result.ir;
    }
    catch (std::exception &err)
    {
        // Re-throw FatalError so main() catches it and returns the right exit code
        if (dynamic_cast<cli::FatalError*>(&err))
            throw;
        cli::PrintErr(err.what(), 1);
    }
}

void cli::Compiler::Cleanup()
{
    yylex_destroy();
    llvm::llvm_shutdown();
}
