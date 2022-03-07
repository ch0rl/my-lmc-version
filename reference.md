# My assembly reference
_loosely_ based off [lmc](https://en.wikipedia.org/wiki/Little_man_computer)
<hr>

## Commands / Operators
**Acronym: code: description**
- `HLT`: 0: Halts execution
- `ADD x`: 1: Adds `x` to the value in the accumulator, storing the result in the accumulator (see addressing for `x`)
- `SUB x`: 2: Subtracts `x` from the value in the accumulator, storing the result in the accumulator (see addressing for `x`)
- `STA x`: 3: Stores the value in the accumulator in address `x`
- `LDA x`: 4: Loads `x` into the accumulator
- `BRA x`: 5: Always branch to address `x` (sets PC to `x`)
- `BRZ x`: 6: Branch to `x` if the value in the accumulator is 0
- `BRP x`: 7: Branch to `x` if the value in the accumulator is positive (strictly greater than 0)
- `INP`: 8: Stores an integer input (from STDIN) to the accumulator
- `OUT`: 9: Outputs the value stored in the accumulator

## Addressing
***The following only applies to: `ADD`, `SUB`, and `LDA`***  
Simply `x` indicates an address (ie., `SUB 6` subtracts the value at address 6 from the accumulator)  
`$x` indicates a raw value (ie., `ADD $5` adds the number 5 to the accumulator)  
***The following applies to: `ADD`, `SUB`, `LDA`, `STA`***  
`&x` indicates a general purpose register, where `x` is an integer (ie., `STA &3` stores the value of the accumulator in register 3)

## Registers
**Acronym: long name: initial value**
- PC: Program Counter: 0
- MAR: Memory Address Register: x
- MDR: Memory Data Register: x
- CIR: Current Instruction Register: x
- ACC: Accumulator: 0
- General Purpose Registers: 0

## Background stuff
Each part of 'ram' is made up of (all base 10):
- 2 digits: the operator
- 4 digits: any data
- 1 digit: addressing flag (0 = memory, 1 = raw value, 2 = register)

Examples (separated for readability):
- `01 0100 0`: Add the value stored at address 100 to the accumulator
- `02 0063 1`: Subtract the literal value `63` from the accumulator
- `06 0013 0`: Branch to address 13 if the accumulator is zero
