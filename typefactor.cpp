#include "typefactor.h"

std::map<llvm::Type*, std::map<llvm::Type*, TypeCast> > castfuncs;
std::map<llvm::Type*, std::map<llvm::Type*, bool> > upper;
std::map<std::string, llvm::Type*> types;
std::map<llvm::Type*, std::string> names;
std::map<std::string, llvm::Constant*> defaultV;

__F_TYPECAST(SIToFP) {
    return context.builder.CreateSIToFP(v, t);
}
__F_TYPECAST(FPToSI) {
    return context.builder.CreateFPToSI(v, t);
}
__F_TYPECAST(BLToSI) {
    return context.builder.CreateZExt(v, t);
}
__F_TYPECAST(BLToFP) {
    return SIToFP(context, BLToSI(context, v, t), t);
}
__F_TYPECAST(SIToBL) {
    return context.builder.CreateICmpNE(v, TypeFactor::defaultValue("int"));
}
__F_TYPECAST(FPToBL) {
    return context.builder.CreateFCmpONE(v, TypeFactor::defaultValue("int"));
}

void TypeFactor::init(llvm::LLVMContext &context) {
    addType("bool",     llvm::Type::getInt1Ty(context));
    addType("int",      llvm::Type::getInt64Ty(context));
    addType("float",    llvm::Type::getDoubleTy(context));
    addType("string",   llvm::Type::getInt8PtrTy(context));
    addType("void",     llvm::Type::getVoidTy(context));

    addType("p_int",    llvm::Type::getInt64PtrTy(context));
    addType("p_float",    llvm::Type::getDoublePtrTy(context));
    addType("p_bool",    llvm::Type::getInt1PtrTy(context));


    addCast("int",      "float",    SIToFP);
    addCast("int",      "bool",     SIToBL);
    addCast("float",    "int",      FPToSI);
    addCast("float",    "bool",     FPToBL);
    addCast("bool",     "int",      BLToSI);
    addCast("bool",     "float",    BLToFP);

    addUpper("int", "float");
    addUpper("bool", "float");
    addUpper("bool", "int");

    addDefault("int", llvm::ConstantInt::get(getType("int"), 0));
    addDefault("float", llvm::ConstantFP::get(getType("float"), 0));
    addDefault("string", llvm::ConstantPointerNull::get((llvm::PointerType*)getType("string")));
    addDefault("bool", llvm::ConstantInt::get(getType("bool"), 0));
};

void TypeFactor::addType(std::string name, llvm::Type* lltype) {
    types[name] = lltype;
    names[lltype] = name;
}
void TypeFactor::addCast(std::string from, std::string to, TypeCast func) {
    castfuncs[getType(from)][getType(to)] = func;
}
void TypeFactor::addUpper(std::string from, std::string to) {
    upper[getType(from)][getType(to)] = 1;
}
void TypeFactor::addDefault(std::string t, llvm::Constant* v) {
    defaultV[t] = v;
}

llvm::Type* TypeFactor::getType(std::string s) {
    EXCEPT_MSG_RET(types.count(s), "No such type " + s, nullptr);
    return types[s];
}

llvm::Value* TypeFactor::Cast(CodegenContext &context, llvm::Value* v, llvm::Type* to) {
    llvm::Type* from = v->getType();
    if(from == to) return v;
    EXCEPT_MSG_RET(castfuncs.count(from) && castfuncs[from].count(to), 
        "Failed cast from " + names[from] + " to " + names[to], nullptr);
    TypeCast f = castfuncs[from][to];
    return f(context, v, to);
}
    
llvm::Type* TypeFactor::highType(llvm::Type* x, llvm::Type* y) {
    EXCEPT_RET(x == y, x);
    if(upper.count(x) && upper[x].count(y))
        return y;
    return x;
}

bool TypeFactor::isEqual(llvm::Type* x, llvm::Type* y) {
    return x->getTypeID() == y->getTypeID();
}
bool TypeFactor::isEqual(llvm::Type* x, std::string y) {
    return isEqual(x, getType(y));
}

llvm::Constant* TypeFactor::defaultValue(std::string t) {
    return defaultV[t];
}