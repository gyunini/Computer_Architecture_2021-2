## Project #1: MIPS Assembly to Machine Instruction Translator

### ***Due on 12am, October 20 (Wednesday)***

### Goal

Translate MIPS assembly code into corresponding MIPS machine code.


### Problem Specification

- Implement a MIPS assembly translator that translate MIPS assembly into MIPS machine code one line at a time. With the command parser of PA0, translate tokens into bits, and merge the bits, and produce one 32-bit machine instruction.

- The framework will get a line of input from CLI, make it to lowercase, and call `parse_command()`. After getting the number of tokens and `tokens[]` as of PA0, the framework will call `translate()` function with them. Write your code in the function to translate the tokenized assembly and return a 32-bit MIPS machine instruction.

- The translator should support following MIPS assembly instructions.

  | Name   | Opcode / opcode + funct |
  | ------ | ----------------------- |
  | `add`  | 0 + 0x20                |
  | `addi` | 0x08                    |
  | `sub`  | 0 + 0x22                |
  | `and`  | 0 + 0x24                |
  | `andi` | 0x0c                    |
  | `or`   | 0 + 0x25                |
  | `ori`  | 0x0d                    |
  | `nor`  | 0 + 0x27                |
  | `sll`  | 0 + 0x00                |
  | `srl`  | 0 + 0x02                |
  | `sra`  | 0 + 0x03                |
  | `lw`   | 0x23                    |
  | `sw`   | 0x2b                    |


- R-format instructions are inputted as follow:
  ```
  add s0 t1 gp    /* s0 <- t1 + gp */
  sub s4 s1 zero  /* s4 <- s1 + zero */
  sll s0 s2 3     /* s0 <- s2 << 3. shift amount comes to the last */
  sra s1 t0 -5    /* s1 <- t0 >> -5 w/ sign extension */
  ```

- I-format instructions are the similar
  ```
  addi s1 s2 0x16 /* s1 <- 0x16(s2). Immeidate values and address offset
                     come to the last */
  lw t0 s1 32     /* Load t0 with a word at 32(s1) (s1 + 32) */
  ```

- The machine has 32 registers and they are specified in the assembly as follow;

  | Name   | Number |
  | ------ | ------ |
  | zero   | 0      |
  | at     | 1      |
  | v0, v1 | 2-3    |
  | a0-a3  | 4-7    |
  | t0-t7  | 8-15   |
  | s0-s7  | 16-23  |
  | t8-t9  | 24-25  |
  | k1-k2  | 26-27  |
  | gp     | 28     |
  | sp     | 29     |
  | fp     | 30     |
  | ra     | 31     |

- `shamt` and immediate values are inputted as either (1) decimal numbers without any prefix or (2) hexadecimal numbers with `0x` as its prefix. Followings are the examples
  ```
  10    /* 10 */
  -22   /* -22 */
  0x1d  /* 29 */
  -0x42 /* -66 */
  ```

- Unspecified register and `shamt` parts should be all 0's. For example, `shamt` part should be 0b00000 for `add` instruction. Likewise, `rs` part should be 0b00000 for `sll` instruction.


### Example
```
*********************************************************
*          >> SCE212 MIPS translator  v0.01 <<          *
*                                                       *
*                                       .---.           *
*                           .--------.  |___|           *
*                           |.------.|  |=. |           *
*                           || >>_  ||  |-- |           *
*                           |'------'|  |   |           *
*                           ')______('~~|___|           *
*                                                       *
*                                   Fall 2021           *
*********************************************************

>> add t0 t1 t2
0x012a4020
>> addi sp sp -0x10
0x23bdfff0
>> sll t0 t1 10
0x00094280
```

- DO NOT MODIFY CODES IN SPECIFIED ZONES.

- You can submit up to 30 times for this PA.

### Hints/Tips

- `parse_command()` is provided for your convenience, but you may use your own reimplement if you want to.

- Now you can use any standard C library functions such as `strlen` and `strcpy`. However, Windows-specific functions are banned and it will make a compile error on the server.

- You may use *a mask* to include `shamt` part in R-format and constant part in I-format into the machine code properly. Use bitwise operations (`<<, >>, |, &`).

- Helpful functions:
  - `strcmp/strncmp`: For matching commands and register names
  - `strtol/strtoimax`: Converting decimal/hexadecimal numbers (regardless of sign) in string to corresponding long/int numbers

- Tip: You can use `0b` and `0x` prefix in C code to directly specify a binary/hexadecimal number.


### Submission / Grading

- Use [PAsubmit](https://sslab.ajou.ac.kr/pasubmit) for submission
  - 180 pts in total

- Source: pa1.c (150 pts)
  - Will be tested with the testcase files under the `testcases` directory.
  - Have a look at `Makefile` for testing your implementation with these testcase files.

- Document: *One PDF* document (30 pts)
  - Must include all the followings;
    - How do you translate the instructions
      - R-format / I-format
    - How do you translate the register names to corresponding register numbers
    - How do you convert shamt/immediate values from strings to corresponding numbers
    - Lesson learned (if you have any)
  - NO MORE THAN ***TWO*** PAGES
  - DO NOT INCLUDE COVER PAGE
    - No need to specify your name nor student ID on the document
  - Please, do not literally read C code nor paste a screenshot of it. If you cannot explain your implementation without showing it, it means that something is not right in the way of your code writing or code explanation.

- WILL NOT ANSWER THE QUESTIONS ABOUT THOSE ALREADY SPECIFIED ON THE HANDOUT.
