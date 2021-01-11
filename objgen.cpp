#include "objgen.h"

int GenerateObject(CodegenContext &context, std::string objname) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    context.module->setTargetTriple(TargetTriple);
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, errmsg);
    EXCEPT_RET(Target != nullptr, -1);

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    context.module->setDataLayout(TargetMachine->createDataLayout());
    context.module->setTargetTriple(TargetTriple);

    std::error_code EC;
    llvm::raw_fd_ostream dest(objname, EC, llvm::sys::fs::OF_None);
    EXCEPT_MSG_RET(!EC, "Can not open file: " + EC.message(), -1);

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;
    EXCEPT_MSG_RET(!TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType), 
        "TargetMachine can't emit a file of this type", -1);
    pass.run(*context.module);
    dest.flush();
    return 0;
}