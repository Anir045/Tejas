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
      for(auto it=BB.begin();it!=BB.end();){
        Instruction &I = *it;
        ++it;
        if(I.getOpcode()==Instruction::Add){
          for(unsigned i=0;i<I.getNumOperands();++i){
            Value *a = I.getOperand(i);

            if(auto *ai=dyn_cast<ConstantInt>(a)){
              if(ai->isZero()){
                Value *otherOperand = I.getOperand(1-i);
                errs()<<"Zero Found!\n";
                errs()<<"Other Operand: ";
                otherOperand->print(errs(), false);
                errs()<<"\n";

                BasicBlock::iterator ii = I.getIterator();
                ReplaceInstWithValue(ii, otherOperand);
                break;
              }
            }
          }
        }
        if(I.getOpcode()==Instruction::Mul){
          for(unsigned i=0;i<I.getNumOperands();++i){
            Value *b = I.getOperand(i);

            if(auto *bi = dyn_cast<ConstantInt>(b)){
              if(bi->getValue()==1){
                Value *otherOperand = I.getOperand(1-i);
                errs()<<"Found MUL by 1\n";
                I.print(errs(), false);
                errs()<<"\n";
                b->print(errs(), false);
                errs()<<"\n";

                BasicBlock::iterator ii = I.getIterator();
                ReplaceInstWithValue(ii, otherOperand);
                break;
              }
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