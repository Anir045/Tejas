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
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemoryLocation.h"

using namespace llvm;

struct OperandPass : PassInfoMixin<OperandPass>{
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM){
    AAResults &AA = FAM.getResult<AAManager>(F);
    LoadInst *L1 = nullptr;
    StoreInst *L2 = nullptr;

    for(BasicBlock &B : F){
      for(Instruction &I : B){
        if(auto *li = dyn_cast<LoadInst>(&I)){
          if(li->getType()->isIntegerTy(32)){
            L1 = li;
          }
        }
        if(auto *li = dyn_cast<StoreInst>(&I)){
          if(li->getValueOperand()->getType()->isIntegerTy(32)){
            L2 = li;
          }
        }
      }
    }
    AliasResult res = AA.alias(MemoryLocation::get(L1),MemoryLocation::get(L2));
    if(res==AliasResult::NoAlias) errs()<<"No Alias\n";
    else if(res==AliasResult::MayAlias) errs()<<"May Alias\n";
    else if(res==AliasResult::MustAlias) errs()<<"Must Alias\n";
    else if(res==AliasResult::PartialAlias) errs()<<"Partial Alias\n";
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