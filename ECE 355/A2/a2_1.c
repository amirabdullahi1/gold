#define PAIN (volatile unsigned char *) 0xFFFFFFF0
#define PAOUT (volatile unsigned char *) 0xFFFFFFF1
#define PADIR (volatile unsigned char *) 0xFFFFFFF2
#define PBOUT (volatile unsigned char *) 0xFFFFFFF4
#define PBDIR (volatile unsigned char *) 0xFFFFFFF5 
#define PCONT (volatile unsigned char *) 0xFFFFFFF7 
#define CNTM (volatile unsigned int *) 0xFFFFFFD0 
#define CTCON (volatile unsigned char *) 0xFFFFFFD8 
#define CTSTAT (volatile unsigned char *) 0xFFFFFFD9 
#define IVECT (volatile unsigned int *) (0x20)
#define LED1MASK (0x04)
#define LED2MASK (0x01)
#define SWMASK (0x80)

interrupt void intserv(); 

unsigned char digit_1 = 0;	            /* 1st Digit to be displayed */ 
unsigned char digit_2 = 0;	            /* 2nd Digit to be displayed */ 
unsigned int alt_led = 0;               /* 0 for Led1 and 1 for Led2 */

int main() {
  *PADIR = 0x78;                        /* Set Port A direction */
  *PBDIR = 0xF5;                        /* Set Port B direction */
  *CTCON = 0x02;                        /* Stop Timer */
  *CTSTAT = 0x0;                        /* Clear “reached 0” flag */
  *CNTM = 100000000;                    /* Initialize Timer */
  *IVECT = (unsigned int *) &intserv;   /* Set interrupt vector */
  asm("MoveControl PSR,#0x40");         /* CPU IRQ enabled */
  *PCONT = 0x10;                        /* Enable port A interrupts */
  *CTCON = 0x01;                        /* Start counting w/o timer interrupt enable */
  *PAOUT &= SWMASK;                     /* Init Port A w/ &ing SWMASK so Dig1 = 0 */
  *PBOUT = LED2MASK;                    /* Init Port B w/ LED2MASK so Led1 on, Led2 off and Dig2 = 0 */
    
  while (1) {
    while ((*CTSTAT & 0x1) == 0x0);     /* Wait until 0 is reached */
    *CTSTAT = 0x0;                      /* Clear “reached 0” flag */

    if (alt_led == 0) {
      /* Increment digit_1 */
      digit_1 = (digit_1 + 1)%10;    
      /* Update Port A w/o changing SW */                
      *PAOUT = (unsigned char)((digit_1 << 3) | (*PAOUT & SWMASK)); 
    } else {
      /* Increment digit_2 */
      digit_2 = (digit_2 + 1)%10;
      /* Update Port B so Led1 is off and Led2 is on */                               
      *PBOUT = (unsigned char)((digit_2 << 4) | LED1MASK);          
    }
  }

  exit(0);
}

interrupt void intserv() {
  if ((*PAIN & SWMASK) != 0) {        /* Only alt when SW releasd */
      if (alt_led == 0) {
        *PBOUT |= LED1MASK;         /* Turn off Led1 */
        *PBOUT &= ~LED2MASK;        /* Turn on Led2 */
        alt_led = 1;
      } else {
        *PBOUT &= ~LED1MASK;        /* Turn on Led1 */
        *PBOUT |= LED2MASK;         /* Turn off Led2 */
        alt_led = 0;
      }
  }
}
