/*
 * HID Reader Wiegand Interface for Arduino Uno

 
  Modifications by Jorge Villicana, 2014.06.01 
 *
 * This program will capture the data  from a PACS  Reader with Wiegand Output.
 * The Wiegand interface has two data lines, DATA0 and DATA1.  These lines are normall held
 * high at 5V.  When a 0 is sent, DATA0 drops to 0V for a few us.  When a 1 is sent, DATA1 drops
 * to 0V for a few us.  There is usually a few ms between the pulses.
 *
 * Your reader have  4 connections: RED - VDC, BLACK - GND, GREEN - DO and WHITE - D1. Connect the Red wire 
 * to 5V.  Connect the black to ground.  Connect DATA0 to Digital Pin 2 (INT0).  
 * Connect DATA1 to Digital Pin 3 (INT1).  
 *
 * How does it work? Each data line is connected to hardware interrupt lines.  When
 * one drops low, an interrupt routine is called and some bits are flipped.  After some time of
 * of not receiving any bits, the Arduino send the bits as string  through serial port and show on LCD.

*/


#include <LiquidCrystal.h>

LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );

#define MAX_BITS 100                 // max number of bits 
#define WIEGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.  
#define MAXBYTES 11

const int Data0 = 2;
const int Data1= 3;
const int Led = 13;

unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char datahex[MAXBYTES];
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int wiegand_counter;        // countdown until we assume there are no more bits



// interrupt that happens when INTO goes low (0 bit)
void ISR_INT0()
{
  Serial.print("0");
  bitCount++;
  flagDone = 0;
  wiegand_counter = WIEGAND_WAIT_TIME;  
  
}

// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1()
{
  Serial.print("1");
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  wiegand_counter = WIEGAND_WAIT_TIME;  
}

void setup()
{
  pinMode(13, OUTPUT);  // LED
  pinMode(2, INPUT);     // DATA0 (INT0)
  pinMode(3, INPUT);     // DATA1 (INT1)
  
  
  Serial.begin(9600);
  Serial.println("***Wiegand - USB Converter*** ");
  Serial.println("Present a Card...");
  
  
  //Initialize LCD
  lcd.begin(16, 2);
  lcd.print("***Wiegand - USB Converter***");
  lcd.setCursor(0,1);
  lcd.print(" Present a Card");
  
  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(0, ISR_INT0, FALLING);  
  attachInterrupt(1, ISR_INT1, FALLING);
  

  wiegand_counter = WIEGAND_WAIT_TIME;
}

void loop()
{
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--wiegand_counter == 0)
      flagDone = 1;	
  }
  
  // if we have bits and we the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;
    unsigned char j;
    unsigned char k = 0;
    
    //Display how many bits were read.
    Serial.print(" ( ");
    Serial.print(bitCount);
    Serial.print(" bits)");
    Serial.println();
    
    
    //Show card data in hexadecimal format
   
       j = 0;
       k = 0;
       
       lcd.clear();
       lcd.print("RAW [hex]: ");
       lcd.setCursor(0,1);
       for(i=0;i< bitCount && k < MAXBYTES;i++)
       {    
         datahex[k] |= (databits[bitCount-i -1]<< j);   
             
         j++;
         if (j==8)
         {         
           j=0;
           k++;        
         }
       }
      for (i =0; i<=k;i++)
      {
        if (datahex[k-i] < 16)
        {
          Serial.print("0");
          lcd.print("0");
        }
        
           Serial.print(datahex[k-i], HEX);
           lcd.print(datahex[k-i],HEX); 
        
      }
     Serial.println(" hex");
     lcd.setCursor(13,0);
     lcd.print(" (");
     lcd.print(bitCount);
     lcd.print(")");

     // cleanup and get ready for the next card
     bitCount = 0;
     
     for (i=0; i<MAX_BITS; i++) 
     {
       databits[i] = 0;
     }
     for (i=0; i<MAXBYTES; i++) 
     {
       datahex[i] = 0;
     }
     Serial.println("Present a Card...");
  }
}


