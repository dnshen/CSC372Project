#include "defines.h"
#include "main.h"
#include "kernel.h"

#ifdef NATIVE
/* The assembly language code below handles CPU reset processing */

void the_reset (void) __attribute__ ((section (".reset")));
void the_reset (void)
/************************************************************************************
 * Reset code. By giving the code a section attribute with the name ".reset" we     *
 * allow the linker program to locate this code at the proper reset vector address. *
 * This code just calls the main program.                                           *
 ***********************************************************************************/
{
  asm (".set		noat");					// Magic, for the C compiler
  asm (".set		nobreak");				// Magic, for the C compiler
  asm ("br		main");		// Call the C language main program
}


void the_isr (void) __attribute__ ((section (".exceptions")));
void the_isr (void)
/*****************************************************************************/
/* Interrupt Service Routine                                                 */
/*   Calls the interrupt handler and performs return from exception.         */
/*****************************************************************************/
{

  asm (".set		noat");
  asm (".set		nobreak");
  asm (	"subi	sp,  sp, 116");
  asm (	"rdctl	et,  ctl4");
  asm (	"beq	et,  r0, SOFT_INT");	/* Interrupt is not external         */
  asm (	"subi	ea,  ea, 4");		

  //Hardware Interrupt
  asm ("SKIP_EA_DEC:");
  SAVE_REGS;
  asm (	"addi	fp,  sp, 128");
  asm (	"call	interrupt_handler");// Call the interrupt handler
  LOAD_REGS;
  asm (	"addi	sp,  sp, 116");
  asm (	"eret");

  //Software Interrupt - Trap OR Illegal Insruction(not handled)
  asm ("SOFT_INT:");
  asm ("bne r8, r0, SOFT_INT_EXIT");

  //System call enter - go to kernel
  asm ("SOFT_INT_ENTER:");
  
  SAVE_REGS;
  MOVE_SP_TO_ACTIVE;
  MOVE_PC_TO_ACTIVE;
  MOVE_SR_TO_ACTIVE;

  SET_KERNEL_PC;
  SET_KERNEL_SP;
  SET_KERNEL_FP;
  SET_KERNEL_SR;
    
  asm (	"eret" );

  //System call exit - exit kernel, go to user
  asm ("SOFT_INT_EXIT:");
  
  MOVE_ACTIVE_TO_PC;
  MOVE_ACTIVE_TO_SP;
  MOVE_ACTIVE_TO_SR;
  LOAD_REGS;
  asm ( "addi sp,  sp, 116");
  asm ( "eret" );    
}


#endif /* NATIVE */

void interrupt_exit() {
}

void interrupt_handler(void)
{

}
