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

    SmallVector<Value *, 8> SSAPtrs;
    SmallVector<Value *, 8> SSAPtrs2;
    for(BasicBlock &B : F){
      for(Instruction &I : B){
          if(auto *loadi = dyn_cast<LoadInst>(&I)){
            if(loadi->getType()->isPointerTy()){
              Value *ssa_ptr = loadi;
              ssa_ptr->printAsOperand(errs(), false);
              errs()<<"\n";
              SSAPtrs.push_back(ssa_ptr);
            }
            

          }
          if(auto *alloci = dyn_cast<AllocaInst>(&I)){
            if(alloci->getAllocatedType()->isIntegerTy(32)){
              Value *ssa_ptr = alloci;
              ssa_ptr->printAsOperand(errs(), false);
              errs()<<"\n";
              SSAPtrs2.push_back(ssa_ptr);
            }
          }
      }
    }
    Value *p = SSAPtrs[0];
    Value *q = SSAPtrs[1];

    AliasResult res = AA.alias(MemoryLocation(p, 4),MemoryLocation(q, 4));
    if(res==AliasResult::NoAlias) errs()<<"No Alias\n";
    else if(res==AliasResult::MayAlias) errs()<<"May Alias\n";
    else if(res==AliasResult::MustAlias) errs()<<"Must Alias\n";
    else if(res==AliasResult::PartialAlias) errs()<<"Partial Alias\n";

    Value *r = SSAPtrs2[0];
    Value *s = SSAPtrs2[1];

    AliasResult res2 = AA.alias(MemoryLocation(r, 4),MemoryLocation(s, 4));
    if(res2==AliasResult::NoAlias) errs()<<"No Alias\n";
    else if(res2==AliasResult::MayAlias) errs()<<"May Alias\n";
    else if(res2==AliasResult::MustAlias) errs()<<"Must Alias\n";
    else if(res2==AliasResult::PartialAlias) errs()<<"Partial Alias\n";
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