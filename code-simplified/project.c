#include <stdlib.h>

#define keyChanges ((volatile long *) 0x10000040)
#define lcdDisplay ((char *) 0x10003050)
#define RLEDs ((volatile long *) 0x10000000)

#define Timer 0x10002000
#define TimerStatus ((volatile short*) (Timer))
#define TimerControl ((volatile short*) (Timer+4))
#define TimerTimeoutL ((volatile short*) (Timer+8))
#define TimerTimeoutH ((volatile short*) (Timer+12))

#define pushButtons ((volatile long *) 0x10000050)
#define pushButtons_Interrupt ((volatile long *) (pushButtons + 8))
#define PushbuttonChanges ((volatile long *) 0x1000005C)

#define ADDR_7SEG1 ((volatile long *) 0x10000020)
#define ADDR_7SEG2 ((volatile long *) 0x10000030)

#define KEY1 2
#define KEY2 4
#define KEY3 8

#define SEVEN_SEG_0 0x3F
#define SEVEN_SEG_1 0x6
#define SEVEN_SEG_2 0x5B
#define SEVEN_SEG_3 0x4F
#define SEVEN_SEG_4 0x66
#define SEVEN_SEG_5 0x6D
#define SEVEN_SEG_6 0x7D
#define SEVEN_SEG_7 0x7
#define SEVEN_SEG_8 0x7F
#define SEVEN_SEG_9 0x6F

typedef enum { LToR, RToL } LED_DIRECTION; 
typedef enum { READY, P1_START, P2_START, IN_GAME, GAME_OVER } GAME_STATE; 

typedef int bool;
#define TRUE (bool)1
#define FALSE (bool)0

#define GAME_TIME 60
#define MEDIUM_SPEED 30

LED_DIRECTION direction = RToL;
GAME_STATE game_state = READY;
int timer;
int p1_score = 00;
int p2_score = 00;
int count_down = GAME_TIME;
int count_down_timer = 0;
int game_speed = MEDIUM_SPEED;

void interrupt(void) __attribute__ ((section(".exceptions")));
void interrupt(void) {
	asm("rdctl et, ctl4");
	asm("andi et, et, 0x1");
	asm("beq et, r0, PUSH_BUTTON_INTERRUPT");
	
	asm("TIMER_INTERRUPT:");
	asm("call ContextSave");
	asm("call runGame");
	asm("call ContextRestore");
	asm("movia et, 0x10002000");
	asm("stwio r0, 0(et)");
	asm("br EXIT_IHANDLER");
	
	asm("PUSH_BUTTON_INTERRUPT:");
	asm("rdctl et, ctl4");
	asm("andi et, et, 0x2");
	asm("beq et, r0, EXIT_IHANDLER");
	
	asm("call ContextSave");
	asm("call buttons");
	asm("call ContextRestore");
	asm("movia et, 0x10002000");
	asm("stwio r0, 0(et)");
	asm("movia r2, 0x10000050");
	asm("stwio r0, 12(r2)");
	
	asm("EXIT_IHANDLER:");
	asm("subi ea, ea, 4");
	asm("eret");
}

void ContextSave(void) __attribute__ ((section(".exceptions")));
void ContextSave(void) {
	asm("addi sp,sp,-124");
	asm("stw r1, 120(sp)");
	asm("stw r2, 116(sp)");
	asm("stw r3, 112(sp)");
	asm("stw r4, 108(sp)");
	asm("stw r5, 114(sp)");
	asm("stw r6, 100(sp)");
	asm("stw r7, 96(sp)");
	asm("stw r8, 92(sp)");
	asm("stw r9, 88(sp)");
	asm("stw r10, 84(sp)");
	asm("stw r11, 80(sp)");
	asm("stw r12, 76(sp)");
	asm("stw r13, 72(sp)");
	asm("stw r14, 68(sp)");
	asm("stw r15, 64(sp)");
	asm("stw r16, 60(sp)");
	asm("stw r17, 56(sp)");
	asm("stw r18, 52(sp)");
	asm("stw r19, 48(sp)");
	asm("stw r20, 44(sp)");
	asm("stw r21, 40(sp)");
	asm("stw r22, 36(sp)");
	asm("stw r23, 32(sp)");	
	asm("ret");
}

void ContextRestore(void) __attribute__ ((section(".exceptions")));
void ContextRestore(void) {	
	asm("ldw r1,120(sp)");
	asm("ldw r2,116(sp)");
	asm("ldw r3,112(sp)");
	asm("ldw r4,108(sp)");
	asm("ldw r5,114(sp)");
	asm("ldw r6,100(sp)");
	asm("ldw r7,96(sp)");
	asm("ldw r8,92(sp)");
	asm("ldw r9,88(sp)");
	asm("ldw r10,84(sp)");
	asm("ldw r11,80(sp)");
	asm("ldw r12,76(sp)");
	asm("ldw r13,72(sp)");
	asm("ldw r14,68(sp)");
	asm("ldw r15,64(sp)");
	asm("ldw r16,60(sp)");
	asm("ldw r17,56(sp)");
	asm("ldw r18,52(sp)");
	asm("ldw r19,48(sp)");
	asm("ldw r20,44(sp)");
	asm("ldw r21,40(sp)");
	asm("ldw r22,36(sp)");
	asm("ldw r23,32(sp)");
	asm("addi sp,sp,124");
	asm("ret");	
}

void displayTextLCD(char * text_ptr){
    volatile char * LCD_display_ptr = (char *) lcdDisplay;  // 16x2 character display
	
	if (text_ptr == NULL) {
		*(char *) lcdDisplay = 0x00000001;			//Set command to clear LCD
	} else {    
		while ( *(text_ptr) ){
			*(LCD_display_ptr + 1) = *(text_ptr);   // write to the LCD data register
			++text_ptr;
		}
	}
}

void initTimerInterrupt() {
   //interrupt every 1/100s
   // Configure the timeout period to one second
   *(TimerTimeoutL)=0xA120;
   *(TimerTimeoutH)=0x0007;
   // Configure timer to start counting and set status upon time out
   *(TimerControl)=7;
   //asm("movia r9, 0x1");
   //asm("wrctl ctl3, r9");
   //asm("wrctl ctl0, r9");
}

void initPushButtonsInterrupt() {
  //*pushButtons_Interrupt = 0xe; /* Enable interrupts on push buttons 1,2, and 3 */

  asm("movia r2,0x10000050"); 
  asm("movia r3,0xe"); 
  asm("stwio r3,8(r2)"); 
  
  //asm("movia r2, 0x02");  		/*movia r2,IRQ_PUSHBUTTONS  push button is 2, or with timer to be 3*/ 
  //asm("wrctl ctl3,r2");   	/* Enable bit 5 - button interrupt on Processor */

  //asm("movia r2,1");
  //asm("wrctl ctl0,r2");   	/* Enable global Interrupts on Processor */  
}

void enableInterrupts() {
   asm("movia r9, 0x3");  /* 1 for timer + 2 for buttons*/
   asm("wrctl ctl3, r9");
   asm("movia r2,1");
   asm("wrctl ctl0,r2");   	/* Enable global Interrupts on Processor */  
}

void displayGameInstructions() {
	displayTextLCD(NULL);
	switch (game_state) {
		case READY:
			displayTextLCD("Key 2: start/end ");
			break;
		case P1_START:
			displayTextLCD("P1 Key 3: serve");
			break;
		case P2_START:
			displayTextLCD("P2 Key 1: serve");
			break;
		case IN_GAME:
			displayTextLCD("In game");
			break;
		case GAME_OVER:
			if (p1_score > p2_score) {
				displayTextLCD("p1 Won! K2 restart");
			} else if (p2_score > p1_score) {
				displayTextLCD("p2 Won! K2 restart");
			} else {
				displayTextLCD("Tie. K2 restart");
			}
			break;
	}
}

void buttons() {
	long buttonVal = *PushbuttonChanges;
	
	switch (game_state) {
		case READY:
			timer = 0;
			count_down_timer = 0;
			p1_score = 0;
			p2_score = 0;
			if (buttonVal == KEY2) {
				count_down = GAME_TIME;
				if ((timer % 2) == 0) {
					game_state = P2_START;
				} else {
					game_state = P1_START;
				}
			}
		break;
		
		case P1_START:
			if (buttonVal == KEY2 && count_down != GAME_TIME) {
				game_state = GAME_OVER;
				break;
			}
			if (buttonVal == KEY3) {
				*RLEDs = 0x20000;
				direction = LToR;
				game_state = IN_GAME;
			}
		break;
		
		case P2_START:
			if (buttonVal == KEY2 &&  count_down != GAME_TIME) {
				game_state = GAME_OVER;
				break;
			}
			if (buttonVal == KEY1) {
				*RLEDs = 0x1;
				game_state = IN_GAME;
				direction = RToL;
			}
		break;
		
		case IN_GAME:
			if (buttonVal == KEY2) {
				game_state = GAME_OVER;
			}
			if (buttonVal == KEY3 && *RLEDs == 0x20000) {				
				direction = LToR;
				game_speed -= 2;
			} else if(buttonVal == KEY1 && *RLEDs == 0x1){
				direction = RToL;
				game_speed -= 2;
			}
		break;
		
		case GAME_OVER:
			if (buttonVal == KEY2) {
				game_state = READY;
				game_speed = MEDIUM_SPEED;
			}
		break;
	}
}

void shiftLEDs() {
	if (direction == RToL) {
		*RLEDs = *RLEDs<<1;
	} else {
		*RLEDs = *RLEDs>>1;
	}
}

void flashLEDs() {
	if (*RLEDs == 0) {
		*RLEDs = 0x2aaaa;
	} else {
		*RLEDs = 0;
	}
}

void sevenSegDisplayScore(){
	int total_score = p1_score * 100 + p2_score;
	*ADDR_7SEG2 = 0;
	
	int i = 0;
	for (i = 0; i < 4; i++) {
		int digit = total_score % 10;
		total_score /= 10;
		switch (digit) {
			case 0:
				*ADDR_7SEG2 |= (SEVEN_SEG_0 << i*8);
				break;
			case 1:
				*ADDR_7SEG2 |= (SEVEN_SEG_1 << i*8);
				break;
			case 2:
				*ADDR_7SEG2 |= (SEVEN_SEG_2 << i*8);
				break;
			case 3:
				*ADDR_7SEG2 |= (SEVEN_SEG_3 << i*8);
				break;
			case 4:
				*ADDR_7SEG2 |= (SEVEN_SEG_4 << i*8);
			break;
			case 5:
				*ADDR_7SEG2 |= (SEVEN_SEG_5 << i*8);
			break;
			case 6:
				*ADDR_7SEG2 |= (SEVEN_SEG_6 << i*8);
			break;
			case 7:
				*ADDR_7SEG2 |= (SEVEN_SEG_7 << i*8);
			break;
			case 8:
				*ADDR_7SEG2 |= (SEVEN_SEG_8 << i*8);
			break;
			case 9:
				*ADDR_7SEG2 |= (SEVEN_SEG_9 << i*8);
			break;				
		}
	}
}

void sevenSegDisplayTime(){
	if (game_state == IN_GAME) {
		count_down_timer = (count_down_timer + 1) % 100;
		
		if (count_down_timer == 0)
			count_down--;
			
		if (count_down == 0) {
			game_state = GAME_OVER;
		}
	}
	
	*ADDR_7SEG1 = 0;
	int to_display = count_down;
	
	int i = 0;
	for (i = 0; i < 4; i++) {
		int digit = to_display % 10;
		to_display /= 10;
		switch (digit) {
			case 0:
				*ADDR_7SEG1 |= (SEVEN_SEG_0 << i*8);
				break;
			case 1:
				*ADDR_7SEG1 |= (SEVEN_SEG_1 << i*8);
				break;
			case 2:
				*ADDR_7SEG1 |= (SEVEN_SEG_2 << i*8);
				break;
			case 3:
				*ADDR_7SEG1 |= (SEVEN_SEG_3 << i*8);
				break;
			case 4:
				*ADDR_7SEG1 |= (SEVEN_SEG_4 << i*8);
			break;
			case 5:
				*ADDR_7SEG1 |= (SEVEN_SEG_5 << i*8);
			break;
			case 6:
				*ADDR_7SEG1 |= (SEVEN_SEG_6 << i*8);
			break;
			case 7:
				*ADDR_7SEG1 |= (SEVEN_SEG_7 << i*8);
			break;
			case 8:
				*ADDR_7SEG1 |= (SEVEN_SEG_8 << i*8);
			break;
			case 9:
				*ADDR_7SEG1 |= (SEVEN_SEG_9 << i*8);
			break;				
		}
	}
}

void runGame() {
	timer++;
	sevenSegDisplayScore();
	sevenSegDisplayTime();
	displayGameInstructions();
	switch (game_state) {
		case READY:
		case GAME_OVER:
			if ((timer % 100) == 0) {
				flashLEDs();
			}
		break;
			
		case P1_START:
		case P2_START:
			*RLEDs = 0;
		break;
		
		case IN_GAME:
			if ((timer % game_speed) == 0) {
				shiftLEDs();
				if (*RLEDs == 0) {
					game_speed += 5;
					if ( direction == LToR) {
						p1_score++;
						game_state = P1_START;
					} else {
						p2_score++;
						game_state = P2_START;
					}
				}
				
			}
		break;
	}	
}

int main() {
	timer = 0;
	initTimerInterrupt();
	initPushButtonsInterrupt();
	enableInterrupts();
	displayGameInstructions();
	sevenSegDisplayScore();
	while(1) {
	}
}
