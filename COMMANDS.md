# bfdb Commands

## Table of Contents

- [Abbreviations](#abbreviations)
- [help](#help)
- [quit](#quit)
- [file](#file)
- [run](#run)
- [next](#next)
- [jump](#jump)
- [continue](#continue)
    - [Execution ended](#execution-ended)
    - [Runtime error occured](#runtime-error-occured)
- [dataptr](#dataptr)
- [print](#print)
    - [Without argument](#without-argument)
    - [With argument](#with-argument)
    - [Printable character](#printable-character)
- [tape](#tape)
- [set](#set)

## Abbreviations

Each command can be called by its full name or by its initial letter.

E.g. `help` or `h`, `quit` or `q` and so on.

## help

The help command prints a list of available commands.

```console
(bfdb) h
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
(s)et <value> -- Sets the value of the current cell.
(bfdb)
```

## quit

The quit command exits the debugger.

```console
(bfdb) q

D:\Git\bfdb>
```

## file

The file command reads a brainfuck program from a file.

```console
(bfdb) f doesntExist.bf
doesntExist.bf: No such file or directory.
(bfdb) f exists.bf
Reading exists.bf...
(bfdb)
```

## run

The run command starts the execution of the currently loaded brainfuck program.

```console
(bfdb) r
@1: +
(bfdb)
```

## next

The next command steps instructions.
The count of instructions to step can be specified by a parameter, which is set to 1 by default.

```console
(bfdb) r
@1: +
(bfdb) n
@2: -
(bfdb) n 4
@6: -
(bfdb)
```

## jump

The jump command jumps to an instruction specified by a parameter.

```console
(bfdb) n
@7: +
(bfdb) j 20
@20: -
(bfdb)
```

## continue

The continue command continues the execution until its end or until a runtime error occurs.

### Execution ended

```console
Reading testContinue.bf...
(bfdb) r
@1: +
(bfdb) c
Brainfuck exited normally.
(bfdb)
```

### Runtime error occured

```console
Reading testRuntimeError.bf...
(bfdb) r
@1: >
(bfdb) c
error: trying to decrement the data pointer below 0
At instruction 3 ('<'). $[$ptr: 0]: 0.
Brainfuck exited with error.
(bfdb)
```

## dataptr

The dataptr command prints the current data pointer.

```console
(bfdb) d
$ptr: 11
@20: +
(bfdb)
```

## print

The print command prints the contents of a cell and displays the character if it is printable.
The index of the cell can be specified by a parameter that defaults to the cell to which the data pointer is currently pointing.

### Without argument

```console
(bfdb) p
$[1]: 0.
@22: +
(bfdb) d
$ptr: 1
@22: +
(bfdb)
```

### With argument

```console
(bfdb) p 4
$[4]: 0.
@19: +
(bfdb)
```

### Printable character

```console
(bfdb) p
$[1]: 65 ('A').
@25: .
(bfdb)
```

## tape

The tape command prints the tape around the data pointer.

```console
(bfdb) t
| $[2]: 2 | $[3]: 3 | $[4]: 4 | $[5]: 5 | >>$[6]: 5 | $[7]: 0 | $[8]: 0 | $[9]: 0 | $[10]: 0 |
@27: +
(bfdb)
```

## set

The set command sets the value of the cell the data pointer is currently pointing to.

```console
(bfdb) p
$[0]: 0.
@1: +
(bfdb) s 100
@1: +
(bfdb) p
$[0]: 100 ('d').
@1: +
(bfdb)
```