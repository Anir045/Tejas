#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

struct OperandPass : PassInfoMixin<OperandPass>{
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM){
    errs()<<"FUNCTION: "<<F.getName()<<"\n";

  for(BasicBlock &BB : F){
    for(auto it = BB.begin();it!=BB.end();){
      Instruction &I = *it;
      it++;
      if(auto *ai = dyn_cast<AllocaInst>(&I)){
        bool hasLoad = false;
        errs()<<"Alloca: ";
        ai->print(errs(), false);
        errs()<<"\n";
        for(User *U : ai->users()){
          errs()<<"   User: ";
          U->print(errs(), false);
          errs()<<"\n";
          if(auto *load = dyn_cast<LoadInst>(U)){
            hasLoad = true;
            break;
          }
      }
      SmallVector<StoreInst *, 8> dead;
      if(!hasLoad){
        for(auto *ui : ai->users() ){
          if(auto store_inst = dyn_cast<StoreInst>(ui)){
            dead.push_back(store_inst);
          }
        }
        for(StoreInst *si : dead){
          si->eraseFromParent();
        }
        it = ai->eraseFromParent();
      }
    }

    }
  }
    return PreservedAnalyses::none();
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