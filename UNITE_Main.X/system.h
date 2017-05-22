/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/

/* TODO Define system operating frequency */

/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        32000000L
#define FCY             SYS_FREQ/2


#define LED_ON 0
#define LED_OFF 1
/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/

/* Custom oscillator configuration funtions, reset source evaluation
functions, and other non-peripheral microcontroller initialization functions
go here. */

void ConfigureOscillator(void); /* Handles clock switching/osc initialization */
void wait_sec(unsigned long);
void wait_for(unsigned long);
uint16_t Pow(int, int);
int add_sec(unsigned long *);

//void wait_ms(unsigned long);

