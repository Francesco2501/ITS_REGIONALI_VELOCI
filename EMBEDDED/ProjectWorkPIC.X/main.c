#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>

//TODO
//LETTURA INTERRUTTORE PORTA BAGNO

#define _XTAL_FREQ 8000000

#define L_ON  0x0F
#define L_OFF 0x08
#define L_CLR 0x01
#define L_L1  0x80
#define L_L2  0xC0
#define L_CR  0x0F
#define L_NCR 0x0C
#define L_CFG 0x38
#define L_CUR 0x0E
#define AUX 0x04;

#define LEFT_BUTTON 4
#define RIGHT_BUTTON 6
#define UP_BUTTON 2
#define DOWN_BUTTON 8

#define TOTAL_DISPLAYS 4
#define SET_TEMP_DISPLAY 3
#define TEMP_HUM_DISPLAY 2

#define CLOCKWISE_ROTATION 0x00
#define COUNTER_CLOCKWISE_ROTATION 0x01

#define SBIT_TXEN     5
#define SBIT_SPEN     7
#define SBIT_CREN     4
#define ADDRESS       1
#define BROADCAST 0x23//broadcast is # --> 0x23 in UTF 8
#define TEMP_HUM_TYPE 0x01
#define ALARM_TYPE 0x20
#define DESIRED_TEMPERATURE_TYPE 0x04
#define DOOR_TYPE 0x10
#define TOILET_TYPE 0x08
#define ADDRESS_TYPE 0x40
#define START_OF_PACKET 0xFF
#define END_OF_PACKET 0xFE


#define BOOL_PACKET_SIZE 5 //0xFF|ADDRESS|TYPE|BOOL|0xFE
#define TEMP_HUM_PACKET_SIZE 8 //0xFF|ADDRESS|TYPE|INT_TEMP|DEC_TEMP|INT_HUM|DEC_HUM|0xFE
#define DESIRED_TEMP_PACKET_SIZE 6 //0xFF|ADDRESS|TYPE|INT_TEMP|DEC_TEMP|0xFE
#define REQUEST_PACKET_SIZE 5 //0xFF|ADDRESS|TYPE|VALUE|0xFD

//FLAGS FOR SERIAL
char flagsSerial;
#define BIT_RECEIVED 0x01
#define TX_ADDRESS_WAITER 0x08
#define TRANSMISSION_RUNNING 0x02
#define START_RETRANSMISSION_COUNTER 0x04

//FLAGS 
unsigned char flags = 0x00;
//|newReadingAvailable|keyPressed|alarmOn|
#define NEW_READING_AVAILABLE 0x01
#define KEY_PRESSED 0x02
#define ALARM_ON 0x04
#define CHANNEL 0x02

unsigned char colScan = 0;
unsigned char rowScan = 0;
unsigned char dutyHeater = 0;
unsigned char dutyCooler = 0;
int counterAntibouncing = 0;
int counterAntibouncingAlarm = 0;
int counterAntibouncingDoor = 0;
int ADCReading = 0;
unsigned char keyValue = 99;
unsigned char keypadReading;
unsigned char dutyCounter = 0;
char currentDisplay = 0;
char oldDisplay = 0;
float setTemp = 22.0;
char minSetTemp = 12;
char maxSetTemp = 35;
float temperature = 0.0;
float temperatureBuffer = 0.5;
float humidity = 50.0;
int postScalerTemp = 0;
int postScalerHum = 0;
char oldRB2;
char oldRB3;
char doorOpen = 0;
char oldDoorOpen = 0;

const unsigned char colMask[3] = 
{
    0b11111110,
    0b11111101,
    0b11111011
};
const unsigned char rowMask[4] = 
{
    0b00001000,
    0b00000100,
    0b00000010,
    0b00000001
};
const char clockWise[5] = {0x30, 0x60, 0xC0, 0x90, 0x30};
const char counterClockWise[5] = {0x90, 0xC0, 0x60, 0x30, 0x90};
const unsigned char keypad[] = {1,4,7,'*',2,5,8,0,3,6,9,'#'}; 
const char *startDisplay = {"System Started"};
const char *displays[] = 
{
    "",
    "  Welcome To        Coach 1", 
    "Temp    :       Humidity:      %",
    "Set Temperature:"
};

unsigned char alarmOn = 0;
unsigned char txSuccess = 1;
unsigned char receivedData;
unsigned char txReceivedData;
unsigned char txAttempts = 0;
unsigned char transmittingData;
unsigned char partCounter = 0;
unsigned char i;
unsigned char boolDataPacketTx[BOOL_PACKET_SIZE]; 
unsigned char tempHumDataPacketTx[TEMP_HUM_PACKET_SIZE]; 
unsigned char desiredTempPacketTx[DESIRED_TEMP_PACKET_SIZE]; 
unsigned char dataPacketRx[REQUEST_PACKET_SIZE]; 
unsigned int transmissionCounter = 0;
unsigned int retransmissionCounter = 0;
unsigned int addressTransmissionCounter = 0;

void CheckAlarmButton(void);
void CheckOpenDoorButton(void);
unsigned char CheckKeypad(void);
void DriveMotor(char);
void HandleKeypadReading(unsigned char);
void InitADC(void);
void InitLCD(void);
void InitPic(void);
void ManageDisplays(void);
void ManageCooler(char duty);
void ManageHeater(char duty);
void ManageTemperature(void);
void MotorRotation(char, char);
int ReadADC(char channel);
void ReadHumidity(void); //SIMULATED
void ReadTemperature(void); //SIMULATED
void SendLCD(char, char);
void SendLCDString(const char*);
void UpdateLCDViewWithNumber(char, float);
void UpdateTempHumDisplay(void);

void UART_Init(unsigned long baudRate);
void UART_TxChar(unsigned char ch);
void UART_TxTempHumDataPacket(unsigned char a[TEMP_HUM_PACKET_SIZE]);
void UART_TxDesiredTempDataPacket(unsigned char dataPacket[DESIRED_TEMP_PACKET_SIZE]);
void UART_TxBoolDataPacket(unsigned char a[BOOL_PACKET_SIZE]);
void PrepareBoolDataPacket(char type, char state);
void PrepareTempHumDataPacket(float temp, float hum);
void PrepareDesiredTempDataPacket(float temp);
void CleanTxDataPacket(void);
void HandleRequest(void);
void ManageReceivedPacket(void);

int main()
{
    InitPic();
    InitLCD();
    InitADC();
    UART_Init(9600);
    SendLCDString(startDisplay);

    while(1)
    {
        keypadReading = CheckKeypad();
        ReadHumidity();
        ReadTemperature();
        CheckAlarmButton();
        CheckOpenDoorButton();
        ManageTemperature();
        ManageHeater(dutyHeater);
        ManageCooler(dutyCooler);
        ManageReceivedPacket();
        if(currentDisplay == TEMP_HUM_DISPLAY) UpdateTempHumDisplay();
        if(flags & NEW_READING_AVAILABLE)
        {
            ManageDisplays(); 
            flags &= ~NEW_READING_AVAILABLE;
        } 
    }
}

void __interrupt() ISR()
{
    if(RCIF && !(flagsSerial & TRANSMISSION_RUNNING))
    {
        receivedData = RCREG;
        flagsSerial |= 1<<BIT_RECEIVED;   
    } 
    if(TXIF && transmissionCounter>5) //TRANSMISSION STARTED AND WAIT FOR RECEIVING TANSMITTED
    {
        txReceivedData=RCREG;
        if(txReceivedData != transmittingData) txSuccess=0;
    }
    if(INTCON & 0x04)
    {  
        if(flagsSerial & TRANSMISSION_RUNNING) transmissionCounter++;
        if(flagsSerial & START_RETRANSMISSION_COUNTER) retransmissionCounter++;
        if(flagsSerial & TX_ADDRESS_WAITER) addressTransmissionCounter++;
        
        if(flags & KEY_PRESSED)
            counterAntibouncing++;
        
        if(dutyCounter >= 255) dutyCounter = 0;
        else dutyCounter ++;
        
        INTCON &= ~0x04;
        TMR0 = 178;
        postScalerTemp++;
        postScalerHum++;
    }
}

//ALARM ON RB2
void CheckAlarmButton()
{
    TRISB |= 0x04;

    if(!(PORTB & 0x04) && oldRB2)
    {
        oldRB2 = 0;
        if(flags & ALARM_ON)
        {
            flags &= ~ALARM_ON;
        }
        else flags |= ALARM_ON;
    }
    if((PORTB & 0x04) && !oldRB2)
    {
        __delay_ms(10);
        if((PORTB & 0x04) && !oldRB2) oldRB2 = 1;
        
    }
    if(flags & ALARM_ON) PORTC |= 0x02;
    else PORTC &= ~0x02;
}

unsigned char CheckKeypad()
{
    TRISB &= ~0x07;
    TRISD |= 0x0F;
    for(colScan = 0; colScan < 3; colScan++)
    {
        PORTB |= 0x07;
        PORTB &= colMask[colScan];
        __delay_us(15); //GIVE TIME FOR PORT STATE CHANGE
        for( rowScan = 0; rowScan < 4; rowScan++)
        {
            if(!(PORTD & rowMask[rowScan]))
            {
                flags |= KEY_PRESSED;
                if(counterAntibouncing > 10)
                {
                    keyValue = rowScan+(4*colScan);
                    flags &= ~KEY_PRESSED;
                    counterAntibouncing = 0;
                    flags |= NEW_READING_AVAILABLE;
                }
            }
        }
    }
    return keypad[keyValue];
}

//OPEN DOOR ON RB3
void CheckOpenDoorButton()
{
    TRISB |= 0x08;
    if(!(PORTB & 0x08) && oldRB3)
    {
        oldRB3 = 0;
        if(doorOpen)
        {
            doorOpen = 0;
        }
        else doorOpen = 1;
    }
    if((PORTB & 0x08) && !oldRB3)
    {
        __delay_ms(10);
        if((PORTB & 0x08) && !oldRB3) oldRB3 = 1;
    }
    if(!oldDoorOpen && doorOpen)
    {
        MotorRotation(CLOCKWISE_ROTATION, 25);
        oldDoorOpen = 1;
    }
    if(oldDoorOpen && !doorOpen)
    {
        MotorRotation(COUNTER_CLOCKWISE_ROTATION, 25);
        oldDoorOpen = 0;
    }
}

void DriveMotor(char direction)
{
    if (direction == CLOCKWISE_ROTATION)
    {
        for (char i = 0; i<5; i++)
        {
            PORTD = clockWise[i];
            __delay_ms(10);
        }
    }
    if (direction == COUNTER_CLOCKWISE_ROTATION)
    {
        for (char i = 0; i<5; i++)
        {
            PORTD = counterClockWise[i];
            __delay_ms(10);
        }
    }
}

void HandleKeypadReading(unsigned char value)
{
    if(value == RIGHT_BUTTON)
        currentDisplay ++;
    
    if(value == LEFT_BUTTON)
        currentDisplay --;
    
    if(value == UP_BUTTON && currentDisplay == SET_TEMP_DISPLAY)
    {
        setTemp += 0.1;
        PrepareDesiredTempDataPacket(setTemp);
        UART_TxDesiredTempDataPacket(desiredTempPacketTx);
    }
        

    if(value == DOWN_BUTTON && currentDisplay == SET_TEMP_DISPLAY) 
    {
        setTemp -= 0.1;
        PrepareDesiredTempDataPacket(setTemp);
        UART_TxDesiredTempDataPacket(desiredTempPacketTx);
    }

    if(currentDisplay >= TOTAL_DISPLAYS)
        currentDisplay = 1;
    
    if(currentDisplay == 0)
        currentDisplay = TOTAL_DISPLAYS-1;
    
    if(setTemp > maxSetTemp)
        setTemp = maxSetTemp;
    
    if(setTemp < minSetTemp)
        setTemp = minSetTemp;
}

void InitADC()
{
    TRISA |= 0x04;
    ADCON0 = 0x01;
    ADCON1 = 0x80;
    __delay_us(20);
}

void InitLCD()
{
    PORTE = PORTE & 0x02;
    PORTE = PORTE & 0x04;
    __delay_ms(20);
    PORTE = PORTE | 0x02;
    SendLCD(L_CFG, 0x00);
    __delay_ms(5);
    SendLCD(L_CFG, 0x00);
    __delay_ms(1);
    SendLCD(L_CFG, 0x00);
    SendLCD(L_ON, 0x00);
    SendLCD(L_CLR, 0x00);
    SendLCD(L_CUR, 0x00);
    SendLCD(L_L1, 0x00);
    SendLCD(L_NCR, 0x00); //CURSOR OFF
}

void InitPic()
{
    TRISE = 0x00;
    TRISD = 0x00;
    PORTD = 0x00;
    INTCON |= 0xA0;
    OPTION_REG = 0x02; //0.0312 MS
    TMR0 = 6;
}

void ManageDisplays()
{
    HandleKeypadReading(keypadReading);
    if(currentDisplay != oldDisplay)
    {
        SendLCD(L_CLR,0x00);
        SendLCDString(displays[currentDisplay]);
        oldDisplay = currentDisplay;
    }
    if(currentDisplay == 3)
            UpdateLCDViewWithNumber(0xC6,setTemp);
}

void ManageCooler(char duty)
{
    if(dutyCounter<duty)
    {
        PORTC |= 1<<2;
    }
    else
    {
        PORTC &= ~(1<<2);
    }
}

void ManageHeater(char duty)
{
    if(dutyCounter<duty)
    {
        PORTC |= 1<<5;
    }
    else
    {
        PORTC &= ~(1<<5);
    }
}

void ManageTemperature(void)
{
    if(temperature < (setTemp-temperatureBuffer))
        {
            dutyHeater = 200;
            dutyCooler = 0;
        }
        else if (temperature > (setTemp+temperatureBuffer))
        {
            dutyCooler = 200;
            dutyHeater = 0;
        }
        else
        {
            dutyHeater = 0;
            dutyCooler = 0; 
        }
}

void MotorRotation(char direction, char amplitude)
{
    TRISD &= 0b00001111;    
    for(char i = 0; i<amplitude; i++)
    {
        DriveMotor(direction);
    }
}

int ReadADC(char channel)
{
    ADCON0 &= 0xC7; 
    ADCON0 |= ((channel & 0x07)<<3); 
    __delay_us(10); 
    ADCON0 |= 0x04;
    while(ADCON0 & 0x04);
    return ADRESL | (ADRESH << 8);
}

//SIMULATED HUMIDITY
void ReadHumidity()
{
    if(postScalerHum > 500)
    {
        if(humidity >= 100) humidity = 0;
        else humidity += 0.1;
        postScalerHum = 0;
    }
}

//TEMP FROM LM35
void ReadTemperature()
{
    temperature = (float)(ReadADC(CHANNEL) * 0.489);
}

//TYPE --> 0x00 COMMAND, 0x01 CHARACTER
void SendLCD(char data,char type)
{
    TRISD = 0x00;
    TRISE = 0x00;
    PORTE |= 0x02;
    PORTD = data;
    if(type) PORTE |= 0x04;
    else PORTE &= ~0x04;
    __delay_ms(3);
    PORTE &= ~0x02;
    __delay_ms(3);
    PORTE |= 0x02; 
}

void SendLCDString(const char *string)
{
    char i = 0;
    while(string[i] != '\0')
    {
        if (i<16)
        {
            SendLCD(string[i], 0x01);
            i++;
        }
        else if(i==16)
        {
            SendLCD(L_L2, 0x00);
            SendLCD(string[i], 0x01);
            i++;
        }
        else
        {
            SendLCD(string[i], 0x01);
            i++;
        }
    }
}

//WRITES AT position FLOAT WITH MAX ONE DECIMAL AND IN RANGE -999.9 TO 999.9
void UpdateLCDViewWithNumber(char position, float number)
{
    SendLCD(position, 0x00);
    if(number < 0)
    {
        SendLCD('-', 0x01);
        number *= -1;
    }
    char hundreds = (char)(number/100);
    if(hundreds != 0) SendLCD(hundreds + '0', 0x01);
    number -= hundreds*100;
    char tens = (char)(number/10);
    if(tens != 0) SendLCD(tens + '0', 0x01);
    number -= tens * 10;
    char unit = (char)(number/1);
    SendLCD(unit + '0', 0x01);
    number -= unit;
    char decimal = (char)(number/0.1);
    SendLCD('.', 0x01);
    SendLCD(decimal + '0', 0x01);
}

void UpdateTempHumDisplay()
{
        UpdateLCDViewWithNumber(0xCA, humidity);
        UpdateLCDViewWithNumber(0x8A, temperature);
}

void UART_Init(unsigned long baudRate)
{    
    TRISC=0x80;            // CONFIGURE RX PIN AS INPUT AND TX AS OUTPUT  
    TXSTA=(1<<SBIT_TXEN);  // ASYNC MODE, 8-BIT, ENABLE TX
    RCSTA=(1<<SBIT_SPEN) | (1<<SBIT_CREN);  // ENABLE SERIAL PORT AND CONTINUOUS RECEIVE
    SPBRG = (char)(_XTAL_FREQ/(long)(16UL*baudRate))-1;
    
    INTCON |= 0xC0; //ENABLE GIE AND PEIE
    PIE1 |= 0x22; //ENABLE RCIE AND TXIE
    
    flagsSerial = 0;
}

void UART_TxChar(unsigned char ch)
{
    transmittingData = ch;
    transmissionCounter = 0;
    PORTE=0x01;
    while(!TXIF);
    flagsSerial |= TRANSMISSION_RUNNING;
    TXREG=ch;
    //WAIT BEFORE START LISTENING (RE0 LOW)
    while(transmissionCounter<7);
    flagsSerial &= ~TRANSMISSION_RUNNING;
    transmissionCounter = 0;
    PORTE = 0x00;
}

//TODO: APPLY RETRANSMISSION TO ALL TX FUNXTIONS --> MERGE IN ONE FUNCTION?
void UART_TxTempHumDataPacket(unsigned char dataPacket[TEMP_HUM_PACKET_SIZE])
{
    txAttempts = 0;
    retransmissionCounter = 0;
    //IF AT LEAST ONE BYTE IS CORRUPTED RETRANSMIT ALL PACKET
    //MAX 9 ATTEMPTS OF RETRASMIT
    //while(txAttempts<9)
    while(txAttempts<1)//IN SIMULATED NO CHECK COLLISION
    {
        flagsSerial |= START_RETRANSMISSION_COUNTER;
        //WAIT FOR RETRASMIT A TIME = NUMBER_OF_ATTEMPTS * COACH_ADDRESS * ~10 MS
        while (retransmissionCounter < (txAttempts*ADDRESS*300));
        retransmissionCounter = 0;
        flagsSerial &= ~START_RETRANSMISSION_COUNTER;
        txSuccess = 1;
        for(i=0;i<TEMP_HUM_PACKET_SIZE;i++)
        {
            UART_TxChar(dataPacket[i]); 
        }
        if(txSuccess) break;
        txAttempts++;
    }
}

void UART_TxDesiredTempDataPacket(unsigned char dataPacket[DESIRED_TEMP_PACKET_SIZE])
{
    txAttempts = 0;
    retransmissionCounter = 0;
    //IF AT LEAST ONE BYTE IS CORRUPTED RETRANSMIT ALL PACKET
    //MAX 9 ATTEMPTS OF RETRASMIT
    //while(txAttempts<9)
    while(txAttempts<1)//IN SIMULATED NO CHECK COLLISION
    {
        flagsSerial |= START_RETRANSMISSION_COUNTER;
        //WAIT FOR RETRASMIT A TIME = NUMBER_OF_ATTEMPTS * COACH_ADDRESS * ~10 MS
        while (retransmissionCounter < (txAttempts*ADDRESS*300));
        retransmissionCounter = 0;
        flagsSerial &= ~START_RETRANSMISSION_COUNTER;
        txSuccess = 1;
        for(i=0;i<DESIRED_TEMP_PACKET_SIZE;i++)
        {
            UART_TxChar(dataPacket[i]); 
        }
        if(txSuccess) break;
        txAttempts++;
    } 
}

void UART_TxBoolDataPacket(unsigned char dataPacket[BOOL_PACKET_SIZE])
{
    txAttempts = 0;
    retransmissionCounter = 0;
    //IF AT LEAST ONE BYTE IS CORRUPTED RETRANSMIT ALL PACKET
    //MAX 9 ATTEMPTS OF RETRASMIT
    //while(txAttempts<9)
    while(txAttempts<1)//IN SIMULATED NO CHECK COLLISION
    {
        flagsSerial |= START_RETRANSMISSION_COUNTER;
        //WAIT FOR RETRASMIT A TIME = NUMBER_OF_ATTEMPTS * COACH_ADDRESS * ~10 MS
        while (retransmissionCounter < (txAttempts*ADDRESS*300));
        retransmissionCounter = 0;
        flagsSerial &= ~START_RETRANSMISSION_COUNTER;
        txSuccess = 1;
        for(i=0;i<BOOL_PACKET_SIZE;i++)
        {
            UART_TxChar(dataPacket[i]); 
        }
        if(txSuccess) break;
        txAttempts++;
    }
}

void PrepareTempHumDataPacket(float temp, float hum)
{
    tempHumDataPacketTx[0] = START_OF_PACKET;
    tempHumDataPacketTx[1] = ADDRESS;
    tempHumDataPacketTx[2] = TEMP_HUM_TYPE;
    if(temp<0)
    { 
        tempHumDataPacketTx[3] = 0x80 + (char)(temp*(-1));
        float decimal = (float)(((temp * (-1)) + (int)temp)/0.1);
        tempHumDataPacketTx[4] = (char)decimal;
     }
    else 
    {
        tempHumDataPacketTx[3] = (char)(temp);;
        tempHumDataPacketTx[4] = (char)((temp-(char)temp)/0.1);
    }
    tempHumDataPacketTx[5] = (char)hum;
    tempHumDataPacketTx[6] = (char)((hum-(char)hum)/0.1);
    tempHumDataPacketTx[7] = END_OF_PACKET;
}

void PrepareBoolDataPacket(char type, char state)
{
    boolDataPacketTx[0] = START_OF_PACKET;
    boolDataPacketTx[1] = ADDRESS;
    boolDataPacketTx[2] = type;
    boolDataPacketTx[3] = state;
    boolDataPacketTx[4] = END_OF_PACKET;
}

void PrepareDesiredTempDataPacket(float temp)
{
    desiredTempPacketTx[0] = START_OF_PACKET;
    desiredTempPacketTx[1] = ADDRESS;
    desiredTempPacketTx[2] = DESIRED_TEMPERATURE_TYPE;
    desiredTempPacketTx[3] = (char)temp;
    desiredTempPacketTx[4] = (char)((temp-(char)temp)/0.1);
    desiredTempPacketTx[5] = END_OF_PACKET;
}

void HandleRequest()
{
    if(dataPacketRx[1] == BROADCAST)
    {
        if(dataPacketRx[2] == ADDRESS_TYPE)
        {
            flagsSerial |= TX_ADDRESS_WAITER;
            while(addressTransmissionCounter < (ADDRESS * 10));
            addressTransmissionCounter = 0;
            flagsSerial &= ~TX_ADDRESS_WAITER;
            PrepareBoolDataPacket(0x40,0x01);
            UART_TxBoolDataPacket(boolDataPacketTx);
        }
    }
    if(dataPacketRx[1] != ADDRESS)
    {
        return;
    }
    else
    {
        if(dataPacketRx[2] == TEMP_HUM_TYPE)
        {
            //TODO: substitute with send reading
            PrepareTempHumDataPacket(temperature, humidity);
            UART_TxTempHumDataPacket(tempHumDataPacketTx);
        }
        if(dataPacketRx[2] == ALARM_TYPE)
        {
            //ACTIVATE ALARM
            alarmOn = dataPacketRx[3];
        }
        if(dataPacketRx[2] == DESIRED_TEMPERATURE_TYPE)
        {
            //desired_temp = dataPacketRX[2] + data?acketRX[3]*0.1;
        }
        if(dataPacketRx[2] == DOOR_TYPE)
        {
            //desired_temp = dataPacketRX[2] + data?acketRX[3]*0.1;
        }
    }
}

void ManageReceivedPacket(void)
{
    if(alarmOn)
    {
        TRISD = 0x00;
        PORTD = 0XFF;
    }
    if(!alarmOn)
    {
       TRISD = 0x00;
       PORTD = 0X00; 
    }
    if(flagsSerial & 1<<BIT_RECEIVED)
    {
        flagsSerial &= ~(1<<BIT_RECEIVED);

        if(receivedData == START_OF_PACKET)
        {
            partCounter = 0;
        } 
        if(receivedData == END_OF_PACKET)
        {
            HandleRequest();
        }
        if(!(partCounter >=  REQUEST_PACKET_SIZE))
        {
            dataPacketRx[partCounter] = receivedData;
            partCounter ++;
        }
    }
}



