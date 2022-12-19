/**********************************************************************
 * Copyright (c) 2021
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
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>

/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
#define MAX_NR_TOKENS	32	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	64	/* Maximum length of single token */
#define MAX_ASSEMBLY	256 /* Maximum length of assembly string */

typedef unsigned char bool;
#define true	1
#define false	0
/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/


/***********************************************************************
 * translate()
 *
 * DESCRIPTION
 *   Translate assembly represented in @tokens[] into a MIPS instruction.
 *   This translate should support following 13 assembly commands
 *
 *    - add
 *    - addi
 *    - sub
 *    - and
 *    - andi
 *    - or
 *    - ori
 *    - nor
 *    - lw
 *    - sw
 *    - sll
 *    - srl
 *    - sra
 *
 * RETURN VALUE
 *   Return a 32-bit MIPS instruction
 *
 */

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
    char addressBin[33];
    getBin(address, addressBin, 32); // 32bit 주소를 바이너리 스트링으로 바꿈
    
    instcode = strtoimax(opcode, &endptr, 2);
    code32 = (instcode & 0x3f) << 26; // opcode 만듬

    instcode = strtoimax(addressBin, &endptr, 2);
    instcode = instcode & 0x0fffffff; // 0x2048같이 입력된 타겟주소에서 맨앞 4비트 pc값을 뗌
    instcode = instcode>>2;             // 글고 2비트 쉬프트=> 000000,address(26bit)이렇게 됨
    code32 |= instcode;                 // opcode랑 address 합침
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



/***********************************************************************
 * parse_command()
 *
 * DESCRIPTION
 *   Parse @assembly, and put each assembly token into @tokens[] and the number of
 *   tokes into @nr_tokens. You may use this implemention or your own from PA0.
 *
 *   A assembly token is defined as a string without any whitespace (i.e., *space*
 *   and *tab* in this programming assignment). For exmaple,
 *     command = "  add t1   t2 s0 "
 *
 *   then, nr_tokens = 4, and tokens is
 *     tokens[0] = "add"
 *     tokens[1] = "t0"
 *     tokens[2] = "t1"
 *     tokens[3] = "s0"
 *
 *   You can assume that the input string is all lowercase for testing.
 *
 * RETURN VALUE
 *   Return 0 after filling in @nr_tokens and @tokens[] properly
 *
 */
static bool __is_separator(char *c)
{
	char *separators = " \t\r\n,.";

	for (size_t i = 0; i < strlen(separators); i++) {
		if (*c == separators[i]) return true;	
	}

	return false;
}
static int parse_command(char *assembly, int *nr_tokens, char *tokens[])
{
	char *curr = assembly;
	int token_started = false;
	*nr_tokens = 0;

	while (*curr != '\0') {  
		if (__is_separator(curr)) {  
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

	return 0;
}



/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */

/***********************************************************************
 * The main function of this program.
 */
int main(int argc, char * const argv[])
{
	char assembly[MAX_ASSEMBLY] = { '\0' };
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
		printf("*          >> SCE212 MIPS translator  v0.01 <<          *\n");
		printf("*                                                       *\n");
		printf("*                                       .---.           *\n");
		printf("*                           .--------.  |___|           *\n");
		printf("*                           |.------.|  |=. |           *\n");
		printf("*                           || >>_  ||  |-- |           *\n");
		printf("*                           |'------'|  |   |           *\n");
		printf("*                           ')______('~~|___|           *\n");
		printf("*                                                       *\n");
		printf("*                                   Fall 2021           *\n");
		printf("*********************************************************\n\n");
		printf(">> ");
	}

	while (fgets(assembly, sizeof(assembly), input)) {
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;
		unsigned int machine_code;

		for (size_t i = 0; i < strlen(assembly); i++) {
			assembly[i] = tolower(assembly[i]);
		}

		if (parse_command(assembly, &nr_tokens, tokens) < 0)
			continue;

		machine_code = translate(nr_tokens, tokens);

		fprintf(stderr, "0x%08x\n", machine_code);

		if (input == stdin) printf(">> ");
	}

	if (input != stdin) fclose(input);

	return EXIT_SUCCESS;
}
