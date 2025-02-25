#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_helper.h"
#include "dynarec_rv64_functions.h"

#define GETG        gd = ((nextop&0x38)>>3)+(rex.r<<3)

uintptr_t dynarec64_64(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t u8;
    uint8_t gd, ed, eb1, eb2, gb1, gb2;
    uint8_t wback, wb1, wb2, wb;
    int64_t i64, j64;
    int v0, v1;
    int q0;
    int d0;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(j64);
    MAYUSE(d0);
    MAYUSE(q0);
    MAYUSE(v0);
    MAYUSE(v1);

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {
        case 0x03:
            INST_NAME("ADD Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0, x5);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0, x5);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;

        case 0x33:
            INST_NAME("XOR Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0, x5);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x66:
            addr = dynarec64_6664(dyn, addr, ip, ninst, rex, seg, ok, need_epilog);
            break;
        case 0x80:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: // ADD
                    INST_NAME("ADD Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                case 1: // OR
                    INST_NAME("OR Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                case 2: // ADC
                    INST_NAME("ADC Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 3: // SBB
                    INST_NAME("SBB Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 4: // AND
                    INST_NAME("AND Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 5: // SUB
                    INST_NAME("SUB Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 6: // XOR
                    INST_NAME("XOR Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 7: // CMP
                    INST_NAME("CMP Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEBO(x1, 1);
                    u8 = F8;
                    if(u8) {
                        MOV32w(x2, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    } else {
                        emit_cmp8_0(dyn, ninst, x1, x3, x4);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x6, seg);
            switch((nextop>>3)&7) {
                case 0: // ADD
                    if(opcode==0x81) {INST_NAME("ADD Ed, Id");} else {INST_NAME("ADD Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1, x5);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x9);
                    WBACKO(x6);
                    break;
                case 1: // OR
                    if(opcode==0x81) {INST_NAME("OR Ed, Id");} else {INST_NAME("OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1, x5);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACKO(x6);
                    break;
                case 2: // ADC
                    if(opcode==0x81) {INST_NAME("ADC Ed, Id");} else {INST_NAME("ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1, x5);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    SD(x6, xEmu, offsetof(x64emu_t, scratch));
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4, x6, x9);
                    LD(x6, xEmu, offsetof(x64emu_t, scratch));
                    WBACKO(x6);
                    break;
                case 3: // SBB
                    if(opcode==0x81) {INST_NAME("SBB Ed, Id");} else {INST_NAME("SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1, x5);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4, x9);
                    WBACKO(x6);
                    break;
                case 4: // AND
                    if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1, x5);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACKO(x6);
                    break;
                case 5: // SUB
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1, x5);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x9);
                    WBACKO(x6);
                    break;
                case 6: // XOR
                    if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1, x5);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACKO(x6);
                    break;
                case 7: // CMP
                    if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDO(x6, (opcode==0x81)?4:1, x5);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    if(i64) {
                        MOV64xw(x2, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x2, x3, x4, x5, x6);
                    } else
                        emit_cmp32_0(dyn, ninst, rex, ed, x3, x4);
                    break;
            }
            break;
        case 0x88:
            INST_NAME("MOV Seg:Eb, Gb");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            gd = ((nextop&0x38)>>3)+(rex.r<<3);
            if(rex.rex) {
                gb2 = 0;
                gb1 = xRAX + gd;
            } else {
                gb2 = ((gd&4)>>2);
                gb1 = xRAX+(gd&3);
            }
            gd = x5;
            if(gb2) {
                SRLI(x5, gb1, 8);
                gb1 = x5;
            }
            if(MODREG) {
                ed = (nextop&7) + (rex.b<<3);
                if(rex.rex) {
                    eb1 = xRAX+ed;
                    eb2 = 0;
                } else {
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = ((ed&4)>>2);    // L or H
                }
                ANDI(gd, gb1, 0xff);
                if(eb2) {
                    MOV64x(x1, 0xffffffffffff00ffLL);
                    ANDI(x1, eb1, x1);
                    SLLI(gd, gd, 8);
                    OR(eb1, x1, gd);
                } else {
                    ANDI(x1, eb1, ~0xff);
                    OR(eb1, x1, gd);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADD(x4, ed, x4);
                SB(gb1, x4, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x89:
            INST_NAME("MOV Seg:Ed, Gd");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MVxw(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADD(x4, ed, x4);
                SDxw(gd, x4, fixedaddress);
                SMWRITE2();
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Seg:Ed");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MVxw(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {       // mem <= reg
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADD(x4, ed, x4);
                LDxw(gd, x4, fixedaddress);
            }
            break;
        case 0xC6:
            INST_NAME("MOV Seg:Eb, Ib");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            if(MODREG) {   // reg <= u8
                u8 = F8;
                if(!rex.rex) {
                    ed = (nextop&7);
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                } else {
                    eb1 = xRAX+(nextop&7)+(rex.b<<3);
                    eb2 = 0;            
                }

                if (eb2) {
                    // load a mask to x3 (ffffffffffff00ff)
                    LUI(x3, 0xffffffffffff0);
                    ORI(x3, x3, 0xff);
                    // apply mask
                    AND(eb1, eb1, x3);
                    if(u8) {
                        if((u8<<8)<2048) {
                            ADDI(x4, xZR, u8<<8);
                        } else {
                            ADDI(x4, xZR, u8);
                            SLLI(x4, x4, 8);
                        }
                        OR(eb1, eb1, x4);
                    }
                } else {
                    ANDI(eb1, eb1, 0xf00);  // mask ffffffffffffff00
                    ORI(eb1, eb1, u8);
                }
            } else {                    // mem <= u8
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 1);
                u8 = F8;
                if(u8) {
                    ADDI(x3, xZR, u8);
                    ed = x3;
                } else
                    ed = xZR;
                ADD(x4, wback, x4);
                SB(ed, x4, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xC7:
            INST_NAME("MOV Seg:Ed, Id");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop=F8;
            if(MODREG) {   // reg <= i32
                i64 = F32S;
                ed = xRAX+(nextop&7)+(rex.b<<3);
                MOV64xw(ed, i64);
            } else {                    // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 4);
                i64 = F32S;
                if(i64) {
                    MOV64xw(x3, i64);
                    ed = x3;
                } else
                    ed = xZR;
                ADD(x4, wback, x4);
                SDxw(ed, x4, fixedaddress);
                SMWRITE2();
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
