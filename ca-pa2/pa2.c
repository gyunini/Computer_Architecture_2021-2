/**********************************************************************
 * Copyright (c) 2019-2021
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */

/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define MAX_NR_TOKENS    32    /* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN    64    /* Maximum length of single token */
#define MAX_COMMAND    256 /* Maximum length of command string */

typedef unsigned char bool;
#define true    1
#define false    0


/**
 * memory[] emulates the memory of the machine
 */
static unsigned char memory[1 << 20] = {    /* 1MB memory at 0x0000 0000 -- 0x0010 0000 */
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0xde, 0xad, 0xbe, 0xef, 0x00, 0x00, 0x00, 0x00,
    'h',  'e',  'l',  'l',  'o',  ' ',  'w',  'o',
    'r',  'l',  'd',  '!',  '!',  0x00, 0x00, 0x00,
    'a',  'w',  'e',  's',  'o',  'm',  'e',  ' ',
    'c',  'o',  'm',  'p',  'u',  't',  'e',  'r',
    ' ',  'a',  'r',  'c',  'h',  'i',  't',  'e',
    'c',  't',  'u',  'r',  'e',  '!',  0x00, 0x00,
};

#define INITIAL_PC    0x1000    /* Initial value for PC register */
#define INITIAL_SP    0x8000    /* Initial location for stack pointer */

/**
 * Registers of the machine
 */
static unsigned int registers[32] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0x10, INITIAL_PC, 0x20, 3, 0xbadacafe, 0xcdcdcdcd, 0xffffffff, 7,
    0, 0, 0, 0, 0, INITIAL_SP, 0, 0,
};

/**
 * Names of the registers. Note that $zero is shorten to zr
 */
const char *register_names[] = {
    "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

/**
 * Program counter register
 */
static unsigned int pc = INITIAL_PC;

/**
 * strmatch()
 *
 * DESCRIPTION
 *   Compare strings @str and @expect and return 1 if they are the same.
 *   You may use this function to simplify string matching :)
 *
 * RETURN
 *   1 if @str and @expect are the same
 *   0 otherwise
 */
static inline bool strmatch(char * const str, const char *expect)
{
    return (strlen(str) == strlen(expect)) && (strncmp(str, expect, strlen(expect)) == 0);
}

/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/


#define OPCODE_BS   26
#define RS_BS       21
#define RT_BS       16
#define RD_BS       11
#define SHAMT_BS    6
#define SIX_MASK    0x0000003f
#define FIVE_MASK   0x0000001f
#define IMMED_MASK  0x0000ffff
#define ADDR_MASK   0x03ffffff


typedef struct r_instruction {
    int opcode;
    int rs;
    int rt;
    int rd;
    int shamt;
    int funct;
} r_instruction;

typedef struct i_instruction {
    int opcode;
    int rs;
    int rt;
    int immediate;
} i_instruction;

typedef struct j_instruction {
    int opcode;
    int address;
} j_instruction;

//unsigned long long result;
unsigned int result;
int *inst_p;

int opcode(int instruction){
    return (instruction >> OPCODE_BS) & SIX_MASK;
}

int rs(int instruction){
    return (instruction >> RS_BS) & FIVE_MASK;
}

int rt(int instruction){
    return (instruction >> RT_BS) & FIVE_MASK;
}

int rd(int instruction){
    return (instruction >> RD_BS) & FIVE_MASK;
}

int shamt(int instruction){
    return (instruction >> SHAMT_BS) & FIVE_MASK;
}

int funct(int instruction){
    return instruction & SIX_MASK;
}

int immediate(int instruction){
    return instruction & IMMED_MASK;
}

int address(int instruction){
    return instruction & ADDR_MASK;
}

//void trap(char arg[]){
//    printf("TRAP (%s)\n", arg);
//    exit(1);
//}
//
//int mask_reg(long long reg){
//    return reg & 0xffffffff;
//}

int process_r(r_instruction inst){
    // printf("%d %d %d %d %d %d\n", inst.opcode, inst.rs, inst.rt, inst.rd, inst.shamt, inst.funct);
    switch(inst.funct){
        case 0x20: // add
            result = registers[inst.rs] + registers[inst.rt];
            registers[inst.rd] = result;
            break;

        case 0x22: // sub
            result = registers[inst.rs] - registers[inst.rt];
            registers[inst.rd] = result;
            break;

        case 0x24: // and
            registers[inst.rd] = registers[inst.rs] & registers[inst.rt];
            break;

        case 0x25: // or
            registers[inst.rd] = registers[inst.rs] | registers[inst.rt];
            break;

        case 0x27: // nor
            registers[inst.rd] = ~(registers[inst.rs] | registers[inst.rt]);
            break;

        case 0x2a: // slt
            //registers[inst.rd] = ((signed int)(*(registers + inst.rs)) < (signed int)(*(registers+inst.rt)));
            registers[inst.rd] = ((signed int)registers[inst.rs] < (signed int)registers[inst.rt]);
            break;

        case 0x0: // sll
            registers[inst.rd] = registers[inst.rt] << inst.shamt;
            break;

        case 0x2: // srl
            registers[inst.rd] = registers[inst.rt] >> inst.shamt;
            break;

        case 0x3: // sra
            registers[inst.rd] = ((signed int) registers[inst.rt]) >> inst.shamt;
            break;

        case 0x8: // jr
            pc = registers[inst.rs];
            break;
            
        default:
            return 0;
            break;
    }
    return 1;
}

int process_j(j_instruction inst){
    // printf("%x %d\n", inst.opcode, inst.address);
    switch(inst.opcode){
        case 0x2: // j
            //pc = ((unsigned int)inst.address)<<6 >>4;
            pc = (pc & 0xf0000000) | (inst.address) << 2 ;
            break;

        case 0x3: // jal
//            registers[31] = *(memory + pc + 4); // ra
//            pc = ((unsigned int)inst.address)<<6 >>4;
            registers[31] = pc ;
            pc = (pc & 0xf0000000) | ((inst.address) << 2) ;
            break;
            
        default:
            return 0;
            break;
    }
    return 1;
}

int process_i(i_instruction inst){
    // printf("%x %d %d %d\n", inst.opcode, inst.rs, inst.rt, inst.immediate);
    switch(inst.opcode){
        case 0x8: // addi
            result = registers[inst.rs] + (short)inst.immediate; // signextimm
            //if (result != mask_reg(result)) trap("addi");
            registers[inst.rt] = result;
            break;
            
        case 0xc: // andi
            result = registers[inst.rs] & (unsigned short)inst.immediate; // zeroextimm
            registers[inst.rt] = result;
            break;

        case 0xd: // ori
            result = registers[inst.rs] | (unsigned short)inst.immediate; // zeroextimm
            registers[inst.rt] = result;
            break;

        case 0x23: // lw
            result = registers[inst.rs] + (short)inst.immediate; // signextimm
            registers[inst.rt] =   *(memory + result)<<24; //*((int*)(memory + result))  //memory[result/4];
            registers[inst.rt] |=  *(memory + result+1)<<16;
            registers[inst.rt] |=  *(memory + result+2)<<8;
            registers[inst.rt] |=  *(memory + result+3);
//            registers[inst.rt] =  *(memory + result); //*((int*)(memory + result))  //memory[result/4];
//            registers[inst.rt] |=  *(memory + result+1)<<8;
//            registers[inst.rt] |=  *(memory + result+2)<<16;
//            registers[inst.rt] |=  *(memory + result+3)<<24;
            break;

        case 0x2b: // sw
            result = registers[inst.rs] + (short)inst.immediate; // signextimm
            memory[result] = registers[inst.rt]>>24; //*((int*)(memory + result)) //memory[result/4] = registers[inst.rt];
            memory[result+1] = registers[inst.rt]>>16;
            memory[result+2] = registers[inst.rt]>>8;
            memory[result+3] = registers[inst.rt];
//            *(memory + result) = registers[inst.rt]; //*((int*)(memory + result)) //memory[result/4] = registers[inst.rt];
//            *(memory + result+1) = registers[inst.rt]>>8;
//            *(memory + result+2) = registers[inst.rt]>>16;
//            *(memory + result+3) = registers[inst.rt]>>24;
            break;

        case 0xa: // slti
            registers[inst.rt] = (registers[inst.rs] < (short)inst.immediate); // signextimm
            break;

        case 0x4: // beq
            if (registers[inst.rs] == registers[inst.rt])
                //inst_p = inst_p + inst.immediate;
                pc = (short)pc + (short)inst.immediate*4; // signextimm
            break;

        case 0x5: // bne
            if (registers[inst.rs] != registers[inst.rt])
                //inst_p = inst_p + inst.immediate;
                pc = (short)pc + (short)(inst.immediate)*4; // signextimm
            break;
            
        default:
            return 0;
            break;
    }
    return 1;
}

/**********************************************************************
 * process_instruction
 *
 * DESCRIPTION
 *   Execute the machine code given through @instr. The following table lists
 *   up the instructions to support. Note that a pseudo instruction 'halt'
 *   (0xffffffff) is added for the testing purpose. Also '*' instrunctions are
 *   the ones that are newly added to PA2.
 *
 * | Name   | Format    | Opcode / opcode + funct |
 * | ------ | --------- | ----------------------- |
 * | `add`  | r-format  | 0 + 0x20                |
 * | `addi` | i-format  | 0x08                    |
 * | `sub`  | r-format  | 0 + 0x22                |
 * | `and`  | r-format  | 0 + 0x24                |
 * | `andi` | i-format  | 0x0c                    |
 * | `or`   | r-format  | 0 + 0x25                |
 * | `ori`  | i-format  | 0x0d                    |
 * | `nor`  | r-format  | 0 + 0x27                |
 * | `sll`  | r-format  | 0 + 0x00                |
 * | `srl`  | r-format  | 0 + 0x02                |
 * | `sra`  | r-format  | 0 + 0x03                |
 * | `lw`   | i-format  | 0x23                    |
 * | `sw`   | i-format  | 0x2b                    |
 * | `slt`  | r-format* | 0 + 0x2a                |
 * | `slti` | i-format* | 0x0a                    |
 * | `beq`  | i-format* | 0x04                    |
 * | `bne`  | i-format* | 0x05                    |
 * | `jr`   | r-format* | 0 + 0x08                |
 * | `j`    | j-format* | 0x02                    |
 * | `jal`  | j-format* | 0x03                    |
 * | `halt` | special*  | @instr == 0xffffffff    |
 *
 * RETURN VALUE
 *   1 if successfully processed the instruction.
 *   0 if @instr is 'halt' or unknown instructions
 */
static int process_instruction(unsigned int instr)
{
    int errcode;
    
    if(instr == 0xffffffff)
        return 0;
    if (opcode(instr) == 0){
        // R instructioneke
        r_instruction inst;
        inst.opcode = opcode(instr);
        inst.rs = rs(instr);
        inst.rt = rt(instr);
        inst.rd = rd(instr);
        inst.shamt = shamt(instr);
        inst.funct = funct(instr);
        errcode = process_r(inst);
    }

    else if (opcode(instr) == 2 || opcode(instr) == 3){
        // J instruction
        j_instruction inst;
        inst.opcode = opcode(instr);
        inst.address = address(instr);
        errcode = process_j(inst);
    }

    else{
        // I instruction
        i_instruction inst;
        inst.opcode = opcode(instr);
        inst.rs = rs(instr);
        inst.rt = rt(instr);
        inst.immediate = immediate(instr);
        errcode = process_i(inst);
    }
    //pc+=4;
    return errcode;
}


/**********************************************************************
 * load_program
 *
 * DESCRIPTION
 *   Load the instructions in the file @filename onto the memory starting at
 *   @INITIAL_PC. Each line in the program file looks like;
 *
 *     [MIPS instruction started with 0x prefix]  // optional comments
 *
 *   For example,
 *
 *   0x8c090008
 *   0xac090020    // sw t1, zero + 32
 *   0x8c080000
 *
 *   implies three MIPS instructions to load. Each machine instruction may
 *   be followed by comments like the second instruction. However you can simply
 *   call strtoimax(linebuffer, NULL, 0) to read the machine code while
 *   ignoring the comment parts.
 *
 *     The program DOES NOT include the 'halt' instruction. Thus, make sure the
 *     'halt' instruction is appended to the loaded instructions to terminate
 *     your program properly.
 *
 *     Refer to the @main() for reading data from files. (fopen, fgets, fclose).
 *
 * RETURN
 *     0 on successfully load the program
 *     any other value otherwise
 */

static int load_program(char * const filename)
{
    char command[MAX_COMMAND] = {'\0'};
    FILE* fp = fopen(filename, "r");
    unsigned int ptr = INITIAL_PC;
    unsigned int temp;
    if(fp == NULL)
        return 1;
    while (fgets(command, sizeof(command), fp)){
        temp = strtoimax(command, NULL, 0);
        *(memory + ptr) = (0xff000000 & temp) >> 24;
        *(memory + ptr+1) = (0x00ff0000 & temp) >> 16;
        *(memory + ptr+2) = (0x0000ff00 & temp) >> 8;
        *(memory + ptr+3) = 0x000000ff & temp;
//        printf("%x\n", strtoimax(command, NULL, 0));
//        printf("%x\n", *((int*)(memory + ptr)));
       // *((int*)(memory + ptr)) = temp;rh
        ptr+=4;
    }
    *((int*)(memory + ptr)) = 0xffffffff; // halt
    fclose(fp);
    return 0;
}


/**********************************************************************
 * run_program
 *
 * DESCRIPTION
 *   Start running the program that is loaded by @load_program function above.
 *   If you implement @load_program() properly, the first instruction is placed
 *   at @INITIAL_PC. Using @pc, which is the program counter of this processor,
 *   you can emulate the MIPS processor by
 *
 *   1. Read instruction from @pc
 *   2. Increment @pc by 4
 *   3. Call @process_instruction(instruction)
 *   4. Repeat until @process_instruction() returns 0
 *
 * RETURN
 *   0
 */
static int run_program(void)
{
    //unsigned int temp = INITIAL_PC;
    int ret=1;
    unsigned int code;
    
    while(ret){
        code = (0xff000000 & *((unsigned int *)(memory + pc)))>> 24;
        code |= (0x00ff0000 & *((unsigned int *)(memory + pc)))>> 8;
        code |= (0x0000ff00 & *((unsigned int *)(memory + pc)))<< 8;
        code |= (0x000000ff & *((unsigned int *)(memory + pc)))<<24;


//        *(memory + pc) = (0xff000000 & temp) >> 24;
//        *(memory + pc+1) = (0x00ff0000 & temp) >> 16;
//        *(memory + pc+2) = (0x0000ff00 & temp) >> 8;
//        *(memory + pc+3) = 0x000000ff & temp;
        pc+=4;
        ret = process_instruction(code);
        //process_instruction(*((unsigned int *)(memory + pc)));
        //pc+=4;
    }
    return 0;
}


/*====================================================================*/

//#define INPUT_ASSEMBLY if(argv[0] == add ||\
//                          argv[0] == sub ||\
//                          argv[0] == and ||\
//                          argv[0] == nor ||\
//                          argv[0] == or ||\
//                          argv[0] == sll ||\
//                          argv[0] == srl ||\
//                          argv[0] == sra ||\
//                          argv[0] == lw ||\
//                          argv[0] == sw ||\
//                          argv[0] == andi ||\
//                          argv[0] == ori ||\
//                          argv[0] == addi ||\
//                          argv[0] == slt ||\
//                          argv[0] == jr ||\
//                          argv[0] == slti ||\
//                          argv[0] == beq ||\
//                          argv[0] == bne ||\
//                        argv[0] == j ||\
//                        argv[0] == jal ||\
//                          argv[0] == halt ||\
//)
#define INPUT_ASSEMBLY  add || \
                        sub || \
                        and || \
                        nor || \
                        or  || \
                        sll || \
                        srl || \
                        sra || \
                        lw  || \
                        sw  || \
                        andi|| \
                        ori || \
                        addi|| \
                        slt || \
                        jr  || \
                        slti|| \
                        beq || \
                        bne || \
                        j   || \
                        jal || \
                        halt

// Struct that stores registers and their respective binary reference
struct {
    const char *name;
    char *address;
} registerMap[] = {
        //{ "zero", "00000" },
        { "zr", "00000" },
        { "at", "00001" },
        { "v0", "00010" },
        { "v1", "00011" },
        { "a0", "00100" },
        { "a1", "00101" },
        { "a2", "00110" },
        { "a3", "00111" },
        { "t0", "01000" },
        { "t1", "01001" },
        { "t2", "01010" },
        { "t3", "01011" },
        { "t4", "01100" },
        { "t5", "01101" },
        { "t6", "01110" },
        { "t7", "01111" },
        { "s0", "10000" },
        { "s1", "10001" },
        { "s2", "10010" },
        { "s3", "10011" },
        { "s4", "10100" },
        { "s5", "10101" },
        { "s6", "10110" },
        { "s7", "10111" },
        { "t8", "11000" },
        { "t9", "11001" },
        { "k1", "11010" },
        { "k2", "11011" },
        { "gp", "11100" },
        { "sp", "11101" },
        { "fp", "11110" },
        { "ra", "11111" },
        { NULL, 0 }
    
};
// Struct for R-Type instructions mapping for the 'function' field in the instruction
struct {
    const char *name;
    char *function;
} rMap[] = {
        { "add", "100000" },
        { "sub", "100010" },
        { "and", "100100" },
        { "or" , "100101" },
        { "nor", "100111" },
        { "sll", "000000" },
        { "sra", "000011" },
        { "srl", "000010" },
        { "slt", "101010" },
        { "jr" , "001000" },
        { NULL, 0 }
};

// Struct for I-Type instructions
struct {
    const char *name;
    char *address; //=> opcode
} iMap[] = {
        { "lw"  , "100011" },
        { "sw"  , "101011" },
        { "andi", "001100" },
        { "ori" , "001101" },
        { "addi", "001000" },
        { "slti", "001010" },
        { "beq" , "000100" },
        { "bne" , "000101" },
        { NULL, 0 }
};

// Struct for J-Type instructions
struct {
    const char *name;
    char *opcode;
} jMap[] = {
        { "j"  , "000010" },
        { "jal", "000011" },
        { NULL, 0 }
};

int checkAssembly(char* instruction)
{
    if (strcmp(instruction, "add") == 0 || strcmp(instruction, "sub") == 0 || strcmp(instruction, "and") == 0 || strcmp(instruction, "or") == 0 || strcmp(instruction, "sll") == 0 || strcmp(instruction,"nor") == 0 || strcmp(instruction, "srl") == 0 || strcmp(instruction, "sra") == 0|| strcmp(instruction, "slt") == 0 || strcmp(instruction, "jr") == 0 || strcmp(instruction, "lw") == 0 || strcmp(instruction, "sw") == 0|| strcmp(instruction, "andi") == 0 || strcmp(instruction, "ori")== 0 || strcmp(instruction, "addi") == 0|| strcmp(instruction, "slti") == 0 || strcmp(instruction, "beq") == 0 || strcmp(instruction, "bne") == 0 || strcmp(instruction, "j") == 0 || strcmp(instruction, "jal")==0)
        return 1;
    else
        return 0;
}

char instruction_type(char *instruction) {

    if (strcmp(instruction, "add") == 0 || strcmp(instruction, "sub") == 0
            || strcmp(instruction, "and") == 0 || strcmp(instruction, "or")
            == 0 || strcmp(instruction, "sll") == 0 || strcmp(instruction,
            "nor") == 0 || strcmp(instruction, "srl") == 0 || strcmp(
            instruction, "sra") == 0|| strcmp(instruction, "slt") == 0 || strcmp(instruction, "jr") == 0 ) {

        return 'r';
    }

    else if (strcmp(instruction, "lw") == 0 || strcmp(instruction, "sw") == 0
            || strcmp(instruction, "andi") == 0 || strcmp(instruction, "ori")
            == 0 || strcmp(instruction, "addi") == 0|| strcmp(instruction, "slti") == 0 || strcmp(instruction, "beq") == 0 || strcmp(instruction, "bne") == 0 ) {

        return 'i';
    }
    else if(strcmp(instruction, "j") == 0 || strcmp(instruction, "jal") == 0){
        return 'j';
    }
    return 0;
}
void getBin(int num, char *str, int padding) {

    *(str + padding) = '\0';

    long pos;
    if (padding == 5)
        pos = 0x10;
    else if (padding == 16)
        pos = 0x8000;
    else if (padding == 26)
        pos = 0x2000000;
    else if (padding == 32)
            pos = 0x80000000;

    long mask = pos;
    while (mask){         // 이해 잘 안감.....
        *str++ = !!(mask & num) + '0';
        mask >>= 1;
    }
}

// Return the binary representation of the register
char *register_address(char *registerName) {

    size_t i;
    for (i = 0; registerMap[i].name != NULL; i++) {
        if (strcmp(registerName, registerMap[i].name) == 0) {
            return registerMap[i].address;
        }
    }
    return NULL;
}

// Write out the R-Type instruction
unsigned int rtype_instruction(char *instruction, char *rs, char *rt, char *rd, int shamt) {

    long instcode;
    unsigned int code32;
    char* endptr;
    // Set the instruction bits
    char *opcode = "000000";
    
    char *rdBin = "00000"; // 0초기화
    if (strcmp(rd, "00000") != 0)
        rdBin = register_address(rd);

    char *rsBin = "00000";
    if (strcmp(rs, "00000") != 0)
        rsBin = register_address(rs);

    char *rtBin = "00000";
    if (strcmp(rt, "00000") != 0)
        rtBin = register_address(rt);

    char *func = NULL;
    char shamtBin[6];

    // Convert shamt to binary and put in shamtBin as a char*
    getBin(shamt, shamtBin, 5);

    size_t i;
    for (i = 0; rMap[i].name != NULL; i++) {
        if (strcmp(instruction, rMap[i].name) == 0) {
            func = rMap[i].function;
        }
    }

    // Print out the instruction to the file
    //printf("%s%s%s%s%s%s\n", opcode, rsBin, rtBin, rdBin, shamtBin, func);
    
    instcode = strtoimax(opcode, &endptr, 2);
    code32 = (instcode & 0x3f) << 26;
    instcode = strtoimax(rsBin, &endptr, 2);
    code32 |= (instcode & 0x1f) << 21;
    instcode = strtoimax(rtBin, &endptr, 2);
    code32 |= (instcode & 0x1f) << 16;
    instcode = strtoimax(rdBin, &endptr, 2);
    code32 |= (instcode & 0x1f) << 11;
    instcode = strtoimax(shamtBin, &endptr, 2);
    code32 |= (instcode & 0x1f) << 6;
    instcode = strtoimax(func, &endptr, 2);
    code32 |= (instcode & 0x3f);

    
    return code32;
}

// Write out the I-Type instruction
unsigned int itype_instruction(char *instruction, char *rs, char *rt, int immediateNum) {

    long instcode;
    unsigned int code32;
    char* endptr;
    // Set the instruction bits
    char *rsBin = "00000";
    if (strcmp(rs, "00000") != 0)
        rsBin = register_address(rs);

    char *rtBin = "00000";
        if (strcmp(rt, "00000") != 0)
            rtBin = register_address(rt);

    char *opcode = NULL;
    char immediate[17];

    size_t i;
    for (i = 0; iMap[i].name != NULL; i++) {
        if (strcmp(instruction, iMap[i].name) == 0) {
            opcode = iMap[i].address;
        }
    }

    // Convert immediate to binary
    getBin(immediateNum, immediate, 16);

    // Print out the instruction to the file
    //printf("%s%s%s%s\n", opcode, rsBin, rtBin, immediate);
    instcode = strtoimax(opcode, &endptr, 2);
    code32 = (instcode & 0x3f) << 26;
    instcode = strtoimax(rsBin, &endptr, 2);
    code32 |= (instcode & 0x1f) << 21;
    instcode = strtoimax(rtBin, &endptr, 2);
    code32 |= (instcode & 0x1f) << 16;
    instcode = strtoimax(immediate, &endptr, 2);
    code32 |= (instcode & 0xffff);
    
    return code32;
}

unsigned int jtype_instruction(char *instruction, int address) {

    long instcode;
    char *opcode = NULL;
    char* endptr;
    unsigned int code32;

    size_t i;
    for (i = 0; jMap[i].name != NULL; i++) {
        if (strcmp(instruction, jMap[i].name) == 0) {
            opcode = jMap[i].opcode;
        }
    }
//    char addressBin[27];
//    getBin(address, addressBin, 26);
//
//    instcode = strtoimax(opcode, &endptr, 2);
//    code32 = (instcode & 0x3f) << 26;
//
//    instcode = strtoimax(addressBin, &endptr, 2);
//    instcode = instcode>>2;
//    code32 |= instcode;
//    return code32;
    char addressBin[33];
    getBin(address, addressBin, 32);
    
    instcode = strtoimax(opcode, &endptr, 2);
    code32 = (instcode & 0x3f) << 26;

    instcode = strtoimax(addressBin, &endptr, 2);
    instcode = instcode & 0x0fffffff;
    instcode = instcode>>2;
    code32 |= instcode;
    return code32;
}

static unsigned int translate(int nr_tokens, char *tokens[])
{
    /* TODO:
     * This is an example MIPS instruction. You should change it accordingly.
     */
    unsigned int code32 =0;
    char inst_type;
    char* token = tokens[0];
    char* endptr;
    inst_type = instruction_type(token);
    
    if(inst_type == 'r')
    {
        // R-Type with $rd, $rs, $rt format
        if (strcmp(token, "add") == 0 || strcmp(token, "sub") == 0|| strcmp(token, "and") == 0|| strcmp(token, "or") == 0 || strcmp(token, "nor") == 0 || strcmp(token, "slt") == 0){
          code32 = rtype_instruction(tokens[0], tokens[2], tokens[3], tokens[1], 0);
        }
        // R-Type with $rd, $rt, shamt format
        else if (strcmp(token, "sll") == 0 || strcmp(token, "srl") == 0|| strcmp(token, "sra") == 0){
            //shamt가 16진수로 들어올 경우를 처리
            if(strncmp(tokens[3], "0x", 2) == 0 || strncmp(tokens[3], "-0x", 3) == 0){
                code32 = rtype_instruction(tokens[0], "00000", tokens[2], tokens[1], (int)(strtoimax(tokens[3], &endptr, 0)));
            }                                     // rs가 없음 -> 0
            else{ //shamt가 정수일 경우
                code32 = rtype_instruction(tokens[0], "00000", tokens[2], tokens[1], atoi(tokens[3]));
            }
        }
        else // jr
        {
            code32 = rtype_instruction(tokens[0], tokens[1], "00000", "00000", 0);

        }
    }
    else if (inst_type == 'i')
    {
        //address가 16진수로 들어올 경우를 처리
        if(strncmp(tokens[3], "0x", 2) == 0 || strncmp(tokens[3], "-0x", 3) == 0){
            code32 = itype_instruction(tokens[0], tokens[2], tokens[1], (int)(strtoimax(tokens[3], &endptr, 0)));
        }
        else{        //address가 정수로 들어올 경우를 처리
            code32 = itype_instruction(tokens[0], tokens[2], tokens[1], atoi(tokens[3]));
        }
    }
    else if (inst_type == 'j')
    {
        code32 = jtype_instruction(tokens[0], (int)(strtoimax(tokens[1], &endptr, 0)));
    }
    else if (!strcmp(token, "halt"))
    {
        // code32 = 0xffffffff
        return 0xffffffff;
    }
    return code32;
}

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
static void __show_registers(char * const register_name)
{
    int from = 0, to = 0;
    bool include_pc = false;

    if (strmatch(register_name, "all")) {
        from = 0;
        to = 32;
        include_pc = true;
    } else if (strmatch(register_name, "pc")) {
        include_pc = true;
    } else {
        for (int i = 0; i < sizeof(register_names) / sizeof(*register_names); i++) {
            if (strmatch(register_name, register_names[i])) {
                from = i;
                to = i + 1;
            }
        }
    }

    for (int i = from; i < to; i++) {
        fprintf(stderr, "[%02d:%2s] 0x%08x    %u\n", i, register_names[i], registers[i], registers[i]);
    }
    if (include_pc) {
        fprintf(stderr, "[  pc ] 0x%08x\n", pc);
    }
}

static void __dump_memory(unsigned int addr, size_t length)
{
    for (size_t i = 0; i < length; i += 4) {
        fprintf(stderr, "0x%08lx:  %02x %02x %02x %02x    %c %c %c %c\n",
                addr + i,
                memory[addr + i    ], memory[addr + i + 1],
                memory[addr + i + 2], memory[addr + i + 3],
                isprint(memory[addr + i    ]) ? memory[addr + i    ] : '.',
                isprint(memory[addr + i + 1]) ? memory[addr + i + 1] : '.',
                isprint(memory[addr + i + 2]) ? memory[addr + i + 2] : '.',
                isprint(memory[addr + i + 3]) ? memory[addr + i + 3] : '.');
    }
}

static void __process_command(int argc, char *argv[])
{
    if (argc == 0) return;

    if (strmatch(argv[0], "load")) {
        if (argc == 2) {
            load_program(argv[1]);
        } else {
            printf("Usage: load [program filename]\n");
        }
    } else if (strmatch(argv[0], "run")) {
        if (argc == 1) {
            run_program();
        } else {
            printf("Usage: run\n");
        }
    } else if (strmatch(argv[0], "show")) {
        if (argc == 1) {
            __show_registers("all");
        } else if (argc == 2) {
            __show_registers(argv[1]);
        } else {
            printf("Usage: show { [register name] }\n");
        }
    } else if (strmatch(argv[0], "dump")) {
        if (argc == 3) {
            __dump_memory(strtoimax(argv[1], NULL, 0), strtoimax(argv[2], NULL, 0));
        } else {
            printf("Usage: dump [start address] [length]\n");
        }
    } else {
        /**
         * You may hook up @translate() from pa1 here to allow assembly input!
         */
        
//#ifdef INPUT_ASSEMBLY
        if(checkAssembly(argv[0]))
        {
            unsigned int instr = translate(argc, argv);
            process_instruction(instr);
        }
        else
            process_instruction(strtoimax(argv[0], NULL, 0));
//#elsee
            //process_instruction(strtoimax(argv[0], NULL, 0));
//#endif
    }
}

static int __parse_command(char *command, int *nr_tokens, char *tokens[])
{
    char *curr = command;
    int token_started = false;
    *nr_tokens = 0;

    while (*curr != '\0') {
        if (isspace(*curr)) {
            *curr = '\0';
            token_started = false;
        } else {
            if (!token_started) {
                tokens[*nr_tokens] = curr;
                *nr_tokens += 1;
                token_started = true;
            }
        }
        curr++;
    }

    /* Exclude comments from tokens */
    for (int i = 0; i < *nr_tokens; i++) {
        if (strmatch(tokens[i], "//") || strmatch(tokens[i], "#")) {
            *nr_tokens = i;
            tokens[i] = NULL;
        }
    }

    return 0;
}

int main(int argc, char * const argv[])
{
    char command[MAX_COMMAND] = {'\0'};
    FILE *input = stdin;

    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "No input file %s\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (input == stdin) {
        printf("*********************************************************\n");
        printf("*          >> SCE212 MIPS Simulator v0.01 <<            *\n");
        printf("*                                                       *\n");
        printf("*                                       .---.           *\n");
        printf("*                           .--------.  |___|           *\n");
        printf("*                           |.------.|  | =.|           *\n");
        printf("*                           || >>_  ||  |---|           *\n");
        printf("*                           |'------'|  |   |           *\n");
        printf("*                           ')______('~~|___|           *\n");
        printf("*                                                       *\n");
        printf("*                                   Fall 2021           *\n");
        printf("*********************************************************\n\n");
        printf(">> ");
    }

    while (fgets(command, sizeof(command), input)) {
        char *tokens[MAX_NR_TOKENS] = { NULL };
        int nr_tokens = 0;

        for (size_t i = 0; i < strlen(command); i++) {
            command[i] = tolower(command[i]);
        }

        if (__parse_command(command, &nr_tokens, tokens) < 0)
            continue;

        __process_command(nr_tokens, tokens);

        if (input == stdin) printf(">> ");
    }

    if (input != stdin) fclose(input);

    return EXIT_SUCCESS;
}
