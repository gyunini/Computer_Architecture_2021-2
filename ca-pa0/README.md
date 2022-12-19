## Project #0: Command Parser

### ***Due on  September 18, 12am ***

### Goal

Implement a simple string parser to get familiar with PASubmit system.


### Problem Specification

We will implement a MIPS interpreter soon. To that end, you should firstly make the shell program to understand user inputs and commands.

In this project, the framework passes input strings, and your task is to **split the input strings into command tokens**. By C standard, a string is defined as a sequence of characters which is terminated with `\0`. Command tokens are strings that are obtained by splitting the input string with delimitors, which are whitespace characters in this PA. Use `isspace()` function to check whether a character is whitespace or not. Any heading and trailing whitespaces should be also removed from the command tokens.

For example, if the input string is " Hello   world ", the command tokes are "Hello" and "world". **Note that the input string is split into tokens delimited by whitespaces (' ' in this example), and the tokens do not contain whitespaces**.

Another example string is " add   r1  r2   r3   ". It should be broken into "add", "r1", "r2", and "r3".

The required functionality must be implemented in `parse_command()` function. The user input is passed through `command`. After parsing `command` into tokens, the tokens should be assigned to `tokens[]` in order, which is passed as the argument of the `parse_command()` function. Also, the number of tokens should be assigined to `*nr_tokens` which will be used by the framework.

```c
char *command   --> "  add   r0   r1 r2  "

*nr_tokens = 4
tokens[0]  -->  "add"
tokens[1]  -->  "r0"
tokens[2]  -->  "r1"
tokens[3]  -->  "r2"
tokens[4]... =  NULL
```

### Restrictions

- You **should not use any string manipulation functions from any libraries**. The banned functions include `strtok, strtok_r, strlen, strcpy, fscanf, strdup` and/or similars. This implies that you should implement your own string manipulation functions if it is needed. **You will get 0 point if you use any of them**. Note that `malloc()` and `free()` are not string manipulation functions and OK to use. If unsure, question to the instructor through AjouBb.
- Use `isspace()` C library function to check whether the character can be assumed as a whitespace.
- Printing messages to stand output (e.g., `printf()`) is totally OK. However, do not `fprintf(stderr ...)` otherwise the grading system cannot grade your submission properly.


### Logistics

- This is an individual project; you work on the assignement alone.
- You can use **up to 3 slip tokens throughout this semester**.

- All submission and grading are done automatically through https://sslab.ajou.ac.kr/pasubmit. Please follow the instruction explained in the class.
	- Submit `pa0.c` file to the PA submission site at https://sslab.ajou.ac.kr/pasubmit. (80 pts)
	- Explain your parsing strategy on a **1-page PDF document**. Submit the document through the PASubmit. (20 pts)
- `input` file in the provided code contains the input sequence for the grading.


### Tips and Notes

- Briefly speaking, the implementation might look like; start scanning from the beginning of `command` skipping all whitespaces until meet a non-whitespace character. That will be the beginning of the token. Then, resume scanning until meet any whitespace character. That will be the end of the token. Repeat this until entire `command` is scanned. Assign the starting position of each token into `tokens[]` and count `nr_tokens` accordingly.
- Read comments in the skeleton code carefully.
- Make sure every string is terminated with '\0'.
- Post freely on QnA board on AjouBb to question about the project.

Good luck and have fun!
