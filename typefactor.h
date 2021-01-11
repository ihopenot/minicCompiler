#ifndef TYPEFACTOR_H
#define TYPEFACTOR_H

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <map>
#include "codegen.h"

extern std::string errmsg;
#define EXCEPT_MSG_RET(except, msg, ret) if(!(except)) {errmsg = msg; return ret;}
#define EXCEPT_RET(except, ret) if(!(except)) {return ret;}

typedef llvm::Value*(*TypeCast)(CodegenContext&, llvm::Value*, llvm::Type*);

namespace TypeFactor{

#define __F_TYPECAST(name) llvm::Value* name(CodegenContext &context, llvm::Value* v, llvm::Type* t)

void addType(std::string name, llvm::Type* lltype);
void addCast(std::string from, std::string to, TypeCast func);
void addUpper(std::string from, std::string to);
void addDefault(std::string type, llvm::Constant* v);

llvm::Type* getType(std::string s);

void init(llvm::LLVMContext &context);

llvm::Value* Cast(CodegenContext &context, llvm::Value* v, llvm::Type* to);

llvm::Type* highType(llvm::Type* x, llvm::Type* y);

bool isEqual(llvm::Type* x, llvm::Type* y);
bool isEqual(llvm::Type* x, std::string y);

llvm::Constant* defaultValue(std::string type);

}

#endif