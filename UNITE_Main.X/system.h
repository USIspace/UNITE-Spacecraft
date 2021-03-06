/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/

/* TODO Define system operating frequency */

/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        32000000L
#define FCY             SYS_FREQ/2


#define LED_ON 0
#define LED_OFF 1

#define max_s(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/

/* Custom oscillator configuration funtions, reset source evaluation
functions, and other non-peripheral microcontroller initialization functions
go here. */

void ConfigureOscillator(void); /* Handles clock switching/osc initialization */
void wait_sec(unsigned long);
void wait_for(unsigned long);
unsigned long Pow(int, int);
void ClearQueue(uint8_t *,int,int);
void Clear(void *,int,int);
int CopyIntToByte(int *source, uint8_t *destination, int sourceStart, int destStart, int numberOfItems);
int CopyIntToDoubleByte(int *source, uint16_t *destination, int sourceStart, int destStart, int numberOfItems);
int CopyIntToByteArray(int *source, uint8_t *destination, int sourceStart, int destStart, int numberOfItems);
int CopyBytes(uint8_t *source, uint8_t *destination, int sourceStart, int destStart, int numberOfItems);
int add_sec(unsigned long *);

//void wait_ms(unsigned long);

