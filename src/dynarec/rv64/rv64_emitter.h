#ifndef __RV64_EMITTER_H__
#define __RV64_EMITTER_H__
/*
    RV64 Emitter

*/

// RV64 ABI
/*
reg     name    description                     saver
------------------------------------------------------
x0      zero    Hard-wired zero                 —
x1      ra      Return address                  Caller
x2      sp      Stack pointer                   Callee
x3      gp      Global pointer                  —
x4      tp      Thread pointer                  —
x5–7    t0–2    Temporaries                     Caller
x8      s0/fp   Saved register/frame pointer    Callee
x9      s1      Saved register                  Callee
x10–11  a0–1    Function arguments/return val.  Caller
x12–17  a2–7    Function arguments              Caller
x18–27  s2–11   Saved registers                 Callee
x28–31  t3–6    Temporaries                     Caller
-------------------------------------------------------
f0–7    ft0–7   FP temporaries                  Caller
f8–9    fs0–1   FP saved registers              Callee
f10–11  fa0–1   FP arguments/return values      Caller
f12–17  fa2–7   FP arguments                    Caller
f18–27  fs2–11  FP saved registers              Callee
f28–31  ft8–11  FP temporaries                  Caller
*/
// x86 Register mapping
#define xRAX    16
#define xRCX    17
#define xRDX    18
#define xRBX    19
#define xRSP    20
#define xRBP    21
#define xRSI    22
#define xRDI    23
#define xR8     24
#define xR9     25
#define xR10    26
#define xR11    27
#define xR12    28
#define xR13    29
#define xR14    30
#define xR15    31
#define xFlags  8
#define xRIP    7

// 32bits version
#define wEAX    xRAX
#define wECX    xRCX
#define wEDX    xRDX
#define wEBX    xRBX
#define wESP    xRSP
#define wEBP    xRBP
#define wESI    xRSI
#define wEDI    xRDI
#define wR8     xR8
#define wR9     xR9
#define wR10    xR10
#define wR11    xR11
#define wR12    xR12
#define wR13    xR13
#define wR14    xR14
#define wR15    xR15
#define wFlags  xFlags
// scratch registers
#define x1      11
#define x2      12
#define x3      13
#define x4      14
#define x5      15
#define x6      6
#define x9      9
// used to clear the upper 32bits
#define xMASK   5
// 32bits version of scratch
#define w1      x1
#define w2      x2
#define w3      x3
#define w4      x4
#define w5      x5
#define w6      x6
// emu is r10
#define xEmu    10
// RV64 RA
#define xRA     1
#define xSP     2
// RV64 args
#define A0      10
#define A1      11
#define A2      12
#define A3      13
#define A4      14
#define A5      15
#define A6      16
#define A7      17
// xZR reg is 0
#define xZR     0
#define wZR     xZR

// replacement for F_OF internaly, using a reserved bit. Need to use F_OF2 internaly, never F_OF directly!
#define F_OF2   F_res3

// split a 32bits value in 20bits + 12bits, adjust the upper part is 12bits is negative
#define SPLIT20(A)  (((A)+0x800)>>12)
#define SPLIT12(A)  ((A)&0xfff)

// MOV64x/MOV32w is quite complex, so use a function for this
#define MOV64x(A, B)    rv64_move64(dyn, ninst, A, B)
#define MOV32w(A, B)    rv64_move32(dyn, ninst, A, B, 1)
#define MOV64xw(A, B)   if(rex.w) {MOV64x(A, B);} else {MOV32w(A, B);}

// ZERO the upper part
#define ZEROUP(r)       AND(r, r, xMASK)

#define R_type(funct7, rs2, rs1, funct3, rd, opcode)    ((funct7)<<25 | (rs2)<<20 | (rs1)<<15 | (funct3)<<12 | (rd)<<7 | (opcode))
#define I_type(imm12, rs1, funct3, rd, opcode)    ((imm12)<<20 | (rs1)<<15 | (funct3)<<12 | (rd)<<7 | (opcode))
#define S_type(imm12, rs2, rs1, funct3, opcode)    (((imm12)>>5)<<25 | (rs2)<<20 | (rs1)<<15 | (funct3)<<12 | ((imm12)&31)<<7 | (opcode))
#define B_type(imm13, rs2, rs1, funct3, opcode)      ((((imm13)>>12)&1)<<31 | (((imm13)>>5)&63)<<25 | (rs2)<<20 | (rs1)<<15 | (funct3)<<12 | (((imm13)>>1)&15)<<8 | (((imm13)>>11)&1)<<7 | (opcode))
#define U_type(imm32, rd, opcode)   (((imm32)>>12)<<12 | (rd)<<7 | (opcode))
#define J_type(imm21, rd, opcode)    ((((imm21)>>20)&1)<<31 | (((imm21)>>1)&0b1111111111)<<21 | (((imm21)>>11)&1)<<20 | (((imm21)>>12)&0b11111111)<<12 | (rd)<<7 | (opcode))

// RV32I
// put imm20 in the [31:12] bits of rd, zero [11:0] and sign extend bits31
#define LUI(rd, imm20)                 EMIT(U_type((imm20)<<12, rd, 0b0110111))
// put PC+imm20 in rd
#define AUIPC(rd, imm20)               EMIT(U_type((imm20)<<12, rd, 0b0010111))

#define JAL_gen(rd, imm21)             J_type(imm21, rd, 0b1101111)
// Unconditional branch, no return address set
#define B(imm21)                       EMIT(JAL_gen(xZR, imm21))
#define B__(reg1, reg2, imm21)         B(imm21)
// Unconditional branch, return set to xRA
#define JAL(imm21)                     EMIT(JAL_gen(xRA, imm21))
// Unconditional branch, without link
#define J(imm21)                       EMIT(JAL_gen(xZR, imm21))

#define JALR_gen(rd, rs1, imm12)       I_type(imm12, rs1, 0b000, rd, 0b1100111)
// Unconditionnal branch to r, no return address set
#define BR(r)                          EMIT(JALR_gen(xZR, r, 0))
// Unconditionnal branch to r+i12, no return address set
#define BR_I12(r, imm12)               EMIT(JALR_gen(xZR, r, (imm12)&0b111111111111))
// Unconditionnal branch to r, return address set to xRA
#define JALR(r)                        EMIT(JALR_gen(xRA, r, 0))
// Unconditionnal branch to r+i12, return address set to xRA
#define JALR_I12(r, imm12)             EMIT(JALR_gen(xRA, r, (imm12)&0b111111111111))

// rd = rs1 + imm12
#define ADDI(rd, rs1, imm12)        EMIT(I_type((imm12)&0b111111111111, rs1, 0b000, rd, 0b0010011))
// rd = rs1 - imm12 (pseudo instruction)
#define SUBI(rd, rs1, imm12)        EMIT(I_type((-(imm12))&0b111111111111, rs1, 0b000, rd, 0b0010011))
// rd = (rs1<imm12)?1:0
#define SLTI(rd, rs1, imm12)        EMIT(I_type((imm12)&0b111111111111, rs1, 0b010, rd, 0b0010011))
// rd = (rs1<imm12)?1:0 unsigned
#define SLTIU(rd, rs1, imm12)       EMIT(I_type((imm12)&0b111111111111, rs1, 0b011, rd, 0b0010011))
// rd = rs1 ^ imm12
#define XORI(rd, rs1, imm12)        EMIT(I_type((imm12)&0b111111111111, rs1, 0b100, rd, 0b0010011))
// rd = rs1 | imm12
#define ORI(rd, rs1, imm12)         EMIT(I_type((imm12)&0b111111111111, rs1, 0b110, rd, 0b0010011))
// rd = rs1 & imm12
#define ANDI(rd, rs1, imm12)        EMIT(I_type((imm12)&0b111111111111, rs1, 0b111, rd, 0b0010011))

// rd = imm12
#define MOV_U12(rd, imm12)          ADDI(rd, xZR, imm12)
// nop
#define NOP()                       ADDI(xZR, xZR, 0)

// rd = rs1 + rs2
#define ADD(rd, rs1, rs2)           EMIT(R_type(0b0000000, rs2, rs1, 0b000, rd, 0b0110011))
// rd = rs1 + rs2
#define ADDW(rd, rs1, rs2)          EMIT(R_type(0b0000000, rs2, rs1, 0b000, rd, 0b0111011))
// rd = rs1 + rs2
#define ADDxw(rd, rs1, rs2)         EMIT(R_type(0b0000000, rs2, rs1, 0b000, rd, rex.w?0b0110011:0b0111011))
// rd = rs1 - rs2
#define SUB(rd, rs1, rs2)           EMIT(R_type(0b0100000, rs2, rs1, 0b000, rd, 0b0110011))
// rd = rs1 - rs2
#define SUBW(rd, rs1, rs2)          EMIT(R_type(0b0100000, rs2, rs1, 0b000, rd, 0b0111011))
// rd = rs1 - rs2
#define SUBxw(rd, rs1, rs2)         EMIT(R_type(0b0100000, rs2, rs1, 0b000, rd, rex.w?0b0110011:0b0111011))
// rd = rs1<<rs2
#define SLL(rd, rs1, rs2)           EMIT(R_type(0b0000000, rs2, rs1, 0b001, rd, 0b0110011))
// rd = (rs1<rs2)?1:0
#define SLT(rd, rs1, rs2)           EMIT(R_type(0b0000000, rs2, rs1, 0b010, rd, 0b0110011))
// rd = (rs1<rs2)?1:0 Unsigned
#define SLTU(rd, rs1, rs2)          EMIT(R_type(0b0000000, rs2, rs1, 0b011, rd, 0b0110011))
// rd = rs1 ^ rs2
#define XOR(rd, rs1, rs2)           EMIT(R_type(0b0000000, rs2, rs1, 0b100, rd, 0b0110011))
// rd = rs1 ^ rs2
#define XORxw(rd, rs1, rs2)         do{ XOR(rd, rs1, rs2); if (!rex.w) ZEROUP(rd); }while(0)
// rd = rs1>>rs2 logical
#define SRL(rd, rs1, rs2)           EMIT(R_type(0b0000000, rs2, rs1, 0b101, rd, 0b0110011))
// rd = rs1>>rs2 arithmetic
#define SRA(rd, rs1, rs2)           EMIT(R_type(0b0100000, rs2, rs1, 0b101, rd, 0b0110011))
// rd = rs1 | rs2
#define OR(rd, rs1, rs2)            EMIT(R_type(0b0000000, rs2, rs1, 0b110, rd, 0b0110011))
// rd = rs1 & rs2
#define AND(rd, rs1, rs2)           EMIT(R_type(0b0000000, rs2, rs1, 0b111, rd, 0b0110011))

// rd = rs1 (pseudo instruction)
#define MV(rd, rs1)                 ADDI(rd, rs1, 0)
// rd = rs1 (pseudo instruction)
#define MVxw(rd, rs1)               if(rex.w) {MV(rd, rs1); } else {AND(rd, rs1, xMASK);}
// rd = !rs1
#define NOT(rd, rs1)                XORI(rd, rs1, -1)
// rd = -rs1
#define NEG(rd, rs1)                SUB(rd, xZR, rs1)
// rd = -rs1
#define NEGxw(rd, rs1)              SUBxw(rd, xZR, rs1)
// rd = rs1 == 0
#define SEQZ(rd, rs1)               SLTIU(rd, rs1, 1)
// rd = rs1 != 0
#define SNEZ(rd, rs1)               SLTU(rd, xZR, rs1)


#define BEQ(rs1, rs2, imm13)       EMIT(B_type(imm13, rs2, rs1, 0b000, 0b1100011))
#define BNE(rs1, rs2, imm13)       EMIT(B_type(imm13, rs2, rs1, 0b001, 0b1100011))
#define BLT(rs1, rs2, imm13)       EMIT(B_type(imm13, rs2, rs1, 0b100, 0b1100011))
#define BGE(rs1, rs2, imm13)       EMIT(B_type(imm13, rs2, rs1, 0b101, 0b1100011))
#define BLTU(rs1, rs2, imm13)      EMIT(B_type(imm13, rs2, rs1, 0b110, 0b1100011))
#define BGEU(rs1, rs2, imm13)      EMIT(B_type(imm13, rs2, rs1, 0b111, 0b1100011))

// TODO: Find a better way to have conditionnal jumps? Imm is a relative jump address, so the the 2nd jump needs to be addapted
#define BEQ_safe(rs1, rs2, imm)     if((imm)>-0x1000 && (imm)<0x1000) {BEQ(rs1, rs2, imm); NOP();} else {BNE(rs1, rs2, 8); B(imm-4);}
#define BNE_safe(rs1, rs2, imm)     if((imm)>-0x1000 && (imm)<0x1000) {BNE(rs1, rs2, imm); NOP();} else {BEQ(rs1, rs2, 8); B(imm-4);}
#define BLT_safe(rs1, rs2, imm)     if((imm)>-0x1000 && (imm)<0x1000) {BLT(rs1, rs2, imm); NOP();} else {BGE(rs2, rs1, 8); B(imm-4);}
#define BGE_safe(rs1, rs2, imm)     if((imm)>-0x1000 && (imm)<0x1000) {BGE(rs1, rs2, imm); NOP();} else {BLT(rs2, rs1, 8); B(imm-4);}
#define BLTU_safe(rs1, rs2, imm)    if((imm)>-0x1000 && (imm)<0x1000) {BLTU(rs1, rs2, imm); NOP();} else {BGEU(rs2, rs1, 8); B(imm-4);}
#define BGEU_safe(rs1, rs2, imm)    if((imm)>-0x1000 && (imm)<0x1000) {BGEU(rs1, rs2, imm); NOP();} else {BLTU(rs2, rs1, 8); B(imm-4);}

#define BEQZ(rs1, imm13)           BEQ(rs1, 0, imm13)
#define BNEZ(rs1, imm13)           BNE(rs1, 0, imm13)

#define BEQZ_safe(rs1, imm)         if((imm)>-0x1000 && (imm)<0x1000) {BEQZ(rs1, imm); NOP();} else {BNEZ(rs1, 8); B(imm-4);}
#define BNEZ_safe(rs1, imm)         if((imm)>-0x1000 && (imm)<0x1000) {BNEZ(rs1, imm); NOP();} else {BEQZ(rs1, 8); B(imm-4);}

// rd = 4-bytes[rs1+imm12] signed extended
#define LW(rd, rs1, imm12)          EMIT(I_type(imm12, rs1, 0b010, rd, 0b0000011))
// rd = 2-bytes[rs1+imm12] signed extended
#define LH(rd, rs1, imm12)          EMIT(I_type(imm12, rs1, 0b001, rd, 0b0000011))
// rd = byte[rs1+imm12] signed extended
#define LB(rd, rs1, imm12)          EMIT(I_type(imm12, rs1, 0b000, rd, 0b0000011))
// rd = 2-bytes[rs1+imm12] zero extended
#define LHU(rd, rs1, imm12)         EMIT(I_type(imm12, rs1, 0b101, rd, 0b0000011))
// rd = byte[rs1+imm12] zero extended
#define LBU(rd, rs1, imm12)         EMIT(I_type(imm12, rs1, 0b100, rd, 0b0000011))
// byte[rs1+imm12] = rs2
#define SB(rs2, rs1, imm12)         EMIT(S_type(imm12, rs2, rs1, 0b000, 0b0100011))
// 2-bytes[rs1+imm12] = rs2
#define SH(rs2, rs1, imm12)         EMIT(S_type(imm12, rs2, rs1, 0b001, 0b0100011))
// 4-bytes[rs1+imm12] = rs2
#define SW(rs2, rs1, imm12)         EMIT(S_type(imm12, rs2, rs1, 0b010, 0b0100011))

#define PUSH1(reg)                  do {SD(reg, xRSP, -8); SUBI(xRSP, xRSP, 8);} while(0)
#define POP1(reg)                   do {LD(reg, xRSP, 0); ADDI(xRSP, xRSP, 8);}while(0)

#define FENCE_gen(pred, succ)       (((pred)<<24) | ((succ)<<20) | 0b0001111)
#define FENCE()                     EMIT(FENCE_gen(3, 3))

#define FENCE_I_gen()               ((0b001<<12) | 0b0001111)
#define FENCE_I()                   EMIT(FENCE_I_gen())

#define EBREAK()                    EMIT(I_type(1, 0, 0, 0, 0b1110011))

// RV64I
#define LWU(rd, rs1, imm12)         EMIT(I_type(imm12, rs1, 0b110, rd, 0b0000011))

// rd = [rs1 + imm12]
#define LD(rd, rs1, imm12)          EMIT(I_type(imm12, rs1, 0b011, rd, 0b0000011))
// rd = [rs1 + imm12]
#define LDxw(rd, rs1, imm12)        EMIT(I_type(imm12, rs1, 0b011<<(1-rex.w), rd, 0b0000011))
// [rs1 + imm12] = rs2
#define SD(rs2, rs1, imm12)         EMIT(S_type(imm12, rs2, rs1, 0b011, 0b0100011))
// [rs1 + imm12] = rs2
#define SDxw(rs2, rs1, imm12)       EMIT(S_type(imm12, rs2, rs1, 0b010+rex.w, 0b0100011))

// Shift Left Immediate
#define SLLI(rd, rs1, imm6)         EMIT(I_type(imm6, rs1, 0b001, rd, 0b0010011))
// Shift Right Logical Immediate
#define SRLI(rd, rs1, imm6)         EMIT(I_type(imm6, rs1, 0b101, rd, 0b0010011))
// Shift Right Arithmetic Immediate
#define SRAI(rd, rs1, imm6)         EMIT(I_type((imm6)|(0b010000<<6), rs1, 0b101, rd, 0b0010011))

// rd = rs1 + imm12
#define ADDIW(rd, rs1, imm12)       EMIT(I_type((imm12)&0b111111111111, rs1, 0b000, rd, 0b0011011))
// rd = rs1 + imm12
#define ADDIxw(rd, rs1, imm12)      EMIT(I_type((imm12)&0b111111111111, rs1, 0b000, rd, rex.w?0b0010011:0b0011011))

#define SEXT_W(rd, rs1)             ADDIW(rd, rs1, 0)

// rd = rs1<<rs2
#define SLLW(rd, rs1, rs2)           EMIT(R_type(0b0000000, rs2, rs1, 0b001, rd, 0b0111011))
// rd = rs1>>rs2 logical
#define SRLW(rd, rs1, rs2)           EMIT(R_type(0b0000000, rs2, rs1, 0b101, rd, 0b0111011))
// rd = rs1>>rs2 arithmetic
#define SRAW(rd, rs1, rs2)           EMIT(R_type(0b0100000, rs2, rs1, 0b101, rd, 0b0111011))

#define SLLxw(rd, rs1, rs2)          EMIT(R_type(0b0000000, rs2, rs1, 0b001, rd, rex.w?0b0110011:0b0111011))
#define SRLxw(rd, rs1, rs2)          EMIT(R_type(0b0000000, rs2, rs1, 0b101, rd, rex.w?0b0110011:0b0111011))
#define SRAxw(rd, rs1, rs2)          if(rex.w) {SRA(rd, rs1, rs2);} else {SRAW(rd, rs1, rs2); ZEROUP(rd);}

// Shift Left Immediate, 32-bit, sign-extended
#define SLLIW(rd, rs1, imm5)        EMIT(I_type(imm5, rs1, 0b001, rd, 0b0011011))
// Shift Left Immediate
#define SLLIxw(rd, rs1, imm)        if (rex.w) { SLLI(rd, rs1, imm); } else { SLLIW(rd, rs1, imm); }
// Shift Right Logical Immediate, 32-bit, sign-extended
#define SRLIW(rd, rs1, imm5)        EMIT(I_type(imm5, rs1, 0b101, rd, 0b0011011))
// Shift Right Logical Immediate
#define SRLIxw(rd, rs1, imm)        if (rex.w) { SRLI(rd, rs1, imm); } else { SRLIW(rd, rs1, imm); }
// Shift Right Arithmetic Immediate, 32-bit, sign-extended
#define SRAIW(rd, rs1, imm5)        EMIT(I_type((imm5)|(0b0100000<<5), rs1, 0b101, rd, 0b0011011))
// Shift Right Arithmetic Immediate
#define SRAIxw(rd, rs1, imm)        if (rex.w) { SRAI(rd, rs1, imm); } else { SRAIW(rd, rs1, imm); }

#define CSRRW(rd, rs1, csr)         EMIT(I_type(csr, rs1, 0b001, rd, 0b1110011))
#define CSRRS(rd, rs1, csr)         EMIT(I_type(csr, rs1, 0b010, rd, 0b1110011))
#define CSRRC(rd, rs1, csr)         EMIT(I_type(csr, rs1, 0b011, rd, 0b1110011))
#define CSRRWI(rd, imm, csr)        EMIT(I_type(csr, imm, 0b101, rd, 0b1110011))
#define CSRRSI(rd, imm, csr)        EMIT(I_type(csr, imm, 0b110, rd, 0b1110011))
#define CSRRCI(rd, imm, csr)        EMIT(I_type(csr, imm, 0b111, rd, 0b1110011))

// RV32M
// rd =(lower) rs1 * rs2 (both signed)
#define MUL(rd, rs1, rs2)           EMIT(R_type(0b0000001, rs2, rs1, 0b000, rd, 0b0110011))
// rd =(upper) rs1 * rs2 (both signed)
#define MULH(rd, rs1, rs2)          EMIT(R_type(0b0000001, rs2, rs1, 0b001, rd, 0b0110011))
// rd =(upper) (signed)rs1 * (unsigned)rs2
#define MULHSU(rd, rs1, rs2)        EMIT(R_type(0b0000001, rs2, rs1, 0b010, rd, 0b0110011))
// rd =(upper) rs1 * rs2 (both unsigned)
#define MULHU(rd, rs1, rs2)         EMIT(R_type(0b0000001, rs2, rs1, 0b011, rd, 0b0110011))
// rd =(upper) rs1 / rs2
#define DIV(rd, rs1, rs2)           EMIT(R_type(0b0000001, rs2, rs1, 0b100, rd, 0b0110011))
#define DIVU(rd, rs1, rs2)          EMIT(R_type(0b0000001, rs2, rs1, 0b101, rd, 0b0110011))
// rd = rs1 mod rs2
#define REM(rd, rs1, rs2)           EMIT(R_type(0b0000001, rs2, rs1, 0b110, rd, 0b0110011))
#define REMU(rd, rs1, rs2)          EMIT(R_type(0b0000001, rs2, rs1, 0b111, rd, 0b0110011))

// RV64M
// rd = rs1 * rs2
#define MULW(rd, rs1, rs2)          EMIT(R_type(0b0000001, rs2, rs1, 0b000, rd, 0b0111011))
// rd = rs1 * rs2
#define MULxw(rd, rs1, rs2)         EMIT(R_type(0b0000001, rs2, rs1, 0b000, rd, rex.w?0b0110011:0b0111011))
// rd = rs1 / rs2
#define DIVW(rd, rs1, rs2)          EMIT(R_type(0b0000001, rs2, rs1, 0b100, rd, 0b0111011))
#define DIVxw(rd, rs1, rs2)         EMIT(R_type(0b0000001, rs2, rs1, 0b100, rd, rex.w?0b0110011:0b0111011))
#define DIVUW(rd, rs1, rs2)         EMIT(R_type(0b0000001, rs2, rs1, 0b101, rd, 0b0111011))
#define DIVUxw(rd, rs1, rs2)        EMIT(R_type(0b0000001, rs2, rs1, 0b101, rd, rex.w?0b0110011:0b0111011))
// rd = rs1 mod rs2
#define REMW(rd, rs1, rs2)          EMIT(R_type(0b0000001, rs2, rs1, 0b110, rd, 0b0111011))
#define REMxw(rd, rs1, rs2)         EMIT(R_type(0b0000001, rs2, rs1, 0b110, rd, rex.w?0b0110011:0b0111011))
#define REMUW(rd, rs1, rs2)         EMIT(R_type(0b0000001, rs2, rs1, 0b111, rd, 0b0111011))
#define REMUxw(rd, rs1, rs2)        EMIT(R_type(0b0000001, rs2, rs1, 0b111, rd, rex.w?0b0110011:0b0111011))

#define AQ_RL(f5, aq, rl) ((f5 << 2) | ((aq&1) << 1) | (rl&1))

// RV32A
#define LR_W(rd, rs1, aq, rl)       EMIT(R_type(AQ_RL(0b00010, aq, rl), 0, rs1, 0b010, rd, 0b0101111))
#define SC_W(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b00011, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))

// RV64A
#define LR_D(rd, rs1, aq, rl)       EMIT(R_type(AQ_RL(0b00010, aq, rl), 0, rs1, 0b011, rd, 0b0101111))
#define SC_D(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b00011, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))

#define LRxw(rd, rs1, aq, rl)       EMIT(R_type(AQ_RL(0b00010, aq, rl), 0, rs1, 0b010|rex.w, rd, 0b0101111))
#define SCxw(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b00011, aq, rl), rs2, rs1, 0b010|rex.w, rd, 0b0101111))

// RV32F
// Read round mode
#define FRRM(rd)                    CSRRS(rd, xZR, 0x002)
// Swap round mode
#define FSRM(rd, rs)                CSRRW(rd, rs, 0x002)
// Write FP exception flags, immediate
#define FSFLAGSI(imm)               CSRRWI(xZR, imm, 0x0001)
// Read  FP exception flags to rd
#define FRFLAGS(rd)                 CSRRS(rd, xZR, 0x0001)
// Inexact
#define FR_NX   0
// Underflow
#define FR_UF   1
// Overflow
#define FR_OF   2
// Divide by Zero
#define FR_DZ   3
// Invalid Operation
#define FR_NV   4

// Round to Nearest, ties to Even
#define RD_RNE      0b000
// Round towards Zero
#define RD_RTZ      0b001
// Round Down (towards −∞)
#define RD_RDN      0b010
// Round Up (towards +∞)
#define RD_RUP      0b011
// Round to Nearest, ties to Max Magnitude
#define RD_RMM      0b100
// In instruction’s rm field, selects dynamic rounding mode;
#define RD_RM       0b111
#define RD_DYN      RD_RM

// load single precision from rs1+imm12 to frd
#define FLW(frd, rs1, imm12)        EMIT(I_type(imm12, rs1, 0b010, frd, 0b0000111))
// store single precision frs2 to rs1+imm12
#define FSW(frs2, rs1, imm12)       EMIT(S_type(imm12, frs2, rs1, 0b010, 0b0100111))
// store rs1 with rs2 sign bit to rd
#define FSGNJS(rd, rs1, rs2)        EMIT(R_type(0b0010000, rs2, rs1, 0b000, rd, 0b1010011))
// move rs1 to rd
#define FMVS(rd, rs1)               FSGNJS(rd, rs1, rs1)
// store rs1 with oposite rs2 sign bit to rd
#define FSGNJNS(rd, rs1, rs2)       EMIT(R_type(0b0010000, rs2, rs1, 0b001, rd, 0b1010011))
// -rs1 => rd
#define FNEGS(rd, rs1)              FSGNJNS(rd, rs1, rs1)
// store rs1 with rs1^rs2 sign bit to rd
#define FSGNJXS(rd, rs1, rs2)       EMIT(R_type(0b0010000, rs2, rs1, 0b010, rd, 0b1010011))
// |rs1| => rd
#define FABSS(rd, rs1)              FSGNJXS(rd, rs1, rs1)
// Move from Single
#define FMVXW(rd, frs1)             EMIT(R_type(0b1110000, 0b00000, frs1, 0b000, rd, 0b1010011))
// Move to Single
#define FMVWX(frd, rs1)             EMIT(R_type(0b1111000, 0b00000, rs1, 0b000, frd, 0b1010011))
// Convert from signed 32bits to Single
#define FCVTSW(frd, rs1, rm)        EMIT(R_type(0b1101000, 0b00000, rs1, rm, frd, 0b1010011))
// Convert from Single to signed 32bits (trucated)
#define FCVTWS(rd, frs1, rm)        EMIT(R_type(0b1100000, 0b00000, frs1, rm, rd, 0b1010011))

#define FADDS(frd, frs1, frs2)      EMIT(R_type(0b0000000, frs2, frs1, 0b000, frd, 0b1010011))
#define FSUBS(frd, frs1, frs2)      EMIT(R_type(0b0000100, frs2, frs1, 0b000, frd, 0b1010011))
#define FMULS(frd, frs1, frs2)      EMIT(R_type(0b0001000, frs2, frs1, 0b000, frd, 0b1010011))
#define FDIVS(frd, frs1, frs2)      EMIT(R_type(0b0001100, frs2, frs1, 0b000, frd, 0b1010011))
#define FSQRTS(frd, frs1)           EMIT(R_type(0b0101100, 0b00000, frs1, 0b000, frd, 0b1010011))
#define FMINS(frd, frs1, frs2)      EMIT(R_type(0b0010100, frs2, frs1, 0b000, frd, 0b1010011))
#define FMAXS(frd, frs1, frs2)      EMIT(R_type(0b0010100, frs2, frs1, 0b001, frd, 0b1010011))

// compare
#define FEQS(rd, frs1, frs2)        EMIT(R_type(0b1010000, frs2, frs1, 0b010, rd, 0b1010011))
#define FLTS(rd, frs1, frs2)        EMIT(R_type(0b1010000, frs2, frs1, 0b001, rd, 0b1010011))
#define FLES(rd, frs1, frs2)        EMIT(R_type(0b1010000, frs2, frs1, 0b000, rd, 0b1010011))

// RV64F
// Convert from signed 64bits to Single
#define FCVTSL(frd, rs1, rm)        EMIT(R_type(0b1101000, 0b00010, rs1, rm, frd, 0b1010011))
// Convert from unsigned 64bits to Single
#define FCVTSLU(frd, rs1, rm)       EMIT(R_type(0b1101000, 0b00011, rs1, rm, frd, 0b1010011))
// Convert from Single to signed 64bits
#define FCVTLS(rd, frs1, rm)        EMIT(R_type(0b1100000, 0b00010, frs1, rm, rd, 0b1010011))
// Convert from Single to unsigned 64bits
#define FCVTLUS(rd, frs1, rm)       EMIT(R_type(0b1100000, 0b00011, frs1, rm, rd, 0b1010011))
// onvert from Single to signed 32/64bits (trucated)
#define FCVTSxw(rd, frs1, rm)       EMIT(R_type(0b1100000, rex.w?0b00010:0b00000, frs1, rm, rd, 0b1010011))

// RV32D
// load double precision from rs1+imm12 to frd
#define FLD(frd, rs1, imm12)        EMIT(I_type(imm12, rs1, 0b011, frd, 0b0000111))
// store double precision frs2 to rs1+imm12
#define FSD(frs2, rs1, imm12)       EMIT(S_type(imm12, frs2, rs1, 0b011, 0b0100111))
// Convert Double frs1 to Single frd
#define FCVTSD(frd, frs1)           EMIT(R_type(0b0100000, 0b00001, frs1, 0b000, frd, 0b1010011))
// Convert Single frs1 to Double frd
#define FCVTDS(frd, frs1)           EMIT(R_type(0b0100001, 0b00000, frs1, 0b000, frd, 0b1010011))
// Convert from Double to signed 32bits
#define FCVTWD(rd, frs1, rm)        EMIT(R_type(0b1100001, 0b00000, frs1, rm, rd, 0b1010011))
// Convert from Double to unsigned 32bits
#define FCVTWUD(rd, frs1, rm)       EMIT(R_type(0b1100001, 0b00001, frs1, rm, rd, 0b1010011))
// store rs1 with rs2 sign bit to rd
#define FSGNJD(rd, rs1, rs2)        EMIT(R_type(0b0010001, rs2, rs1, 0b000, rd, 0b1010011))
// move rs1 to rd
#define FMVD(rd, rs1)               FSGNJD(rd, rs1, rs1)
// store rs1 with oposite rs2 sign bit to rd
#define FSGNJND(rd, rs1, rs2)       EMIT(R_type(0b0010001, rs2, rs1, 0b001, rd, 0b1010011))
// -rs1 => rd
#define FNEGD(rd, rs1)              FSGNJND(rd, rs1, rs1)
// store rs1 with rs1^rs2 sign bit to rd
#define FSGNJXD(rd, rs1, rs2)       EMIT(R_type(0b0010001, rs2, rs1, 0b010, rd, 0b1010011))
// |rs1| => rd
#define FABSD(rd, rs1)              FSGNJXD(rd, rs1, rs1)
// Convert from signed 32bits to Double
#define FCVTDW(frd, rs1, rm)        EMIT(R_type(0b1101001, 0b00000, rs1, rm, frd, 0b1010011))

#define FEQD(rd, frs1, frs2)        EMIT(R_type(0b1010001, frs2, frs1, 0b010, rd, 0b1010011))
#define FLTD(rd, frs1, frs2)        EMIT(R_type(0b1010001, frs2, frs1, 0b001, rd, 0b1010011))
#define FLED(rd, frs1, frs2)        EMIT(R_type(0b1010001, frs2, frs1, 0b000, rd, 0b1010011))

#define FADDD(frd, frs1, frs2)      EMIT(R_type(0b0000001, frs2, frs1, 0b000, frd, 0b1010011))
#define FSUBD(frd, frs1, frs2)      EMIT(R_type(0b0000101, frs2, frs1, 0b000, frd, 0b1010011))
#define FMULD(frd, frs1, frs2)      EMIT(R_type(0b0001001, frs2, frs1, 0b000, frd, 0b1010011))
#define FDIVD(frd, frs1, frs2)      EMIT(R_type(0b0001101, frs2, frs1, 0b000, frd, 0b1010011))
#define FSQRTD(frd, frs1)           EMIT(R_type(0b0101101, 0b00000, frs1, 0b000, frd, 0b1010011))
#define FMIND(frd, frs1, frs2)      EMIT(R_type(0b0010101, frs2, frs1, 0b000, frd, 0b1010011))
#define FMAXD(frd, frs1, frs2)      EMIT(R_type(0b0010101, frs2, frs1, 0b001, frd, 0b1010011))

//RV64D
// Move from Double
#define FMVXD(rd, frs1)             EMIT(R_type(0b1110001, 0b00000, frs1, 0b000, rd, 0b1010011))
// Move to Double
#define FMVDX(frd, rs1)             EMIT(R_type(0b1111001, 0b00000, rs1, 0b000, frd, 0b1010011))
// Convert from signed 64bits to Double
#define FCVTDL(frd, rs1, rm)        EMIT(R_type(0b1101001, 0b00010, rs1, rm, frd, 0b1010011))
// Convert from unsigned 64bits to Double
#define FCVTDLU(frd, rs1, rm)       EMIT(R_type(0b1101001, 0b00011, rs1, rm, frd, 0b1010011))
// Convert from Double to signed 64bits
#define FCVTLD(rd, frs1, rm)        EMIT(R_type(0b1100001, 0b00010, frs1, rm, rd, 0b1010011))
// Convert from Double to unsigned 64bits
#define FCVTLUD(rd, frs1, rm)       EMIT(R_type(0b1100001, 0b00011, frs1, rm, rd, 0b1010011))

// Convert from Double to signed integer
#define FCVTLDxw(rd, frs1, rm)      EMIT(R_type(0b1100001, 0b00000+(rex.w?0b10:0b00), frs1, rm, rd, 0b1010011))
// Convert from Double to unsigned integer
#define FCVTLUDxw(rd, frs1, rm)     EMIT(R_type(0b1100001, 0b00001+(rex.w?0b10:0b00), frs1, rm, rd, 0b1010011))

//Zba
// Add unsigned word (Wz(rs1) + X(rs2))
#define ADDUW(rd, rs1, rs2)         EMIT(R_type(0b0000100, rs2, rs1, 0b000, rd, 0b0111011))
// Zero-extend Word
#define ZEXTW(rd, rs1)              ADDUW(rd, rs1, xZR)
// Shift left by 1 and add (rd = X(rs2) + X(rs1)<<1)
#define SH1ADD(rd, rs1, rs2)        EMIT(R_type(0b0010000, rs2, rs1, 0b010, rd, 0b0110011))
// Shift unsigned word left by 1 and add (rd = X(rs2) + Wz(rs1)<<1)
#define SH1ADDUW(rd, rs1, rs2)      EMIT(R_type(0b0010000, rs2, rs1, 0b010, rd, 0b0111011))
// Shift left by 2 and add (rd = X(rs2) + X(rs1)<<2)
#define SH2ADD(rd, rs1, rs2)        EMIT(R_type(0b0010000, rs2, rs1, 0b100, rd, 0b0110011))
// Shift unsigned word left by 2 and add (rd = X(rs2) + Wz(rs1)<<2)
#define SH2ADDUW(rd, rs1, rs2)      EMIT(R_type(0b0010000, rs2, rs1, 0b100, rd, 0b0111011))
// Shift left by 3 and add (rd = X(rs2) + X(rs1)<<3)
#define SH3ADD(rd, rs1, rs2)        EMIT(R_type(0b0010000, rs2, rs1, 0b110, rd, 0b0110011))
// Shift unsigned word left by 3 and add (rd = X(rs2) + Wz(rs1)<<3)
#define SH3ADDUW(rd, rs1, rs2)      EMIT(R_type(0b0010000, rs2, rs1, 0b110, rd, 0b0111011))
// Shift left unsigned word (immediate)
#define SLLIUW(rd, rs1, imm)        EMIT(R_type(0b0000100, imm, rs1, 0b001, rd, 0b0011011))
// Shift left by 1,2 or 3 and add (rd = X(rs2) + X(rs1)<<x)
#define SHxADD(rd, rs1, x, rs2)        EMIT(R_type(0b0010000, rs2, rs1, (x)<<1, rd, 0b0110011))
// Shift unsigned word left by 1,2 or 3 and add (rd = X(rs2) + Wz(rs1)<<x)
#define SHxADDUW(rd, rs1, x, rs2)      EMIT(R_type(0b0010000, rs2, rs1, (x)<<1, rd, 0b0111011))

//Zbb
// AND with reverted operand (rs1 & ~rs2)
#define ANDN(rd, rs1, rs2)      EMIT(R_type(0b0100000, rs2, rs1, 0b111, rd, 0b0110011))
// OR with reverted operand (rs1 | ~rs2)
#define ORN(rd, rs1, rs2)       EMIT(R_type(0b0100000, rs2, rs1, 0b110, rd, 0b0110011))
// Exclusive NOR (~(rs1 ^ rs2))
#define XNOR(rd, rs1, rs2)      EMIT(R_type(0b0100000, rs2, rs1, 0b100, rd, 0b0110011))
// Count leading zero bits
#define CLZ(rd, rs)             EMIT(R_type(0b0110000, 0b00000, rs, 0b001, rd, 0b0010011))
// Count leading zero bits in word
#define CLZW(rd, rs)            EMIT(R_type(0b0110000, 0b00000, rs, 0b001, rd, 0b0011011))
// Count leading zero bits
#define CLZxw(rd, rs)           EMIT(R_type(0b0110000, 0b00000, rs, 0b001, rd, rex.w?0b0010011:0b0011011))
// Count trailing zero bits
#define CTZ(rd, rs)             EMIT(R_type(0b0110000, 0b00001, rs, 0b001, rd, 0b0010011))
// Count trailing zero bits in word
#define CTZW(rd, rs)            EMIT(R_type(0b0110000, 0b00001, rs, 0b001, rd, 0b0011011))
// Count trailing zero bits
#define CTZxw(rd, rs)           EMIT(R_type(0b0110000, 0b00001, rs, 0b001, rd, rex.w?0b0010011:0b0011011))
// Count set bits
#define CPOP(rd, rs)            EMIT(R_type(0b0110000, 0b00010, rs, 0b001, rd, 0b0010011))
// Count set bits in word
#define CPOPW(rd, rs)           EMIT(R_type(0b0110000, 0b00010, rs, 0b001, rd, 0b0011011))
// Count set bits
#define CPOPxw(rd, rs)          EMIT(R_type(0b0110000, 0b00010, rs, 0b001, rd, rex.w?0b0010011:0b0011011))
// Maximum
#define MAX(rd, rs1, rs2)       EMIT(R_type(0b0000101, rs2, rs1, 0b110, rd, 0b0110011))
// Unisgned maximum
#define MAXU(rd, rs1, rs2)      EMIT(R_type(0b0000101, rs2, rs1, 0b111, rd, 0b0110011))
// Minimum
#define MIN(rd, rs1, rs2)       EMIT(R_type(0b0000101, rs2, rs1, 0b100, rd, 0b0110011))
// Unsigned minimum
#define MINU(rd, rs1, rs2)      EMIT(R_type(0b0000101, rs2, rs1, 0b101, rd, 0b0110011))
// Sign-extend byte
#define SEXTB(rd, rs)           EMIT(R_type(0b0110000, 0b00100, rs, 0b001, rd, 0b0010011))
// Sign-extend half-word
#define SEXTH(rd, rs)           EMIT(R_type(0b0110000, 0b00101, rs, 0b001, rd, 0b0010011))
// Zero-extend half-word
#define ZEXTH_(rd, rs)          EMIT(R_type(0b0000100, 0b00000, rs, 0b100, rd, 0b0111011))
// Zero-extend half-word
#define ZEXTH(rd, rs)           if(rv64_zbb) ZEXTH_(rd, rs); else {SLLI(rd, rs, 48); SRLI(rd, rd, 48);}
// Rotate left (register)
#define ROL(rd, rs1, rs2)       EMIT(R_type(0b0110000, rs2, rs1, 0b001, rd, 0b0110011))
// Rotate left word (register)
#define ROLW(rd, rs1, rs2)      EMIT(R_type(0b0110000, rs2, rs1, 0b001, rd, 0b0111011))
// Rotate left (register)
#define ROLxw(rd, rs1, rs2)     EMIT(R_type(0b0110000, rs2, rs1, 0b001, rd, rex.w?0b0110011:0b0111011))
// Rotate right (register)
#define ROR(rd, rs1, rs2)       EMIT(R_type(0b0110000, rs2, rs1, 0b101, rd, 0b0110011))
// Rotate right (immediate)
#define RORI(rd, rs1, shamt)    EMIT(R_type(0b0110000, shamt, rs1, 0b101, rd, 0b0010011))
// Rotate right word (immediate)
#define RORIW(rd, rs1, shamt)   EMIT(R_type(0b0110000, shamt, rs1, 0b101, rd, 0b0011011))
// Rotate right (immediate)
#define RORIxw(rd, rs1, shamt)  EMIT(R_type(0b0110000, shamt, rs1, 0b101, rd, rex.w?0b0010011:0b0011011))
// Rotate right word (register)
#define RORW(rd, rs1, rs2)      EMIT(R_type(0b0110000, rs2, rs1, 0b101, rd, 0b0111011))
// Rotate right (register)
#define RORxw(rd, rs1, rs2)     EMIT(R_type(0b0110000, rs2, rs1, 0b101, rd, rex.w?0b0110011:0b0111011))
// Bitwise OR Combine, byte granule (for all byte, if byte==0, res.byte=0, else res.byte=0xff)
#define ORCB(rd, rs)            EMIT(I_type(0b001010000111, rs, 0b101, rd, 0b0010011))
// Byte-reverse register
#define REV8(rd, rs)            EMIT(I_type(0b011010111000, rs, 0b101, rd, 0b0010011))

//Zbc
// Carry-less multily (low-part)
#define CLMUL(rd, rs1, rs2)         EMIT(R_type(0b0000101, rs2, rs1, 0b001, rd, 0b0110011))
// Carry-less multiply (high-part)
#define CLMULH(rd, rs1, rs2)        EMIT(R_type(0b0000101, rs2, rs1, 0b011, rd, 0b0110011))
// Carry-less multiply (reversed)
#define CLMULR(rd, rs1, rs2)        EMIT(R_type(0b0000101, rs2, rs1, 0b010, rd, 0b0110011))

//Zbs
// encoding of the "imm" on RV64 use a slight different mask, but it will work using R_type with high bit of imm ovewriting low bit op func
// Single-bit Clear (Register)
#define BCLR(rd, rs1, rs2)          EMIT(R_type(0b0100100, rs2, rs1, 0b001, rd, 0b0110011))
// Single-bit Clear (Immediate)
#define BCLI(rd, rs1, imm)          EMIT(R_type(0b0100100, imm, rs1, 0b001, rd, 0b0010011))
// Single-bit Extreact (Register)
#define BEXT(rd, rs1, rs2)          EMIT(R_type(0b0100100, rs2, rs1, 0b101, rd, 0b0110011))
// Single-bit Extract (Immediate)
#define BEXTI(rd, rs1, imm)         EMIT(R_type(0b0100100, imm, rs1, 0b101, rd, 0b0010011))
// Single-bit Invert (Register)
#define BINV(rd, rs1, rs2)          EMIT(R_type(0b0110100, rs2, rs1, 0b001, rd, 0b0110011))
// Single-bit Invert (Immediate)
#define BINVI(rd, rs1, imm)         EMIT(R_type(0b0110100, imm, rs1, 0b001, rd, 0b0010011))
// Single-bit Set (Register)
#define BSET(rd, rs1, rs2)          EMIT(R_type(0b0010100, rs2, rs1, 0b001, rd, 0b0110011))
// Single-bit Set (Immediate)
#define BSETI(rd, rs1, imm)         EMIT(R_type(0b0010100, imm, rs1, 0b001, rd, 0b0010011))


#endif //__RV64_EMITTER_H__
