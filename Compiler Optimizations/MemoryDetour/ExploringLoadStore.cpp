#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"

using namespace llvm;

struct OperandPass : PassInfoMixin<OperandPass>{
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM){
    for(BasicBlock &BB : F){
      for(Instruction &I: BB){
        if(auto *L = dyn_cast<LoadInst>(&I)){
          Value *ptr = L->getPointerOperand();
          errs()<<"Load ";
          L->print(errs(), false);
          errs()<<"\n";

          errs()<<"Reads from: ";
          ptr->printAsOperand(errs(), false);
          errs()<<"\n";
        }
        if(auto *L = dyn_cast<StoreInst>(&I)){
          Value *ptr = L->getPointerOperand();
          Value *v = L->getValueOperand();
          errs()<<"Store ";
          L->print(errs(), false);
          errs()<<"\n";

          errs()<<"Stores in: ";
          ptr->printAsOperand(errs(), false);
          errs()<<"\n";
          ptr->print(errs(), false);
          errs()<<"\n";
          v->printAsOperand(errs(), false);
        }
      }
    }

    return PreservedAnalyses::all();
  }
};

extern "C" LLVM_ATTRIBUTE_WEAK
PassPluginLibraryInfo llvmGetPassPluginInfo() {

    return {
        LLVM_PLUGIN_API_VERSION,
        "Opcode-Pass",
        LLVM_VERSION_STRING,

        [](PassBuilder &PB) {

            PB.registerPipelineParsingCallback(
                [](StringRef Name,
                   FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {

                    if (Name == "opcode-pass") {

                        FPM.addPass(OperandPass());

                        return true;
                    }

                    return false;
                }
            );
        }
    };
}