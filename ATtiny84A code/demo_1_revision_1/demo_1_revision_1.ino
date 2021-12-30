/* 
 * Okay dang I didn't read the ATtinycore very well and
 * I put my RGB LED signals on pins that don't have AnalogWrite()
 * functionality, so it's time for binary code modulation
 * for more info: http://www.batsocks.co.uk/readme/art_bcm_3.htm
 */

#include <avr/interrupt.h>

#define RED_PIN 9
#define GREEN_PIN 5
#define BLUE_PIN 10

volatile unsigned int frame = 6;
volatile unsigned char red = 10;
volatile signed int redDir = 1;
volatile unsigned char green = 0;
volatile signed int greenDir = 1;
volatile unsigned char blue = 70;
volatile signed int blueDir = 1;
volatile unsigned int cyclesToWait = 8191;

void setup() {
  CLKPR |= 1<<CLKPCE; // Enable Clock prescaler change
  CLKPR &= ~(15<<CLKPS0); // Clock prescaler changed to 1/1 (8 MHz) -- Must happen within 4 cycles of the last line
  
  // Timer/Counter 0 interrupts every 10 ms or so
  TCCR0A = 3<<WGM00;           // Fast PWM
  TCCR0B = 1<<WGM02 | 5<<CS00; // Fast PWM | 1:1024 prescaling, Page 84
  TIMSK0 = 1<<TOIE0;  // Timer Overflow Interrupt Enable
  OCR0A = 240; // Interrupt after 78 cycles (100 FPS ish)
  
  // Timer/Counter 1 interrupts after 4096 clock cycles, then 2048, then 1024, ...
  TCCR1A = 3<<WGM10;    // Fast PWM mode 15, page 110 // Try modes 11 or 9 maybe
  TCCR1B = 3<<WGM12 | 1<<CS10;    // Fast PWM mode 15 | No prescaling
  TIMSK1 = 1<<TOIE1;
  //OCR1A = 127; // Interrupt after 4096 cycles to start (approx 2 kHz) // Currently .5 khz?
  OCR1A = 2047;
  
  // Set port directions
  DDRB = 3<<DDB0;
  DDRA = 1<<DDA5;
  
  PORTB |= 1<<PORTB1;
  PORTA |= 1<<PORTA5;
  
  sei();
}

void loop() {
  
}

// on a Timer 1 overflow: WORKS NOW, FORGOT PRESCALER // Gets weirdly bright when jumping from 31 to 32 D:
ISR(TIM1_OVF_vect) {
  // Disable interrupts during this!
  unsigned char sreg;
  sreg = SREG; // Global interrupt flag
  cli(); // Disable interrupts
  cyclesToWait = OCR1A;
  if (blue & (1<<frame)) {
    PORTB &= ~(1<<PORTB0);
  } else {
    PORTB |= 1<<PORTB0;
  }
  if (red & (1<<frame)) {
    PORTB &= ~(1<<PORTB1);
  } else {
    PORTB |= 1<<PORTB1;
  }
  if (green & (1<<frame)) {
    PORTA &= ~(1<<PORTA5);
    } else {
    PORTA |= 1<<PORTA5;
  }
  if (frame <= 0) {
    cyclesToWait = 8191;
    frame = 6;
  } else {
    cyclesToWait = cyclesToWait>>1;
    frame--;
  }
  OCR1A = cyclesToWait;
  SREG = sreg; // Restore interrupts
}

// On a Timer 0 overflow: every 10 ms, update brightness (this works)
ISR(TIM0_OVF_vect) {
  // Disable interrupts??
  unsigned char sreg;
  sreg = SREG;
  cli();
  
  green += greenDir;
  if (green>=50) {
    green = 50;
    greenDir = -1;
    } else if (green <= 10) {
    green = 10;
    greenDir = 1;
  }
  
  red += redDir;
  if (red>=125) {
    red = 125;
    redDir = -1;
    } else if (red <= 10) {
    red = 10;
    redDir = 1;
  }
  
  blue += blueDir;
  if (blue>=110) {
    blue = 110;
    blueDir = -1;
  } else if (blue <= 0) {
    blue = 0;
    blueDir = 1;
  }
  
  SREG = sreg;
}
