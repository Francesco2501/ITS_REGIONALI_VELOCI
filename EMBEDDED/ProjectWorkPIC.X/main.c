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
//PULSANTE PORTE CHE ATTIVA MOTORE IN DUE DIRAZIONI OPPOSTE (RB3-RB4)
//TRASMISSIONE/RICEZIONE E GESTIONE PACCHETTI DATI
//LETTURA INTERRUTTORE PORTA BAGNO
//LETTURA VALORI DTH 22
//ATTIVA VENTOLA/RESISTENZA PER REGOLARE TEMPERATURA

#define _XTAL_FREQ 16000000

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

//flags
#define NEW_READING_AVAILABLE 0x01
#define KEY_PRESSED 0x02

unsigned char flags = 0x00;
//|newReadingAvailable|keyPressed|

unsigned char colScan = 0;
unsigned char rowScan = 0;
int counterAntibouncing = 0;
unsigned char keyValue = 99;
unsigned char keypadReading;
char currentDisplay = 0;
char oldDisplay = 0;
float setTemp = 22.0;
char minSetTemp = 12;
char maxSetTemp = 35;
float temperature = 0.0;
float oldTemperature = 0.0;
float humidity = 50.0;
float oldHumidity = 50.0;
int postScalerTemp = 0;
int postScalerHum = 0;
char alarmOn = 0;
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
    "Temp    : 22.3  Humidity: 80.3 %",
    "Set Temperature:"
};

void CheckAlarmButton(void);
void CheckOpenDoorButton(void);
unsigned char CheckKeypad(void);
void DriveMotor(char);
void HandleKeypadReading(unsigned char);
void InitLCD(void);
void InitPic(void);
void ManageDisplays(void);
void MotorRotation(char, char);
void ReadHumidity(void); //SIMULATED
void ReadTemperature(void); //SIMULATED
void SendLCD(char, char);
void SendLCDString(const char*);
void UpdateLCDView(char, const char*);
void UpdateLCDViewWithNumber(char, float);
void UpdateTempHumDisplay(void);
void Toggle(unsigned char*, char);

int main()
{
    InitPic();
    InitLCD();
    SendLCDString(startDisplay);
    while(1)
    {
        keypadReading = CheckKeypad();
        ReadHumidity();
        ReadTemperature();
        CheckAlarmButton();
        CheckOpenDoorButton();
        if(currentDisplay == TEMP_HUM_DISPLAY) UpdateTempHumDisplay();
        if(flags & NEW_READING_AVAILABLE)
        {
            ManageDisplays(); 
            //Toggle(&flags, NEW_READING_AVAILABLE);
            flags &= ~NEW_READING_AVAILABLE;
        } 
    }
}

void __interrupt() ISR()
{
    if(INTCON & 0x04)
    {  
        if(flags & KEY_PRESSED)
            counterAntibouncing++;
        
        INTCON &= ~0x04;
        TMR0 = 6;
        postScalerTemp++;
        postScalerHum++;
    }
}

//ALARM ON RB2
void CheckAlarmButton()
{
    TRISB |= 0x04;
    TRISC = 0x00;

    if(!(PORTB & 0x04) && oldRB2)
    {
        oldRB2 = 0;
        if(alarmOn)
        {
            alarmOn = 0;
        }
        else alarmOn = 1;
    }
    if((PORTB & 0x04) && !oldRB2)
    {
        __delay_ms(10);
        if((PORTB & 0x04) && !oldRB2) oldRB2 = 1;
    }
    if(alarmOn) PORTC |= 0x02;
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
                if(counterAntibouncing > 20)
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
        setTemp += 0.1;

    if(value == DOWN_BUTTON && currentDisplay == SET_TEMP_DISPLAY)
        setTemp -= 0.1;

    if(currentDisplay >= TOTAL_DISPLAYS)
        currentDisplay = 1;
    
    if(currentDisplay == 0)
        currentDisplay = TOTAL_DISPLAYS-1;
    
    if(setTemp > maxSetTemp)
        setTemp = maxSetTemp;
    
    if(setTemp < minSetTemp)
        setTemp = minSetTemp;
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
    TRISC = 0x00;
    TRISE = 0x00;
    INTCON = 0xA0;
    OPTION_REG = 0x06;
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

void MotorRotation(char direction, char amplitude)
{
    TRISD &= 0b00001111;    
    for(char i = 0; i<amplitude; i++)
    {
        DriveMotor(direction);
    }
}

void ReadHumidity()
{
    if(postScalerHum > 100)
    {
        if(humidity >= 100) humidity = 0;
        else humidity += 0.1;
        postScalerHum = 0;
    }
    
}

void ReadTemperature()
{
    if(postScalerTemp > 70)
    {
        if(temperature >= 90)  temperature = -90;
        else temperature += 0.1;
        postScalerTemp = 0;
    }
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

void UpdateLCDView(char position, const char *string)
{
    SendLCD(position, 0x00); //move cursor to position
    SendLCDString(string);
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
    if (oldHumidity != humidity)
    {
        UpdateLCDViewWithNumber(0xCA, humidity);
        oldHumidity = humidity;
    }
    if (oldTemperature != temperature)
    {
        UpdateLCDViewWithNumber(0x8A, temperature);
        oldTemperature = temperature;
    }
}




