#include "ASTnode.h"
#include <iostream>

ASTnode::ASTnode(std::string kind, std::string spelling, std::string type) {
    SetKind(kind);
    SetSpelling(spelling);
    SetType(type);
}

void ASTnode::AddSon(ASTnode* son) {
    this->sons.push_back(son);
}

void ASTnode::Traverse(int intend) {
    for(int i=0; i<intend; i++) putchar(' ');
    std::cout << kind << " : " << spelling << " - " << type << std::endl;
    for(int i=0; i<sons.size(); i++) {
        sons[i]->Traverse(intend+2);
    }
}

void ASTnode::SetKind(std::string s) {
    this->kind = s;
}
void ASTnode::SetSpelling(std::string s) {
    this->spelling = s;
}
void ASTnode::SetType(std::string s) {
    this->type = s;
}
void ASTnode::SetPos(int l, int c) {
    line = l;
    column = c;
}
std::string ASTnode::DumpPos() {
    return "l: "+std::to_string(line)+", c: "+std::to_string(column)+" : ";
}
