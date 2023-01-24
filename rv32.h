#pragma once

#include <stdio.h>
#include <stdint.h>


typedef void (*instr_callback_t)(struct cpu_t*, struct instr_t*);

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
    uint8_t immPartJ[4];

    uint32_t immI, immS, immB, immU, immJ;

    uint8_t funct3;
    uint8_t funct7;

};

struct cpu_t {
    uint32_t reg[32]; //GPR
    uint32_t PC;
    struct instr_t currentInstr;

    uint8_t memory[1024];
};

void Reset(struct cpu_t* cpu){
    cpu->reg[0] = 0;
    cpu->PC = 0;
}

uint32_t SignExtend(uint32_t num, uint8_t bitNo){
    uint32_t sign = (num >> bitNo) & 1;
    uint32_t orMask = (-sign) << bitNo;

    return num | orMask;
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

void ADD(struct cpu_t* cpu, struct instr_t* instr){
    cpu->reg[instr->rdNo] = cpu->reg[instr->rs1No] + cpu->reg[instr->rs2No];
}

void DecodeCallback(struct instr_t* instr){

    switch (instr->opcode) {
        case 0b0010011:
            switch (instr->funct3) {
                case 0b000:
                    instr->callback = &ADDI;
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
                case 0b110:
                    instr->callback = &ORI;
                    break;
            }
            break;
        case 0b0110011:
            switch (instr->funct3) {
                case 0b000:
                    instr->callback = &ADD;
                    break;
            }
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
    currentInstr->immPartS[0] = (instruction >> 6) & 0x1F;
    currentInstr->immPartS[1] = (instruction >> 24) & 0x7F;
    currentInstr->immPartB[0] = (instruction >> 6) & 0x1;
    currentInstr->immPartB[1] = (instruction >> 7) & 0xF;
    currentInstr->immPartB[2] = (instruction >> 24) & 0x3F;
    currentInstr->immPartB[3] = (instruction >> 30) & 0x1;

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
            (currentInstr->immPartJ[3] << 20), 19);

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

void Tick(struct cpu_t* cpu){

    uint32_t instruction = Fetch(cpu);
    Decode(&cpu->currentInstr,instruction);
    cpu->PC += 4;
    cpu->currentInstr.callback(cpu, &cpu->currentInstr);
}

void Write4B(uint8_t* memory, uint32_t address, uint32_t value){
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
    memory[address + 2] = (value >> 16) & 0xFF;
    memory[address + 3] = (value >> 24) & 0xFF;
}