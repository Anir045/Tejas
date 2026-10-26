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
    LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
    ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
    MemorySSA &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
    MemorySSAUpdater MSSAU(&MSSA);
    bool Changed = false;
    for(Loop *L : LI){
        for(BasicBlock *BB : L->blocks()){
          for(auto it=BB->begin(); it!=BB->end();){
            Instruction &I = *it;
            ++it;
          //I.print(errs(), false);
          errs()<<"\n";
          if(I.isTerminator()){
            continue;
          }
          if(!isSafeToSpeculativelyExecute(&I)){
            continue;
          }
          if(L->hasLoopInvariantOperands(&I)){
            errs()<<"Loop Invariant Instruction spotted!\n";
            I.print(errs(), false);
            errs() << "\n";
              if(L->makeLoopInvariant(&I, Changed, nullptr, &MSSAU));{
                errs()<<"Invariant Code removed and hoisted to preheader\n"; 
            }
             
          }

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