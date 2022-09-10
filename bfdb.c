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

/// Brainfuck's instructions as well as EOF to signal the end of the program
const char* INSTRUCTIONS[] = { "EOF", ">", "<", "+", "-", ".", ",", "[", "]" };

enum {
    OP_END, OP_INC, OP_DEC, OP_ADD, OP_SUB, OP_OUT, OP_IN, OP_JMP, OP_RET
};

/// An instruction containing an operator and an operand
typedef struct instruction_t {
    unsigned short operator;
    unsigned short operand;
} instruction_t;

// Helper functions

/// Splits a string by delimiters and returns a c-string array and the count of strings splitted
/// @param str The string to split
/// @param at The delimiters to split at
/// @param count The count of substrings
/// @return An array of c-strings that contains the substrings
char **split(const char *const str, const char *const at, int *count);

/// Converts a string to a number
/// @param str The string to convert
/// @param base The base to use
/// @param allow_neg Whether or not negative numbers should be allowed
/// @param converted The converted number
/// @return Whether or not the conversion succeeded
bool to_int(const char *const str, int base, bool allow_neg, int *converted);

// Compilation

/// A brainfuck program
typedef struct program_t {
    /// The instructions of the brainfuck program
    instruction_t instructions[PROGRAM_SIZE];

    /// The count of instructions
    unsigned short instr_count;

    /// The stack that is used to keep track of jumps during compilation
    unsigned short stack[STACK_SIZE];

    /// The stack pointer
    unsigned int esp;
} program_t;

/// The program currently associated with bfdb
program_t program = { .instr_count = 0, .esp = 0 };

/// Compiles the brainfuck program in fp to the intermediate representation
/// @param fp The file to read
/// @param prog The program structure to write the program to
/// @return Whether or not the compilation succeeded
bool compile(FILE *fp, program_t *prog);

/// Prints a formatted error as well as line and column information to stderr
/// @param line The line the error occured in
/// @param col The column in the line
/// @param fmt The format
void compile_error(int line, int col, const char *fmt, ...);

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

/// The runtime currently associated with bfdb
runtime_t runtime = { .running = false, .pc = 0, .ptr = 0 };

// Commands

/// The handler of a command
typedef void (*command_handler_t)(char*);

/// A command
typedef struct command_t {
    /// The name of the command
    const char *name;

    /// The abbreviation that the user can use
    const char abbr;

    /// Short description
    const char *desc;

    /// Argument description
    const char *arg_desc;

    /// The commands handler
    command_handler_t handler;
} command_t;

/// Parses the command given in the cli
/// @param cmd The command to parse
void parse_command(const char *cmd);

/// The help command, prints the available commands
void cmd_help(char *unused);

/// The quit command, exits the debugger
void cmd_quit(char *unused);

/// The file command, reads a file to debug
/// @param file_name The name of the file
void cmd_file(char *file_name);

/// The run command, starts execution
void cmd_run(char *unused);

/// The next command, steps instructions
/// @param count The count of instructions to step
void cmd_next(char *count);

/// The jump command, jumps to an instruction
/// @param index The index of the instruction to jump to
void cmd_jump(char *index);

/// The continue command, continues the execution until the end or until a runtime error occurs
void cmd_continue(char *unused);

/// The dataptr command, prints the data pointer
void cmd_dataptr(char *unused);

/// The print command, prints a cell
/// @param index The index of the cell to print
void cmd_print(char *index);

/// The tape command, shows the tape around the current data pointer
void cmd_tape(char *unused);

/// The commands
command_t commands[] = {
    { .name = "help",     .abbr = 'h', .desc = "Print this help",                       .arg_desc = NULL,             .handler = &cmd_help     },
    { .name = "quit",     .abbr = 'q', .desc = "Exit debugger",                         .arg_desc = NULL,             .handler = &cmd_quit     },
    { .name = "file",     .abbr = 'f', .desc = "Use file",                              .arg_desc = "<filename>",     .handler = &cmd_file     },
    { .name = "run",      .abbr = 'r', .desc = "Start execution",                       .arg_desc = NULL,             .handler = &cmd_run      },
    { .name = "next",     .abbr = 'n', .desc = "Steps instructions",                    .arg_desc = "[count = 1]",    .handler = &cmd_next     },
    { .name = "jump",     .abbr = 'j', .desc = "Jumps to an instruction",               .arg_desc = "<instr_index>",  .handler = &cmd_jump     },
    { .name = "continue", .abbr = 'c', .desc = "Continue execution",                    .arg_desc = NULL,             .handler = &cmd_continue },
    { .name = "dataptr",  .abbr = 'd', .desc = "Prints the data pointer",               .arg_desc = NULL,             .handler = &cmd_dataptr  },
    { .name = "print",    .abbr = 'p', .desc = "Print cell",                            .arg_desc = "[index = $ptr]", .handler = &cmd_print    },
    { .name = "tape",     .abbr = 't', .desc = "View the tape around the data pointer", .arg_desc = NULL,             .handler = &cmd_tape     }
};

/// The count of available commands
const int command_count = sizeof(commands) / sizeof(command_t);

// Debugger actions

/// Load a brainfuck program from a file
/// @param file_name The name of the file
void dbg_load(const char *const file_name);

/// Prints a formatted error as well as runtime information to stderr and stops execution
/// @param fmt The format
void dbg_runtime_error(const char *fmt, ...);

/// Start execution of the loaded brainfuck program
void dbg_run();

/// Interprets an instruction on the given runtime
/// @param runtime The runtime to use
/// @param instruction The instruction to interpret
/// @return Whether the runtime was terminated either by OP_END or a runtime error
bool dbg_interpret(runtime_t *runtime, instruction_t instruction);

/// Steps in execution
/// @param count The count of instructions to step
/// @return Whether the interpretation of the instructions terminated the runtime (see dbg_interpret's return)
bool dbg_next(int count);

/// Jumps to the instruction at the given index
/// @param prog The current running program
/// @param index The index of the instruction to jump to
void dbg_jump(program_t *prog, int index);

/// Prints the data pointer
void dbg_print_dataptr();

/// Print the cell at the given index
/// @param index The index of the cell to print
void dbg_print(int index);

/// Prints the tape around the current data pointer
void dbg_print_tape();

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

bool to_int(const char *const str, int base, bool allow_neg, int *converted) {
    if (str && converted) {
        char *endptr;

        int result = (int) strtol(str, &endptr, base);
        *converted = result;

        // If endptr points to the beginning of the string, no conversion happened
        if (endptr == str) {
            fprintf(stderr, "error: '%s' invalid numeric argument.\n", str);
            return false;
        } else if (!allow_neg && result < 0) {
            fprintf(stderr, "error: '%s' negative not allowed.\n", str);
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

bool compile(FILE *fp, program_t *prog) {
    // Make sure that a program structure is provided
    if (!prog) {
        return false;
    }

    /// The current line in the file
    int line = 1;
    /// The current column in the file
    int col = 1;

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
                    compile_error(line, col, "loop count exceeds bfdb's capacity (%d)\n", STACK_SIZE);
                    return false;
                }
                prog->stack[prog->esp++] = pc;
                break;
            case ']':
                if (prog->esp == 0) {
                    compile_error(line, col, "unmatched ']'\n");
                    return false;
                }
                jmp_pc = prog->stack[--prog->esp];
                prog->instructions[pc].operator = OP_RET;
                prog->instructions[pc].operand = jmp_pc;
                prog->instructions[jmp_pc].operator = pc;
                break;
            default:
                pc--;
                break;
        }

        pc++;

        if (c == '\n') {
            col = 1;
            line++;
        } else {
            col++;
        }
    }

    if (pc == PROGRAM_SIZE) {
        compile_error(line, col, "instruction count exceeds bfdb's capacity (%d)\n", PROGRAM_SIZE);
        return false;
    }

    if (prog->esp != 0) {
        return false;
    }
    
    prog->instructions[pc].operator = OP_END;
    prog->instr_count = pc + 1;

    return true;
}

void compile_error(int line, int col, const char *fmt, ...) {
    fprintf(stderr, "%d:%d: compilation error: ", line, col);

    va_list vl;
    va_start(vl, fmt);
    vfprintf(stderr, fmt, vl);
    va_end(vl);

    fprintf(stdout, "Compilation exited with error.\n");
}

void parse_command(const char *cmd) {
    size_t sz = strlen(cmd);

    if (sz == 0) {
        return;
    }
    
    int count = 0;
    char **split_cmd = split(cmd, " ", &count);

    // TODO: Print help on unknown command
    for (int i = 0; i < command_count; ++i) {
        const command_t command = commands[i];

        if (strcmp(split_cmd[0], command.name) == 0 || split_cmd[0][0] == command.abbr) {
            // Whether or not an argument was provided
            char *arg = (count == 2) ? split_cmd[1] : NULL;

            command.handler(arg);
        }
    }

    for (int i = 0; i < count; ++i) {
            if (split_cmd[i]) {
                free(split_cmd[i]);
            }
        }

    free(split_cmd);
}

void cmd_help(char *unused) {
    (void) unused;
    
    fprintf(stdout, "List of commands:\n\n");

    for (int i = 0; i < command_count; ++i) {
        const command_t command = commands[i];

        // Skip first character in the command's name as it is already printed in the brackets (the abbreviation)
        if (command.arg_desc) {
            fprintf(stdout, "(%c)%s %s -- %s.\n", command.abbr, &command.name[1], command.arg_desc, command.desc);
        } else {
            fprintf(stdout, "(%c)%s -- %s.\n", command.abbr, &command.name[1], command.desc);
        }
    }
}

void cmd_quit(char *unused) {
    (void) unused;

    run = false;
}

void cmd_file(char *file_name) {
    if (file_name) {
        dbg_load(file_name);
    } else {
        fprintf(stderr, "error: 'file' takes exactly one file path argument.\n");
    }
}

void cmd_run(char *unused) {
    (void) unused;
    
    if (loaded) {
        dbg_run();
    } else {
        fprintf(stdout, "No brainfuck file specified, use 'file'.\n");
    }
}

void cmd_next(char *count) {
    if (runtime.running) {
        if (count) {
            int c;
            if (to_int(count, 10, false, &c)) {
                dbg_next(c);
            }
        } else {
            dbg_next(1);
        }
    } else {
        fprintf(stdout, "The program is not being run.\n");
    }
}

void cmd_jump(char *index) {
    if (runtime.running) {
        if (index) {
            int i;
            if (to_int(index, 10, false, &i)) {
                dbg_jump(&program, i);
            }
        } else {
            fprintf(stderr, "error: 'jump' takes exactly one instruction index argument.\n");
        }
    } else {
        fprintf(stdout, "The program is not being run.\n");
    }
}

void cmd_continue(char *unused) {
    (void) unused;

    if (runtime.running) {
        // Continue stepping in execution until the runtime stops because of OP_END or a runtime error
        while (!dbg_next(1)) {}
    } else {
        fprintf(stdout, "The program is not being run.\n");
    }
}

void cmd_dataptr(char *unused) {
    (void) unused;
    
    if (runtime.running) {
        dbg_print_dataptr();
    } else {
        fprintf(stdout, "The program is not being run.\n");
    }
}

void cmd_print(char *index) {
    if (runtime.running) {
        if (index) {
            int i;
            if (to_int(index, 10, false, &i)) {
                dbg_print(i);
            }
        } else {
            dbg_print(runtime.ptr);
        }
    } else {
        fprintf(stdout, "The program is not being run.\n");
    }
}

void cmd_tape(char *unused) {
    (void) unused;

    if (runtime.running) {
        dbg_print_tape();
    } else {
        fprintf(stdout, "The program is not being run.\n");
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
            fprintf(stderr, "Could not read from %s.\n", file_name);
        }

        fclose(fp);
    } else {
        fprintf(stderr, "%s: No such file or directory.\n", file_name);
    }
}

void dbg_runtime_error(const char *fmt, ...) {
    fprintf(stderr, "Runtime error: ");

    va_list vl;
    va_start(vl, fmt);
    vfprintf(stderr, fmt, vl);
    va_end(vl);

    unsigned short operator = program.instructions[runtime.pc].operator;
    fprintf(stderr, "At instruction %d ('%s'). $[$ptr: %d]: %d.\n", runtime.pc + 1, INSTRUCTIONS[operator], runtime.ptr, runtime.data[runtime.ptr]);

    fprintf(stdout, "Brainfuck exited with error.\n");
    runtime.running = false;
}

void dbg_run() {
    memset(runtime.data, 0, sizeof(unsigned short) * DATA_SIZE);

    runtime.pc = 0;
    runtime.ptr = 0;
    runtime.running = true;
}

bool dbg_interpret(runtime_t *runtime, instruction_t instruction) {
    // Make sure that a runtime is provided
    if (!runtime) {
        return false;
    }

    if (instruction.operator == OP_END) {
        fprintf(stdout, "Brainfuck exited normally.\n");
        runtime->running = false;

        return true;
    } else {
        switch (instruction.operator) {
            case OP_INC:
                if (runtime->ptr + 1 < DATA_SIZE) {
                    runtime->ptr++;
                } else {
                    dbg_runtime_error("trying to increment the data pointer out of range (%d)\n", DATA_SIZE);
                    return true;
                }
                break;
            case OP_DEC:
                if (runtime->ptr > 0) {
                    runtime->ptr--;
                } else {
                    dbg_runtime_error("trying to decrement the data pointer below 0\n");
                    return true;
                }
                break;
            case OP_ADD:
                runtime->data[runtime->ptr]++;
                break;
            case OP_SUB:
                runtime->data[runtime->ptr]--;
                break;
            case OP_OUT:
                putchar(runtime->data[runtime->ptr]);
                break;
            case OP_IN:
                runtime->data[runtime->ptr] = (unsigned int) getchar();
                break;
            case OP_JMP:
                if (!runtime->data[runtime->ptr]) {
                    runtime->pc = instruction.operand;
                }
                break;
            case OP_RET:
                if (runtime->data[runtime->ptr]) {
                    runtime->pc = instruction.operand;
                }
                break;
            }

        runtime->pc++;

        return false;
    }
}

bool dbg_next(int count) {
    if (count < 0) {
        fprintf(stderr, "%d: Count has to be greater than 0!\n", count);
        return false;
    }

    bool ret = false;
    for (int i = 0; i < count; ++i) {
        ret = dbg_interpret(&runtime, program.instructions[runtime.pc]);

        if (ret) {
            break; // Break out of the loop as the runtime was terminated either by OP_END or a runtime error
        }
    }

    return ret;
}

void dbg_jump(program_t *prog, int index) {
    // Make sure that a program structure is provided
    if (!prog) {
        return;
    }

    if (index < 1 || index > prog->instr_count) {
        fprintf(stderr, "%d: Not in range of program's instructions [1..%d]\n", index, prog->instr_count);
    } else {
        runtime.pc = index - 1;
    }
}

void dbg_print_dataptr() {
    fprintf(stdout, "$ptr: %d\n", runtime.ptr);
}

void dbg_print(int index) {
    if (index < 0 || index >= DATA_SIZE) {
        fprintf(stderr, "%d: Not in range [0..%d).\n", index, DATA_SIZE);
    } else {
        int c = runtime.data[index];
        if (isprint(c)) {
            fprintf(stdout, "$[%d]: %d ('%c').\n", index, runtime.data[index], c);
        } else {
            fprintf(stdout, "$[%d]: %d.\n", index, runtime.data[index]);
        }
    }
}

void dbg_print_tape() {
    fputc('|', stdout);

    for (int dptr = -4; dptr < 5; ++dptr) {
        int ptr = runtime.ptr + dptr;

        if (ptr < 0 || ptr >= DATA_SIZE) {
            continue;
        }

        // dptr == 0 => dptr == runtime.ptr
        if (dptr == 0) {
            fprintf(stdout, " >>$[%d]: %d |", ptr, runtime.data[ptr]);
        } else {
            fprintf(stdout, " $[%d]: %d |", ptr, runtime.data[ptr]);
        }
    }

    fputc('\n', stdout);
}

void dbg_print_op() {
    fprintf(stdout, "@%d: ", runtime.pc + 1);

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