#pragma once

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "rv32.h"

#define TEST_N 20

#define VA_NARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12,_13, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(_, ## __VA_ARGS__, 13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define CODE_HELPER(...) uint32_t t[] = {__VA_ARGS__}; WriteCode(cpu.memory, t, VA_NARGS(__VA_ARGS__), 0);
#define CODE_HELPER_ARBITRARY(offset0, ...) uint32_t t2[] = {__VA_ARGS__}; WriteCode(cpu.memory, t2, VA_NARGS(__VA_ARGS__), offset0);

void WriteCode(uint8_t* memory, const uint32_t* code, uint32_t n, uint32_t offset){
    for(uint32_t i = 0; i < n; i++){
        Write4B(memory,4 * i + offset, code[i]);
    }
}

void TickN(struct cpu_t* cpu,uint32_t N){
    for(uint32_t i = 0; i < N; i++){
        Tick(cpu);
    }
}

void test_ADDI(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0xfff00093, // addi x1,x0, -1
            0x00008113, // addi x2, x1, 0
            0x00110193  //addi x3, x2, 1
            )

    TickN(&cpu, 3);

    assert(cpu.reg[1] == 0xffffffff);
    assert(cpu.reg[2] == 0xffffffff);
    assert(cpu.reg[3] == 0);
}

void test_ADD(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00100093, // addi x1,x0, 1
            0x00200113, // addi x2, x0, 2
            0x002081b3, // add x3, x1, x2
            0x00308133  // add x2, x1, x3
            )

    TickN(&cpu, 3);

    assert(cpu.reg[1] == 1);
    assert(cpu.reg[2] == 2);
    assert(cpu.reg[3] == 3);

    Tick(&cpu);

    assert(cpu.reg[2] == 4);

}

void test_SLTI(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00100093, // addi x1, x0, 1
            0xffe0a093, // slti x1, x1, -2
            0xffe00093, // addi x1, x0, -2
            0x0000a093, // slti x1, x1, 0
            0x00200093, // addi x1, x0, 2
            0x0000b113, // sltiu x2, x1, 0
            0xfff0b113 // sltiu x2, x1, -1
            )


    Tick(&cpu);
    Tick(&cpu);
    assert(cpu.reg[1] == 0);
    Tick(&cpu);
    Tick(&cpu);
    assert(cpu.reg[1] == 1);

    Tick(&cpu);
    Tick(&cpu);
    assert(cpu.reg[2] == 0);
    Tick(&cpu);
    assert(cpu.reg[2] == 1);

}

void test_LOGI(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00100093, // addi x1, x0, 1
            0xfff0c113, // xori x2, x1, -1
            0x0010c193, // xori x3, x1, 1
            0x00116113, // ori x2, x2, 1
            0x00306193, // ori x3, x0, 3
            0x0050e093, // ori x1, x1, 5
            0x00900113, // addi x2, x0, 9
            0x00017193, // andi x3, x2, 0
            0x00717093  // andi x1, x2, 7
            )

    TickN(&cpu,3);
    assert(cpu.reg[1] == 1);
    assert(cpu.reg[2] == 0xFFFFFFFE);
    assert(cpu.reg[3] == 0);

    TickN(&cpu, 3);
    assert(cpu.reg[1] == 5);
    assert(cpu.reg[2] == -1);
    assert(cpu.reg[3] == 3);

    TickN(&cpu, 3);
    assert(cpu.reg[1] == 1);
    assert(cpu.reg[2] == 9);
    assert(cpu.reg[3] == 0);

}

void test_SHIFTI(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00100093, // addi x1, x0, 1
            0xffe00113, // addi x2, x0, -2
            0x02209193, // slli x3, x1, 33
            0x00111213, // slli x4, x2, 1
            0x0010d293, // srli x5, x1, 1
            0x00215313, // srli x6, x2, 2
            0x4030d393, // srai x7, x1, 3
            0x40115413, // srai x8, x2, 1
            0x40215493 // srai x9, x2, 2
                )

    TickN(&cpu, 9);

    assert(cpu.reg[3] == 4);
    assert(cpu.reg[4] == -4);
    assert(cpu.reg[5] == 0);
    assert(cpu.reg[6] == 0x3FFFFFFF);
    assert(cpu.reg[7] == 0);
    assert(cpu.reg[8] == -1);
    assert(cpu.reg[9] == -1);
}

void test_SUB(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00300093, // addi x1, x0, 3
            0x40100133, // sub x2, x0, x1
            0x402001b3 // sub x3, x0, x2
            )

    TickN(&cpu, 3);

    assert(cpu.reg[1] == 3);
    assert(cpu.reg[2] == -3);
    assert(cpu.reg[3] == 3);
}

void test_SHIFT(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00200093, // addi x1, x0, 2
            0x00109133, // sll x2, x1, x1
            0x002091b3, // sll x3, x1, x2
            0x02200213, // addi x4, x0, 34
            0x004092b3, // sll x5, x1, x4
            0x0012d333, // srl x6, x5, x1
            0xffc00393, // addi x7, x0, -4
            0x0013d433, // srl x8, x7, x1
            0x4013d4b3, // sra  x9, x7, x1
            0x00100513, // addi x10, x0, 1
           0x00a3d5b3, // srl x11, x7, x10
           0x40a3d633  // sra x12, x7, x10
            )

    TickN(&cpu, 12);
    assert(cpu.reg[1] == 2);
    assert(cpu.reg[2] == 8);
    assert(cpu.reg[3] == 512);
    assert(cpu.reg[4] == 34);
    assert(cpu.reg[5] == 8);
    assert(cpu.reg[6] == 2);
    assert(cpu.reg[7] == -4);
    assert(cpu.reg[8] == 0x3FFFFFFF);
    assert(cpu.reg[9] == -1);
    assert(cpu.reg[10] == 1);
    assert(cpu.reg[11] == 0x7FFFFFFE);
    assert(cpu.reg[12] == -2);

}

void test_SLT(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00300093, // addi x1, x0, 3
            0xffe00113, // addi x2, x0, -2
            0x00500193, // addi x3, x0, 5
            0x00113233, // sltu x4, x2, x1
            0x001122b3, // slt x5, x2, x1
            0x00313333, // sltu x6, x2, x3
            0x003133b3, // sltu x7, x2, x3
            0x0030a433, // slt x8, x1, x3
            0x0030b4b3, // sltu x9, x1, x3
            )
    TickN(&cpu, 9);
    assert(cpu.reg[1] == 3);
    assert(cpu.reg[2] == -2);
    assert(cpu.reg[3] == 5);
    assert(cpu.reg[4] == 0);

}

void test_LOG(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00100093, // addi x1, x0, 1
            0x00200113, // addi x2, x0, 2
            0xfff00193, // addi x3, x0, -1
            0x0020e333, // or x6, x1, x2
            0x0060c233, // xor x4, x1, x6
            0x001372b3, // and x5, x6, x1
            0x0030c3b3 // xor x7, x1, x3
            )
    TickN(&cpu, 7);
    assert(cpu.reg[1] == 1);
    assert(cpu.reg[2] == 2);
    assert(cpu.reg[3] == -1);
    assert(cpu.reg[4] == 2);
    assert(cpu.reg[5] == 1);
    assert(cpu.reg[6] == 3);
    assert(cpu.reg[7] == 0xFFFFFFFE);

}

void test_LB(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
        0x20000083, //lb x1, 512(x0)
        0x00100113, // addi x2, x0, 1
        0x20010183, // lb x3, 512(x2)
        0x20004203, // lbu x4, 512(x0)
        0x10224283 // lbu x5, 258(x4)
            )
    WriteB(cpu.memory, 512, -1);
    WriteB(cpu.memory, 513, 5);

    TickN(&cpu, 5);

    assert(cpu.reg[1] == -1);
    assert(cpu.reg[2] == 1);
    assert(cpu.reg[3] == 5);
    assert(cpu.reg[4] == 0xFF);
    assert(cpu.reg[5] == 5);
}

void test_LH(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x20001083, // lh x1, 512(x0)
            0x20109103, // lh x2, 513(x1)
            0x20005183, // lhu x3, 512(x0)
            0x2011d203  // lhu x4, 513(x3)
    )

    Write2B(cpu.memory, 512, 1);
    Write2B(cpu.memory, 514, -1);

    TickN(&cpu, 4);

    assert(cpu.reg[1] == 1);
    assert(cpu.reg[2] == -1);
    assert(cpu.reg[3] == 1);
    assert(cpu.reg[4] == 0xFFFF);

}

void test_LW(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00002083, // lw x1, 0(x0)
            0x00800113, // addi x2, x0, 8
            0x00012183 // lw x3, 0(x2)
    )

    TickN(&cpu, 3);

    assert(cpu.reg[1] == 0x00002083);
    assert(cpu.reg[3] == 0x00012183);

}

void test_STORE(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x12300093, // addi x1, x0, 0x123
            0x00c09093, // slli x1, x1, 12
            0x25608093, // addi x1, x1, 0x256
            0x20100023, // sb x1, 512(x0)
            0x04101023, // sh x1, 64(x0)
            0x2c102c23  // sw x1, 728(x0)
            )

    TickN(&cpu, 6);

    assert(cpu.memory[512] == 0x56);
    assert(cpu.memory[64] == 0x56);
    assert(cpu.memory[65] == 0x32);
    assert(cpu.memory[728] == 0x56);
    assert(cpu.memory[729] == 0x32);
    assert(cpu.memory[730] == 0x12);
    assert(cpu.memory[731] == 0x00);

}

void test_UI(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x000010b7, // _start: lui x1, 1
            0xdeadb137, // lui x2, 0xdeadb
            0xeefcc197, // auipc x3, 0xeefcc

    )
    TickN(&cpu, 3);

    Tick(&cpu);
    Tick(&cpu);

    assert(cpu.reg[1] == 0x1000);
    assert(cpu.reg[2] == 0xdeadb000);
    assert(cpu.reg[3] == 0xeefcc008);

}

void test_JAL1() {
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00000013, // et: nop
            0xffdff0ef, // jal x1, et
    )

    TickN(&cpu, 2);
    assert(cpu.PC == 0);
    TickN(&cpu,100);
}

void test_JAL2(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x008000ef, // jal p
            0x0dd00113, // addi x2, x0, 0xdd
            0x0cc00093, // p:	addi x1, x0, 0xcc
            0x00c0056f, // jal x10, q
            0x0bb00193, // addi x3, x0, 0xbb
            0x00000013, // nop
            0x0ee00213 // q: addi x4, x0, 0xee

    )

    TickN(&cpu,4);

    assert(cpu.reg[1] == 0xcc);
    assert(cpu.reg[2] != 0xdd);
    assert(cpu.reg[3] != 0xbb);
    assert(cpu.reg[4] == 0xee);
}

void test_JALR(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00c000ef, // jal x1, fun
            0x0ee00393, // addi x7, x0, 0xee
            0x00418067, // jalr x0, 4(x3)
            0x0dd00313, // fun: addi x6, x0, 0xdd
            0x000081e7, // jalr x3, 0(x1)
            0x0cc00313, // addi x6, x0, 0xcc
            0x0aa00413  // end: addi x8, x0, 0xaa
            )

    TickN(&cpu,6);
    assert(cpu.reg[1] == 4);
    assert(cpu.reg[3] == 20);
    assert(cpu.reg[6] == 0xdd);
    assert(cpu.reg[7] == 0xee);
    assert(cpu.reg[8] == 0xaa);
}

void test_BRANCH1(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00100093, // addi x1, x0, 1
            0xfff00113, // addi x2, x0, -1
            0x00209463, // bne x1, x2, l1
            0x01c000ef, // jal end
            0x0dd00193, // l1: addi x3, x0, 0xdd
            0x0020c863, // blt x1, x2, l2
            0x0ee00213, // addi x4, x0, 0xee
            0x0020e463, // bltu x1, x2, l2
            0x008000ef, // jal end
            0x0aa00293  // l2: addi x5, x0, 0xaa
                            // end:
    )

    TickN(&cpu,8);

    assert(cpu.reg[3] == 0xdd);
    assert(cpu.reg[4] == 0xee);
    assert(cpu.reg[5] == 0xaa);
}

void test_CSR(){
    struct cpu_t cpu;
    Reset(&cpu);

    CODE_HELPER(
            0x00700093, // addi x1, x0, 7
            0x0cc00113, // addi x2, x0, 0xcc
            0x00009073, // csrrw x0, 0, x1

            0x0012d073, // csrrwi x0, 1, 5
            0x002f5073, // csrrwi x0, 2, 0x1e
            0x000121f3, // csrrs x3, 0, x2

            0x0010b273, // csrrc x4, 1, x1
            0x0020e2f3  // csrrsi x5, 2, 1
    )

    TickN(&cpu, 3);
    assert(cpu.reg[1] == 7);
    assert(cpu.reg[2] == 0xcc);
    assert(cpu.csr[0] == 7);

    TickN(&cpu,3);
    assert(cpu.csr[1] == 5);
    assert(cpu.csr[2] == 0x1e);
    assert(cpu.csr[0] == 0xcf);
    assert(cpu.reg[3] == 7);

    TickN(&cpu, 2);
    assert(cpu.csr[1] == 0);
    assert(cpu.csr[2] == 0x1F);
}

void test_INTERRUPT(){
    struct cpu_t cpu;
    Reset(&cpu);

    cpu.csr[CSR_MTVEC] = 2048;

    CODE_HELPER(
            0x00000073 // ecall
    )

    CODE_HELPER_ARBITRARY(512,
            0x0cc00193 // addi x3, x0, 0xcc
    )

    TickN(&cpu,2);

    assert(cpu.reg[3] == 0xcc);

}

void TestRunner(){

     typedef void(*testfptr)();
     testfptr tests[TEST_N] = {
             &test_ADDI,
             &test_ADD,
             &test_SLTI,
             &test_LOGI,
             &test_SHIFTI,
             &test_SUB,
             &test_SHIFT,
             &test_SLT,
             &test_LOG,
             &test_LB,
             &test_LH,
             &test_LW,
             &test_STORE,
             &test_UI,
             &test_JAL1,
             &test_JAL2,
             &test_JALR,
             &test_BRANCH1,
             &test_CSR,
             &test_INTERRUPT
     };

     for(int i = 0; i < TEST_N; i++){
         printf("running test %d/%d... \n", i+1, TEST_N);
         tests[i]();
     }
}

