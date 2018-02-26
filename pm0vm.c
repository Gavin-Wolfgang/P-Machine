/*
* COP3402 - Spring 2018
* System Software Assignment 1
* Submitted by: Suraj Singireddy (su365398), Gavin Knopp (ga803888)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// definitions
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3
#define INSTRUCTION_REGISTERS 8

int haltFlag = 0;

typedef struct instruction {
	int op; // opcode
	int r;	// register
	int l;	// L : lexicographical level or register
	int m;	// M : number, program address, data address, register
} instruction;

int base(int level, int bp, int stack[]) {
	if (level == 0)
		return bp;

	return base(level-1, stack[bp+1], stack);
}

instruction *fetchCycle(instruction **code, int *pc) {
	return code[(*pc)++];
}

void printStack(int sp, int bp, int *stack, int lex){
	 int i;
	 if (bp == 1) {
		if (lex > 0) {
			printf("|");
	   }
	 }	   
	 else {
		  //Print the lesser lexical level
		  printStack(bp-1, stack[bp + 2], stack, lex-1);
	  printf("|");
	 }
	 //Print the stack contents - at the current level
	 for (i = bp; i <= sp; i++) {
		 printf("%3d ", stack[i]);	
	 }
}


void executeCycle(instruction *inst, int *sp, int *bp, int *pc, int *lex, int stack[], int registers[]) {
	char* str = malloc(sizeof(char) * 4);
	int i;
	// most likely a switch statement with the various instructions in them
	// ***will have pointer to instruction struct ***
	switch(inst->op) {
		// Will probably print differently later but this is for all the important info
		case 1:
			// LIT R, 0, M  reg[r] = M
			// Load constant M into register R
			strcpy(str, "LIT");
			registers[inst->r] = inst->m;
			break;
		case 2:
			// RTN 0, 0, 0  sp <- bp - 1; 
			//				bp <- stack[sp + 3]; 
			//				pc <- stack[sp + 4]
			// return from subroutine and restore caller enviornment 
			strcpy(str, "RTN");
			*sp = *bp - 1;
			*bp = stack[*sp + 3];
			*pc = stack[*sp + 4];
			break;
		case 3:
			// LOD R, L, M  R[i] <- stack[base(L, bp) + M]
			// load value from stack to register at offset M from L lexicographical levels down
			strcpy(str, "LOD");
			registers[inst->r] = stack[base(inst->l, *bp, stack) + inst->m - 1];
			break;
		case 4:
			// STO R, L, M  stack[base(L, bp) + M] <- R[i]
			// store value from register to stack at offset M from L lexicographical levels down
			strcpy(str, "STO");
			stack[base(inst->l, *bp, stack) + inst->m - 1] = registers[inst->r];
			break;
		case 5:
			// CAL 0, L, M  then whats below
			// Call procedure with code at M. Creates new activation record
			strcpy(str, "CAL");
			stack[*sp + 1] = 0;
			stack[*sp + 2] = base(inst->l, *bp, stack);
			stack[*sp + 3] = *bp;
			stack[*sp + 4] = *pc;
			*bp = *sp + 1;
			*sp += 4;
			*pc = inst->m;
			break; 
		case 6:
			// INC 0, 0, M  sp <- sp + M
			// Allocates M locals, first 4 are FV, SL, DL, RA
			strcpy(str, "INC");
			*sp = *sp + inst->m;
			break; 
		case 7:
			// jmp 0, 0, M  pc <- M
			// jumps to inst m
			strcpy(str, "JMP");
			*pc = inst->m;
			break;
		case 8:
			// JPC R, 0, M  r[i] <- stack[base(L, bp) + M]
			// jump to M if R = 0		conditional jump, WOOT!@!
			strcpy(str, "JPC");
			if(registers[inst->r] == 0)	
				*pc = inst->m;
			break;
		case 9:
			// SIO R, 0, 1 	Writes register to screen
			// SIO R, 0, 2 	Read input from user and store in register R
			// SIO 0, 0, 3	EOP
			strcpy(str, "SIO");
			switch(inst->m) {
				case 1:
					printf("%d\n", registers[inst->r]);
					break;
				case 2:
					scanf("%d", &registers[inst->r]);
					break;
				case 3:
					haltFlag = 1;
					break;
			}

			break;
		case 10:
			// NEG R, L, 0	stores negative of L in R
			strcpy(str, "NEG");
			registers[inst->r] = -registers[inst->l];
			break;
		case 11:
			// ADD R, L, M adds values in registers l and M and stores result in R
			strcpy(str, "ADD");
			registers[inst->r] = registers[inst->l] + registers[inst->m];
			break;
		case 12:
			// SUB R, L, M subtract values in registers L and M and stores result in R
			strcpy(str, "SUB");
			registers[inst->r] = registers[inst->l] - registers[inst->m];
			break;
		case 13:
			// MUL R, L, M multiply values in registers L and M and store result in R
			strcpy(str, "MUL");
			registers[inst->r] = registers[inst->l] * registers[inst->m];
			break;
		case 14:
			// DIV R, L, M divide values in registers L and M and store result in R
			strcpy(str, "DIV");
			registers[inst->r] = registers[inst->l] / registers[inst->m];
			break;
		case 15:
			// ODD R, 0, 0 mod value in register R by 2 and store result in R
			strcpy(str, "ODD");
			registers[inst->r] = registers[inst->r] % 2;
			break;
		case 16:
			// MOD R, L, M MOD values in registers L and M and store result in R
			strcpy(str, "MOD");
			registers[inst->r] = registers[inst->l] % registers[inst->m];
			break;
		case 17:
			// EQL R, L, M R[L] == R[M] stored in R[R]
			strcpy(str, "EQL");
			registers[inst->r] = (registers[inst->l] == registers[inst->m]);
			break;
		case 18:
			// NEQ R, L, M R[L] != R[M] stored in R[R]
			strcpy(str, "NEQ");
			registers[inst->r] = (registers[inst->l] != registers[inst->m]);
			break;
		case 19:
			// LSS R, L, M R[L] < R[M] stored in R[R]
			strcpy(str, "LSS");
			registers[inst->r] = (registers[inst->l] < registers[inst->m]);
			break;
		case 20:
			// LEQ R, L, M R[L] <= R[M] stored in R[R]
			strcpy(str, "LEQ");
			registers[inst->r] = (registers[inst->l] <= registers[inst->m]);
			break;
		case 21:
			// GTR R, L, M R[L] > R[M] stored in R[R]
			strcpy(str, "GTR");
			registers[inst->r] = (registers[inst->l] > registers[inst->m]);
			break;
		case 22:
			// GEQ R, L, M R[L] >= R[M] stored in R[R]
			strcpy(str, "GEQ");
			registers[inst->r] = (registers[inst->l] >= registers[inst->m]);
			break;
	}

	if(strcmp(str, "RTN") == 0) {
		*lex -= 1;
	}

	printf("%-4s%3d%3d%3d[%3d%3d%3d] ", str, inst->r, inst->l, inst->m, *pc, *bp, *sp);
	printStack(*sp, *bp, stack, *lex);
	printf("\n\tRegisters:[");
	for(i=0; i<8; i++) {
		printf("%3d", registers[i]);
	}
	printf("]\n");

	if(strcmp(str, "CAL") == 0) {
		*lex += 1;
	}

	free(str);
}

int cycle(instruction **cs, int *sp, int *bp, int *pc, int *lex, int stack[], int registers[]) {

	// Lots of code will go here, for now just outlining the steps we need to go through
	// first we get an instruction from the "code" store and put it in the IR
	instruction *inst = fetchCycle(cs, pc);
	
	// Executes the instruction fetched
	executeCycle(inst, sp, bp, pc, lex, stack, registers);
	// The above two process are considered one cycle in the P-Machine
	return 0;
}

int getInstructions(instruction** cs, char* fName) {
	int i = 0;
	instruction* curInst = malloc(sizeof(instruction));
	FILE *file = fopen(fName, "r");

	if (!file) {
		printf("Error: File does not exist.");
		haltFlag = 1;
	}

	while(fscanf(file, "%d", &(curInst->op)) != EOF && i < MAX_CODE_LENGTH) {
		fscanf(file, "%d", &(curInst->r));
		fscanf(file, "%d", &(curInst->l));
		fscanf(file, "%d", &(curInst->m));
		cs[i++] = curInst;
		curInst = malloc(sizeof(instruction));
	}

	fclose(file);

	return i;
}

int main(int argc, char** argsv) {
	int i, j, sp=0, bp=1, pc=0, lex=0;
	// stack initialized to 0
	int stack[MAX_STACK_HEIGHT] = {};
	// initializes register files
	int registers[INSTRUCTION_REGISTERS] = {};
	instruction** cs = malloc(sizeof(instruction*) * MAX_CODE_LENGTH);

	if(argc < 2)
		return 0;


	printf("\n OP   Rg Lx Vl[ PC BP SP]\n");
	i = getInstructions(cs, argsv[1]);

	while (pc < i && !haltFlag) {
		cycle(cs, &sp, &bp, &pc, &lex, stack, registers);
	}


	free(cs);

	return 0;
}