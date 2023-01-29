# 2kmcc

[作戦部屋](https://docs.google.com/document/d/1hgJ0aRh7KJBRPPZLc7yDAkGntGJLEPjdysq1h8404i4/edit)

## Goal
C compiler self-host struct% 
- compile a subset of C into x64 asm System-V ABI
- the source code of the compiler must be *correctly* compilable by the compiler itself

## Current status
- [Step 18 of Rui Ueyama's compilerbook](https://www.sigbus.info/compilerbook#%E3%82%B9%E3%83%86%E3%83%83%E3%83%9718-%E3%83%9D%E3%82%A4%E3%83%B3%E3%82%BF%E5%9E%8B%E3%82%92%E5%B0%8E%E5%85%A5%E3%81%99%E3%82%8B) completed
- See test.py for the test codes 

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

## Why aim for 2k lines?
[@hiromi-mi](https://github.com/hiromi-mi/)'s [hanando-fukui v1.1.1](https://github.com/hiromi-mi/hanando-fukui/tree/v1.1.1) is astonishingly short while satisfying the conditions that I put up, with 2203 lines of `main.c` and 169 lines of `main.h`. Therefore, I know that this goal is attainable in 2.5k lines; that's why I aim for sub 2k.

## Special thanks to
- Rui Ueyama ([@rui314](https://github.com/rui314)) for writing the compilerbook
- [@spinachpasta](https://github.com/spinachpasta) for actually typing my experimental design up to step12
- [@hiromi-mi](https://github.com/hiromi-mi/) for inspiring me to work on this project
