#include <p18f4620.h>
#include <stdio.h>
#include <math.h>
#include <usart.h>

#pragma config OSC = INTIO67
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config BOREN = OFF
#pragma config CCP2MX = PORTBE
    
void putch(char c);
void init_UART();
void init_ADC();
unsigned int Get_Full_ADC();
void Activate_Buzzer();
void Deactivate_Buzzer();

#define DECIMAL PORTDbits.RD7                               // Define 7 seg decimal bit
#define BLUELED PORTBbits.RB5                               // Define LED bit
#define VREF 4.096                                          // Define Reference Voltage
#define RREF 99.9                                           // Define Reference Resistance

int main()
{
    init_ADC();
    init_UART();
      
    TRISA = 0xFF;                                           // Set PORTA to input
    TRISB = 0x00;                                           // Set PORTB to output
    TRISC = 0x00;                                           // Set PORTC to output
    TRISD = 0x00;                                           // Set PORTD to output
    TRISE = 0x00;                                           // Set PORTE to output
    
    int arrayDigits[11] = {0x40,0x79,0x24,0x30,0x19,        // Array to hold bit values for 7-seg display
                           0x12,0x02,0x78,0x00,0x10,0x80};
    
    while(1)
    {
        char upperNum;
        char lowerNum;
        int decON;
        int num_step = Get_Full_ADC();                      // Convert Analog signal to Digital signal that can be used
        float volt = num_step * 4.0 / 1000;                 // Calculate, in volts, the difference from the potentiometer
        float kiloOhms = volt / (VREF - volt) * RREF;
        
        printf("Resistance: %f kohms\r\n",kiloOhms);        // Print value of the resistance on TeraTerm
        
        if(kiloOhms < 0.02)
        {
            Activate_Buzzer();                              // The buzzer sounds if resistance is lower than 20 ohms
            BLUELED = 1;                                    // The blue LED2 turns on if resistance is lower than 20 ohms
        }
        else
        {
            Deactivate_Buzzer();                            // Turn off the buzzer if resistance is greater than 20 ohms
            BLUELED = 0;                                    // The blue LED2 turns off if resistance is greater than 20 ohms
        }
        if(kiloOhms < 10)
        {
            decON = 0;                                      // Set the decimal on 7 seg to ON
            upperNum = (int)kiloOhms;                       // Assign upper number to ones place of resistance reading
            lowerNum = (int)((kiloOhms-upperNum)*10);       // Assign lower number to tenths place of resistance reading
        }
        else if(kiloOhms < 100)
        {
            decON = 1;                                      // Set the decimal on 7 seg to OFF
            upperNum = (int)kiloOhms / 10;                  // Assign upper number to tens place of resistance reading
            lowerNum = (int)kiloOhms % 10;                  // Assign lower number to ones place of resistance reading
        }
        else
        {
            decON = 1;                                      // Set the decimal on 7 seg to OFF
            upperNum = 10;                                  // Assign upper number to out of range index
            lowerNum = 10;                                  // Assign lower number to out of range index
        }
        int LED1 = (int)kiloOhms / 10;
        if(LED1 > 7)                                        // Display white on LED for anything greater than 70k
            LED1 = 7;
        PORTB = LED1;                                       // Set LED1 according to the resistance value
        PORTC = arrayDigits[upperNum];                      // Set upperNum to the upper 7 seg display
        PORTD = arrayDigits[lowerNum];                      // Set lowerNum to the lower 7 seg display
        if(decON == 1)                                      // Set DECIMAL after setting PORTC
            DECIMAL = 1;
        else 
            DECIMAL = 0;
    }
}

void putch(char c)                                          // Function is called by TeraTerm
{
    while(!TRMT);
    TXREG = c;
}

void init_UART()                                            // Used to communicate with microcontroller and display on TeraTerm.
{
    OpenUSART(USART_TX_INT_OFF&USART_RX_INT_OFF&
    USART_ASYNCH_MODE&USART_EIGHT_BIT&USART_CONT_RX&
    USART_BRGH_HIGH,25);
    OSCCON = 0x60;                                          // Set oscillator control
}


void init_ADC()
{
    ADCON0 = 0x11;                                          // Select channel AN1, 
    ADCON1 = 0x1B;                                          // Select pins AN0 through AN3 as analog signal, VDD-VSS as
                                                            // reference voltage
    ADCON2 = 0xA9;                                          // Right justify the result. Set the bit conversion time (TAD)
                                                            // and acquisition time
}

unsigned int Get_Full_ADC()                                 // Analog Digital Converter
{
int result;  
    ADCON0bits.GO = 1;                                      // Start conversion                            
    while(ADCON0bits.DONE == 1);                            // Wait for conversion to be completed
    result = (ADRESH * 0x100) + ADRESL;                     // Combine result of upper byte and lower byte into result
    return result;                                          // Return the result
}


void Activate_Buzzer()
{
    PR2 = 0b11111001 ;
    T2CON = 0b00000101 ;
    CCPR2L = 0b01001010 ;
    CCP2CON = 0b00111100 ;
}

void Deactivate_Buzzer()
{
    CCP2CON = 0x0;
    PORTBbits.RB3 = 0;
}
