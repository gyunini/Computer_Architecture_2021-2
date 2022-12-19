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
#include <inttypes.h>
#include <ctype.h>

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

enum cache_simulator_constants {
    CACHE_HIT = 0,
    CACHE_MISS,

    CB_INVALID = 0,    /* Cache block is invalid */
    CB_VALID = 1,    /* Cache block is valid */

    CB_CLEAN = 0,    /* Cache block is clean */
    CB_DIRTY = 1,    /* Cache block is dirty */

    BYTES_PER_WORD = 4,    /* This is 32 bit machine (1 word is 4 bytes) */
    MAX_NR_WORDS_PER_BLOCK = 32,    /* Maximum cache block size */
};


typedef unsigned char bool;
#define true  1
#define false 0

/* 8 KB Main memory */
static unsigned char memory[8 << 10] = {
    0xde, 0xad, 0xbe, 0xef, 0xba, 0xda, 0xca, 0xfe,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    'h',  'e',  'l',  'l',  'o',  ' ' , 'w' , 'o',
    'r',  'l',  'd',  '!',  0x89, 0xab, 0xcd, 0xef,
    0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e,
    0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e,
    0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e,
};

/* Cache block */
struct cache_block {
    bool valid;                /* Whether the block is valid or invalid.
                               Use CB_INVALID or CB_VALID macro above  */
    bool dirty;                /* Whether te block is updated or not.
                               Use CB_CLEAN or CB_DIRTY macro above */
    unsigned int tag;        /* Tag */
    unsigned int timestamp;    /* Timestamp or clock cycles to implement LRU */
    unsigned char data[BYTES_PER_WORD * MAX_NR_WORDS_PER_BLOCK];
                            /* Each block holds 4 words */
};

/* An 1-D array for cache blocks. */
static struct cache_block *cache = NULL;

/* The size of cache block. The value is set during the initialization */
static int nr_words_per_block = 4;

/* Number of cache blocks. The value is set during the initialization */
static int nr_blocks = 16;

/* Number of ways for the cache. Note @nr_ways == 1 means direct mapped cache
 * and @nr_ways == nr_blocks implies fully associative cache */
static int nr_ways = 2;

/* Number of @nr_ways-way sets in the cache. This value will be set according to
 * @nr_blocks and @nr_ways values */
static int nr_sets = 8;

/* Clock cycles */
const int cycles_hit = 1;
const int cycles_miss = 100;

/* Clock cycles so far */
static unsigned int cycles = 0;


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

/**
 * log2_discrete
 *
 * DESCRIPTION
 *   Return the integer part of log_2(@n). FREE TO USE IN YOUR IMPLEMENTATION.
 *   Will be useful for calculating the length of tag for a given address.
 */
static int log2_discrete(int n)
{
    int result = -1;
    do {
        n = n >> 1;
        result++;
    } while (n);

    return result;
}
/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/

int check_cache_data_hit(unsigned int addr) {
    int i, first_val=0, compare_tag=0, entry_index=0;
    int byte_address = addr;
    int block_address = byte_address/(BYTES_PER_WORD * nr_words_per_block);
    int byte_offset = byte_address % (BYTES_PER_WORD * nr_words_per_block);
    int cache_index = block_address % (nr_sets);
    int Tag = block_address / (nr_sets);


    for(i=0; i < nr_ways; i++){
        struct cache_block *pEntry = &cache[cache_index * nr_ways + i];
        if(pEntry->valid != 0) {
            first_val = 1;
        }
    }
    if(first_val != 1){
        return -1;
    }
    else{ // hit인 경우
        for(i=0; i < nr_ways; i++){
            struct cache_block *pEntry = &cache[cache_index * nr_ways + i];
            if(pEntry->tag == Tag){
                entry_index = i;
                compare_tag = 1;
                break;
            }
        }
        if(compare_tag != 1){
            return -1;
        }
    }

    /* In hit case */
    struct cache_block *pEntry = &cache[cache_index * nr_ways + entry_index];
    pEntry->timestamp = cycles;
    
    return 1;
}

int find_entry_index_in_set(unsigned int addr, int cache_index) {
    int entry_index, i, LRU_timestamp, min_index;
    int byte_address = addr;
    int block_address = byte_address/(BYTES_PER_WORD * nr_words_per_block);
    int Tag = block_address / (nr_sets);

   if(nr_ways == 1){
        entry_index = 0;
       struct cache_block *pEntry = &cache[cache_index * nr_ways];
        pEntry->timestamp = cycles;
    }
    else{
        for(i=0; i<nr_ways; i++){
            struct cache_block *pEntry = &cache[cache_index * nr_ways + i];
            if(pEntry->tag == Tag){ // 이미 데이터가 캐시에 있음
                entry_index = i;
                pEntry->timestamp = cycles;
                return entry_index;
            }
        }
        for(i=0; i<nr_ways; i++){
            struct cache_block *pEntry = &cache[cache_index * nr_ways + i];
            if(pEntry->valid == 0){
                entry_index = i;
                pEntry->timestamp = cycles;
                return entry_index;
            }
        }
       
        struct cache_block *pEntry = &cache[cache_index * nr_ways];
        LRU_timestamp = pEntry->timestamp;
        min_index = 0;
        for(i=1; i<nr_ways; i++){
            struct cache_block *pEntry = &cache[cache_index * nr_ways + i];
            if(LRU_timestamp > pEntry->timestamp ){
                LRU_timestamp = pEntry->timestamp;
                min_index = i;
            }
        }
        entry_index = min_index;
        cache[cache_index * nr_ways + min_index].timestamp = cycles;
    }
    return entry_index;
}

void access_memory(unsigned int addr, int check_hit) {
    int byte_address = addr;
    int block_address = byte_address/(BYTES_PER_WORD * nr_words_per_block);
    int cache_index = block_address % (nr_sets);
    int Tag = block_address / (nr_sets);
    int entry_index;
    int block_offset = block_address % nr_ways;
    
    entry_index = find_entry_index_in_set(addr, cache_index);
    struct cache_block *pEntry = &cache[cache_index * nr_ways + entry_index];
    if(pEntry->dirty)
    {
        if(nr_blocks == nr_ways){
            memcpy(memory + pEntry->tag * (BYTES_PER_WORD * nr_words_per_block), pEntry->data, BYTES_PER_WORD * nr_words_per_block); // write back
        }
        else{
            memcpy(memory + pEntry->tag * nr_ways * (BYTES_PER_WORD * nr_words_per_block) + block_offset * (BYTES_PER_WORD * nr_words_per_block), pEntry->data, BYTES_PER_WORD * nr_words_per_block); // write back
        }
        pEntry->dirty = false;
    }
    if(check_hit == -1)
    {
        pEntry->valid = 1;
        pEntry->tag = Tag;
        memcpy(pEntry->data, memory + block_address * BYTES_PER_WORD * nr_words_per_block, BYTES_PER_WORD * nr_words_per_block);
    }
    
}
/**************************************************************************
 * load_word
 *
 * DESCRIPTION
 *   Simulate the case when the processor is handling a lw instruction for @addr.
 *   To that end, you should look up the cache blocks to find the block
 *   containing the target address @addr. If exists, it's cache hit; return
 *   CACHE_HIT after updating the cache block's timestamp with @cycles.
 *   If not, replace the LRU cache block in the set. Should handle dirty blocks
 *   properly according to the write-back semantic.
 *
 * PARAMAMETERS
 *   @addr: Target address to load
 *
 * RETURN
 *   CACHE_HIT on cache hit, CACHE_MISS otherwise
 *
 */
// 더티비트가 있으면 메모리에 write back 해야함 근데 캐시 다 차있으면 클럭이 젤 작은거를 빼내야함 근데 이때도 더티비트 있는지 고려해야 함
int load_word(unsigned int addr)
{
    /* TODO: Implement your load_word function */
    int check_hit = check_cache_data_hit(addr);

    if(check_hit == -1){
        access_memory(addr, check_hit);
        return CACHE_MISS;
    }
    else{
        return CACHE_HIT;
    }
}


/**************************************************************************
 * store_word
 *
 * DESCRIPTION
 *   Simulate the case when the processor is handling sw instruction.
 *   Cache should be write-back and write-allocate. Note that the least
 *   recently used (LRU) block should be replaced in case of eviction.
 *
 * PARAMETERS
 *   @addr: Starting address for @data
 *   @data: New value for @addr. @data is 1-word in size
 *
 * RETURN
 *   CACHE_HIT on cache hit, CACHE_MISS otherwise
 *
 */
// 메모리에 안쓰고 캐시에만 쓰고 이때 클럭 사이클 업데이트, 더티비트 세팅,
int store_word(unsigned int addr, unsigned int data)
{
    /* TODO: Implement your store_word function */
    int check_hit = check_cache_data_hit(addr);
    int entry_index;
    int byte_address = addr;
    int block_address = byte_address/(BYTES_PER_WORD * nr_words_per_block);
    int cache_index = block_address % (nr_sets);
    int Tag = block_address / (nr_sets);
    int word_offset = (byte_address / BYTES_PER_WORD) % nr_words_per_block;
    int block_offset = block_address % nr_ways;

    entry_index = find_entry_index_in_set(addr, cache_index);
    struct cache_block *pEntry = &cache[cache_index * nr_ways + entry_index];
    pEntry->valid = 1;
    pEntry->timestamp = cycles;

    if(check_hit == -1){ // miss났으면 써야됨
       
        if(pEntry->dirty){
            if(nr_blocks == nr_ways){
                memcpy(memory + pEntry->tag * (BYTES_PER_WORD * nr_words_per_block), pEntry->data, BYTES_PER_WORD * nr_words_per_block); // write back
            }
            else{
                memcpy(memory + pEntry->tag * nr_ways * (BYTES_PER_WORD * nr_words_per_block) + block_offset * (BYTES_PER_WORD * nr_words_per_block), pEntry->data, BYTES_PER_WORD * nr_words_per_block); // write back
            }
        }
        pEntry->tag = Tag;
        pEntry->dirty = true;
        
        memcpy(pEntry->data, memory + block_address * BYTES_PER_WORD * nr_words_per_block, BYTES_PER_WORD * nr_words_per_block);
        memcpy(pEntry->data + word_offset * BYTES_PER_WORD + 3, (unsigned char*)&data, 1);
        memcpy(pEntry->data + word_offset * BYTES_PER_WORD + 2, (unsigned char*)&data + 1, 1);
        memcpy(pEntry->data + word_offset * BYTES_PER_WORD + 1, (unsigned char*)&data + 2, 1);
        memcpy(pEntry->data + word_offset * BYTES_PER_WORD, (unsigned char*)&data + 3, 1);
        return CACHE_MISS;
    }
    else
    {
//        if(pEntry->dirty){
//            if(nr_blocks == nr_ways){
//                memcpy(memory + block_offset * (BYTES_PER_WORD * nr_words_per_block), pEntry->data, BYTES_PER_WORD * nr_words_per_block); // write back
//            }
//            else{
//                memcpy(memory + pEntry->tag * nr_ways * (BYTES_PER_WORD * nr_words_per_block) + block_offset * (BYTES_PER_WORD * nr_words_per_block), pEntry->data, BYTES_PER_WORD * nr_words_per_block); // write back
//            }
//        }
        pEntry->tag = Tag;
        pEntry->dirty = true;
        
        memcpy(pEntry->data + word_offset * BYTES_PER_WORD + 3, (unsigned char*)&data, 1);
        memcpy(pEntry->data + word_offset * BYTES_PER_WORD + 2, (unsigned char*)&data + 1, 1);
        memcpy(pEntry->data + word_offset * BYTES_PER_WORD + 1, (unsigned char*)&data + 2, 1);
        memcpy(pEntry->data + word_offset * BYTES_PER_WORD, (unsigned char*)&data + 3, 1);
        return CACHE_HIT;
    }
}


/**************************************************************************
 * init_simulator
 *
 * DESCRIPTION
 *   This function is called before starting the simulation. This is the
 *   perfect place to put your initialization code.
 */
void init_simulator(void)
{
    /* TODO: You may place your initialization code here */
}



/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
static void __show_cache(void)
{
    for (int i = 0; i < nr_blocks; i++) {
        fprintf(stderr, "[%3d] %c%c %8x %8u | ", i,
                cache[i].valid == CB_VALID ? 'v' : ' ',
                cache[i].dirty == CB_DIRTY ? 'd' : ' ',
                cache[i].tag, cache[i].timestamp);
        for (int j = 0; j < BYTES_PER_WORD * nr_words_per_block; j++) {
            fprintf(stderr, "%02x", cache[i].data[j]);
            if ((j + 1) % 4 == 0) fprintf(stderr, " ");
        }
        fprintf(stderr, "\n");
        if (nr_ways > 1 && ((i + 1) % nr_ways == 0)) printf("\n");
    }
}

static void __dump_memory(unsigned int start)
{
    for (int i = start; i < start + 64; i++) {
        if (i % 16 == 0) {
            fprintf(stderr, "[0x%08x] ", i);
        }
        fprintf(stderr, "%02x", memory[i]);
        if ((i + 1) % 4 == 0) fprintf(stderr, " ");
        if ((i + 1) % 16 == 0) fprintf(stderr, "\n");
    }
}

static void __init_cache(void)
{
    cache = (struct cache_block *)malloc(sizeof(struct cache_block) * nr_blocks);

    for (int i = 0; i < nr_blocks; i++) {
        struct cache_block *c = cache + i;

        c->valid = CB_INVALID;
        c->dirty = CB_CLEAN;
        c->tag = 0;
        c->timestamp = 0;
        memset(c->data, 0x00, sizeof(c->data));
    }
}

static void __fini_cache(void)
{
    free(cache);
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

static void __simulate_cache(FILE *input)
{
    int argc;
    char *argv[10];
    char command[80];

    unsigned int hits = 0, misses = 0;

    __init_cache();
    if (input == stdin) printf(">> ");

    while (fgets(command, sizeof(command), input)) {
        unsigned int addr;
        int hit;

        __parse_command(command, &argc, argv);

        if (argc == 0) continue;

        if (strmatch(argv[0], "show")) {
            __show_cache();
            goto next;
        } else if (strmatch(argv[0], "dump")) {
            addr = argc == 1 ? 0 : strtoimax(argv[1], NULL, 0) & 0xfffffffc;
            __dump_memory(addr);
            goto next;
        } else if (strmatch(argv[0], "cycles")) {
            fprintf(stderr, "%3u %3u   %u\n", hits, misses, cycles);
            goto next;
        } else if (strmatch(argv[0], "quit")) {
            break;
        } if (strmatch(argv[0], "lw")) {
            if (argc == 1) {
                printf("Wrong input for lw\n");
                printf("Usage: lw <address to load>\n");
                goto next;
            }
            addr = strtoimax(argv[1], NULL, 0);
            hit = load_word(addr);
        } else if (strmatch(argv[0], "sw")) {
            if (argc != 3) {
                printf("Wrong input for sw\n");
                printf("Usage: sw <address to store> <word-size value to store>\n");
                goto next;
            }
            addr = strtoimax(argv[1], NULL, 0);
            hit = store_word(addr, strtoimax(argv[2], NULL, 0));
        } else {
            goto next;
        }

        if (hit == CACHE_HIT) {
            hits++;
            cycles += cycles_hit;
        } else {
            misses++;
            cycles += cycles_miss;
        }
next:
        if (input == stdin) printf(">> ");
    }

    __fini_cache();
}

int main(int argc, const char *argv[])
{
    FILE *input = stdin;

    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (!input) {
            perror("Input file error");
            return EXIT_FAILURE;
        }
    }


    if (input == stdin) {
        printf("*****************************************************\n");
        printf("*                    _                              *\n");
        printf("*      ___ __ _  ___| |__   ___                     *\n");
        printf("*     / __/ _` |/ __| '_ \\ / _ \\                    *\n");
        printf("*    | (_| (_| | (__| | | |  __/                    *\n");
        printf("*     \\___\\__,_|\\___|_| |_|\\___|                    *\n");
        printf("*          _                 _       _               *\n");
        printf("*     ___(_)_ __ ___  _   _| | __ _| |_ ___  _ __   *\n");
        printf("*    / __| | '_ ` _ \\| | | | |/ _` | __/ _ \\| '__|  *\n");
        printf("*    \\__ \\ | | | | | | |_| | | (_| | || (_) | |     *\n");
        printf("*    |___/_|_| |_| |_|\\__,_|_|\\__,_|\\__\\___/|_|     *\n");
        printf("*                                                   *\n");
        printf("*                                   2021.12         *\n");
        printf("*****************************************************\n\n");
    }

#ifndef _USE_DEFAULT
    if (input == stdin) printf("- words per block:  ");
    fscanf(input, "%d", &nr_words_per_block);
    if (input == stdin) printf("- number of blocks: ");
    fscanf(input, "%d", &nr_blocks);
    if (input == stdin) printf("- number of ways:   ");
    fscanf(input, "%d", &nr_ways);

    nr_sets = nr_blocks / nr_ways;
#endif

    init_simulator();
    __simulate_cache(input);

    if (input != stdin) fclose(input);

    return EXIT_SUCCESS;
}
