#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "bfdb"
#define COMMAND_SZ 32

#define PROGRAM_SIZE 4096
#define STACK_SIZE 512
#define DATA_SIZE 65535

enum {
    OP_END, OP_INC, OP_DEC, OP_ADD, OP_SUB, OP_OUT, OP_IN, OP_JMP, OP_RET
};

typedef struct {
    unsigned short operator;
    unsigned short operand;
} instruction_t;

char **split(const char *const str, const char *const at, int *count);

bool compile(FILE *fp);

void dbg_load(const char *const file_name);
void dbg_run();
void dbg_next();
void dbg_print(int index);
void dbg_print_op();

void parse_command(const char *cmd);

static instruction_t program[PROGRAM_SIZE];
static unsigned short stack[STACK_SIZE];
static unsigned int esp = 0;

static bool run = true;
static bool loaded = false;

static bool running = false;
static unsigned short data[DATA_SIZE];
static unsigned short pc = 0;
static unsigned int ptr = 0;

int main(int argc, char **argv) {
    if (argc > 1) {
        dbg_load(argv[1]);
    }

    while (run) {
        if (running) {
            dbg_print_op();
        }

        fprintf(stdout, "(%s) ", TAG);

        char buf[COMMAND_SZ] = {0};
        fgets(buf, COMMAND_SZ, stdin);
        buf[strcspn(buf, "\n")] = '\0';

        parse_command(buf);
    }
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

bool compile(FILE *fp) {
    unsigned short pc = 0;
    unsigned short jmp_pc;

    int c;
    while ((c = getc(fp)) != EOF && pc < PROGRAM_SIZE) {
        switch (c) {
            case '>':
                program[pc].operator = OP_INC;
                break;
            case '<':
                program[pc].operator = OP_DEC;
                break;
            case '+':
                program[pc].operator = OP_ADD;
                break;
            case '-':
                program[pc].operator = OP_SUB;
                break;
            case '.':
                program[pc].operator = OP_OUT;
                break;
            case ',':
                program[pc].operator = OP_IN;
                break;
            case '[':
                program[pc].operator = OP_JMP;
                if (esp == STACK_SIZE) {
                    return false;
                }
                stack[esp++] = pc;
                break;
            case ']':
                if (esp == 0) {
                    return true;
                }
                jmp_pc = stack[--esp];
                program[pc].operator = OP_RET;
                program[pc].operator = jmp_pc;
                program[jmp_pc].operator = pc;
                break;
            default:
                pc--;
                break;
        }

        pc++;
    }

    if (esp != 0 || pc == PROGRAM_SIZE) {
        return false;
    }
    
    program[pc].operator = OP_END;

    return true;
}

void dbg_load(const char *const file_name) {
    // TODO: Inform user if another file is already being debugged and ask if he wants to continue
    running = false;

    FILE *fp = fopen(file_name, "r");

    if (fp) {
        fprintf(stdout, "Reading %s...\n", file_name);

        loaded = compile(fp);

        if (!loaded) {
            fprintf(stdout, "Could not read from %s.\n", file_name);
        }

        fclose(fp);
    } else {
        fprintf(stderr, "%s: No such file or directory.\n", file_name);
    }
}

void dbg_run() {
    if (loaded) {
        memset(data, 0, sizeof(unsigned short) * DATA_SIZE);

        pc = 0;
        ptr = 0;
        running = true;
    } else {
        fprintf(stdout, "No brainfuck file specified, use 'file'.\n");
    }
}

void dbg_next() {
    if (program[pc].operator == OP_END) {
        fprintf(stdout, "Brainfuck exited.\n");
        running = false;
    } else {
        switch (program[pc].operator) {
            case OP_INC:
                ptr++;
                break;
            case OP_DEC:
                ptr--;
                break;
            case OP_ADD:
                data[ptr]++;
                break;
            case OP_SUB:
                data[ptr]--;
                break;
            case OP_OUT:
                putchar(data[ptr]);
                break;
            case OP_IN:
                data[ptr] = (unsigned int) getchar();
                break;
            case OP_JMP:
                if (!data[ptr]) {
                    pc = program[pc].operand;
                }
                break;
            case OP_RET:
                if (data[ptr]) {
                    pc = program[pc].operand;
                }
                break;
            }

        pc++;
    }
}

void dbg_print(int index) {
    if (index < 0 || index >= DATA_SIZE) {
        fprintf(stdout, "%d: Not in range [0..%d]\n", index, DATA_SIZE);
    } else {
        int c = data[index];
        if (isprint(c)) {
            fprintf(stdout, "Cell %d: %d ('%c').\n", index, data[index], c);
        } else {
            fprintf(stdout, "Cell %d: %d.\n", index, data[index]);
        }
    }
}

void dbg_print_op() {
    switch (program[pc].operator) {
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
                if (running) {
                    dbg_next();
                } else {
                    fprintf(stdout, "The program is not being run.\n");
                }
            }
        } else if (count == 2) {
            if (strcmp(split_cmd[0], "file") == 0 || split_cmd[0][0] == 'f') {
                dbg_load(split_cmd[1]);
            } else if (strcmp(split_cmd[0], "print") == 0 || split_cmd[0][0] == 'p') {
                if (running) {
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