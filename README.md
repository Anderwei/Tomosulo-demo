# Tomasulo Algorithm Demo

### A program to simulate tomasulo algorithm in computer organization

```
 ════════════════════════════════════════════════════════ 
CYCLE : 27
      ┌──RF──┐
  F1  │    4 │
  F2  │    2 │
  F3  │    4 │
  F4  │    2 │
  F5  │    8 │
      └──────┘

      ┌──RAT─┐
  F1  │      │
  F2  │  RS1 │
  F3  │      │
  F4  │      │
  F5  │  RS4 │
      └──────┘

      ┌───────┬───────┬───────┐               ┌───────┬───────┬───────┐
  RS1 │   ADD │     6 │    24 │           RS4 │   MUL │     8 │     8 │
  RS2 │       │       │       │           RS5 │       │       │       │
  RS3 │       │       │       │               └───────┴───────┴───────┘
      └───────┴───────┴───────┘

      ┌───────┬───────┬───────┐               ┌───────┬───────┬───────┐
  Buf │   ADD │     6 │    24 │           Buf │   MUL │     8 │     8 │ 
      └───────┴───────┴───────┘               └───────┴───────┴───────┘

  Remain Cycle : 0                        Remain Cycle : 8   

 ════════════════════════════════════════════════════════ 
 ```
`If have render issue, github is recommended.`

 ## Build
 Tested Environment : Ubuntu 20.04.2 LTS  
 `g++ main.cpp -o main -std=c++14`

## Run
simple use `./main [input file name]` to run the program

## Output


### Register File
```
      ┌──RF──┐
  F1  │    4 │
  F2  │    2 │
  F3  │    4 │
  F4  │    2 │
  F5  │    8 │
      └──────┘
```
As Register File in current cycle

### Register Alias Table
```
      ┌──RAT─┐
  F1  │      │
  F2  │  RS1 │
  F3  │      │
  F4  │      │
  F5  │  RS4 │
      └──────┘
```
Register Alias Table.  
Will be empty if don't have reference.

## Reservation Station
```

      ┌───────┬───────┬───────┐               ┌───────┬───────┬───────┐
  RS1 │   ADD │     6 │    24 │           RS4 │   MUL │     8 │     8 │
  RS2 │       │       │       │           RS5 │       │       │       │
  RS3 │       │       │       │               └───────┴───────┴───────┘
      └───────┴───────┴───────┘
```
As RS, follow FIFO policy.  
Instruction will not be dispatch if not ready or have other instruction in buffer.

### RS Buffer
```
      ┌───────┬───────┬───────┐               ┌───────┬───────┬───────┐
  Buf │   ADD │     6 │    24 │           Buf │   MUL │     8 │     8 │ 
      └───────┴───────┴───────┘               └───────┴───────┴───────┘
```
Represent what instruction in the buffer.  
Will be empty if not run any instruction.  

### Remainder for RS Buffer
```
  Remain Cycle : 0                        Remain Cycle : 8   
```
Show how many cycle instruction need.  
Instruction will broadcast if count down to 0.  
showing NaN if not instruction in buffer.  