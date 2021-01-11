#ifndef ASTNODE_H
#define ASTNODE_H

#include<llvm/IR/Value.h>
#include<vector>
#include<string>

#define EXCEPT_MSG_RET(except, msg, ret) if(!(except)) {errmsg = msg; return ret;}
#define EXCEPT_RET(except, ret) if(!(except)) {return ret;}

class ASTnode;
struct CodegenContext;

extern ASTnode* rootNode;
extern std::string errmsg;

#define __C_NEW_ASTNODE(name) class name : public ASTnode { \
    public: \
    name(std::string kind = "", std::string spelling = "", std::string type = "") \
        : ASTnode(kind, spelling, type){} \
    ~name() {} \
    llvm::Value* CodeGen(CodegenContext &context) override; 
#define __C_ASTNODE_END };

class ASTnode
{
public:
    std::vector<ASTnode*> sons;
    std::string spelling;
    std::string kind;
    std::string type;
    int line, column;
public:
    ASTnode(std::string kind = "", std::string spelling = "", std::string type = "");
    ~ASTnode() {};
    void AddSon(ASTnode* son);
    void SetKind(std::string s);
    void SetSpelling(std::string s);
    void SetType(std::string s);
    void SetPos(int l, int c);
    std::string DumpPos();
    void Traverse(int intend);

    void MaintainType() {
        for(auto son : sons) {
            son->SetType(type);
        }
    }

    virtual llvm::Value* CodeGen(CodegenContext &context) {
        EXCEPT_MSG_RET(0, "Abstract node codegen, wtf?", nullptr);
    };
};

__C_NEW_ASTNODE(BinaryNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(StmtsNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(VardeclNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(VardefNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(LiteralNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(DeclrefNode)
public:
    bool isL;
__C_ASTNODE_END

__C_NEW_ASTNODE(UnaryNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(CompstmtNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(FunctionDeclNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(CallNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(ReturnNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(IfStmtNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(WhileNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(ArrayDefNode);
__C_ASTNODE_END

__C_NEW_ASTNODE(ArrayDimNode)
public:
    std::vector<llvm::Value*> dims;
__C_ASTNODE_END

__C_NEW_ASTNODE(ForNode);
__C_ASTNODE_END

#endif