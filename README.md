###  源代码清单结构

|- test/*               //测试样例和测试程序

|- ASTnode.cpp         //AST节点的声明与定义

|- ASTnode.h

|- CMakeLists.txt     //cmake项目文件

|- codegen.cpp         //代码生成过程实现

|- codegen.h          //代码生成中使用的结构声明

|- grammar.y          //语法规则

|- lex.l               //词法规则

|- main.cpp           //主程序，处理命令行参数

|- objgen.cpp          //生成Object文件

|- objgen.h

|- typefactor.cpp     //管理所有的类型以及类型转换，默认值等

|- typefactor.h



**编译环境**：LLVM 11.0, bison 3.5.1, flex 2.6.4