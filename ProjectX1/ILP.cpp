/*
An attempt to exploit ILP in the LinLay program
1. Loop unrolling
2. Loop vectorization
3. Software scheduling
Still a work in progress
*/
/*
acc0 = b[j]
acc1 = 0
acc2 = 0
acc3 = 0

while (k < K) {

    acc0 = X[k+0] * W[k+0] + acc0
    acc1 = X[k+1] * W[k+1] + acc1
    acc2 = X[k+2] * W[k+2] + acc2
    acc3 = X[k+3] * W[k+3] + acc3

    k += 4
}

sum = acc0 + acc1 + acc2 + acc3
*/
/*
%acc0 = phi float [ %33, %34 ], [ %update0, %41 ]
%acc1 = phi float [ 0.0, %34 ], [ %update1, %41 ]
%acc2 = phi float [ 0.0, %34 ], [ %update2, %41 ]
%acc3 = phi float [ 0.0, %34 ], [ %update3, %41 ]
*/

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

struct ILPPass : PassInfoMixin<ILPPass>{
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM){
    LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
    StringRef fname = F.getName();
    errs()<<"\nFunction Name: "<<fname<<"\n";
    if (F.getName() != "linLay"){
      return PreservedAnalyses::all();
    }
    for(Loop *li : LI){
      errs()<<"\nLevel 1 loop\n";
      BasicBlock *header = li->getHeader();
      BasicBlock *latch = li->getLoopLatch();
      errs()<<"\n Header: ";
      header->printAsOperand(errs(), false);
      errs()<<"\n Latch: ";
      latch->printAsOperand(errs(), false);
      errs()<<"\n";

      const std::vector<Loop *> &sl1 = li->getSubLoops();
      for(Loop *lii : sl1){
        errs()<<"\nLevel 2 loop\n";
        BasicBlock *header = lii->getHeader();
        BasicBlock *latch = lii->getLoopLatch();
        errs()<<"\n Header: ";
        header->printAsOperand(errs(), false);
        errs()<<"\n Latch: ";
        latch->printAsOperand(errs(), false);
        errs()<<"\n";

          if(header){
            SmallVector<PHINode *, 5> phi_vector;
            for(Instruction &I : *header){
              if(auto *phi = dyn_cast<PHINode>(&I)){
                errs()<<"PHINode ";
                phi->printAsOperand(errs(), false);
                errs()<<"\n";
                phi_vector.push_back(phi);
                for(auto *a : phi_vector){
                  Value *incoming = a->getIncomingValueForBlock(latch);
                  errs()<<"\n";
                  phi->printAsOperand(errs(), false);
                  errs()<<"-> incoming from latch: ";
                  incoming->printAsOperand(errs(), false);
                  errs()<<"\n";
                }
            }
          }
        }

        const std::vector<Loop *> &sl2 = lii->getSubLoops();
        for(Loop *liii : sl2){
          BasicBlock *preheader = liii->getLoopPreheader();
          errs()<<"\nLevel 3 loop\n";
          BasicBlock *header = liii->getHeader();
          BasicBlock *latch = liii->getLoopLatch();
          errs()<<"\n Header: ";
          header->printAsOperand(errs(), false);
          errs()<<"\n Latch: ";
          latch->printAsOperand(errs(), false);
          errs()<<"\n";
          if(header && latch){

            PHINode *phiK = nullptr;

            for(auto &Inst : *header){
                if (auto *P = dyn_cast<PHINode>(&Inst)) {
                    if (P->getType()->isIntegerTy()) {
                        phiK = P;
                        break;
                    }
                }
            }
            if(!phiK){
              continue;
            }

            for(auto it = header->begin();it!=header->end();){
              Instruction &I = *it;
              ++it;
              if(auto *phi = dyn_cast<PHINode>(&I)){
                errs()<<"PHINode ";
                phi->printAsOperand(errs(), false);
                errs()<<"\n";

                Value *incoming = phi->getIncomingValueForBlock(latch);
                errs()<<"Incoming from latch: ";
                incoming->printAsOperand(errs(), false);
                errs()<<"\n";
                if(auto *II = dyn_cast<IntrinsicInst>(incoming)){
                  if(II->getIntrinsicID()==Intrinsic::fmuladd){
                    errs()<<"Identified llvm.fmuladd: "<< *II<<"\n";

                    Value *A = II->getArgOperand(0);
                    Value *B = II->getArgOperand(1);
                    Value *C = II->getArgOperand(2);
                    A->printAsOperand(errs(), false);
                    errs()<<"\n";
                    B->printAsOperand(errs(), false);
                    errs()<<"\n";
                    C->printAsOperand(errs(), false);
                    errs()<<"\n";

                    if(C==phi){
                      errs()<<"Found Reduction Accumulator: ";
                      phi->printAsOperand(errs(), false);
                      errs()<<"\n";
                    }
                    Value *initialAcc = phi->getIncomingValueForBlock(preheader);


                    PHINode *ac1 = PHINode::Create(Type::getFloatTy(F.getContext()),2,"ac1");
                    ac1->insertBefore(&*header->begin());
                    Constant *zero = ConstantFP::get(Type::getFloatTy(F.getContext()), 0.0);
                    ac1->addIncoming(zero, initialAcc);
                    Instruction *I1 = II->clone();
                    I1->insertBefore(II);
                    I1->setOperand(2, ac1);
                    ac1->addIncoming(I1, latch);

                    //====================================================================================================================

                    PHINode *ac2 = PHINode::Create(Type::getFloatTy(F.getContext()),2,"ac2");
                    ac2->insertBefore(&*header->begin());
                    ac2->addIncoming(zero, preheader);
                    Value *one = ConstantInt::get(Type::getInt64Ty(F.getContext()), 1);
                    BinaryOperator *k1 = BinaryOperator::CreateAdd(phiK, one, "k1", II);

                    // ---------- X[k+1] ----------

                    Instruction *XLoadOriginal = cast<Instruction>(A);
                    Instruction *XGEPOriginal = cast<Instruction>(XLoadOriginal->getOperand(0));
                    Instruction *XGEP2 = XGEPOriginal->clone();
                    XGEP2->insertBefore(II);
                    XGEP2->setOperand(1, k1);
                    Instruction *XLoad2 = XLoadOriginal->clone();
                    XLoad2->insertBefore(II);
                    XLoad2->setOperand(0, XGEP2);


                    // ---------- W[k+1] ----------

                    Instruction *WLoadOriginal = cast<Instruction>(B);
                    Instruction *WGEPOriginal = cast<Instruction>(WLoadOriginal->getOperand(0));
                    Instruction *WMulOriginal = cast<Instruction>(WGEPOriginal->getOperand(1));
                    Instruction *WMul2 = WMulOriginal->clone();
                    WMul2->insertBefore(II);
                    WMul2->setOperand(0, k1);
                    Instruction *WGEP2 = WGEPOriginal->clone();
                    WGEP2->insertBefore(II);
                    WGEP2->setOperand(1, WMul2);
                    Instruction *WLoad2 = WLoadOriginal->clone();
                    WLoad2->insertBefore(II);
                    WLoad2->setOperand(0, WGEP2);

                    Instruction *I2 = II->clone();
                    I2->insertBefore(II);
                    I2->setOperand(0, XLoad2);
                    I2->setOperand(1, WLoad2);
                    I2->setOperand(2, ac2);
                    ac2->addIncoming(I2, latch);

                    //=======================================================================================================================

                    PHINode *ac3 = PHINode::Create(Type::getFloatTy(F.getContext()),2,"ac3");
                    ac3->insertBefore(&*header->begin());
                    ac3->addIncoming(zero, preheader);
                    Value *two = ConstantInt::get(Type::getInt64Ty(F.getContext()), 2);
                    BinaryOperator *k2 = BinaryOperator::CreateAdd(phiK, two, "k2", II);

                    Instruction *XGEP3 = XGEPOriginal->clone();

                    //------------X[k+2]----------
                    XGEP3->insertBefore(II);
                    XGEP3->setOperand(1, k2);

                    Instruction *XLoad3 = XLoadOriginal->clone();
                    XLoad3->insertBefore(II);
                    XLoad3->setOperand(0, XGEP3);


                    // ---------- W[k+2] ----------

                    Instruction *WMul3 = WMulOriginal->clone();
                    WMul3->insertBefore(II);
                    WMul3->setOperand(0, k2);

                    Instruction *WGEP3 = WGEPOriginal->clone();
                    WGEP3->insertBefore(II);
                    WGEP3->setOperand(1, WMul3);

                    Instruction *WLoad3 = WLoadOriginal->clone();
                    WLoad3->insertBefore(II);
                    WLoad3->setOperand(0, WGEP3);

                    Instruction *I3 = II->clone();
                    I3->insertBefore(II);
                    I3->setOperand(0, XLoad3);
                    I3->setOperand(1, WLoad3);
                    I3->setOperand(2, ac3);
                    ac3->addIncoming(I3, latch);

                    //================================================================================================================
                    PHINode *ac4 = PHINode::Create(Type::getFloatTy(F.getContext()),2,"ac4");
                    ac4->insertBefore(&*header->begin());
                    ac4->addIncoming(zero, preheader);
                    Value *three = ConstantInt::get(Type::getInt64Ty(F.getContext()), 3);
                    BinaryOperator *k3 = BinaryOperator::CreateAdd(phiK, three, "k3", II);

                    // ---------- X[k+3] ----------

                    Instruction *XGEP4 = XGEPOriginal->clone();
                    XGEP4->insertBefore(II);
                    XGEP4->setOperand(1, k3);

                    Instruction *XLoad4 = XLoadOriginal->clone();
                    XLoad4->insertBefore(II);
                    XLoad4->setOperand(0, XGEP4);


                    // ---------- W[k+3] ----------

                    Instruction *WMul4 = WMulOriginal->clone();
                    WMul4->insertBefore(II);
                    WMul4->setOperand(0, k3);

                    Instruction *WGEP4 = WGEPOriginal->clone();
                    WGEP4->insertBefore(II);
                    WGEP4->setOperand(1, WMul4);

                    Instruction *WLoad4 = WLoadOriginal->clone();
                    WLoad4->insertBefore(II);
                    WLoad4->setOperand(0, WGEP4);


                    Instruction *I4 = II->clone();
                    I4->insertBefore(II);
                    I4->setOperand(0, XLoad4);
                    I4->setOperand(1, WLoad4);
                    I4->setOperand(2, ac4);
                    ac4->addIncoming(I4, latch);

                  }
                }
                }
              }
            }
        }

      }

      if(header){
        for(Instruction &I : *header){
          if(auto *phi = dyn_cast<PHINode>(&I)){
            errs()<<"PHINode ";
            phi->printAsOperand(errs(), false);
            errs()<<"\n";
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
        "ILPPass",
        LLVM_VERSION_STRING,

        [](PassBuilder &PB) {

            PB.registerPipelineParsingCallback(
                [](StringRef Name,
                   FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {

                    if (Name == "ILPPass") {

                        FPM.addPass(ILPPass());

                        return true;
                    }

                    return false;
                }
            );
        }
    };
}