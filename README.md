# bfdb

bfdb is a cli debugger for brainfuck similar to gdb.

## Quick Start

Clone the repo and use make to build.

```console
$ make
$ ./bfdb example.bf
```

## Commands

```console
List of commands:

(h)elp -- Print this help.
(q)uit -- Exit debugger.
(f)ile <filename> -- Use file.
(r)un -- Start execution.
(n)ext -- Step one instruction.
(j)ump <instr_index> -- Jumps to an instruction.
(c)ontinue -- Continue execution.
(d)ataptr -- Prints the data pointer.
(p)rint [index = $ptr] -- Print cell.
```