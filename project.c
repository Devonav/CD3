#include "spimcore.h"
/*Devon Villalona 
CDA3103C-23Fall 0002
Project
*/


// Prototype for a helper function used in the int ALU function
char determineALUOperation(char ALUOp, unsigned funct);

/* ALU Operations */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero) {

    // Execute operations based on the ALU control code
    switch(ALUControl) {
        case 0: // Addition
            *ALUresult = A + B;
            break;
        case 1: // Subtraction
            *ALUresult = A - B;
            break;
        case 2: // Set if less than (signed)
            *ALUresult = ((int)A < (int)B) ? 1 : 0;
            break;
        case 3: // Set if less than (unsigned)
            *ALUresult = (A < B) ? 1 : 0;
            break;
        case 4: // AND operation
            *ALUresult = A & B;
            break;
        case 5: // OR operation
            *ALUresult = A | B;
            break;
        case 6: // Shift left by 16
            *ALUresult = B << 16;
            break;
        case 7: // NOT operation
            *ALUresult = ~A;
            break;
        default:
            // Handle unexpected ALUControl values
            break;
    }

    // Set the Zero flag if the ALU result is zero
    *Zero = (*ALUresult == 0) ? 1 : 0;
}


/* Instruction Fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction) {
    // Ensure the instruction is aligned correctly
    if (PC % 4 != 0) return 1; // Halt on misalignment

    // Fetch the instruction from memory
    *instruction = Mem[PC / 4]; // Address translation from byte to word address
    return 0; // Successful fetch
}



/* Instruction Partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec) {
    // Extracting various fields from the instruction
    *op = instruction >> 26; // Opcode
    *r1 = (instruction >> 21) & 0x1F; // Source register 1
    *r2 = (instruction >> 16) & 0x1F; // Source register 2 or target register
    *r3 = (instruction >> 11) & 0x1F; // Destination register
    *funct = instruction & 0x3F; // Function field for R-type instructions
    *offset = instruction & 0xFFFF; // Immediate value or address offset
    *jsec = instruction & 0x3FFFFFF; // Target address for jump instructions
}




/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls) {
    // Clear all control signals
    memset(controls, 0, sizeof(struct_controls));

    // Decoding based on the opcode
    switch (op) {
        case 0x0:  // R-type Instructions
            controls->RegDst = 1;  // The destination register is rd
            controls->RegWrite = 1;  // Enable writing to the register
            controls->ALUOp = 7;  // Special ALU operation for R-type
            break;

        case 0x8:  // addi
            controls->ALUSrc = 1;  // Immediate value as the second operand
            controls->RegWrite = 1;  // Write to the register
            break;

        case 0x23:  // lw (Load Word)
            controls->MemRead = 1;  // Reading from memory
            controls->MemtoReg = 1;  // Data comes from memory
            controls->ALUSrc = 1;  // Immediate value as the second operand
            controls->RegWrite = 1;  // Write to the register
            break;

        case 0x2b:  // sw (Store Word)
            controls->MemWrite = 1;  // Writing to memory
            controls->ALUSrc = 1;  // Immediate value as the second operand
            break;

        case 0xf:  // lui (Load Upper Immediate)
            controls->RegWrite = 1;  // Write to the register
            controls->ALUSrc = 1;  // Immediate value as the second operand
            controls->ALUOp = 6;  // Special operation for LUI
            break;

        case 0x4:  // beq (Branch on Equal)
            controls->Branch = 1;  // Branching decision
            controls->ALUOp = 1;  // Subtraction for comparison
            break;

        case 0xa:  // slti (Set on Less Than Immediate)
            controls->RegWrite = 1;  // Write to the register
            controls->ALUSrc = 1;  // Immediate value as the second operand
            controls->ALUOp = 2;  // Set less than operation
            break;

        case 0xb:  // sltiu (Set on Less Than Immediate, Unsigned)
            controls->RegWrite = 1;  // Write to the register
            controls->ALUSrc = 1;  // Immediate value as the second operand
            controls->ALUOp = 3;  // Set less than operation (unsigned)
            break;

        case 0x2:  // j (Jump)
            controls->Jump = 1;  // Jump to a new location
            break;

        default:  // Unknown Opcode
            return 1;  // Return a halt condition
    }
    return 0;  // Successful decoding
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2) {
    // Read values from specified registers
    *data1 = Reg[r1]; // First operand
    *data2 = Reg[r2]; // Second operand
}



/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value) {
    // Extend the 16-bit immediate value to a 32-bit value
    *extended_value = (offset & 0x8000) ? (offset | 0xFFFF0000) : offset;
}


/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero) {
    // Initialize ALUControl with the value of ALUOp
    char ALUControl = determineALUOperation(ALUOp, funct);

    // Select the second operand for the ALU based on ALUSrc
    unsigned secondOperand = ALUSrc ? extended_value : data2;

    // Perform the ALU operation
    ALU(data1, secondOperand, ALUControl, ALUresult, Zero);

    // Check for successful operation
    return ALUControl != -1 ? 0 : 1; // Return 0 for success, 1 for failure (halt condition)
}

// Helper function to determine the ALU control signal based on ALUOp and the function field
char determineALUOperation(char ALUOp, unsigned funct) {
    // Handling R-type instructions separately
    if (ALUOp == 7) {
        switch (funct) {
            case 0x20: return 0; // Addition
            case 0x22: return 1; // Subtraction
            case 0x24: return 4; // AND
            case 0x25: return 5; // OR
            case 0x2A: return 2; // Set on less than (signed)
            case 0x2B: return 3; // Set on less than (unsigned)
            default:   return -1; // Invalid funct, indicates a halt
        }
    }
    // For non-R-type instructions, ALUOp directly serves as the control signal
    return ALUOp;
}

/* Read/Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem) {
    // ALUresult is an address if MemWrite or MemRead is asserted and must be word-aligned
    if (MemWrite == 1 || MemRead == 1) {
        if (ALUresult % 4 != 0) {
            return 1; // Halt if the address is not word-aligned
        }
    }

    // Write operation
    if (MemWrite == 1) {
        Mem[ALUresult >> 2] = data2; // Write data to the memory location
    }

    // Read operation
    if (MemRead == 1) {
        *memdata = Mem[ALUresult >> 2]; // Read data from the memory location
    }

    return 0; // No halt condition
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg) {
    if (RegWrite) {
        unsigned destReg = (RegDst) ? r3 : r2;
        Reg[destReg] = (MemtoReg) ? memdata : ALUresult;
    }
}


/* PC Update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC) {
    if (Jump) {
        *PC = (*PC & 0xF0000000) | (jsec << 2);
    } else if (Branch && Zero) {
        *PC += 4 + (extended_value << 2);
    } else {
        *PC += 4;
    }
}


