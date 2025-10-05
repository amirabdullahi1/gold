#define PAOUT (volatile unsigned char *) 0xFFFFFFF1
#define PADIR (volatile unsigned char *) 0xFFFFFFF2
#define PBIN (volatile unsigned char *) 0xFFFFFFF3
#define PBOUT (volatile unsigned char *) 0xFFFFFFF4
#define PBDIR (volatile unsigned char *) 0xFFFFFFF5 
#define CNTM (volatile unsigned int *) 0xFFFFFFD0 
#define CTCON (volatile unsigned char *) 0xFFFFFFD8 
#define CTSTAT (volatile unsigned char *) 0xFFFFFFD9 
#define IVECT (volatile unsigned int *) (0x20)
#define LED1MASK (0x04)
#define LED2MASK (0x80)
#define SWMASK (0x10)

interrupt void intserv(); 

unsigned char digit_1 = 0;	            /* 1st Digit to be displayed */ 
unsigned char digit_2 = 0;	            /* 2nd Digit to be displayed */ 
volatile unsigned int alt_led = 0;      /* 0 for Led1 and 1 for Led2 */

int main() {
  *PADIR = 0xF4;                        /* Set Port A direction */
  *PBDIR = 0x8F;                        /* Set Port B direction */
  *CTCON = 0x02;                        /* Stop Timer */
  *CTSTAT = 0x0;                        /* Clear “reached 0” flag */
  *CNTM = 100000000;                    /* Initialize Timer */
  *IVECT = (unsigned int *) &intserv;   /* Set interrupt vector */
  asm("MoveControl PSR,#0x40");         /* CPU IRQ enabled */
	*CTCON = 0x11;                      	/* Enable Timer interrupts and start counting */
  *PAOUT = 0x0;        									/* Init Port A w/ Dig1 = 0 and Led1 on */
	*PBOUT |= LED2MASK;										/* Init Port B w/ Led2 off and Dig2 = 0 */

	while (1) {
		while((*PBIN & SWMASK) != 0);				/* Wait until SW is pressed */
		while((*PBIN & SWMASK) == 0);				/* Wait until SW is released */
		
		if (alt_led == 0) {
			*PAOUT |= LED1MASK;         /* Turn off Led1 */
			*PBOUT &= ~LED2MASK;        /* Turn on Led2 */
			alt_led = 1;
		} else {
			*PAOUT &= ~LED1MASK;        /* Turn on Led1 */
			*PBOUT |= LED2MASK;         /* Turn off Led2 */
			alt_led = 0;
		}
	}
	
	exit(0);
}

interrupt void intserv() {
	*CTSTAT = 0x0;               	/* Clear “reached 0” flag */

	if (alt_led == 0) {
		/* Increment digit_1 */
		digit_1 = (digit_1 + 1)%10;    
		/* Update Port A w/ Led1 on */                
		*PAOUT = (unsigned char)((digit_1 << 4) & ~LED1MASK); 
	} else {
		/* Increment digit_2 */
		digit_2 = (digit_2 + 1)%10;
		/* Update Port B w/ Led2 on */                               
		*PBOUT = (unsigned char)((digit_2) & ~LED2MASK);          
	}
}
