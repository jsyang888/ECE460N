/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND2, COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    LD_EV,
    LD_USP,
    LD_SSP,
    LD_PSR,
    GATE_SP,
    GATE_VECTOR,
    GATE_PSR,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX1, DRMUX0,
    SR1MUX1, SR1MUX0, 
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    FAULTMUX,
    PSRMUX,
    MARMUX,
    SPMUX1, SPMUX0,
    EVMUX1, EVMUX0,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CLRINT,
/* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return(((x[COND2] << 2) + (x[COND1] << 1) + (x[COND0]))); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetLD_EV(int *x)         { return(x[LD_EV]); }
int GetLD_USP(int *x)        { return(x[LD_USP]); }
int GetLD_SSP(int *x)        { return(x[LD_SSP]); }
int GetLD_PSR(int *x)        { return(x[LD_PSR]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }   
int GetGATE_SP(int *x)      { return(x[GATE_SP]); } 
int GetGATE_VECTOR(int *x)   { return(x[GATE_VECTOR]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + (x[PCMUX0])); }
int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + (x[DRMUX0])); }
int GetSR1MUX(int *x)        { return((x[SR1MUX1] << 1) + (x[SR1MUX0])); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetSPMUX(int *x)         { return((x[SPMUX1] << 1) + (x[SPMUX0])); }
int GetEVMUX(int *x)         { return((x[EVMUX1] << 1) + (x[EVMUX0] << 0)); }
int GetFAULTMUX(int *x)      { return(x[FAULTMUX]); }
int GetPSRMUX(int *x)        { return(x[PSRMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
int GetCLRINT(int *x)        { return(x[CLRINT]); }
/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: You may add system latches that are required by your implementation */

int USP; // user stack ptr variable
int PSR; // program status register
int EventVector; // which exception / interrupt occurs?
int INT_PENDING; // flag that gets set whenever an interrupt will happen -- CLR INT sets it equal to 0
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;
  CYCLE_COUNT++;
  if (CYCLE_COUNT == 300) {
    CURRENT_LATCHES.INT_PENDING = 1;
    NEXT_LATCHES.INT_PENDING = 1;

    CURRENT_LATCHES.INTV = 0x01;
    NEXT_LATCHES.INTV = 0x01;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) { 
    int i;
    init_control_store(argv[1]);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 2]);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
    CURRENT_LATCHES.PSR = 0x8000; // initially in user mode
    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/


int MEM_ALU();

void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */
  // looking at figure C5:
    int *u = CURRENT_LATCHES.MICROINSTRUCTION;
    int next_state;
    if (GetIRD(u) == 1) { // if the mux is selected to the IR
        next_state = (CURRENT_LATCHES.IR >> 12) & 0xF; // keeps IR 15:12 and discards all else 
    }
    else {
        int COND_0 = u[COND0];
        int COND_1 = u[COND1];
        int COND_2 = u[COND2];
        int user_mode = (CURRENT_LATCHES.PSR >> 15) & 1;
        int IR11  = (CURRENT_LATCHES.IR >> 11) & 1;
        int and1 = COND_0 & COND_1 & IR11 & (!COND_2); // first and gate in microsequencer
        int and2 = COND_0 & (!COND_1) & CURRENT_LATCHES.READY & (!COND_2); // second and gate in the microsequencer
        int and3 = !COND_0 & COND_1 & CURRENT_LATCHES.BEN & (!COND_2); // third and gate in the microsequencer
        int and4 = COND_2 & (!COND_1) & (!COND_0) & (CURRENT_LATCHES.INT_PENDING); // fourth and gate
        int and5 = COND_2 & COND_1 & (!COND_0) & user_mode; // fifth and gate
        int address;
        int marmux_val; // the current value in the marmux before it goes on the bus: evaluate both possibilities
        // depending on the mux input
        if (GetMARMUX(u) == 0) {
            marmux_val = (CURRENT_LATCHES.IR & 0xFF) << 1;
        }
        else {
            marmux_val = Low16bits(MEM_ALU());
        }
        if (GetFAULTMUX(u) == 0) {
            address = marmux_val;
        }
        else {
            address = CURRENT_LATCHES.MAR;
        }
        int protected = (address < 0x3000 && user_mode) ? 1 : 0; // if mar < 0x3000, this is an access issue 
        int data_size = GetDATA_SIZE(u);
        int unaligned = data_size & Low16bits(address & 1);
        int j0 = u[J0] | and1;
        int j1 = u[J1] | and2;
        int j2 = u[J2] | and3; 
        int j3 = u[J3] | and4;
        int j4 = u[J4] | and5;
        int j5 = u[J5] | and5; // j5 and j4 use the same and gate, useful for exactly one state (lol
        next_state = j0 | (j1 << 1) | (j2 << 2) | (j3 << 3) | (j4 << 4) | (j5 << 5); // build next state
        if (COND_2 & (!COND_1) & (COND_0) & (protected | unaligned) & (!(GetIRD(u)))) {
            // means that you go to exception handler state, or 39, emulates second mux
            next_state = 39;
        }
    }
    NEXT_LATCHES.STATE_NUMBER = next_state; // assign the next state number to the mux switch
    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[next_state], sizeof(int) * CONTROL_STORE_BITS);    
}


int cycle_count = 0;
void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
    if (CURRENT_LATCHES.MICROINSTRUCTION[MIO_EN] == 0) {
        // mio isn't enabled, do nothing
        cycle_count = 0;
        NEXT_LATCHES.READY = 0; // keep ready low, reset cycles
    }
    else { // MIO.EN = 1
        cycle_count++;
        if (cycle_count == 4) {
            NEXT_LATCHES.READY = 1;
        }
        else if (cycle_count == 5) {
            if (CURRENT_LATCHES.MICROINSTRUCTION[R_W] == 1) { // this means it is a write, do nothing for a read
                // case of write
                if (CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE] == 0) {
                    // byte-sized data not word-sized
                    if ((CURRENT_LATCHES.MAR & 1) == 0) {
                        // that means store in even address
                        MEMORY[CURRENT_LATCHES.MAR/2][0] = CURRENT_LATCHES.MDR & 0xFF;
                    }
                    else {
                        // store in odd address
                        MEMORY[CURRENT_LATCHES.MAR/2][1] = (CURRENT_LATCHES.MDR >> 8) & 0xFF;                    
                    }
                    // doing CURRENT_LATCHES.MAR & 1 essentially just checks odd vs even  
                }
                else {
                    // word-siz
                    MEMORY[CURRENT_LATCHES.MAR/2][0] = (CURRENT_LATCHES.MDR) & 0xFF;
                    MEMORY[CURRENT_LATCHES.MAR/2][1] = (CURRENT_LATCHES.MDR >> 8) & 0xFF;
                }    
            }
            NEXT_LATCHES.READY = 0;
            cycle_count = 0;
        }
    }
}


int MARMUX_out, PC_Out, ALU_Out, SHF_Out, MDR_Out, VECTOR_Out, SP_Out, PSR_Out;
void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   as well as the other tristate drivers added for lab 4
   */    
    int* u = CURRENT_LATCHES.MICROINSTRUCTION;
    // what happens in MARMUX
    if (GetMARMUX(u) == 0) { // means that the MARMUX takes ZEXT(IR[7:0])
        int zextIR = Low16bits((CURRENT_LATCHES.IR) & 0x00FF);
        zextIR = zextIR << 1; // LSHF 1
        MARMUX_out = Low16bits(zextIR);
    }
    else {
        MARMUX_out = MEM_ALU();
    }
    // what happens in PC
    PC_Out = CURRENT_LATCHES.PC;
    // what happens in ALU
    // do SR2 mux first
    int SR2MUX_out;
    if (CURRENT_LATCHES.IR & 0x20) { 
        // if the bit 4 is a 1, use the imm5
        int IR_input = Low16bits(CURRENT_LATCHES.IR & 0x1F);
        if (IR_input & 0x10) {
            // need to sign extend
            IR_input = Low16bits(0xFFE0 | IR_input);
        }
        SR2MUX_out = IR_input;
    }
    else {
        SR2MUX_out = CURRENT_LATCHES.REGS[(CURRENT_LATCHES.IR & 0x7)]; // mask for the last 3 bits, which are the SR2 bits
    }
    int SR1_val = 0;
    if (GetSR1MUX(u) == 0) {
        // this means that SR = IR[11:9]
        SR1_val = CURRENT_LATCHES.REGS[Low16bits((CURRENT_LATCHES.IR >> 9) & 0x7)];
    } 
    else if (GetSR1MUX(u) == 1) {
        // this means that SR = IR[8:6]
        SR1_val = CURRENT_LATCHES.REGS[Low16bits((CURRENT_LATCHES.IR >> 6) & 0x7)];
    }
    else {
        // means that it is R6
        SR1_val = CURRENT_LATCHES.REGS[6];
    }
    int ALU_MUX = GetALUK(u);
    if (ALU_MUX == 0) {
        // add
        ALU_Out = Low16bits(SR1_val + SR2MUX_out);
    }
    else if (ALU_MUX == 1) {
        // and
        ALU_Out = Low16bits(SR1_val & SR2MUX_out);
    }
    else if (ALU_MUX == 2) {
        // XOR
        ALU_Out = Low16bits(SR1_val ^ SR2MUX_out);
    }
    else {
        // PASS A, which is SR1
        ALU_Out = SR1_val;
    }
    // what happens in SHF
    // get SR1Out
    if (GetSR1MUX(u) == 0) {
        // this means that SR = IR[11:9]
        SR1_val = CURRENT_LATCHES.REGS[Low16bits((CURRENT_LATCHES.IR >> 9) & 0x7)];
    } 
    else if (GetSR1MUX(u) == 1) {
        // this means that SR = IR[8:6]
        SR1_val = CURRENT_LATCHES.REGS[Low16bits((CURRENT_LATCHES.IR >> 6) & 0x7)];
    }
    else {
        // R6
        SR1_val = CURRENT_LATCHES.REGS[6];
    }
    int steer = (CURRENT_LATCHES.IR >> 4) & 0x3; // mask for the two steer bits 
    int shift_amt = (CURRENT_LATCHES.IR) & 0xF; // last four bits
    if (steer == 0) {
        // LSHF
        SHF_Out = Low16bits(SR1_val << shift_amt); 
    }
    else if (steer == 1) {
        // RSHFL
        SHF_Out = Low16bits(SR1_val >> shift_amt);
    }
    else if (steer == 3) {
        // RSHFA
        int sign_bit = (SR1_val >> 15); // msb
        SHF_Out = Low16bits(SR1_val >> shift_amt);
        if (sign_bit) {
            for (int i = 0; i < shift_amt; i++) {
                SHF_Out |= 1 << (15-i); // bit padding
            }
        }    
    }
    // what happens in MDR
    // first block encountered after bus
    int mar = CURRENT_LATCHES.MAR;
    if (GetDATA_SIZE(u) == 1) {
        //word sized
        MDR_Out = Low16bits(CURRENT_LATCHES.MDR);
    }
    else {
        //byte sized, now look at MAR[0] to see if to load into high or low byte of bus
        int byte;
        if ((mar & 0x1) == 0) {
            // this means that even, load into low bits?
            byte = CURRENT_LATCHES.MDR & 0xFF;
        }
        else {
            // this means odd, put into high bits
            byte = (CURRENT_LATCHES.MDR >> 8) & 0xFF;
        }
        if (byte & 0x80) {
            // need sext
            byte |= 0xFF00;
        }
        MDR_Out = Low16bits(byte);
    }
    // what happens in VECTOR
    VECTOR_Out = Low16bits(0x0200 + ((CURRENT_LATCHES.EventVector & 0xFF) << 1));
    // what happens in SP
    int SP_MUX = GetSPMUX(u);
    if (SP_MUX == 0) {
        // R6 - 2
        SP_Out = CURRENT_LATCHES.REGS[6] - 2;
    }
    else if (SP_MUX == 1) {
        // R6 + 2
        SP_Out = CURRENT_LATCHES.REGS[6] + 2;
    }
    else if (SP_MUX == 2) {
        // return SSP
        SP_Out = CURRENT_LATCHES.SSP;
    }
    else {
        // return USP
        SP_Out = CURRENT_LATCHES.USP;
    }
    // what happens in PSR
    PSR_Out = Low16bits((CURRENT_LATCHES.PSR & 0x8000) | (CURRENT_LATCHES.N << 2) | (CURRENT_LATCHES.Z << 1) | CURRENT_LATCHES.P);     
}


void drive_bus() {
  /* 
   * Datapath routine for driving the bus from one of the 5 + 3 possible 
   * tristate drivers. 
   */  
  int* u = CURRENT_LATCHES.MICROINSTRUCTION;
  BUS = 0; // reset bus
  if (GetGATE_PC(u) == 1) {
    BUS = PC_Out;
  }     
  else if (GetGATE_MARMUX(u)) {
    BUS = MARMUX_out;
  }
  else if (GetGATE_ALU(u)) {
    BUS = ALU_Out;
  } 
  else if (GetGATE_SHF(u)) {
    BUS = SHF_Out;
  }
  else if (GetGATE_MDR(u)) {
    BUS = MDR_Out;
  }
  else if (GetGATE_VECTOR(u)) {
    BUS = VECTOR_Out;
  }
  else if (GetGATE_SP(u)) {
    BUS = SP_Out;
  }
  else if (GetGATE_PSR(u)) {
    BUS = PSR_Out;
  }
  BUS = Low16bits(BUS); // limit to 16 bits to prevent overflows
}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       
    int* u = CURRENT_LATCHES.MICROINSTRUCTION;
    // LD MAR
    if (GetLD_MAR(u)) {
        NEXT_LATCHES.MAR = BUS;
    }
    // LD MDR
    // gets next MDR
    if (GetLD_MDR(u)) {
        if (GetMIO_EN(u)) {
            if (CURRENT_LATCHES.READY) {
                // ready
                int row = CURRENT_LATCHES.MAR >> 1;
                NEXT_LATCHES.MDR = Low16bits(MEMORY[row][0] | (MEMORY[row][1] << 8));

            }    
        }  
        else {
            // get from bus
            if (GetDATA_SIZE(u)) {
                NEXT_LATCHES.MDR = Low16bits(BUS);
            }
            else {
                // byte sized
                int byte = BUS & 0xFF;
                if ((CURRENT_LATCHES.MAR & 1) == 0) {
                    NEXT_LATCHES.MDR = byte;
                }
                else {
                    NEXT_LATCHES.MDR = byte << 8;
                }
            }
        }  
    }
    // LD IR
    // gets next IR
    if (GetLD_IR(u)) {
        NEXT_LATCHES.IR = BUS;
    }
    // LD REG
    // uses DR MUX to select 
    if (GetLD_REG(u)) {
        if (GetDRMUX(u) == 0) {
            // this means that the next reg is from IR[11:9]
            int DR = ((CURRENT_LATCHES.IR) & 0xE00) >> 9; // masks for 11:9 and then moves them into three lowest positions
            NEXT_LATCHES.REGS[DR] = BUS;
        }
        else if (GetDRMUX(u) == 1) {
            // that means that R7 is chosen
            NEXT_LATCHES.REGS[7] = BUS;
        }
        else {
            //R6
            NEXT_LATCHES.REGS[6] = BUS;
        }
    }
    // LD CC
    if (GetLD_CC(u)) {
        if (GetPSRMUX(u) == 1) {
            NEXT_LATCHES.N = (BUS >> 2) & 1;
            NEXT_LATCHES.Z = (BUS >> 1) & 1;
            NEXT_LATCHES.P = BUS & 1; // just gets data from the bus when RTI
        }
        else {
            if (Low16bits(BUS) == 0) {
                // set Z bit
                NEXT_LATCHES.N = 0;
                NEXT_LATCHES.Z = 1;
                NEXT_LATCHES.P = 0;
            }
            else if (Low16bits(BUS) & 0x8000) {
                // first bit is a 1, negative, set N bit
                NEXT_LATCHES.N = 1;
                NEXT_LATCHES.Z = 0;
                NEXT_LATCHES.P = 0;
            }
            else {
                // must be positive
                NEXT_LATCHES.N = 0;
                NEXT_LATCHES.Z = 0;
                NEXT_LATCHES.P = 1;
            }
        }    
    }
    // LD BEN
    if (GetLD_BEN(u)) {
        int IR = CURRENT_LATCHES.IR;
        NEXT_LATCHES.BEN = (((IR >> 11) & 1) & CURRENT_LATCHES.N) | (((IR >> 10) & 1) & CURRENT_LATCHES.Z) | (((IR >> 9) & 1) & CURRENT_LATCHES.P);
        // if any one of these is true, then branches
    }    
    // LD PC
    if (GetLD_PC(u)) {
        int PCMUX = GetPCMUX(u);
        if (PCMUX == 0) {
            // selects PC + 2
            NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
        }
        else if (PCMUX == 1) {
            // selects BUS value
            NEXT_LATCHES.PC = BUS;
        }
        else if (PCMUX == 2) {
            // selects address adder
            NEXT_LATCHES.PC = MEM_ALU();
        }
        else {
            // it's PC - 2
            NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC - 2);  
        }
    }
    // LD EV
    if (GetLD_EV(u)) {
        int EV_MUX = GetEVMUX(u);
        if (EV_MUX == 0) {
            NEXT_LATCHES.EventVector = 0x01;
        }
        else if (EV_MUX == 1) {
            int user_mode = (CURRENT_LATCHES.PSR >> 15) & 1;
            int address;
            int marmux_val; // the current value in the marmux before it goes on the bus: evaluate both possibilities
            // depending on the mux input
            if (GetMARMUX(u) == 0) {
                marmux_val = (CURRENT_LATCHES.IR & 0xFF) << 1;
            }
            else {
                marmux_val = Low16bits(MEM_ALU());
            }
            if (GetFAULTMUX(u) == 0) {
                address = marmux_val;
            }
            else {
                address = CURRENT_LATCHES.MAR;
            }
            int protected = (address < 0x3000 && user_mode) ? 1 : 0; // if mar < 0x3000, this is an access issue 
            int data_size = GetDATA_SIZE(u);
            int unaligned = data_size & Low16bits(address & 1);
            NEXT_LATCHES.EventVector = 0x02 + (unaligned & (!protected));
        }
        else {
            NEXT_LATCHES.EventVector = 0x04;
        }    
    }
    // LD USP
    if (GetLD_USP(u)) {
        // set to r6 if you want to load
        NEXT_LATCHES.USP = Low16bits(CURRENT_LATCHES.REGS[6]);
    }
    // LD SSP
    if (GetLD_SSP(u)) {
        // set to r6 if you want to load
        NEXT_LATCHES.SSP = Low16bits(CURRENT_LATCHES.REGS[6]);
    }
    // LD PSR
    if (GetLD_PSR(u)) {
        int priv;
        if (GetPSRMUX(u) == 0) {
            priv = 0;
        }
        else {
            priv = BUS & 0x8000; // bit 15 of bus
        }
        NEXT_LATCHES.PSR = priv;
    }    
    if (GetCLRINT(u)) {
        // means we need to acknowledge, clear int
        NEXT_LATCHES.INT_PENDING = 0;
    }
}


int MEM_ALU() {
    int* u = CURRENT_LATCHES.MICROINSTRUCTION;
    int addr1val, addr2val;
        // other MARMUX, do address addition
        if (GetADDR1MUX(u) == 0) {
            // equals PC
            addr1val = CURRENT_LATCHES.PC;
        }
        else {
            // equals BaseR
            int REG;
            if (GetSR1MUX(u) == 0) {
                // look at IR[11:9]
                REG = (CURRENT_LATCHES.IR >> 9) & 0x7;
            }
            else if  (GetSR1MUX(u) == 1) {
                // look at IR[8:6]
                REG = (CURRENT_LATCHES.IR >> 6) & 0x7;
            }
            else {
                // means that SR1MUX = 10, so R6
                REG = 6;
            }
            addr1val = CURRENT_LATCHES.REGS[REG]; // gets value of current baseR from sr1mux
        }
        int addr2mux = GetADDR2MUX(u);
        if (addr2mux == 0) {
            addr2val = 0;
        }
        else if (addr2mux == 1) {
            // offset 6
            int offset6 = Low16bits(CURRENT_LATCHES.IR & 0x3F);
            // now sign extend
            if (offset6 & 0x20) {
                // MSB is 1, need sext
                offset6 |= 0xFFC0; // fill in all high bits as 1
            }
            addr2val = offset6;
        }
        else if (addr2mux == 2) {
            // pc offset 9
            int offset6 = Low16bits(CURRENT_LATCHES.IR & 0x01FF);
            // now sign extend
            if (offset6 & 0x100) {
                // MSB is 1, need sext
                offset6 |= 0xFE00; // fill in all high bits as 1
            }
            addr2val = offset6;
        }
        else if (addr2mux == 3) {
            // pc offset 11
            int offset6 = Low16bits(CURRENT_LATCHES.IR & 0x07FF);
            // now sign extend
            if (offset6 & 0x400) {
                // MSB is 1, need sext
                offset6 |= 0xF800; // fill in all high bits as 1
            }
            addr2val = offset6;
        }
        if (GetLSHF1(u)) {
            // if we need to do a shift
            addr2val = addr2val << 1;
        }
        // add addr1val and addr2val, set to marmux output
        return(Low16bits(addr1val + addr2val));
}