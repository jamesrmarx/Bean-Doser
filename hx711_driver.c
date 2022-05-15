#include <XC.h>
#include <sys/attribs.h>
#include <stdio.h>

#pragma config FNOSC = FRCPLL       // Internal Fast RC oscillator (8 MHz) w/ PLL
#pragma config FPLLIDIV = DIV_2     // Divide FRC before PLL (now 4 MHz)
#pragma config FPLLMUL = MUL_20     // PLL Multiply (now 80 MHz)
#pragma config FPLLODIV = DIV_2     // Divide After PLL (now 40 MHz) see figure 8.1 in datasheet for more info
#pragma config FWDTEN = OFF         // Watchdog Timer Disabled
#pragma config FPBDIV = DIV_1       // PBCLK = SYCLK

#define SYSCLK 40000000L
#define Baud2BRG(desired_baud)( (SYSCLK / (16*desired_baud))-1)

//******************************************
// Delay Functions                         *
// *****************************************

// use the core timer to wait set number of ums
void waitums(int t){
  unsigned int ui;
  _CP0_SET_COUNT(0); // reset the core timer

  // get the core timer count
  while(_CP0_GET_COUNT() < t*(SYSCLK/(2*1000*1000)));
}

// use the core timer to wait 1ms
void wait_1ms(void){
  unsigned int ui;
  _CP0_SET_COUNT(0); // reset the core timer

  // get the core timer count
  while(_CP0_GET_COUNT() < (SYSCLK/(2*1000)));
}

// use gore timer to wait set number of ms 
void waitms(int len){
  while(len--) wait_1ms();
}

//*******************************************
// UART serial functions
// ******************************************
void UART2Configure(int baud_rate)
{
    // Peripheral Pin Select
    U2RXRbits.U2RXR = 4;    //SET RX to RB8
    RPB9Rbits.RPB9R = 2;    //SET RB9 to TX

    U2MODE = 0;         // disable autobaud, TX and RX enabled only, 8N1, idle=HIGH
    U2STA = 0x1400;     // enable TX and RX
    U2BRG = Baud2BRG(baud_rate); // U2BRG = (FPb / (16*baud)) - 1
    
    U2MODESET = 0x8000;     // enable UART2
}

/* SerialReceive() is a blocking function that waits for data on
 *  the UART2 RX buffer and then stores all incoming data into *buffer
 *
 * Note that when a carriage return '\r' is received, a nul character
 *  is appended signifying the strings end
 *
 * Inputs:  *buffer = Character array/pointer to store received data into
 *          max_size = number of bytes allocated to this pointer
 * Outputs: Number of characters received */
unsigned int SerialReceive(char *buffer, unsigned int max_size)
{
    unsigned int num_char = 0;
 
    /* Wait for and store incoming data until either a carriage return is received
     *   or the number of received characters (num_chars) exceeds max_size */
    while(num_char < max_size)
    {
        while( !U2STAbits.URXDA);   // wait until data available in RX buffer
        *buffer = U2RXREG;          // empty contents of RX buffer into *buffer pointer

        while( U2STAbits.UTXBF);    // wait while TX buffer full
        U2TXREG = *buffer;          // echo
 
        // insert nul character to indicate end of string
        if( *buffer == '\r')
        {
            *buffer = '\0';     
            break;
        }
 
        buffer++;
        num_char++;
    }
 
    return num_char;
}


void uart_puts(char * s)
{
	while(*s)
	{
		putchar(*s);
		s++;
	}
}

char HexDigit[]="0123456789ABCDEF";
void PrintNumber(long int val, int Base, int digits)
{ 
	int j;
	#define NBITS 32
	char buff[NBITS+1];
	buff[NBITS]=0;

	j=NBITS-1;
	while ( (val>0) | (digits>0) )
	{
		buff[j--]=HexDigit[val%Base];
		val/=Base;
		if(digits!=0) digits--;
	}
	uart_puts(&buff[j+1]);
}


//**********************************************
// Function: Configures all pin directions
// Return: NONE
//
// Pins used:
//
// RB5 - Pin 14 as Digital Input (DOUT)
// RB6 - Pin 15 as Digital Output (SCK)
// *********************************************
void configurePins(){
  TRISBbits.TRISB5 = 1; // RB5 as Input
  TRISBbits.TRISB6 = 0; // RB6 as output
  LATBbits.LATB6   = 0; // Initialize SCK as low
}

//**********************************************
// Function: Read 24 bits of ADC data to 32-bit 
// integer
//
// Return: 32-bit integer with 24-bit 2's 
// complement value
// *********************************************
int read_hx711_24(){
  int  adcVal   = 0;
  int  idx      = 0;
  int  doutMask = 0b100000; // mask off RB5
  int  dout = 0;

  dout = (PORTB&doutMask)>>5;
  uart_puts("DOUT INIT: ");
  PrintNumber(dout,2,1);
  uart_puts("\n\r");

  for(idx=0; idx<24; idx++){
    LATBbits.LATB6 = 1; //clk high
    waitums(2);
    dout = (PORTB&doutMask)>>5;
    waitums(2);
    LATBbits.LATB6 = 0; //clk low
    waitums(4);
    uart_puts("DOUT: ");
    PrintNumber(dout,2,1);
    uart_puts("\n\r");

    adcVal |= dout;
    adcVal = adcVal<<1;
  }

  LATBbits.LATB6 = 1;
  waitums(4);
  LATBbits.LATB6 = 0;
  waitums(4);

  uart_puts("adcVal: ");
  PrintNumber(adcVal,2,32);
  uart_puts("\n\r");



  return dout;

}

void main(){
  DDPCON = 0;
  CFGCON = 0;
  UART2Configure(115200); // Configure UART2 for baud rate of 115200
  configurePins(); // Configure pins and directions

  // Give putty a chance to start
  waitms(500); //wiat 500 ms

  int adc = 0;
 
  uart_puts("PORTB (1): ");
  PrintNumber(PORTB,2, 32);
  uart_puts("\n\r");

  adc = read_hx711_24();


//      PrintNumber(adc, 10, 5);
//      uart_puts("\r");
}

