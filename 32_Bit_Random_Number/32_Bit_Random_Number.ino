/*
                      32 Bit Random Number Generator
                      Vernon Billingsley  c2023

                 Generate a random number using a 32 bit LFSR..
                 Return a random 8 bit number between X and Y..


     Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission
    notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.


    Pin   Mega Function    Sketch Function
    0     PE0   RXO
    1     PE1   TX0
    2     PE4   OC3B  INT4
    3     PE5   OC3C  INT6
    4     PG5   OC0B
    5     PE3   OC3A  AIN(1)
    6     PH3   OC4A
    7     PH4   OC4B
    8     PH5   OC4C
    9     PH6   OC2B
    10    PB4   OC2A
    11    PB5   OC1A
    12    PB6   OC1B
    13    PB7   OC0A  OC1A
    14    PJ1   TX3
    15    PJ0   RX3
    16    PH1   TX1
    17    PH0   RX2
    18    PD3   TX1
    19    PD3   RX1
    20    PD1   SDA
    21    PD0   SCL
    22    PA0
    23    PA1
    24    PA2
    25    PA3
    26    PA4
    27    PA5
    28    PA6
    29    PA7
    30    PC7
    31    PC6
    32    PC5
    33    PC4
    34    PC3
    35    PC2
    36    PC1
    37    PC0
    38    PD7
    39    PG2   BLE
    40    PG1   RD
    41    PG0   WR
    42    PL7
    43    PL6
    44    PL5
    45    PL4
    46    PL3
    47    PL2
    48    PL1
    49    PL0
    50    PB3   MISO
    51    PB2   MOSI
    52    PB1   SCK
    53    PB0   SS
    A0    PF0
    A1    PF1
    A2    PF2
    A3    PF3
    A4    PF4
    A5    PF5
    A6    PF6
    A7    PF7
    A8    PK0
    A9    PK1
    A10   PK2
    A11   PK3
    A12   PK4
    A13   PK5
    A14   PK6
    A14   PK7


*/

/* Optimize for speed over size */
#pragma GCC optimize("-Ofast")

/************************* Defines ********************************/
#define DEBUG 1

#if DEBUG == 1
#define dprint(expression) \
  Serial.print("# "); \
  Serial.print(#expression); \
  Serial.print(": "); \
  Serial.println(expression)
#define dshow(expression) Serial.println(expression)
#else
#define dprint(expression)
#define dshow(expression)
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


/************************** Variables *****************************/
/* Hold the linear feedback shift register */
volatile uint32_t LFSR = 0xBAD00ACEu;
/* Hold an 8 bit random number */
volatile uint8_t random_number = 0;


/**************************  Functions ****************************/
/* Get a random number between from and to */
uint8_t get_random(uint8_t floor, uint8_t ceiling){
  float coef = (1.0 / ((float)255 / (float)(ceiling - floor)));
  uint8_t r = (floor + (byte)((float)random_number * coef));  
  return r;
}

/******************************************************************/
/*************************** Setup ********************************/
/******************************************************************/
void setup() {
  if (DEBUG) {
    Serial.begin(115200);
  }

  /************************* Setup Pins ***************************/


  /*************************  Setup Timer3 ************************/
  /* Stop interrupts */
  cli();
  /* Clear the Timer 1 registers */
  TCCR3A = 0;
  TCCR3B = 0;
  /* Initialize the count to Zero */
  TCNT3 = 0;
  /* Set compare match register value */
  /* F_CPU / ( prescaler * time )  */
  /* 16 mHz / ( 1 * 2 kHz) = 8000  */
  OCR3A = 8000;
  /* Set to Clear Timer on Compare Match */
  cbi(TCCR3B, WGM30);
  cbi(TCCR3B, WGM31);
  sbi(TCCR3B, WGM32);
  cbi(TCCR3B, WGM33);
  /* Set prescaler */
  /*  CS12  CS11  CS10    prescaler
   *   0    0     0       No clock source, disabled
   *   0    0     1       System clock, no prescale
   *   0    1     0       clk/8
   *   0    1     1       clk/64
   *   1    0     0       clk/256
   *   1    0     1       clk/1024
   */
  cbi(TCCR3B, CS32);
  cbi(TCCR3B, CS31);
  sbi(TCCR3B, CS30);
  /* Enable timer compare interrupt */
  /* This will enable the ISR to be used */
  sbi(TIMSK3, OCIE3A);
  /*Allow interrupts */
  sei();

} /**************************  End Setup **************************/

/*ISR to handle the Timer 3 interrupt */
ISR(TIMER3_COMPA_vect) {
  /* Get the first bit of the lfsr , bit 31 */
  uint8_t bit_one = (LFSR >> 30) & 0x01;
  /* Get the second bit, bit 28 */
  uint8_t bit_two = (LFSR >> 27) & 0x01;
  /* Shift the register left 1 bit */
  LFSR = LFSR << 1;
  /* Get the new bit to add */
  uint8_t this_bit = (bit_one ^ bit_two);
  /* XOR and add the bit */
  LFSR = LFSR + this_bit;
  /* Get the OUTPUT byte */
  random_number = LFSR >> 24;
}

/******************************************************************/
/**************************** Loop ********************************/
/******************************************************************/
void loop() {

  /* Send a random number to the Serial port for */
  /* display on the sysem plotter                */
  Serial.print(256);
  Serial.print("\t");
  Serial.print(0);
  Serial.print("\t");
  Serial.print(random_number);
  Serial.print("\t");
  /* Display a numner between x and y */
  Serial.println(get_random(10, 100));
  delay(125);


} /*************************** End Loop *****************************/
