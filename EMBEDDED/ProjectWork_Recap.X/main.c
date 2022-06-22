#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>

#define _XTAL_FREQ 20000000

#define SBIT_TXEN     5
#define SBIT_SPEN     7
#define SBIT_CREN     4
#define ADDRESS       1
#define TEMP_HUM_TYPE 0x01
#define END_OF_PACKET 0xFF

#define BOOL_PACKET_SIZE 4 //ADDRESS|TYPE|BOOL|0xFF
#define TEMP_HUM_PACKET_SIZE 7 //|ADDRESS|TYPE|INT_TEMP|DEC_TEMP|INT_HUM|DEC_HUM|0xFF
#define REQUEST_PACKET_SIZE 3 //|ADDRESS|TYPE|0xFF

//FLAGS BYTE DEFINITION
char flags;
#define BIT_RECEIVED 0
#define TX_SUCCESSFUL 1
#define BIT_TRANSMITTED 2

void Init(void);
void UART_Init(unsigned long baudRate);
void UART_TxChar(unsigned char ch);
void UART_TxTempHumDataPacket(unsigned char a[TEMP_HUM_PACKET_SIZE]);
void UART_TxBoolDataPacket(unsigned char a[BOOL_PACKET_SIZE]);
void PrepareBoolDataPacket(char type, char state);
void PrepareTempHumDataPacket(float temp, float hum);
void CleanTxDataPacket(void);
void HandleRequest(void);
//TX temp/Hum packet
//TX toilet door packet
//TX alarm packet
//TX door packet
//RX door packet
//RX TEMP/HUM req packet (call to TX Temp/Hum packet)
//RX alarm packet

unsigned char receivedData;
unsigned char partCounter = 0;
unsigned char i;
unsigned char boolDataPacketTx[BOOL_PACKET_SIZE]; //packet to transmit
unsigned char tempHumDataPacketTx[TEMP_HUM_PACKET_SIZE]; //packet to transmit
unsigned char dataPacketRx[REQUEST_PACKET_SIZE]; //packet to receive
unsigned int postscaler = 0;

int main()
{
    Init();
    UART_Init(115200); //Initialize the UART module with 9600 baud rate
    TRISD=0x00;
    PORTD=0x00;
    
    PrepareBoolDataPacket(0x08, 0x01);
    UART_TxBoolDataPacket(boolDataPacketTx);
    PrepareBoolDataPacket(0x10, 0x00);
    UART_TxBoolDataPacket(boolDataPacketTx);
    PrepareTempHumDataPacket(20.5, 99.9);
    UART_TxTempHumDataPacket(tempHumDataPacketTx);
  
    while(1)
    {
        if(flags &= 1<<BIT_RECEIVED)
        {
            flags &= ~(1<<BIT_RECEIVED);
            dataPacketRx[partCounter] = receivedData;
            partCounter ++;
            //if counter !=0 and time passed > max wait --> reset counter and flush packet buffer
            if(receivedData == 0xFF || partCounter > REQUEST_PACKET_SIZE)
            {
                partCounter = 0;
                HandleRequest();
            }          
        }
        if(postscaler>500)
        {
            postscaler = 0;
        }
    }
}

void __interrupt() ISR()
{
    if(RCIF){
        receivedData = RCREG;
        flags |= 1<<BIT_RECEIVED;
        RCIF = 0;     
    } 
    if(TXIF){
    }
    if(INTCON & 0x04) //qualunque numero diverso da 0 è vero
    {
        postscaler++;
        INTCON &= ~0x04; 
        TMR0 = 178; //timer parte da 0
    }
}

void Init(void)
{
    INTCON |= 0xA0;
    OPTION_REG = 0x05;
}

void UART_Init(unsigned long baudRate)
{    
    TRISC=0x80;            // Configure Rx pin as input and Tx as output  
    TXSTA=(1<<SBIT_TXEN);  // Asynchronous mode, 8-bit data & enable transmitter
    RCSTA=(1<<SBIT_SPEN) | (1<<SBIT_CREN);  // Enable Serial Port and 8-bit continuous receive
    SPBRG = (char)(_XTAL_FREQ/(long)(64UL*baudRate))-1;      // baud rate @20Mhz Clock
    
    INTCON |= 0xC0; //ENABLE GIE AND PEIE
    PIE1 |= 0x22; //ENABLE RCIE AND TXIE
    
    flags = 0;
}

void UART_TxChar(unsigned char ch)
{
    while(!TXIF);
    TXREG=ch;
    //CHECK COLLISIONS
    //NOT WORKING ??
    //flags |= 1<<TX_SUCCESSFUL;
    /*if(ch != data)
    {
        //flags &= ~(1<<TX_SUCCESSFUL);
        PORTD = 0xFF;
    }*/
    
}

void UART_TxTempHumDataPacket(unsigned char dataPacket[TEMP_HUM_PACKET_SIZE])
{
    for(i=0;i<TEMP_HUM_PACKET_SIZE;i++)
    {
        UART_TxChar(dataPacket[i]); 
    }
}

void UART_TxBoolDataPacket(unsigned char dataPacket[BOOL_PACKET_SIZE])
{
    for(i=0;i<BOOL_PACKET_SIZE;i++)
    {
        UART_TxChar(dataPacket[i]); 
    }
}

//ConvertTempToChar()
//ConvertHumToChar()

void PrepareTempHumDataPacket(float temp, float hum)
{
    tempHumDataPacketTx[0] = ADDRESS;
    tempHumDataPacketTx[1] = TEMP_HUM_TYPE;
    tempHumDataPacketTx[2] = (char)temp;
    tempHumDataPacketTx[3] = (char)((temp-(char)temp)/0.1);
    tempHumDataPacketTx[4] = (char)hum;
    tempHumDataPacketTx[5] = (char)((hum-(char)hum)/0.1);
    tempHumDataPacketTx[6] = 0xFF;
}

void PrepareBoolDataPacket(char type, char state)
{
    boolDataPacketTx[0] = ADDRESS;
    boolDataPacketTx[1] = type;
    boolDataPacketTx[2] = state;
    boolDataPacketTx[3] = 0xFF;
}

void HandleRequest()
{
    if(dataPacketRx[0] != ADDRESS)
    {
        return;
    }
    else
    {
        if(dataPacketRx[1] == TEMP_HUM_TYPE)
        {
            //TODO: substitute with send reading
            PrepareTempHumDataPacket(20.5, 99.9);
            UART_TxTempHumDataPacket(tempHumDataPacketTx);
        }
        //alarm activation request
    }
}
