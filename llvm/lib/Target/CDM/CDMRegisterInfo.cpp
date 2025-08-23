//
// Created by ilya on 21.11.23.
//
#include "MCTargetDesc/CDMMCTargetDesc.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include <unordered_set>
#define DEBUG_TYPE "cdm-reg-info"

#include "CDMRegisterInfo.h"

#include "CDMFrameLowering.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#define GET_REGINFO_TARGET_DESC
#include "CDMGenRegisterInfo.inc"
#include "CDMInstrInfo.h"

namespace llvm {

// TODO: what is this argument? IDK, on ARM it's LR, on x86 it is LR. WTF?
// I'l leave it as PC for now
CDMRegisterInfo::CDMRegisterInfo() : CDMGenRegisterInfo(CDM::PC) {}

BitVector CDMRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  static const std::array ReservedCPURegs{CDM::FP, CDM::SP, CDM::PSR, CDM::PC};

  for (unsigned I = 0; I < ReservedCPURegs.size(); ++I)
    Reserved.set(ReservedCPURegs[I]);

  return Reserved;
}
const MCPhysReg *
CDMRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  switch (MF->getFunction().getCallingConv()) {
  case CallingConv::C:
  case CallingConv::Fast:
  case CallingConv::Cold:
    return CSR_O16_SaveList;
  case CallingConv::CdmIsr:
    return CSR_O16_ALL_SaveList;
  }
  llvm_unreachable("Unknown calling convention");
}
bool CDMRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned int FIOperandNum,
                                          RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();

  unsigned I = 0;
  while (!MI.getOperand(I).isFI()) {
    ++I;
    assert(I < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  LLVM_DEBUG(errs() << "\nFunction : " << MF.getFunction().getName() << "\n";
             errs() << "<--------->\n"
                    << MI);

  int FrameIndex = MI.getOperand(I).getIndex();
  uint64_t StackSize = MF.getFrameInfo().getStackSize();
  int64_t SpOffset = MF.getFrameInfo().getObjectOffset(FrameIndex);

  LLVM_DEBUG(errs() << "FrameIndex : " << FrameIndex << "\n"
                    << "spOffset   : " << SpOffset << "\n"
                    << "stackSize  : " << StackSize << "\n");

  const auto IsInvalidByteFPRelIns = [](unsigned Op, int64_t Offset) {
    const std::unordered_set<unsigned> ByteFPRelIns = {CDM::ssb, CDM::lsb,
                                                       CDM::lssb};

    return ByteFPRelIns.count(Op) == 1 && (Offset > 63 || Offset < -64);
  };
  const auto IsInvalidWordFPRelIns = [](unsigned Op, int64_t Offset) {
    const std::unordered_set<unsigned> WordFPRelIns = {CDM::ssw, CDM::lsw};

    return WordFPRelIns.count(Op) == 1 && (Offset > 127 || Offset < -128);
  };

  // If the immediate operand of ssw, lsw, ssb, lsb, lssb
  // is out of bounds, perform a substitution
  if (IsInvalidByteFPRelIns(MI.getOpcode(), SpOffset) ||
      IsInvalidWordFPRelIns(MI.getOpcode(), SpOffset)) {
    if (!RS) {
      llvm_unreachable("RegScavenger must be initialised");
    }

    Register TmpReg, SrcReg;
    TmpReg =
        RS->scavengeRegisterBackwards(CDM::CPURegsRegClass, II, true, 2, true);
    RS->setRegUsed(TmpReg);

    auto *const It = std::find_if(
        MI.operands_begin(), MI.operands_end(),
        [](const MachineOperand &Operand) { return Operand.isReg(); });

    if (It == MI.operands_end())
      llvm_unreachable("Register MachineOperand wasn't found");

    SrcReg = It->getReg();

    MachineBasicBlock &MBB = *MI.getParent();
    BuildMI(MBB, II, II->getDebugLoc(),
            MF.getSubtarget().getInstrInfo()->get(CDM::ldi), TmpReg)
        .addImm(SpOffset);

    const std::unordered_map<unsigned, unsigned> SubstitutionOpcs = {
        {CDM::ssw, CDM::stwRR},   {CDM::lsw, CDM::ldwRR},
        {CDM::ssb, CDM::stbRR},   {CDM::lsb, CDM::ldbRR},
        {CDM::lssb, CDM::ldsbRR},
    };
    unsigned SubstitutionOpc = SubstitutionOpcs.at(MI.getOpcode());

    BuildMI(MBB, II, II->getDebugLoc(),
            MF.getSubtarget().getInstrInfo()->get(SubstitutionOpc))
        .addReg(SrcReg)
        .addReg(TmpReg)
        .addReg(MF.getSubtarget().getRegisterInfo()->getFrameRegister(MF))
        .addImm(0);

    MI.getParent()->erase(II);
    return true; // original instruction is removed
  }

  // TODO: acknowledge saved regs and other stuff
  // TODO: handle incoming arguments
  MI.getOperand(I).ChangeToImmediate(SpOffset);
  //  llvm_unreachable("Unimplemented");
  return false; // instruction not removed
}
Register CDMRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return CDM::FP;
}
const uint32_t *
CDMRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                      CallingConv::ID Id) const {
  switch (Id) {
  case CallingConv::C:
  case CallingConv::Fast:
  case CallingConv::Cold:
    return CSR_O16_RegMask;
  case CallingConv::CdmIsr:
    return CSR_O16_ALL_RegMask;
  }
  llvm_unreachable("Unknown calling convention");
}

} // namespace llvm
