#define PAIN (volatile unsigned char *) 0xFFFFFFF0
#define PAOUT (volatile unsigned char *) 0xFFFFFFF1
#define PADIR (volatile unsigned char *) 0xFFFFFFF2
#define PBIN (volatile unsigned char *) 0xFFFFFFF3
#define PBOUT (volatile unsigned char *) 0xFFFFFFF4
#define PBDIR (volatile unsigned char *) 0xFFFFFFF5 
#define CNTM (volatile unsigned int *) 0xFFFFFFD0 
#define CTCON (volatile unsigned char *) 0xFFFFFFD8 
#define CTSTAT (volatile unsigned char *) 0xFFFFFFD9 
#define IVECT (volatile unsigned int *) (0x20)

interrupt void intserv(); 

unsigned char digit_1 = 0;	            /* 1st Digit to be displayed */ 
unsigned char digit_2 = 0;	            /* 2nd Digit to be displayed */ 
unsigned int alt_led = 0;               /* 0 for Led1 and 1 for Led2 */

int main() {
    *PADIR = 0x78;                      /* Set Port A direction */
    *PBDIR = 0xF5;                      /* Set Port B direction */
    *CTCON = 0x02;                      /* Stop Timer */
    *CTSTAT = 0x0;                      /* Clear “reached 0” flag */
    *CNTM = 100000000;                  /* Initialize Timer */
    *IVECT = (unsigned int *) &intserv; /* Set interrupt vector */
    asm(“MoveControl PSR,#0x40”);       /* CPU IRQ enabled */
    *CTCON = 0x01;                      /* Start counting w/o timer interrupt enable */
    
    while (1) {
        while (*CTSTAT != 0x0);          /* Wait until 0 is reached */

        if ( ) {

        } else {
          digit_2 = (digit_2 + 1)%10;         /* Increment digit_2 */
          *PBOUT = (digit_2 << 3);            /* Update Port B */
        }
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

    if (increment == 1) {
        digit = (digit + 1)%10;         /* Increment digit */
        *PBOUT = ((digit << 4) | 0x0);  /* Update Port B */
    }
}
