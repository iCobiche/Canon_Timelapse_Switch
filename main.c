#include <msp430.h>
#include <std_types.h>

/////////////////////  DEVICE PINOUTS  ///////////////////
//
//                       MSP4302332
//                      ____________
//                     |            |
//              P1.0 1 |*           | 16 DVCC
//              P1.1 2 |            | 15 AVCC
//              P1.2 3 |            | 14 DVSS
//              P1.3 4 |            | 13 AVSS
//              P1.4 5 |            | 12 P2.6
//              P1.5 6 |            | 11 P2.7
//              P1.6 7 |            | 10 TEST
//              P1.7 8 |            |  9 RST
//                     |____________|
//             
// 
/////////////////////  PIN DISTRIBUTIONS  ///////////////////
//
// PIN  | FUNCTION                        | CONNECTION
//------+---------------------------------+------------------
//  01  | P1.0/TA0CLK/ACLK/A0             |
//  02  | P1.1/TA0.0/A1                   |
//  03  | P1.2/TA0.1/A2                   |
//  04  | P1.3/ADC10CLK/A3/VREF-/VEREF    |
//  05  | P1.4/SMCLK/A4/VREF+/VEREF+/TCK  |
//  06  | P1.5/TA0.0/SCLK/A5/TMS          |
//  07  | P1.6/TA0.1/SDO/SCL/A6/TDI/TCLK  |
//  08  | P1.7/SDI/TDO/TDI                |
//  09  | RST/NMI/SBWTDIO                 |
//  10  | TEST/SBWTCK                     |
//  11  | XOUT/P2.7                       |
//  12  | XIN/P2.6/TA0.1                  |
//  13  | AVSS                            |
//  14  | DVSS                            |
//  15  | AVCC                            |
//  16  | DVCC                            |
//------+---------------------------------+------------------

#define BTN_SHOT  (BIT2)
#define BTN_UP    (BIT1)
#define BTN_DOWN  (BIT0)
#define FOCUS     (BIT3)
#define CAPTURE   (BIT4)
#define LED_A     (BIT5)
#define LED_B     (BIT6)
#define LED_C     (BIT7)

// Types defs
typedef enum
{
  s_1s5 = 0u,
  s_2s,
  s_3s,
  s_5s,
  s_10s,
  s_20s,
  s_30s,
  s_60s
} Selection_t;

typedef enum
{
  pause = 0u,
  play
} State_t;

// Global variables
uint8       counter       = 0u;         // Up to 120 counts
State_t     state         = pause;
Selection_t selection     = s_1s5;      // Default
uint8       selectTime[]  = {6u, 8u, 12u, 20u, 40u, 80u, 120u, 240u};

void startTimer()
{
  TAR    = 0u;
  TACTL |= MC_1;
}

void stopTimer()
{
  TACTL &= ~MC_3;
  TAR    = 0u;
}

void disactiveLeds(void)
{
  P1OUT &= ~(LED_A | LED_B | LED_C);   // Turn off led
}

void activeLeds(Selection_t selection)
{
  disactiveLeds();
  // 0b00000CBA -> 0bABC00000
  P1OUT |= ((selection & BIT0) << 7) | ((selection & BIT1) << 5) | ((selection & BIT2) << 3);
}

/**
 * main.c
 */
void main(void)
{
  // Stop watchdog timer

  WDTCTL = WDTPW | WDTHOLD;

  // Configure clock to 16 MHz
  DCOCTL    = CALDCO_1MHZ;
  BCSCTL1   = CALBC1_1MHZ;

  // Port init
  // PORT 1
  P1IN    = 0u;
  P1OUT   = (BTN_SHOT | BTN_UP | BTN_DOWN);
  P1DIR   = (FOCUS | CAPTURE | LED_A | LED_B | LED_C);
  P1IFG   = 0u;
  P1IES   = (BTN_SHOT | BTN_UP | BTN_DOWN);   // Falling edge
  P1IE    = (BTN_SHOT | BTN_UP | BTN_DOWN);   // Enabled interrupt
  P1SEL   = 0u;
  P1SEL2  = 0u;
  P1REN   = (BTN_SHOT | BTN_UP | BTN_DOWN);
  // PORT 2
  P2IN    = 0u;
  P2OUT   = 0u;
  P2DIR   = 0u;
  P2IFG   = 0u;
  P2IES   = 0u;
  P2IE    = 0u;
  P2SEL   = 0u;
  P2SEL2  = 0u;
  P2REN   = 0u;
	
  // TIMER
  TACCTL0   = CCIE;                           // Interrupt enabled
  TACCR0    = 31250;                          // 1MHz / 8 * 31250 = every 250ms
  TAR       = 0;                              // Reset counter
  TACTL     = TASSEL_2 + MC_1 + ID_3;         // SMCLK, Up mode, /8

  // Enable interrupt
  _enable_interrupt();

  // Sleep up to start
  LPM0;
	
  // Maon loop
  while(1)
	{
    if(state == play)
    {
      // Press
      P1OUT |= (FOCUS | CAPTURE);
      startTimer();
      disactiveLeds();
    }
    else if(state == pause)
    {
      // Just to ensure unpress
      P1OUT &= ~(FOCUS | CAPTURE);
      stopTimer();
      activeLeds(selection);
    }
    LPM0;
	}
}

// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
  if(state == play)
  {
    counter++;

    // Unpress
    if(counter == 1u)
    {
      P1OUT &= ~(FOCUS | CAPTURE);
    }

    // Reset counter
    if(counter == selectTime[selection])
    {
      // PRESS
      P1OUT |= (FOCUS | CAPTURE);
      counter = 0;
    }
  }
}

// Port 1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
  if(P1IFG & BTN_SHOT)
  {
    if(state == pause)
    {
      state = play;
    }
    else
    {
      state = pause;
    }
    P1IFG &= ~BTN_SHOT;
  }
  else if(P1IFG & BTN_UP)
  {
    if(state == pause)
    {
      if(selection == s_60s)
      {
        selection = s_1s5;
      }
      else
      {
        selection++;
      }
    }
    P1IFG &= ~BTN_UP;
  }
  else if(P1IFG & BTN_DOWN)
  {
    if(state == pause)
    {
      if(selection == s_1s5)
      {
        selection = s_60s;
      }
      else
      {
        selection--;
      }
    }
    P1IFG &= ~BTN_DOWN;
  }
  LPM0_EXIT;
}
