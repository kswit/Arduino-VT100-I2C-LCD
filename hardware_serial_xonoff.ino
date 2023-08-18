#include <stdint.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

//char usart_receive(void);
volatile bool receiveQ = true;
volatile bool transmitQ = true;
char  buff[32];
int datacnt;
const char  Xon =0x11;
const  char Xoff=0x13;
//void usart_init(uint16_t ubrr_value)
//{
//  UBRR0 = ubrr_value;
//
//  // 9600-8-E-1
//  // That is, baudrate of 9600bps
//  // 8 databits
//  // Even parity
//  // 1 stopbit
//  UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0); // And enable interrupts
//  UCSR0C = (1 << UPM01) | (1 << UCSZ01) | (1 << UCSZ00);
//}

void uart_init(void)
{

    UBRR0H = 0;
    UBRR0L = 103;
    UCSR0C |= 1 << UCSZ01 | 1 << UCSZ00;
    UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;
  
  
  }

 
ISR(USART_RX_vect)
{
    uint8_t byte = UDR0;
    
    
    switch ( byte)
    {   
        case Xoff:
            sprintf(buff,"rec");
            
            receiveQ = false;
            transmitQ = true;
            break;
            
        case Xon:
            sprintf(buff,"tr17");
            receiveQ = true;
            transmitQ = false;
            
            break;
        
        // process incoming data
        default:
            receiveQ = true;
            //sprintf(buff,byte);
            datacnt++;
            break;
    }
}
void transmitByte(uint8_t byte)
{
    while( !(UCSR0A & 1 << UDRE0));
    UDR0 = byte;
}
void transmitString(char *pstr)
{
    while( *pstr != 0)
    {
        transmitByte(*pstr);
        pstr++;
    }
}
void setup()
{
  uart_init();
  lcd.init();
  lcd.backlight();
   sei();
  //lcd.print(buff);
  transmitByte(Xon);
  transmitString("start");
  
//    UBRR0H = 0;
//    UBRR0L = 103;
//    UCSR0C |= 1 << UCSZ01 | 1 << UCSZ00;
//    UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;
//  
 

  
  }

void  loop()
{   
   
    //sei();
    
    while( 1)
    {
        if( transmitQ)
        {
            //transmitByte(Xoff);
            //transmitString("data");
            //transmitByte(Xon);
            transmitQ = false;
            lcd.print(buff);
            //receiveQ=true;
        };
        //_delay_ms(1000);
       if( receiveQ)
       {        
        lcd.setCursor(0,0);
        lcd.print(datacnt,DEC);
         lcd.print(buff);
        
        if(datacnt>100){
           transmitByte(Xoff);
           lcd.print ("stop"); }
          else{transmitByte(Xon);}
           //transmitQ=false;
           receiveQ=false;
        }
       
    }
    
}


//ISR(TIMER1_OVF_vect){ // 1000 Hz
//      TCNT1H = 0xC1;  
//      TCNT1L = 0x81; //initializing counter
//   }
