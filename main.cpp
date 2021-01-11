#include <string>
#include <cstdio>
#include <llvm/Support/CommandLine.h>
#include "ASTnode.h"
#include "grammar.hpp"
#include "codegen.h"
#include "objgen.h"

ASTnode* rootNode;
std::string errmsg;

void Error(std::string prefix) {
    printf("%s:\n", prefix.c_str());
    if(errmsg.size()>0) {
        printf("\t%s\n", errmsg.c_str());
    }
    exit(0);
}

int main(int argc, char** argv) {
    llvm::cl::ResetCommandLineParser();

    llvm::cl::opt<std::string> OutputFilename("o", 
        llvm::cl::desc("Specify output filename"), llvm::cl::value_desc("filename"));
    llvm::cl::opt<bool> AST("ast", llvm::cl::desc("Print ast structure"));
    llvm::cl::opt<bool> EmitLLVM("emit-llvm", llvm::cl::desc("Output IR code"));
    llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional, llvm::cl::desc("<input file>"));
    llvm::cl::opt<bool> Help("h", llvm::cl::desc("Display available options"));

    llvm::cl::ParseCommandLineOptions(argc, argv);

    if(Help) {
        llvm::cl::PrintHelpMessage();
        return 0;
    }

    if(InputFilename.size() != 0) {
        FILE* _ = freopen(InputFilename.c_str(), "r", stdin);
        if(_ == nullptr) {
            Error("Failed to open file!");
        }
    } else {
        printf("input file required!");
        return 0;
    }
    std::string output_filename;
    if(OutputFilename.size() != 0) {
        output_filename = OutputFilename;
    } else {
        output_filename = InputFilename + ".out";
    }

    int ret;

    ret = yyparse();
    if(ret) Error("AST generate Error");
    if(AST) {
        rootNode->Traverse(2);
    }

    CodegenContext context;

    ret = GenerateCode(context, rootNode);
    if(ret) Error("IRCode generate Error");
    if(EmitLLVM) {
        std::error_code EC;
        llvm::raw_fd_ostream dest(output_filename, EC, llvm::sys::fs::OF_None);
        EXCEPT_MSG_RET(!EC, "Can not open file: " + EC.message(), -1);
        context.module->print(dest, nullptr);
        return 0;
    }

    std::string objname = output_filename+".o";
    ret = GenerateObject(context, objname);
    if(ret) Error("Object generate Error");

    std::string cmdline = "g++ -no-pie -o " + output_filename + " " + objname;
    if(system(cmdline.c_str()) != 0) 
        Error("Executable generate Error");
    cmdline = "rm " + objname;
    system(cmdline.c_str());
    return 0;
}