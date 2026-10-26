%%writefile /content/MyPass/Pass1.cpp
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
    for(auto it=BB.begin(); it!=BB.end();){
      Instruction &I = *it;
      ++it;

      unsigned op = I.getOpcode();
      if(op==Instruction::SDiv||op==Instruction::UDiv||
        op==Instruction::Mul||op==Instruction::Add
        ||op==Instruction::Sub){

            Value *a = I.getOperand(0);
            Value *b = I.getOperand(1);

            if(auto *ai = dyn_cast<ConstantInt>(a)){
              if(auto *bi = dyn_cast<ConstantInt>(b)){
                const APInt &aii = ai->getValue();
                const APInt &bii = bi->getValue();
                APInt c;

                switch(op){
                  case Instruction::SDiv:
                    if(bi->isZero()) continue; //not my problem
                    c = aii.sdiv(bii);
                    break;
                  case Instruction::UDiv:
                    if(bi->isZero()) continue; //not my problem
                    c = aii.udiv(bii);
                    break;
                  case Instruction::Mul:
                    c = aii*bii;
                    break;
                  case Instruction::Add:
                    c = aii+bii;
                    break;
                  case Instruction::Sub:
                    c = aii-bii;
                    break;
                  default: llvm_unreachable("Unhandled opcode\n");
                }
                Type *Ty = I.getType();
                Value *computedVal = ConstantInt::get(Ty, c);

                BasicBlock::iterator ii = I.getIterator();
                ReplaceInstWithValue(ii, computedVal);
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