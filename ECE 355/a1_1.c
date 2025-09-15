#define PBIN (volatile unsigned char *) 0xFFFFFFF3
#define PBOUT (volatile unsigned char *) 0xFFFFFFF4
#define PBDIR (volatile unsigned char *) 0xFFFFFFF5 
#define CNTM (volatile unsigned int *) 0xFFFFFFD0 
#define CTCON (volatile unsigned char *) 0xFFFFFFD8 
#define CTSTAT (volatile unsigned char *) 0xFFFFFFD9 
#define IVECT (volatile unsigned int *) (0x20)

interrupt void intserv(); 

unsigned char digit = 0;	            /* Digit to be displayed */ 
unsigned int increment = 0;

int main() {
    *PBDIR = 0xF0;                      /* Set Port B direction */
    *CTCON = 0x02;                      /* Stop Timer */
    *CTSTAT = 0x0;                      /* Clear “reached 0” flag */
    *CNTM = 100000000;                  /* Initialize Timer */
    *IVECT = (unsigned int *) &intserv; /* Set interrupt vector */
    asm(“MoveControl PSR,#0x40”);       /* CPU IRQ enabled */
    *CTCON = 0x11;                      /* Enable Timer interrupts and start counting */
    
    while(1) {
        while ((*PBIN & 0x1) != 0);     /* Wait until E is pressed */
        while ((*PBIN & 0x1) == 0);     /* Wait until E is released */
        increment = 1;

        while ((*PBIN & 0x2) != 0);     /* Wait until D is pressed */
        while ((*PBIN & 0x2) == 0);     /* Wait until D is released */
        increment = 0;
    }
}

interrupt void intserv() {
    *CTSTAT = 0x0;                      /* Clear “reached 0” flag */

    if(increment == 1) {
        digit = (digit + 1)%10;         /* Increment digit */
        *PBOUT = ((digit << 4) | 0x0);  /* Update Port A */
    }
}
