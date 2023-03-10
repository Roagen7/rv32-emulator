#pragma once

#include <stdio.h>
#include <stdint.h>


#define CSR_MSTATUS     0x300
#define CSR_MISA        0x301
#define CSR_MEDELEG     0x302
#define CSR_MIDELEG     0x303
#define CSR_MIE         0x304
#define CSR_MTVEC       0x305
#define CSR_MCOUNTEREN  0x306
#define CSR_MSTATUSH    0x310
#define CSR_MSCRATCH    0x340
#define CSR_MEPC        0x341
#define CSR_MCAUSE      0x342
#define CSR_MTVAL       0x343
#define CSR_MIP         0x344
#define CSR_MTINST      0x34A
#define CSR_MTVAL2      0x34B

#define INTX_NO_EXCEPTION                    0xFF
#define INT1_MACHINE_SOFTWARE_INTERRUPT      3
#define INT1_MACHINE_TIMER_INTERRUPT         7
#define INT1_MACHINE_EXTERNAL_INTERRUPT      11

#define INT0_INSTRUCTION_ADDRESS_MISALIGNED  0
#define INT0_INSTRUCTION_ACCESS_FAULT        1
#define INT0_ILLEGAL_INSTRUCTION             2
#define INT0_BREAKPOINT                      3
#define INT0_LOAD_ADDRESS_MISALIGNED         4
#define INT0_ENVIRONMENT_CALL_FROM_U_MODE    8
#define INT0_ENVIRONMENT_CALL_FROM_M_MODE    11

typedef void (*instr_callback_t)(struct cpu_t*, struct instr_t*);

enum Privilege {
    PRIV_USER,
    PRIV_MACHINE
};

struct instr_t {

    instr_callback_t callback;

    uint8_t opcode;

    uint8_t rdNo;
    uint8_t rs1No;
    uint8_t rs2No;

    uint32_t immPartI;
    uint16_t immPartS[2];
    uint32_t immPartU;
    uint8_t immPartB[4];
    uint16_t immPartJ[4];

    uint32_t immI, immS, immB, immU, immJ;

    uint8_t funct3;
    uint8_t funct7;

};

struct cpu_t {
    uint32_t reg[32]; // GPR
    uint32_t PC;

    struct instr_t currentInstr;
    uint8_t memory[1024];

    //CSR
    uint32_t csr[4096];
    uint32_t exceptionCalled;
    uint32_t exceptionInterruptNo;

    enum Privilege privilege;

};

void Reset(struct cpu_t* cpu){
    cpu->reg[0] = 0;
    cpu->PC = 0;
    cpu->exceptionCalled = INTX_NO_EXCEPTION;
    cpu->csr[CSR_MISA] = 0x40000100; // implements I extension

}

uint32_t SignExtend(uint32_t num, uint8_t bitNo){
    uint32_t sign = (num >> bitNo) & 1;
    uint32_t orMask = (-sign) << bitNo;

    return num | orMask;
}

int CheckPermissions(uint32_t address, int isWrite){

    uint8_t rw = (address >> 10) & 0x3;
    uint8_t lowPriv = (address >> 8) & 0x3;

    if(rw == 11 && isWrite) return 0;

    return 1;
}

void ADDI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] + instr->immI;
}

void SLTI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = (int32_t) cpu->reg[instr->rs1No] < (int32_t) instr->immI;
}

void SLTIU(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] < instr->immI;
}

void XORI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] ^ instr->immI;
}

void ORI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] | instr->immI;
}

void ANDI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] & instr->immI;
}

void SLLI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = (cpu->reg[instr->rs1No] << instr->rs2No);
}

void SRLI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = (cpu->reg[instr->rs1No] >> instr->rs2No);
}

void SRAI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = SignExtend(cpu->reg[instr->rs1No] >> instr->rs2No, 31 - instr->rs2No);
}

void ADD(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] + cpu->reg[instr->rs2No];
}

void SUB(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] - cpu->reg[instr->rs2No];
}

void SLL(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = (cpu->reg[instr->rs1No] << (cpu->reg[instr->rs2No] & 0x1F));
}

void SRL(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = (cpu->reg[instr->rs1No] >> (cpu->reg[instr->rs2No] & 0x1F));
}

void SRA(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = SignExtend(
            cpu->reg[instr->rs1No] >> (cpu->reg[instr->rs2No] & 0x1F),
            31 - (cpu->reg[instr->rs2No] & 0x1F));
}

void SLT(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = (int32_t) cpu->reg[instr->rs1No] < (int32_t) cpu->reg[instr->rs2No];
}

void SLTU(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] < cpu->reg[instr->rs2No];
}

void XOR(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] ^ cpu->reg[instr->rs2No];
}

void OR(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] | cpu->reg[instr->rs2No];
}

void AND(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] & cpu->reg[instr->rs2No];
}

void LB(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = SignExtend(cpu->memory[cpu->reg[instr->rs1No] + instr->immI], 7);
}

void LBU(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->memory[cpu->reg[instr->rs1No] + instr->immI];
}

void LH(struct cpu_t* cpu, struct instr_t* instr){
    uint32_t address = cpu->reg[instr->rs1No] + instr->immI;
    uint16_t lo = cpu->memory[address];
    uint16_t hi = cpu->memory[address + 1];

    cpu->reg[instr->rdNo] = SignExtend(lo + (hi << 8), 15);
}

void LHU(struct cpu_t* cpu, struct instr_t* instr){
    uint32_t address = cpu->reg[instr->rs1No] + instr->immI;
    uint16_t lo = cpu->memory[address];
    uint16_t hi = cpu->memory[address + 1];

    cpu->reg[instr->rdNo] = lo + (hi << 8);
}

void LW(struct cpu_t* cpu, struct instr_t* instr){
    uint32_t address = cpu->reg[instr->rs1No] + instr->immI;
    uint32_t b0 = cpu->memory[address];
    uint32_t b1 = cpu->memory[address + 1];
    uint32_t b2 = cpu->memory[address + 2];
    uint32_t b3 = cpu->memory[address + 3];

    cpu->reg[instr->rdNo] = b0 + (b1 << 8) + (b2 << 16) + (b3 << 24);
}

void SB(struct cpu_t* cpu, struct instr_t* instr){
    cpu->memory[instr->immS + cpu->reg[instr->rs1No]] = cpu->reg[instr->rs2No] & 0xFF;
}

void SH(struct cpu_t* cpu, struct  instr_t* instr){
    uint32_t address = instr->immS + cpu->reg[instr->rs1No];
    cpu->memory[address] = cpu->reg[instr->rs2No] & 0xFF;
    cpu->memory[address + 1] = ((cpu->reg[instr->rs2No] >> 8) & 0xFF);
}

void SW(struct cpu_t* cpu, struct instr_t* instr){
    uint32_t address = instr->immS + cpu->reg[instr->rs1No];
    cpu->memory[address] = cpu->reg[instr->rs2No] & 0xFF;
    cpu->memory[address + 1] = ((cpu->reg[instr->rs2No] >> 8) & 0xFF);
    cpu->memory[address + 2] = ((cpu->reg[instr->rs2No] >> 16) & 0xFF);
    cpu->memory[address + 3] = ((cpu->reg[instr->rs2No] >> 24) & 0xFF);
}

void LUI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = instr->immU;
}

void AUIPC(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = instr->immU + cpu->PC;
}

void JAL(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->PC + 4;
    cpu->PC += instr->immJ - 4;
}

void JALR(struct cpu_t* cpu, struct instr_t* instr){
    uint32_t temp = cpu->PC + 4;
    cpu->PC = cpu->reg[instr->rs1No] + instr->immI - 4;
    cpu->reg[instr->rdNo] = temp;
}

void BEQ(struct cpu_t* cpu, struct instr_t* instr){
    if(cpu->reg[instr->rs1No] == cpu->reg[instr->rs2No])
        cpu-> PC += instr->immB - 4;
}

void BNE(struct cpu_t* cpu, struct instr_t* instr){
    if(cpu->reg[instr->rs1No] != cpu->reg[instr->rs2No])
        cpu-> PC += instr->immB - 4;
}

void BLT(struct cpu_t* cpu, struct instr_t* instr){
    if((int32_t) cpu->reg[instr->rs1No] < (int32_t) cpu->reg[instr->rs2No])
        cpu-> PC += instr->immB - 4;
}

void BGE(struct cpu_t* cpu, struct instr_t* instr){
    if((int32_t) cpu->reg[instr->rs1No] >= (int32_t) cpu->reg[instr->rs2No])
        cpu->PC += instr->immB - 4;
}

void BLTU(struct cpu_t* cpu, struct instr_t* instr){
    if(cpu->reg[instr->rs1No] < cpu->reg[instr->rs2No])
        cpu-> PC += instr->immB - 4;
}

void BGEU(struct cpu_t* cpu, struct instr_t* instr){
    if(cpu->reg[instr->rs1No] >= cpu->reg[instr->rs2No])
        cpu->PC += instr->immB - 4;
}

void CSRRW(struct cpu_t* cpu, struct instr_t* instr){
    if(!CheckPermissions(instr->immI, instr->rdNo != 0)) {
        cpu->exceptionCalled = INT0_ILLEGAL_INSTRUCTION;
        cpu->exceptionInterruptNo = 0;
        return;
    }
    uint32_t temp = cpu->csr[instr->immI];
    cpu->csr[instr->immI] = cpu->reg[instr->rs1No];
    cpu->reg[instr->rdNo] = temp;
}

void CSRRS(struct cpu_t* cpu, struct instr_t* instr){
    uint32_t temp = cpu->csr[instr->immI];
    cpu->csr[instr->immI] |= cpu->reg[instr->rs1No];
    cpu->reg[instr->rdNo] = temp;
}

void CSRRC(struct cpu_t* cpu, struct instr_t* instr){
    uint32_t temp = cpu->csr[instr->immI];
    cpu->csr[instr->immI] &= ~cpu->reg[instr->rs1No];
    cpu->reg[instr->rdNo] = temp;
}

void CSRRWI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->csr[instr->immI];
    cpu->csr[instr->immI] = instr->rs1No;
}

void CSRRSI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->csr[instr->immI];
    cpu->csr[instr->immI] |= instr->rs1No;
}

void CSRRCI(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->csr[instr->immI];
    cpu->csr[instr->immI] &= ~instr->rs1No;
}

void ECALL(struct cpu_t* cpu, struct instr_t* instr){
    cpu->exceptionCalled = INT0_ENVIRONMENT_CALL_FROM_M_MODE;
    cpu->exceptionInterruptNo = 0;
    cpu->csr[CSR_MTVAL] = 0;
}

void EBREAK(struct cpu_t* cpu, struct instr_t* instr){
    cpu->exceptionCalled = INT0_BREAKPOINT;
    cpu->exceptionInterruptNo = 0;
}

void FENCE(struct cpu_t* cpu, struct instr_t* instr){
    assert(0);
}

void FENCEI(struct cpu_t* cpu, struct instr_t* instr){
    assert(0);
}

void URET(struct cpu_t* cpu, struct instr_t* instr){
    assert(0);
}

void SRET(struct cpu_t* cpu, struct instr_t* instr){
    assert(0);
}

void MRET(struct cpu_t* cpu, struct instr_t* instr){
    assert(0);
}

void WFI(struct cpu_t* cpu, struct instr_t* instr){

}

void DecodeCallback(struct instr_t* instr){

    instr->callback = NULL;
    switch (instr->opcode) {
        case 0b0000011:
            switch (instr->funct3) {
                case 0b000:
                    instr->callback = &LB;
                    break;
                case 0b001:
                    instr->callback = &LH;
                    break;
                case 0b010:
                    instr->callback = &LW;
                    break;
                case 0b100:
                    instr->callback = &LBU;
                    break;
                case 0b101:
                    instr->callback = &LHU;
                    break;
            }
            break;
        case 0b0010011:
            switch (instr->funct3) {
                case 0b000:
                    instr->callback = &ADDI;
                    break;
                case 0b001:
                    instr->callback = &SLLI;
                    break;
                case 0b010:
                    instr->callback = &SLTI;
                    break;
                case 0b011:
                    instr->callback = &SLTIU;
                    break;
                case 0b100:
                    instr->callback = &XORI;
                    break;
                case 0b101:
                    if(instr->funct7 == 0){
                        instr->callback = &SRLI;
                    } else {
                        instr->callback = &SRAI;
                    }
                    break;
                case 0b110:
                    instr->callback = &ORI;
                    break;
                case 0b111:
                    instr->callback = &ANDI;
                    break;
            }
            break;
        case 0b0110011:
            switch (instr->funct3) {
                case 0b000:
                    if(instr->funct7 == 0){
                        instr->callback = &ADD;
                    } else {
                        instr->callback = &SUB;
                    }
                    break;
                case 0b001:
                    instr->callback = &SLL;
                    break;
                case 0b010:
                    instr->callback = &SLT;
                    break;
                case 0b011:
                    instr->callback = &SLTU;
                    break;
                case 0b100:
                    instr->callback = &XOR;
                    break;
                case 0b101:
                    if(instr->funct7 == 0){
                        instr->callback = &SRL;
                    } else {
                        instr->callback = &SRA;
                    }
                    break;
                case 0b110:
                    instr->callback = &OR;
                    break;
                case 0b111:
                    instr->callback = &AND;
                    break;
            }
            break;
        case 0b0100011:
            switch (instr->funct3) {
                case 0b000:
                    instr->callback = &SB;
                    break;
                case 0b001:
                    instr->callback = &SH;
                    break;
                case 0b010:
                    instr->callback = &SW;
                    break;
            }
            break;
        case 0b0110111:
            instr->callback = &LUI;
            break;
        case 0b0010111:
            instr->callback = &AUIPC;
            break;
        case 0b1101111:
            instr->callback = &JAL;
            break;
        case 0b1100111:
            instr->callback = &JALR;
            break;

        case 0b1100011:
            switch (instr->funct3) {
                case 0b000:
                    instr->callback = &BEQ;
                    break;
                case 0b001:
                    instr->callback = &BNE;
                    break;
                case 0b100:
                    instr->callback = &BLT;
                    break;
                case 0b101:
                    instr->callback = &BGE;
                    break;
                case 0b110:
                    instr->callback = &BLTU;
                    break;
                case 0b111:
                    instr->callback = &BGEU;
                    break;
            }
            break;
        case 0b1110011:
            switch (instr->funct3) {
                case 0b000:
                    instr->callback = ECALL;
                    break;
                case 0b001:
                    instr->callback = &CSRRW;
                    break;
                case 0b010:
                    instr->callback = &CSRRS;
                    break;
                case 0b011:
                    instr->callback = &CSRRC;
                    break;
                case 0b101:
                    instr->callback = &CSRRWI;
                    break;
                case 0b110:
                    instr->callback = &CSRRSI;
                    break;
                case 0b111:
                    instr->callback = &CSRRCI;
                    break;
            }
            break;
    }
}

void Decode(struct instr_t* currentInstr, uint32_t instruction) {

    currentInstr->opcode = instruction & 0x7F;

    // rd, rs1, rs2
    currentInstr->rdNo = (instruction >> 7) & 0x1F;
    currentInstr->rs1No = (instruction >> 15) & 0x1F;
    currentInstr->rs2No = (instruction >> 20) & 0x1F;

    // immediate parts
    currentInstr->immPartI = (instruction >> 20) & 0x7FF;

    currentInstr->immPartS[0] = (instruction >> 7) & 0x1F;
    currentInstr->immPartS[1] = (instruction >> 25) & 0x7F;

    currentInstr->immPartB[0] = (instruction >> 7) & 0x1;
    currentInstr->immPartB[1] = (instruction >> 8) & 0xF;
    currentInstr->immPartB[2] = (instruction >> 25) & 0x3F;
    currentInstr->immPartB[3] = (instruction >> 31) & 0x1;

    currentInstr->immPartU = (instruction >> 12) & 0xFFFFF;

    currentInstr->immPartJ[0] = (instruction >> 12) & 0xFF;
    currentInstr->immPartJ[1] = (instruction >> 20) & 0x1;
    currentInstr->immPartJ[2] = (instruction >> 21) & 0x3FF;
    currentInstr->immPartJ[3] = (instruction >> 31) & 0x1;

    // immediate values, TO CHECK CAREFULLY
    currentInstr->immI = SignExtend(currentInstr->immPartI, 10);
    currentInstr->immS = SignExtend(currentInstr->immPartS[0] + (currentInstr->immPartS[1] << 5), 10);

    currentInstr->immB = SignExtend(
            (currentInstr->immPartB[1] << 1) +
            (currentInstr->immPartB[2] << 5) +
            (currentInstr->immPartB[0] << 11) +
            (currentInstr->immPartB[3] << 12), 12);

    currentInstr->immU = SignExtend((currentInstr->immPartU << 12), 31);

    currentInstr->immJ = SignExtend(
            (currentInstr->immPartJ[2] << 1) +
            (currentInstr->immPartJ[1] << 11) +
            (currentInstr->immPartJ[0] << 12) +
            (currentInstr->immPartJ[3] << 20), 20);

    currentInstr->funct3 = (instruction >> 12) & 0x7;
    currentInstr->funct7 = (instruction >> 25) & 0x7F;

    DecodeCallback(currentInstr);

}

uint32_t Fetch(struct cpu_t* cpu) {
    uint8_t* memory = cpu->memory;

    return memory[cpu->PC] +
           (memory[cpu->PC + 1] << 8) +
           (memory[cpu->PC + 2] << 16) +
           (memory[cpu->PC + 3] << 24);
}

void HandleException(struct cpu_t* cpu){

    // TODO: check for MIE/UIE
    cpu->csr[CSR_MCAUSE] = (cpu->exceptionInterruptNo << 31) | (cpu->exceptionCalled);
    // TODO: mtval values in OTHER!! places
    cpu->csr[CSR_MEPC] = cpu->PC;
    cpu->privilege = PRIV_MACHINE;

    uint32_t base = cpu->csr[CSR_MTVEC] >> 2;
    if((cpu->csr[CSR_MTVEC] & 0x3) == 0){
        cpu->PC = base - 4;
    } else {
        cpu->PC = base + 4 * cpu->exceptionCalled - 4;
    }

}

void Tick(struct cpu_t* cpu){

    uint32_t instruction = Fetch(cpu);
    Decode(&cpu->currentInstr,instruction);

    if(cpu->currentInstr.callback == NULL) {
        assert(0);
    }
    cpu->currentInstr.callback(cpu, &cpu->currentInstr);

    if(cpu->exceptionCalled != INTX_NO_EXCEPTION){
        HandleException(cpu);
    }

    // default vals
    cpu->reg[0] = 0; // hardwired zero emulation
    cpu->PC += 4; // increment, unlike in x86, happens AFTER the instruction execution
    cpu->exceptionCalled = INTX_NO_EXCEPTION;
}

struct cpu_t* CreateHart(){
    return NULL;
}

void Write4B(uint8_t* memory, uint32_t address, uint32_t value){
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
    memory[address + 2] = (value >> 16) & 0xFF;
    memory[address + 3] = (value >> 24) & 0xFF;
}

void WriteB(uint8_t* memory, uint32_t address, uint8_t value){
    memory[address] = value;
}

void Write2B(uint8_t* memory, uint32_t address, uint16_t value){
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
}