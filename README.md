# Assembly Project in C
Convert assembly language code into machine code with this assembler, bridging the gap between human-readable instructions and binary execution.

## Overview

This project is centered around creating an assembler program for a custom assembly language using the C programming language. It aims to bridge the gap between human-readable symbolic code and machine code that computers can execute. The project delves into the fundamental concept that computers fundamentally operate in binary code and use assembly language as an intermediary.

## Purpose

The primary purpose of this project is to develop an assembler using C that simplifies the process of programming for computers. It translates assembly language source code into machine code, providing programmers with a more user-friendly and human-readable symbolic representation of machine instructions. This symbolic code can then be loaded onto a computer for execution. It's important to note that this project focuses exclusively on the creation of the assembler and does not encompass linking and loading stages.

## Virtual Computer Model

The project defines an imaginary computer model with essential components:

- CPU (Central Processing Unit)
- General registers (r0, r1, r2, r3, r4, r5, r6, r7)
- Memory with 1024 cells (words), each 12 bits in size
- Arithmetic operations for integers (no support for real numbers)
- Support for characters and ASCII representation

## License

This project is open-source and is released under the [MIT License](LICENSE).
