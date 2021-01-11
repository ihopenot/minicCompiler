#include "codegen.h"

void EscapeString(std::string &s) {
    s.erase(0, 1); s.erase(s.size()-1, 1);
    bool escaping = false;
    for(int i=0; i<s.size(); i++) {
        if(!escaping && s[i] == '\\') {
            escaping = true;
            s.erase(i, 1);
            i--;
        } else 
        if(escaping) {
            escaping = false;
            switch(s[i]) {
                case 'n':
                    s[i] = '\n';
                    break;
                case 't':
                    s[i] = '\t';
                    break;
                default:
                    break;
            }
        }
    }
}

llvm::Value* StmtsNode::CodeGen(CodegenContext &context) {
    llvm::Value* ret = nullptr;
    for(auto son: sons) {
        ret = son->CodeGen(context);
        EXCEPT_RET(ret != nullptr, nullptr);
    }
    return ret;
}

llvm::Value* VardeclNode::CodeGen(CodegenContext &context) {
    EXCEPT_MSG_RET(type != "void",DumpPos() + "can not declare a void variable", nullptr);
    llvm::Value* ret = nullptr;
    for(auto def : sons) {
        ret = def->CodeGen(context);
        EXCEPT_RET(ret != nullptr, nullptr)
    }
    return ret;
}

llvm::Value* VardefNode::CodeGen(CodegenContext &context) {
    llvm::Type* vtype = TypeFactor::getType(type);
    EXCEPT_MSG_RET(vtype != nullptr,DumpPos() + "Type error, no " + type + " such type", nullptr);

    if(context.isGlobal()) {
        EXCEPT_MSG_RET(context.getSymbol(spelling) == nullptr, 
            DumpPos() + "global variable " + spelling + " has already declared", nullptr);
        context.module->getOrInsertGlobal(spelling, vtype);
        llvm::GlobalVariable* v = context.module->getGlobalVariable(spelling);
        if(sons.size() == 1) {
            llvm::Value* rhs = sons[0]->CodeGen(context);
            EXCEPT_RET(rhs != nullptr, nullptr);

            rhs = TypeFactor::Cast(context, rhs, vtype);
            EXCEPT_RET(rhs != nullptr, nullptr);

            v->setInitializer(llvm::cast<llvm::Constant>(rhs));
        } else v->setInitializer(TypeFactor::defaultValue(type));
        context.addLocalSymbol(spelling, v);
        return v;
    }

    EXCEPT_MSG_RET(context.getLocalSymbol(spelling) == nullptr, 
        DumpPos() + "variable " + spelling + " has already declared", nullptr);

    llvm::Value* lhs = context.builder.CreateAlloca(vtype);
    llvm::Value* ret = lhs;
    if (sons.size() == 1) {
        llvm::Value* rhs = sons[0]->CodeGen(context);
        EXCEPT_RET(rhs != nullptr, nullptr);
        rhs = TypeFactor::Cast(context, rhs, vtype);
        EXCEPT_RET(rhs != nullptr, nullptr);
        ret = context.builder.CreateStore(rhs, lhs);
    }
    context.addLocalSymbol(spelling, lhs);
    return ret;
}

llvm::Value* BinaryNode::CodeGen(CodegenContext &context) {
    if(spelling == "=") {
        EXCEPT_MSG_RET(sons[0]->kind == "decl_ref" || sons[0]->kind == "array_ref", 
            DumpPos() + "left side must be mutable", nullptr);
        ((DeclrefNode*)sons[0])->isL = 1;
        llvm::Value* lhs = sons[0]->CodeGen(context);
        EXCEPT_RET(lhs != nullptr, nullptr);
        llvm::Value* rhs = sons[1]->CodeGen(context);
        EXCEPT_RET(rhs != nullptr, nullptr);
        rhs = TypeFactor::Cast(context, rhs, lhs->getType()->getPointerElementType());
        EXCEPT_RET(rhs != nullptr, nullptr);
        return context.builder.CreateStore(rhs, lhs);
    }

    llvm::Value* lhs = sons[0]->CodeGen(context);
    llvm::Value* rhs = sons[1]->CodeGen(context);
    EXCEPT_RET(lhs != nullptr && rhs != nullptr, nullptr);

    llvm::Type* rtype = TypeFactor::highType(lhs->getType(), rhs->getType());
    lhs = TypeFactor::Cast(context, lhs, rtype);
    rhs = TypeFactor::Cast(context, rhs, rtype);
    EXCEPT_RET(lhs != nullptr && rhs != nullptr, nullptr);

    bool fp = rtype == TypeFactor::getType("float");

    if(spelling == "+") {
        return fp ? context.builder.CreateFAdd(lhs, rhs) : context.builder.CreateAdd(lhs, rhs);
    } else
    if(spelling == "-") {
        return fp ? context.builder.CreateFSub(lhs, rhs) : context.builder.CreateSub(lhs, rhs);
    } else
    if(spelling == "*") {
        return fp ? context.builder.CreateFMul(lhs, rhs) : context.builder.CreateMul(lhs, rhs);
    } else
    if(spelling == "/") {
        return fp ? context.builder.CreateFDiv(lhs, rhs) : context.builder.CreateSDiv(lhs, rhs);
    } else
    if(spelling == "&") {
        EXCEPT_MSG_RET(!fp, DumpPos() + "float has no operator &", nullptr);
        return context.builder.CreateAnd(lhs, rhs);
    } else
    if(spelling == "|") {
        EXCEPT_MSG_RET(!fp, DumpPos() + "float has no operator |", nullptr);
        return context.builder.CreateOr(lhs, rhs);
    } else
    if(spelling == "==") {
        return fp ? context.builder.CreateFCmpOEQ(lhs, rhs) : context.builder.CreateICmpEQ(lhs, rhs);
    } else
    if(spelling == "!=") {
        return fp ? context.builder.CreateFCmpONE(lhs, rhs) : context.builder.CreateICmpNE(lhs, rhs);
    } else
    if(spelling == ">=") {
        return fp ? context.builder.CreateFCmpOGE(lhs, rhs) : context.builder.CreateICmpSGE(lhs, rhs);
    } else
    if(spelling == "<=") {
        return fp ? context.builder.CreateFCmpOLE(lhs, rhs) : context.builder.CreateICmpSLE(lhs, rhs);
    } else
    if(spelling == ">") {
        return fp ? context.builder.CreateFCmpOGT(lhs, rhs) : context.builder.CreateICmpSGT(lhs, rhs);
    } else
    if(spelling == "<") {
        return fp ? context.builder.CreateFCmpOLT(lhs, rhs) : context.builder.CreateICmpSLT(lhs, rhs);
    } else 
    if(spelling == ",") {
        return rhs;
    } else {
        EXCEPT_MSG_RET(0, DumpPos() + "unkown operator " + spelling, nullptr);
    }
}

llvm::Value* DeclrefNode::CodeGen(CodegenContext &context) {
    EXCEPT_MSG_RET(context.isGlobal() == 0, DumpPos() + spelling + " must be a literal", nullptr);
    EXCEPT_MSG_RET(context.getSymbol(spelling) != nullptr, 
        DumpPos() + "use undeclared variable " + spelling, nullptr);
    
    llvm::Value* ret;
    if(kind == "decl_ref") {
        ret = context.getSymbol(spelling);
    } else 
    if(kind == "array_ref") {
        auto dims = context.getSymbolArrayDim(spelling);
        EXCEPT_MSG_RET(dims != nullptr, DumpPos() + spelling + " is not a array", nullptr);

        llvm::Value* _;
        _ = sons[0]->CodeGen(context);
        EXCEPT_RET(_ != nullptr, nullptr);

        auto rdims = ((ArrayDimNode*)sons[0])->dims;
        EXCEPT_MSG_RET(dims->size() == rdims.size(), DumpPos() + spelling + " dimension not match", nullptr);
        llvm::Value* idx = llvm::ConstantInt::get(TypeFactor::getType("int"), 0);
        llvm::Value* step = llvm::ConstantInt::get(TypeFactor::getType("int"), 1);
        for(int i = rdims.size()-1; ~i; i--) {
            idx = context.builder.CreateAdd(idx, context.builder.CreateMul(rdims[i], step));
            step = context.builder.CreateMul(step, dims->at(i));
        }

        llvm::Value* ptr = context.getSymbol(spelling);
        llvm::Type* tp = context.getSymbolArrayType(spelling);
        ptr = context.builder.CreateBitCast(ptr, tp);
        ret = context.builder.CreateInBoundsGEP(ptr->getType()->getPointerElementType(), ptr, idx);
    } else {
        return nullptr;
    }
    if(!isL) 
        ret = context.builder.CreateLoad(ret);
    return ret;
}

llvm::Value* LiteralNode::CodeGen(CodegenContext &context) {
    if(type == "float") {
        return llvm::ConstantFP::get(TypeFactor::getType("float"), std::stold(spelling));
    } else 
    if(type == "int") {
        return llvm::ConstantInt::get(TypeFactor::getType("int"), std::stol(spelling));
    } else 
    if(type == "string") {
        EscapeString(spelling);
        return context.builder.CreateGlobalStringPtr(spelling);
    } else {
        EXCEPT_MSG_RET(0, DumpPos() + "unkown type of literal " + spelling, nullptr);
    }
}

llvm::Value* UnaryNode::CodeGen(CodegenContext &context) {
    if(spelling == "&") {
        EXCEPT_MSG_RET(sons[0]->kind == "decl_ref" | sons[0]->kind == "array_ref", 
            DumpPos() + "& must use before a variable", nullptr);
        
        ((DeclrefNode*)sons[0])->isL = true;
        llvm::Value *ret = sons[0]->CodeGen(context);
        EXCEPT_RET(ret != nullptr, nullptr);

        return ret;
    }
    llvm::Value* hs = sons[0]->CodeGen(context);
    EXCEPT_RET(hs != nullptr, nullptr);

    if(spelling == "!") {
        EXCEPT_MSG_RET(TypeFactor::isEqual(hs->getType(), "int"), DumpPos() + "float has no operator !", nullptr);
        return context.builder.CreateNot(hs);
    } else 
    if(spelling == "(") {
        return hs;
    } else {
        EXCEPT_MSG_RET(0, DumpPos() + "unkown operator " + spelling, nullptr);
    }
}

llvm::Value* CompstmtNode::CodeGen(CodegenContext &context) {
    llvm::Value* ret = context.currentFunction();
    context.pushNamespace();
    for(auto stmt : sons) {
        ret = stmt->CodeGen(context);
        EXCEPT_RET(ret != nullptr, nullptr);
    }
    context.popNamespace();
    return ret;
}

llvm::Value* FunctionDeclNode::CodeGen(CodegenContext &context) {
    EXCEPT_MSG_RET(context.isGlobal(), DumpPos() + "function must declare in global", nullptr);
    llvm::Value* ret;
    bool isDefine = sons.size() == 2;

    ASTnode* args = sons[0];

    llvm::Type* rtype = TypeFactor::getType(type), *atype;
    bool isVarg = (args->type == "varg");
    EXCEPT_MSG_RET(rtype != nullptr, DumpPos() + "Type error, no " + type + " such type", nullptr);

    // funtion declare 
    llvm::FunctionType* ftype;
    if(args->sons.size() == 0) 
        ftype = llvm::FunctionType::get(rtype, false);
    else {
        std::vector<llvm::Type*> argtypes;
        for(auto arg : args->sons) {
            atype = TypeFactor::getType(arg->type);
            EXCEPT_MSG_RET(atype != nullptr, DumpPos() + "Type error, no " + arg->type + " such type", nullptr);
            argtypes.push_back(atype);
        }
        ftype = llvm::FunctionType::get(rtype, argtypes, isVarg);
    }
    llvm::Function *newfunc = llvm::Function::Create(ftype, llvm::GlobalValue::ExternalLinkage, spelling, context.module);

    // function define
    if(isDefine) {
        context.curfunc.function = newfunc;

        llvm::BasicBlock *block = llvm::BasicBlock::Create(context.llvmContext, "", newfunc);
        context.curfunc.retblk = llvm::BasicBlock::Create(context.llvmContext, "ret");
        ASTnode *body = sons[1];

        context.pushBlock(block);
        context.pushNamespace();

        if(type != "void") 
            context.curfunc.retval = context.builder.CreateAlloca(rtype);

        int idx = 0;
        for(auto &arg : newfunc->args()) {
            llvm::Value* p = args->sons[idx]->CodeGen(context);
            context.builder.CreateStore(&arg, p);
            idx++;
        }

        llvm::Value* _;
        _ = body->CodeGen(context);
        EXCEPT_RET(_ != nullptr, nullptr);
        context.builder.CreateBr(context.curfunc.retblk);
        context.popBlock();

        context.currentFunction()->getBasicBlockList().push_back(context.curfunc.retblk);
        context.pushBlock(context.curfunc.retblk);
        if(type == "void") 
            context.builder.CreateRetVoid();
        else 
            context.builder.CreateRet(context.builder.CreateLoad(context.curfunc.retval));

        context.popBlock();
        context.popNamespace();
        context.curfunc = CodegenFunction(nullptr, nullptr, nullptr);
    }

    return newfunc;
}

llvm::Value* CallNode::CodeGen(CodegenContext &context) {
    llvm::Function* func = context.module->getFunction(spelling);
    EXCEPT_MSG_RET(func != nullptr, DumpPos() + "no " + spelling + " such function", nullptr);

    ASTnode* rargs = sons[0];
    EXCEPT_MSG_RET(func->arg_size() == rargs->sons.size() || (func->isVarArg() && func->arg_size() < rargs->sons.size()),
        DumpPos() + "arguments size not match", nullptr);

    std::vector<llvm::Value*> args;
    llvm::Value* arg;
    int idx = 0;
    for(auto &farg: func->args()) {
        arg = rargs->sons[idx]->CodeGen(context);
        EXCEPT_RET(arg != nullptr, nullptr);
        arg = TypeFactor::Cast(context, arg, farg.getType());
        EXCEPT_RET(arg != nullptr, nullptr);
        args.push_back(arg);
        idx++;
    }
    for(; idx < rargs->sons.size(); idx++) {
        arg = rargs->sons[idx]->CodeGen(context);
        EXCEPT_RET(arg != nullptr, nullptr);
        args.push_back(arg);
    }
    return context.builder.CreateCall(func, args);
}

llvm::Value* ReturnNode::CodeGen(CodegenContext &context) {
    llvm::Type* rtype = context.currentFunction()->getReturnType();
    if(sons.size() == 1) {
        EXCEPT_MSG_RET(rtype != TypeFactor::getType("void"), DumpPos() + "can't return value in a void function", nullptr);
        llvm::Value* ret = sons[0]->CodeGen(context);
        EXCEPT_RET(ret != nullptr, nullptr);
        ret = TypeFactor::Cast(context, ret, rtype);
        EXCEPT_RET(ret != nullptr, nullptr);
        context.builder.CreateStore(ret, context.curfunc.retval);
    } else 
        EXCEPT_MSG_RET(rtype == TypeFactor::getType("void"), DumpPos() + "can not return void", nullptr);
    llvm::BasicBlock *blk = llvm::BasicBlock::Create(context.llvmContext, "", context.currentFunction());
    context.builder.CreateBr(context.curfunc.retblk);
    context.popBlock();
    context.pushBlock(blk);
    return blk;
}

llvm::Value* IfStmtNode::CodeGen(CodegenContext &context) {
    bool hasElse = (sons.size() == 3);

    llvm::Value* exp = sons[0]->CodeGen(context), *_;
    EXCEPT_RET(exp != nullptr, nullptr);
    exp = TypeFactor::Cast(context, exp, TypeFactor::getType("bool"));
    EXCEPT_RET(exp != nullptr, nullptr);

    llvm::BasicBlock* blkTrue = llvm::BasicBlock::Create(context.llvmContext, "true");
    llvm::BasicBlock* blkGo = llvm::BasicBlock::Create(context.llvmContext, "go");
    llvm::BasicBlock* blkFalse = hasElse ? 
        llvm::BasicBlock::Create(context.llvmContext, "false") : nullptr;

    if(hasElse) context.builder.CreateCondBr(exp, blkTrue, blkFalse);
    else context.builder.CreateCondBr(exp, blkTrue, blkGo);
    context.popBlock();

    context.currentFunction()->getBasicBlockList().push_back(blkTrue);
    context.pushBlock(blkTrue);
    context.pushNamespace();

    _ = sons[1]->CodeGen(context);
    EXCEPT_RET(_ != nullptr, nullptr);
    context.builder.CreateBr(blkGo);

    context.popNamespace();
    context.popBlock();

    if(hasElse) {
        context.currentFunction()->getBasicBlockList().push_back(blkFalse);
        context.pushBlock(blkFalse);
        context.pushNamespace();

        _ = sons[2]->CodeGen(context);
        EXCEPT_RET(_ != nullptr, nullptr);
        context.builder.CreateBr(blkGo);

        context.popNamespace();
        context.popBlock();
    }

    context.currentFunction()->getBasicBlockList().push_back(blkGo);
    context.pushBlock(blkGo);
    return _;
}

llvm::Value* WhileNode::CodeGen(CodegenContext &context) {
    llvm::BasicBlock* blkExp    = llvm::BasicBlock::Create(context.llvmContext, "exp");
    llvm::BasicBlock* blkBody   = llvm::BasicBlock::Create(context.llvmContext, "body");
    llvm::BasicBlock* blkGo     = llvm::BasicBlock::Create(context.llvmContext, "go");

    context.builder.CreateBr(blkExp);
    context.popBlock();

    context.currentFunction()->getBasicBlockList().push_back(blkExp);
    context.pushBlock(blkExp);
    llvm::Value* exp = sons[0]->CodeGen(context), *_;
    EXCEPT_RET(exp != nullptr, nullptr);
    exp = TypeFactor::Cast(context, exp, TypeFactor::getType("bool"));
    EXCEPT_RET(exp != nullptr, nullptr);

    context.builder.CreateCondBr(exp, blkBody, blkGo);
    context.popBlock();

    context.currentFunction()->getBasicBlockList().push_back(blkBody);
    context.pushBlock(blkBody);
    context.pushNamespace();

    _ = sons[1]->CodeGen(context);
    EXCEPT_RET(_ != nullptr, nullptr);
    context.builder.CreateBr(blkExp);

    context.popNamespace();
    context.popBlock();

    context.currentFunction()->getBasicBlockList().push_back(blkGo);
    context.pushBlock(blkGo);
    return _;
}

llvm::Value* ForNode::CodeGen(CodegenContext &context) {
    llvm::BasicBlock* blkInit   = llvm::BasicBlock::Create(context.llvmContext, "init");
    llvm::BasicBlock* blkExp    = llvm::BasicBlock::Create(context.llvmContext, "exp");
    llvm::BasicBlock* blkBody   = llvm::BasicBlock::Create(context.llvmContext, "body");
    llvm::BasicBlock* blkFini   = llvm::BasicBlock::Create(context.llvmContext, "Fini");
    llvm::BasicBlock* blkGo     = llvm::BasicBlock::Create(context.llvmContext, "go");
    llvm::Value* ret;
    
    // Init
    context.builder.CreateBr(blkInit);
    context.popBlock();

    context.pushNamespace();
    context.currentFunction()->getBasicBlockList().push_back(blkInit);
    context.pushBlock(blkInit);

    ret = sons[0]->CodeGen(context);
    EXCEPT_RET(ret != nullptr, nullptr);

    context.builder.CreateBr(blkExp);
    context.popBlock();

    // EXP
    context.currentFunction()->getBasicBlockList().push_back(blkExp);
    context.pushBlock(blkExp);

    ret = sons[1]->CodeGen(context);
    EXCEPT_RET(ret != nullptr, nullptr);
    ret = TypeFactor::Cast(context, ret, TypeFactor::getType("bool"));
    EXCEPT_RET(ret != nullptr, nullptr);

    context.builder.CreateCondBr(ret, blkBody, blkGo);
    context.popBlock();

    // Body
    context.currentFunction()->getBasicBlockList().push_back(blkBody);
    context.pushBlock(blkBody);

    ret = sons[3]->CodeGen(context);
    EXCEPT_RET(ret != nullptr, nullptr);

    context.builder.CreateBr(blkFini);
    context.popBlock();

    // Fini
    context.currentFunction()->getBasicBlockList().push_back(blkFini);
    context.pushBlock(blkFini);

    ret = sons[2]->CodeGen(context);
    EXCEPT_RET(ret != nullptr, nullptr);

    context.builder.CreateBr(blkExp);
    context.popBlock();

    // Go
    context.currentFunction()->getBasicBlockList().push_back(blkGo);
    context.pushBlock(blkGo);
    context.popNamespace();
    return ret;
}

llvm::Value* ArrayDefNode::CodeGen(CodegenContext &context) {
    EXCEPT_MSG_RET(context.getLocalSymbol(spelling) == nullptr,
        DumpPos() + "variable " + spelling + " has already declared", nullptr);

    llvm::Value* _;
    _ = sons[0]->CodeGen(context);
    EXCEPT_RET(_ != nullptr, nullptr);

    auto dims = ((ArrayDimNode*)sons[0])->dims;
    llvm::Value* siz = llvm::ConstantInt::get(TypeFactor::getType("int"), 1);
    for(auto v : dims) {
        siz = context.builder.CreateMul(siz, v);
    }

    llvm::Value* ret;
    if(context.isGlobal()) {
        llvm::ConstantInt* csiz = llvm::dyn_cast<llvm::ConstantInt>(siz);
        EXCEPT_MSG_RET(csiz != nullptr, DumpPos() + "illegal array size", nullptr);
        llvm::ArrayType* t = llvm::ArrayType::get(TypeFactor::getType(type), csiz->getZExtValue());
        llvm::GlobalVariable* gret;
        context.module->getOrInsertGlobal(spelling, t);
        gret = context.module->getGlobalVariable(spelling);
        llvm::ConstantAggregateZero* zarray = llvm::ConstantAggregateZero::get(TypeFactor::getType(type));
        gret->setInitializer(zarray);
        ret = gret;
    } else {
        ret = context.builder.CreateAlloca(TypeFactor::getType(type), siz);
    }
    Var* newarray = new Var(ret, true, &dims, TypeFactor::getType("p_"+type));
    context.addLocalSymbol(spelling, newarray);
    return ret;
}

llvm::Value* ArrayDimNode::CodeGen(CodegenContext &context) {
    llvm::Value* cnt;
    for(auto son : sons) {
        cnt = son->CodeGen(context);
        EXCEPT_RET(cnt != nullptr, nullptr);
        dims.push_back(cnt);
    }
    return cnt;
}

int GenerateCode(CodegenContext &context, ASTnode* root) {
    TypeFactor::init(context.llvmContext);

    llvm::BasicBlock* globalBlock = llvm::BasicBlock::Create(context.llvmContext, "global");

    context.pushBlock(globalBlock);
    context.pushNamespace();
    llvm::Value* ret = root->CodeGen(context);
    context.popNamespace();
    context.popBlock();

    EXCEPT_RET(ret != nullptr, -1);
    return 0;
}