# 2kmcc

2k-line mundane C compiler: a self-hosting toy C compiler that's so mundane, your puny C compiler can compile me

(Actually, currently it's closer to 1.5k lines)

[作戦部屋](https://docs.google.com/document/d/1hgJ0aRh7KJBRPPZLc7yDAkGntGJLEPjdysq1h8404i4/edit)

## Goal
C compiler self-host struct% 
- compile a subset of C into x64 asm System-V ABI
- the source code of the compiler must be *correctly* compilable by the compiler itself
- the source code of the compiler should be compilable by many other toy C compilers

## How to use
Run

```
make
```

to build `2kmcc`; then run

```
cat foo.c | xargs -0 -I XX ./2kmcc XX > foo.s
gcc -o foo foo.s -static
```

to make an executable out of `foo.c`.

## Current status
- self-hosted in 1500 lines
- See `test_*.py` to get an idea on what the compiler can and cannot handle 
- Try `make test` to see what it can handle; try `make embarrass` for what it can't
- Try `make test_2ndgen` and `make test_3rdgen` to see that it self-hosts

## Rules for the compiler source code
- Must self-host
- Must be legible enough
    - codegolf techniques not permitted if it severely hurts legibility
    - it should be such that it is easy to add a new feature
- Should be short. I ambitiously aim for **less than 2k lines**

## Rules for the language that this compiler accepts
The subset of C that this compiler can compile must be such that it is not painfully unfun to write a realistic program in.

In particular, it must support:
- multidimensional arrays
- `struct`
- string literals with `\n`

## Why did I aim for 2k lines?
[@hiromi-mi](https://github.com/hiromi-mi/)'s [hanando-fukui v1.1.1](https://github.com/hiromi-mi/hanando-fukui/tree/v1.1.1) is astonishingly short while satisfying the conditions that I put up, with 2203 lines of `main.c` and 169 lines of `main.h`. Therefore, I know that this goal is attainable in 2.5k lines; that's why I aimed for sub 2k.

It turned out that it was doable in 1500 lines. Yay.

## Special thanks to
- Rui Ueyama ([@rui314](https://github.com/rui314)) for writing the compilerbook
- [@spinachpasta](https://github.com/spinachpasta) for actually typing my experimental design up to step12
- [@hiromi-mi](https://github.com/hiromi-mi/) for inspiring me to work on this project
