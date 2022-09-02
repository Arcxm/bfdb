# bfdb

bfdb is a cli debugger for brainfuck similar to gdb.

## Quick Start

Clone the repo and use make to build.

```console
$ make
$ ./bfdb example.bf
```

## Error checks

bfdb has both compile-time checks (e.g. mismatching '`[`' and '`]`') and run-time checks (e.g. decrementing the data pointer below 0).

## Commands

A more detailed list with examples can be found [here](COMMANDS.md).

```console
List of commands:

(h)elp -- Print this help.
(q)uit -- Exit debugger.
(f)ile <filename> -- Use file.
(r)un -- Start execution.
(n)ext [count = 1] -- Steps instructions.
(j)ump <instr_index> -- Jumps to an instruction.
(c)ontinue -- Continue execution.
(d)ataptr -- Prints the data pointer.
(p)rint [index = $ptr] -- Print cell.
(t)ape -- View the tape around the data pointer.
```