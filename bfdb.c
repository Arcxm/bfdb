#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "bfdb"
#define COMMAND_SZ 32

#define PROGRAM_SIZE 4096
#define STACK_SIZE 512
#define DATA_SIZE 65535

// Intermediate representation

const char* COMMANDS[] = { "EOF", ">", "<", "+", "-", ".", ",", "[", "]" };

enum {
    OP_END, OP_INC, OP_DEC, OP_ADD, OP_SUB, OP_OUT, OP_IN, OP_JMP, OP_RET
};

/// An instruction containing an operator and an operand
typedef struct instruction_t {
    unsigned short operator;
    unsigned short operand;
} instruction_t;

// Helper function

/// Helper function that splits a str by delimiters and returns a c-string array and the count of strings splitted
/// @param str The string to split
/// @param at The delimiters to split at
/// @param count The count of substrings
/// @return An array of c-strings that contains the substrings
char **split(const char *const str, const char *const at, int *count);

// "Compiling" the file to instructions

typedef struct program_t {
    /// The instructions of the brainfuck program
    instruction_t instructions[PROGRAM_SIZE];

    /// The stack that is used to keep track of jumps during compilation
    unsigned short stack[STACK_SIZE];

    /// The stack pointer
    unsigned int esp;
} program_t;

program_t program = { .esp = 0 };

/// "Compiles" the brainfuck program in fp to the intermediate representation
/// @param fp The file to read
/// @param prog The program structure to write the program to
/// @return Whether or not the compilation succeeded
bool compile(FILE *fp, program_t *prog);

// bfdb vars

/// Whether or not bfdb should continue running
static bool run = true;

/// Whether or not a brainfuck program has been loaded
static bool loaded = false;

/// Running brainfuck instance
typedef struct runtime_t {
    /// Whether or not brainfuck is currently running
    bool running;

    /// The cells
    unsigned short data[DATA_SIZE];

    /// The program counter
    unsigned short pc;

    /// The data pointer
    unsigned int ptr;
} runtime_t;

runtime_t runtime = { .running = false, .pc = 0, .ptr = 0 };

// Debugger actions

/// Parses the command given in the cli
/// @param cmd The command to parse
void parse_command(const char *cmd);

/// Load a brainfuck program from a file
/// @param file_name The name of the file
void dbg_load(const char *const file_name);

/// Prints a formatted error as well as runtime information to stderr and stops execution
/// @param fmt The format
void dbg_error(const char *fmt, ...);

/// Start execution of the loaded brainfuck program
void dbg_run();

/// Step in execution
void dbg_next();

/// Prints the data pointer
void dbg_print_dataptr();

/// Print the cell at the given index
/// @param index The index of the cell to print
void dbg_print(int index);

/// Print the operator at the current program counter
void dbg_print_op();

/// The programs entry point
/// @param argc The argument count
/// @param argv A c-string array of the arguments
/// @returns The exit code
int main(int argc, char **argv) {
    if (argc > 1) {
        dbg_load(argv[1]);
    }

    while (run) {
        if (runtime.running) {
            dbg_print_op();
        }

        fprintf(stdout, "(%s) ", TAG);

        char buf[COMMAND_SZ] = {0};
        fgets(buf, COMMAND_SZ, stdin);
        buf[strcspn(buf, "\n")] = '\0';

        parse_command(buf);
    }

    return EXIT_SUCCESS;
}

char **split(const char *const str, const char *const at, int *count) {
	if (str && at) {
		int len = 0;
		char **splitted = NULL;
		
		char *_str = strdup(str);
		char *p = NULL;
		p = strtok(_str, at);

		while (p != NULL) {
			splitted = (char**) realloc(splitted, sizeof(char*) * ++len);
			splitted[len - 1] = strdup(p);

			p = strtok(NULL, at);
		}

		free(_str);

		*count = len;
		return splitted;
	} else {
		*count = 0;

		return NULL;
	}
}

bool compile(FILE *fp, program_t *prog) {
    // Make sure that a program structure is provided
    if (!prog) {
        return false;
    }

    unsigned short pc = 0;
    unsigned short jmp_pc;

    int c;
    while ((c = getc(fp)) != EOF && pc < PROGRAM_SIZE) {
        switch (c) {
            case '>':
                prog->instructions[pc].operator = OP_INC;
                break;
            case '<':
                prog->instructions[pc].operator = OP_DEC;
                break;
            case '+':
                prog->instructions[pc].operator = OP_ADD;
                break;
            case '-':
                prog->instructions[pc].operator = OP_SUB;
                break;
            case '.':
                prog->instructions[pc].operator = OP_OUT;
                break;
            case ',':
                prog->instructions[pc].operator = OP_IN;
                break;
            case '[':
                prog->instructions[pc].operator = OP_JMP;
                if (prog->esp == STACK_SIZE) {
                    return false;
                }
                prog->stack[prog->esp++] = pc;
                break;
            case ']':
                if (prog->esp == 0) {
                    return false;
                }
                jmp_pc = prog->stack[--prog->esp];
                prog->instructions[pc].operator = OP_RET;
                prog->instructions[pc].operator = jmp_pc;
                prog->instructions[jmp_pc].operator = pc;
                break;
            default:
                pc--;
                break;
        }

        pc++;
    }

    if (prog->esp != 0 || pc == PROGRAM_SIZE) {
        return false;
    }
    
    prog->instructions[pc].operator = OP_END;

    return true;
}

void parse_command(const char *cmd) {
    size_t sz = strlen(cmd);

    if (sz == 0) {
        return;
    }
    
    int count = 0;
    char **split_cmd = split(cmd, " ", &count);
    // TODO: Print help on unknown command
    if (split_cmd && count > 0) {
        if (count == 1) {
            if (strcmp(split_cmd[0], "help") == 0 || split_cmd[0][0] == 'h') {
                fprintf(stdout, "List of commands:\n\n");
                fprintf(stdout, "(h)elp -- Print this help.\n");
                fprintf(stdout, "(q)uit -- Exit debugger.\n\n");
                fprintf(stdout, "(f)ile <filename> -- Use file.\n");
                fprintf(stdout, "(r)un -- Start execution.\n");
                fprintf(stdout, "(n)ext -- Step one instruction.\n");
                fprintf(stdout, "(d)ataptr -- Prints the data pointer.\n");
                fprintf(stdout, "(p)rint <index> -- Print cell.\n");
            } else if (strcmp(split_cmd[0], "run") == 0 || split_cmd[0][0] == 'r') {
                dbg_run();
            } else if (strcmp(split_cmd[0], "quit") == 0 || split_cmd[0][0] == 'q') {
                for (int i = 0; i < count; ++i) {
                    if (split_cmd[i]) {
                        free(split_cmd[i]);
                    }
                }

                free(split_cmd);

                run = false;
            } else if (strcmp(split_cmd[0], "next") == 0 || split_cmd[0][0] == 'n') {
                if (runtime.running) {
                    dbg_next();
                } else {
                    fprintf(stdout, "The program is not being run.\n");
                }
            } else if (strcmp(split_cmd[0], "dataptr") == 0 || split_cmd[0][0] == 'd') {
                if (runtime.running) {
                    dbg_print_dataptr();
                } else {
                    fprintf(stdout, "The program is not being run.\n");
                }
            }
        } else if (count == 2) {
            if (strcmp(split_cmd[0], "file") == 0 || split_cmd[0][0] == 'f') {
                dbg_load(split_cmd[1]);
            } else if (strcmp(split_cmd[0], "print") == 0 || split_cmd[0][0] == 'p') {
                if (runtime.running) {
                    int index = (int) strtol(split_cmd[1], (char**) NULL, 10);
                    dbg_print(index);
                } else {
                    fprintf(stdout, "The program is not being run.\n");
                }
            }
        }

        for (int i = 0; i < count; ++i) {
            if (split_cmd[i]) {
                free(split_cmd[i]);
            }
        }

        free(split_cmd);
    }
}

void dbg_load(const char *const file_name) {
    // TODO: Inform user if another file is already being debugged and ask if he wants to continue
    runtime.running = false;

    FILE *fp = fopen(file_name, "r");

    if (fp) {
        fprintf(stdout, "Reading %s...\n", file_name);

        loaded = compile(fp, &program);

        if (!loaded) {
            fprintf(stdout, "Could not read from %s.\n", file_name);
        }

        fclose(fp);
    } else {
        fprintf(stderr, "%s: No such file or directory.\n", file_name);
    }
}

void dbg_error(const char *fmt, ...) {
    fprintf(stderr, "err: ");

    va_list vl;
    va_start(vl, fmt);
    vfprintf(stderr, fmt, vl);
    va_end(vl);

    unsigned short operator = program.instructions[runtime.pc].operator;
    fprintf(stderr, "At instruction %d ('%s'). $[$ptr: %d]: %d.\n", runtime.pc + 1, COMMANDS[operator], runtime.ptr, runtime.data[runtime.ptr]);

    fprintf(stdout, "Brainfuck exited with error.\n");
    runtime.running = false;
}

void dbg_run() {
    if (loaded) {
        memset(runtime.data, 0, sizeof(unsigned short) * DATA_SIZE);

        runtime.pc = 0;
        runtime.ptr = 0;
        runtime.running = true;
    } else {
        fprintf(stdout, "No brainfuck file specified, use 'file'.\n");
    }
}

void dbg_next() {
    if (program.instructions[runtime.pc].operator == OP_END) {
        fprintf(stdout, "Brainfuck exited normally.\n");
        runtime.running = false;
    } else {
        switch (program.instructions[runtime.pc].operator) {
            case OP_INC:
                if (runtime.ptr + 1 < DATA_SIZE) {
                    runtime.ptr++;
                } else {
                    dbg_error("trying to increment the data pointer out of range (%d)\n", DATA_SIZE);
                }
                break;
            case OP_DEC:
                if (runtime.ptr > 0) {
                    runtime.ptr--;
                } else {
                    dbg_error("trying to decrement the data pointer below 0\n");
                }
                break;
            case OP_ADD:
                runtime.data[runtime.ptr]++;
                break;
            case OP_SUB:
                runtime.data[runtime.ptr]--;
                break;
            case OP_OUT:
                putchar(runtime.data[runtime.ptr]);
                break;
            case OP_IN:
                runtime.data[runtime.ptr] = (unsigned int) getchar();
                break;
            case OP_JMP:
                if (!runtime.data[runtime.ptr]) {
                    runtime.pc = program.instructions[runtime.pc].operand;
                }
                break;
            case OP_RET:
                if (runtime.data[runtime.ptr]) {
                    runtime.pc = program.instructions[runtime.pc].operand;
                }
                break;
            }

        runtime.pc++;
    }
}

void dbg_print_dataptr() {
    fprintf(stdout, "$ptr: %d\n", runtime.ptr);
}

void dbg_print(int index) {
    if (index < 0 || index >= DATA_SIZE) {
        fprintf(stdout, "%d: Not in range [0..%d).\n", index, DATA_SIZE);
    } else {
        int c = runtime.data[index];
        if (isprint(c)) {
            fprintf(stdout, "$[%d]: %d ('%c').\n", index, runtime.data[index], c);
        } else {
            fprintf(stdout, "$[%d]: %d.\n", index, runtime.data[index]);
        }
    }
}

void dbg_print_op() {
    switch (program.instructions[runtime.pc].operator) {
        case OP_INC:
            fputc('>', stdout);
            break;
        case OP_DEC:
            fputc('<', stdout);
            break;
        case OP_ADD:
            fputc('+', stdout);
            break;
        case OP_SUB:
            fputc('-', stdout);
            break;
        case OP_OUT:
            fputc('.', stdout);
            break;
        case OP_IN:
            fputc(',', stdout);
            break;
        case OP_JMP:
            fputc('[', stdout);
            break;
        case OP_RET:
            fputc(']', stdout);
            break;
        case OP_END:
            fputs("EOF", stdout);
            break;
    }

    fputc('\n', stdout);
}