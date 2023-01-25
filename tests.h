#pragma once

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "rv32.h"

#define TEST_N 5

#define VA_NARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(_, ## __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define CODE_HELPER(...) uint32_t t[] = {__VA_ARGS__}; WriteCode(cpu.memory, t, VA_NARGS(__VA_ARGS__));

void WriteCode(uint8_t* memory, const uint32_t* code, uint32_t n){
    for(uint32_t i = 0; i < n; i++){
        Write4B(memory,4 * i, code[i]);
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

void test_SETI(){
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

    CODE_HELPER(0x00100093, // addi x1, x0, 1
                0xffe00113, // addi x2, x0, -2
                0x02209193, // slli x3, x1, 33
                0x00111213, // slli x4, x2, 1
                0x0010d293, // srli x5, x1, 1
                0x00215313, // srli x6, x2, 2
                0x4030d393, // srai x7, x1, 3
                0x40115413, // srai x8, x2, 1
                0x40215493 // srai x9, x2, 2
                );

    TickN(&cpu, 9);

    assert(cpu.reg[3] == 4);
    assert(cpu.reg[4] == -4);
    assert(cpu.reg[5] == 0);
    assert(cpu.reg[6] == 0x3FFFFFFF);
    assert(cpu.reg[7] == 0);
    assert(cpu.reg[8] == -1);
    assert(cpu.reg[9] == -1);
}

void TestRunner(){
     typedef void(*testfptr)();
     testfptr tests[TEST_N] = {
             &test_ADDI,
             &test_ADD,
             &test_SETI,
             &test_LOGI,
             &test_SHIFTI
     };

     for(int i = 0; i < TEST_N; i++){
         printf("running test %d/%d... \n", i+1, TEST_N);
         tests[i]();
     }
}

