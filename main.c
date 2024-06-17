#include <msp430.h>
#include <std_types.h>

/////////////////////  MEMORY USED  ///////////////////
//
// Flash/FRAM usage is 870 bytes of 4KB = 21.24% used
//        RAM usage is 94 bytes of 256B = 36.71% used
//
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
//  01  | P1.0/TA0CLK/ACLK/A0             | BTN_DOWN
//  02  | P1.1/TA0.0/A1                   | BTN_UP
//  03  | P1.2/TA0.1/A2                   | BTN_SHOT
//  04  | P1.3/ADC10CLK/A3/VREF-/VEREF    | FOCUS (To BSS138)
//  05  | P1.4/SMCLK/A4/VREF+/VEREF+/TCK  | CAPTURE (To BSS138)
//  06  | P1.5/TA0.0/SCLK/A5/TMS          | LED C
//  07  | P1.6/TA0.1/SDO/SCL/A6/TDI/TCLK  | LED B
//  08  | P1.7/SDI/TDO/TDI                | LED A
//  09  | RST/NMI/SBWTDIO                 | JTAG
//  10  | TEST/SBWTCK                     | JTAG
//  11  | XOUT/P2.7                       | XTAL
//  12  | XIN/P2.6/TA0.1                  | XTAL
//  13  | AVSS                            | GND
//  14  | DVSS                            | GND
//  15  | AVCC                            | 3.3V
//  16  | DVCC                            | 3.3V
//------+---------------------------------+------------------

// PINOUT
#define BTN_SHOT          (BIT2)
#define BTN_UP            (BIT1)
#define BTN_DOWN          (BIT0)
#define FOCUS             (BIT3)
#define CAPTURE           (BIT4)
#define LED_A             (BIT7)
#define LED_B             (BIT6)
#define LED_C             (BIT5)
#define FACTOR            (8u)

// Features
//#define WITH_FOCUS

// Time availables in seconds
#define TIME_XS_1         (2u)        // 001
#define TIME_XS_2         (5u)        // 010
#define TIME_XS_3         (10u)       // 011
#define TIME_XS_4         (30u)       // 100
#define TIME_XS_5         (60u)       // 101
#define TIME_XS_6         (120u)      // 110
#define TIME_XS_7         (300u)      // 111

// Table of times
#define SEL_TIME_1        (TIME_XS_1 * FACTOR)
#define SEL_TIME_2        (TIME_XS_2 * FACTOR)
#define SEL_TIME_3        (TIME_XS_3 * FACTOR)
#define SEL_TIME_4        (TIME_XS_4 * FACTOR)
#define SEL_TIME_5        (TIME_XS_5 * FACTOR)
#define SEL_TIME_6        (TIME_XS_6 * FACTOR)
#define SEL_TIME_7        (TIME_XS_7 * FACTOR)
#define WAITING_TO_IDLE   (5u * FACTOR)

// Types defs
typedef enum
{
  firt_time = 0u,
  last_time = 7u
} Selection_t;

typedef enum
{
  pause = 0u,
  play,
  idle
} State_t;

// Global variables
uint16      counter       = 0u;         // Up to 120 counts
State_t     state         = idle;
Selection_t selection     = firt_time;  // Default
uint16      selectTime[]  = 
{
  0, 
  SEL_TIME_1,
  SEL_TIME_2,
  SEL_TIME_3,
  SEL_TIME_4,
  SEL_TIME_5,
  SEL_TIME_6,
  SEL_TIME_7
};

static inline void startTimer()
{
  counter = 0;
  TAR     = 0u;
  TACTL  |= MC_1;
}

static inline void stopTimer()
{
  counter = 0;
  TACTL &= ~MC_3;
  TAR    = 0u;
}

static inline void turnOffLeds(void)
{
  P1OUT &= ~(LED_A | LED_B | LED_C);   // Turn off led
}

static inline void turnOnLeds(Selection_t selection)
{
  turnOffLeds();
  // 0b00000CBA -> 0bABC00000
  P1OUT |= ((selection & BIT2) << 5) | ((selection & BIT1) << 5) | ((selection & BIT0) << 5);
}

static inline void captureOn(void)
{
  P1OUT |= (CAPTURE);
}

static inline void captureOff(void)
{
  P1OUT &= ~(CAPTURE);
}

static inline void focusOn(void)
{
  P1OUT |= (FOCUS);
}

static inline void focusOff(void)
{
  P1OUT &= ~(FOCUS);
}

static inline void enablingAllButtons(void)
{
  P1IE |= (BTN_SHOT | BTN_UP | BTN_DOWN);   // Enabled interrupt
}

static inline void disablingAllButtons(void)
{
  P1IE &= ~(BTN_SHOT | BTN_UP | BTN_DOWN);   // Disabled interrupt
}

static inline void enablingUpDownButtons(void)
{
  P1IE |= (BTN_UP | BTN_DOWN);   // Enabled interrupt
}

static inline void disablingUpDownButtons(void)
{
  P1IE &= ~(BTN_UP | BTN_DOWN);   // Disabled interrupt
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
  P1OUT   = 0U;
  P1DIR   = (FOCUS | CAPTURE | LED_A | LED_B | LED_C);
  P1IFG   = 0u;
  P1IES   = (BTN_SHOT | BTN_UP | BTN_DOWN);   // Falling edge
  P1IE    = (BTN_SHOT | BTN_UP | BTN_DOWN);   // Enabled interrupt
  P1SEL   = 0u;
  P1SEL2  = 0u;
  P1REN   = 0u;
  // PORT 2
  P2IN    = 0u;
  P2OUT   = 0u;
  P2DIR   = 0u;
  P2IFG   = 0u;
  P2IES   = 0u;
  P2IE    = 0u;
  P2SEL   = (BIT7 | BIT6);
  P2SEL2  = 0u;
  P2REN   = 0u;
	
  // TIMER
  TACCTL0   = CCIE;                           // Interrupt enabled
  TACCR0    = 512-1;                          // 32k / 8 / 8 = 512 = every 125ms
  TAR       = 0;                              // Reset counter
  TACTL     = TASSEL_1 + MC_1 + ID_3;         // SMCLK, Up mode, /8

  // Enable interrupt
  _enable_interrupt();

  // Disabling all
  turnOffLeds();
  captureOff();
  focusOff();

  // Sleep up to start
  LPM4;
	
  // Maon loop
  while(1)
	{
    if(state == play)
    {
      // Press
      #ifdef WITH_FOCUS
      focusOn();
      #else
      captureOn();
      #endif
      startTimer();
      turnOffLeds();
      LPM3;
    }
    else if(state == pause)
    {
      // Just to ensure unpress
      focusOff();
      captureOff();
      startTimer();
      turnOnLeds(selection);
      LPM3;
    }
    else if(state == idle)
    {
      stopTimer();
      turnOffLeds();
      LPM4;
    }
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
  counter++;

  if(state == play)
  {
    if(0u == selectTime[selection])   // On demand
    {
      if(counter == 1u)
      {
        #ifdef WITH_FOCUS
        focusOff();
        captureOn();
        #else
        captureOff();
        #endif
      }
      else if(counter == 2u)
      {
        #ifdef WITH_FOCUS
        captureOff();
        #endif
        counter = 0;
      }
      else
      {
        // Do nothing
      }
    }
    else                              // Selected time
    {
      // Capturing or Finishing
      if(counter == 1u)
      {
        #ifdef WITH_FOCUS
        focusOff();
        captureOn();
        #else
        captureOff();
        #endif
      }

      // Finishing yes or yes
      if(counter == 2u)
      {
        #ifdef WITH_FOCUS
        captureOff();
        #endif
      }

      // Reset counter
      if(counter == selectTime[selection])
      {
        // PRESS
        #ifdef WITH_FOCUS
        focusOn();
        #else
        captureOn();
        #endif
        counter = 0;
      }
    }
  }
  else if(state == pause)
  {
    // Wait 5 seconds only
    if(counter == WAITING_TO_IDLE)
    {
      state = idle;
      counter = 0;
      LPM3_EXIT;
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
  if(state == idle)
  {
    state = pause;
    LPM4_EXIT;
    P1IFG &= ~(BTN_SHOT | BTN_UP | BTN_DOWN);
  }
  else
  {
    if(P1IFG & BTN_SHOT)
    {
      if(state == pause)
      {
        state = play;
        disablingUpDownButtons();
      }
      else if(state == play)
      {
        state = pause;
        enablingUpDownButtons();
      }
      else
      {
        // Do nothing
      }
      P1IFG &= ~BTN_SHOT;
    }
    else if(P1IFG & BTN_UP)
    {
      if(state == pause)
      {
        if(selection == last_time)
        {
          selection = firt_time;
        }
        else
        {
          selection++;
        }
        counter = 0u;
      }
      else
      {
        // Do nothing
      }
      P1IFG &= ~BTN_UP;
    }
    else if(P1IFG & BTN_DOWN)
    {
      if(state == pause)
      {
        if(selection == firt_time)
        {
          selection = last_time;
        }
        else
        {
          selection--;
        }
        counter = 0u;
      }
      else
      {
        // Do nothing
      }
      P1IFG &= ~BTN_DOWN;
    }
    LPM3_EXIT;
  }
  
}
