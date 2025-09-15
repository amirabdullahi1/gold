#define PBIN (volatile unsigned char *) 0xFFFFFFF3
#define PBOUT (volatile unsigned char *) 0xFFFFFFF4
#define PBDIR (volatile unsigned char *) 0xFFFFFFF5 
#define CNTM (volatile unsigned int *) 0xFFFFFFD0 
#define CTCON (volatile unsigned char *) 0xFFFFFFD8 
#define CTSTAT (volatile unsigned char *) 0xFFFFFFD9 
#define IVECT (volatile unsigned int *) (0x20)

interrupt void intserv(); 

unsigned char digit = 0;	    /* Digit to be displayed */ 
unsigned char led = 0x1;	    /* LED state: 0/1 = on/off */ 

int main() {
   *PBDIR = 0xF0;                      /* Set Port B direction */
   *CTCON = 0x02;                      /* Stop Timer */
   *CTSTAT = 0x0;                      /* Clear “reached 0” flag */
   *CNTM = 100000000;                  /* Initialize Timer */
   *IVECT = (unsigned int *) &intserv; /* Set interrupt vector */

   asm(“MoveControl PSR,#0x40”);       /* CPU responds to IRQ */ /* E was pressed last, change digit */

   asm(“MoveControl PSR,#0x00”);       /* CPU responds to IRQ */ /* D was pressed last, do not change digit */

   *CTCON = 0x11;                      /* Enable Timer interrupts and start counting */
   *PAOUT = 0x01;                      /* Display 0, turn LED off */
}

interrupt void intserv() {
    *CTSTAT = 0x0; /* Clear “reached 0” flag */
    digit = (digit + 1)%10; /* Increment digit */
    *PBOUT = ((digit << 4) | led); /* Update Port A */
}
