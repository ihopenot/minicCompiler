#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <map>

#include "ASTnode.h"
#include "typefactor.h"

extern std::string errmsg;

struct CodegenBlock {
    llvm::BasicBlock* block;
};

struct Var{
    llvm::Value* value;
    bool isArray;
    std::vector<llvm::Value*>* arraydims;
    llvm::Type* arraytype;
    Var(llvm::Value* v, bool a=0, const std::vector<llvm::Value*>* dims=nullptr, llvm::Type* t=nullptr) {
        value = v;
        isArray = a;
        arraydims = isArray ? new std::vector<llvm::Value*>(*dims) : nullptr;
        arraytype = t;
    }
};
struct CodegenNamespace {
    std::map<std::string, Var*> localvars;
};

struct CodegenFunction {
    llvm::Function* function;
    llvm::Value* retval;
    llvm::BasicBlock* retblk;
    CodegenFunction(llvm::Function* f, llvm::Value* v, llvm::BasicBlock* b) {
        function = f;
        retval = v;
        retblk = b;
    }
};

struct CodegenContext{
    llvm::LLVMContext llvmContext;
    llvm::IRBuilder<> builder;
    llvm::Module* module;

    CodegenFunction curfunc;

    std::vector<CodegenBlock*> blockStack;
    std::vector<CodegenNamespace*> nameStack;

    CodegenContext() :builder(llvmContext), curfunc(nullptr, nullptr, nullptr) {
        module = new llvm::Module("main", llvmContext);
    }
    
    llvm::BasicBlock* currentBlock() {
        return blockStack.size() ? blockStack.back()->block : nullptr;
    }
    void pushBlock(llvm::BasicBlock* blk) {
        CodegenBlock* cblk = new CodegenBlock;
        cblk->block = blk;
        blockStack.push_back(cblk);
        builder.SetInsertPoint(blk);
    }
    void popBlock() {
        blockStack.pop_back();
        builder.SetInsertPoint(currentBlock());
    }

    llvm::Function* currentFunction() {
        return curfunc.function;
    }
    bool isGlobal() {
        return currentFunction() == nullptr;
    }

    void pushNamespace() {
        nameStack.push_back(new CodegenNamespace);
    }
    void popNamespace() {
        delete nameStack.back();
        nameStack.pop_back();
    }
    llvm::Value* getLocalSymbol(std::string s) {
        EXCEPT_RET(nameStack.back()->localvars.count(s), nullptr);
        return nameStack.back()->localvars[s]->value;
    }
    void addLocalSymbol(std::string s, llvm::Value* v) {
        nameStack.back()->localvars[s] = new Var(v);
    }
    void addLocalSymbol(std::string s, Var* v) {
        nameStack.back()->localvars[s] = v;
    }
    llvm::Value* getSymbol(std::string s) {
        for(int i=nameStack.size()-1; ~i; i--) {
            if(nameStack[i]->localvars.count(s))
                return nameStack[i]->localvars[s]->value;
        }
        return nullptr;
    }
    std::vector<llvm::Value*>* getSymbolArrayDim(std::string s) {
        for(int i=nameStack.size()-1; ~i; i--) {
            if(nameStack[i]->localvars.count(s)) {
                EXCEPT_RET(nameStack[i]->localvars[s]->isArray, nullptr);
                return nameStack[i]->localvars[s]->arraydims;
            }
        }
        return nullptr;
    }
    llvm::Type* getSymbolArrayType(std::string s) {
        for(int i=nameStack.size()-1; ~i; i--) {
            if(nameStack[i]->localvars.count(s)) {
                EXCEPT_RET(nameStack[i]->localvars[s]->isArray, nullptr);
                return nameStack[i]->localvars[s]->arraytype;
            }
        }
        return nullptr;
    }

};

int GenerateCode(CodegenContext &context, ASTnode* root);

#endif