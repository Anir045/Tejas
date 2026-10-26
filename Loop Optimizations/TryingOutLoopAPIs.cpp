%%writefile /content/MyPass/Pass1.cpp
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

using namespace llvm;

struct OperandPass : PassInfoMixin<OperandPass>{
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM){
    LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
    ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
    for(Loop *L : LI){
      BasicBlock *Header = L->getHeader();
      errs()<<"Loop Header: ";
      Header->printAsOperand(errs(), false);
      errs()<<"\n";
      errs()<<"Loop Blocks:\n";
      for(BasicBlock *BB : L->blocks()){
        errs()<<" ";
        BB->printAsOperand(errs(), false);
        errs()<<"\n";
      }
      errs()<<"Loop latch: \n";
      BasicBlock *Latch = L->getLoopLatch();
      Latch->printAsOperand(errs(), false);
      errs()<<"\n";

      if(PHINode *IndVar = L->getInductionVariable(SE)){
      if(std::optional<Loop::LoopBounds> Bo= 
      Loop::LoopBounds::getBounds(*L, *IndVar, SE)){
        errs()<<"Information about Values: \n";
      Value &v1 = Bo->getInitialIVValue();
      Value &v2 = Bo->getStepInst();
      Value *v3 = Bo->getStepValue();
      Value &v4 = Bo->getFinalIVValue();
      errs()<<"Initial Value: ";
      v1.printAsOperand(errs(), false);
      errs()<<"\n";
      errs()<<"Step Instruction: ";
      v2.printAsOperand(errs(), false);
      errs()<<"\n";
      errs()<<"Step Value: ";
      v3->printAsOperand(errs(), false);
      errs()<<"\n";
      errs()<<"Final Value: ";
      v4.printAsOperand(errs(), false);
      errs()<<"\n";
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