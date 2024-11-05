#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

#include <bits/stdc++.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/ConstantFolder.h>
#include <llvm/IR/Instruction.h>

using namespace llvm;
using namespace llvm::sys;

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;

static void InitializeModule() {
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("my cool jit", *TheContext);
    Builder = std::make_unique<IRBuilder<>>(*TheContext);
}

int GenerateObjFile(std::string Filename) {
    // * GENERATE OBJ FILE
    // Initialize the target registry etc.
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto TargetTriple = sys::getDefaultTargetTriple();
    TheModule->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        errs() << Error;
        return 1;
    }

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto TheTargetMachine = Target->createTargetMachine(
        TargetTriple, CPU, Features, opt, Reloc::PIC_);

    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

    if (EC) {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }

    legacy::PassManager pass;
    auto FileType = CodeGenFileType::ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        errs() << "TheTargetMachine can't emit a file of this type";
        return 1;
    }

    pass.run(*TheModule);
    dest.flush();

    outs() << "Wrote " << Filename << "\n";

    return 0;
}

int main() {
    InitializeModule();

    std::vector<Type *> printf_args{ PointerType::get(Type::getInt8Ty(*TheContext), 0) };
    FunctionType *printf_type = FunctionType::get(Type::getInt32Ty(*TheContext), printf_args, true);
    Function *printf_func = Function::Create(printf_type, Function::ExternalLinkage, "printf", TheModule.get());


    FunctionType *main_type = FunctionType::get(Type::getInt64Ty(*TheContext), false);
    Function *main_func = Function::Create(main_type, Function::ExternalLinkage, "main", TheModule.get());
    
    BasicBlock *entry = BasicBlock::Create(*TheContext, "entry", main_func);            
    
    ArrayType *tarr = ArrayType::get(Type::getInt16Ty(*TheContext), 3);
    int num = 3;

    StructType *mystruct = StructType::create(*TheContext, "variable-array");
    mystruct->setBody({ Type::getInt64Ty(*TheContext), PointerType::get(Type::getInt64Ty(*TheContext), 0) });

        
    FunctionType *ft = FunctionType::get(Type::getInt64Ty(*TheContext), { PointerType::get(mystruct, 0), Type::getInt64Ty(*TheContext) }, false);
    Function *F = Function::Create(ft, Function::ExternalLinkage, "test", TheModule.get());

    
    Builder->SetInsertPoint(entry);
    
    AllocaInst *alloc = Builder->CreateAlloca(mystruct, nullptr, "array_ptr");
    AllocaInst *alloc1 = Builder->CreateAlloca(tarr, nullptr, "array");

    Builder->CreateStore(ConstantArray::get(tarr, {
                ConstantInt::get(*TheContext, APInt(64, 1)),
                ConstantInt::get(*TheContext, APInt(64, 2)),
                ConstantInt::get(*TheContext, APInt(64, 3))
            }), alloc1);

    Value *col_ptr = Builder->CreateGEP(mystruct, alloc, {
            ConstantInt::get(*TheContext, APInt(64, 0))
        });
    
    Value *array_ptr = Builder->CreateGEP(mystruct, alloc, {
            ConstantInt::get(*TheContext, APInt(64, 1))
        });

    Builder->CreateStore(ConstantInt::get(*TheContext, APInt(64, 3)), col_ptr);
    Builder->CreateStore(alloc1, array_ptr);

    Builder->CreateCall(F, { alloc, ConstantInt::get(*TheContext, APInt(64, 2)) }, "calltmp");
        
    Builder->CreateRet(ConstantInt::get(*TheContext, APInt(64, 0)));

    BasicBlock *bb = BasicBlock::Create(*TheContext, "entry", F);

    Builder->SetInsertPoint(bb);
    
    
    
    std::vector<Value *> args{
        Builder->CreateGlobalStringPtr("%i\n", "test"),
        // ggg
                            };
    
    Builder->CreateCall(printf_func, args, "calltmp");
    
    Builder->CreateRet(ConstantInt::get(*TheContext, APInt(64, 0)));
    
    TheModule->print(errs(), nullptr);
    
    auto Filename = "lltest.o";
    GenerateObjFile(Filename);

    return 0;
}
