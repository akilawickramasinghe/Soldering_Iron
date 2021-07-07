/*
 * solderingIronv1.cpp
 *
 * Created: 2021-03-23 ප.ව. 2.43.24
 * Author : Akila
 */ 
#ifndef F_CPU
#define F_CPU 16000000UL // Running Speed of the MicroController 
#endif 

#include <avr/io.h>
#include <util/delay.h>//delay library
#include <avr/interrupt.h>//interrupt
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lib/encoder/encoder.h" //Encoder Library

//Functions//
void ironBOOT();
void ironRUN();
int16_t sensorValue;
void analogRead(void);


//////////////Encoder//////////////////////////
int32_t enc;
void encoderValue(int32_t eVal);




//PID//
double kp = 1.0;
double ki = 0.1;
int16_t lastError;
int16_t pWM;
int16_t error;
///////////////


//PWM//
void tempPID();
double dutyCycle = 0;
void setupPWM();
/////////////////


//OLED//
#include "lib/oled/SSD1306.h"
#include "lib/oled/Font5x8.h"

///OLDE Functions////
void oled_intial();
void oled_write_ints(unsigned int data); // Integer to String OLED Print

/////////////////////////////////////////////////////////////////////////////




//setting ports
//i2c
//interrupt 
//pwm
//display
//encoder
//DDR - Data Direction Register DDRx = 1 --> OUTPUT
//PORTx - Port - PORTx = 1 ---> HIGH || If the Ports are set as input ---> 0 to disable pull-up resistor and 1 to enable the pull-up resistor 
//PINx - Generally for read only 
//TCCR - generate pwm pulses 


//Analog Read//
volatile int adc_flag = 0;
volatile int16_t adc_value = 0;
void ConfigureADC(void);
void StartConversionADC(void);
//////////////////////////////////

int main(void)
{
	ironBOOT();
	//Reading Analog Values From the Temperature Sensor//
	ConfigureADC();
	StartConversionADC();
	////////////////////////////////////
	setupPWM();
	//////////////////////////////////
	encoder_init();
	//enc = EncoderRead();
	
    while (1) 
    {
		enc = EncoderRead();
		ironRUN();
		//_delay_ms(10);
		//dutyCycle += 10;
		
    }
}


//Analog Read//
void ConfigureADC(void)
{
	ADMUX = 0b01000011;//3rd Channel
	ADCSRA = 0b10101111;
	ADCSRB = 0b00000000;
	DIDR0 = (1<<ADC3D);
	sei();
}

void StartConversionADC()
{
	ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect)
{
	adc_flag = 1;
	adc_value = ADC;
	analogRead();
}


///////////////////////////////////////////////////////////


//Encoder
void encoderValue(int32_t eVal)
{
	
	
}

//OLED Fucntions//
void oled_write_ints(unsigned int data) // must include #include <stdio.h> #include <stdlib.h> #include <string.h>
{
	char st[16] = "";  //  Make space
	itoa(data,st,10);
	GLCD_PrintString(st);
}

void oled_intial()
{
	
}
//////////////////////////////////
//PWM//
ISR(TIMER0_OVF_vect)
{
	tempPID();
	OCR0A = (dutyCycle/100.0)*255;
}

void setupPWM()
{
	DDRD = (1<<PORTD6);
	TCCR0A = (1 << COM0A1) | (1 << WGM00) | (1 << WGM01);
	TIMSK0 = (1 << TOIE0);
	OCR0A = (dutyCycle/100.0)*255.0;
	sei();
	TCCR0B = (1 << CS00) | (1 << CS02);
}

void tempPID()
{
	//value conversion;
	
	error = (-1*enc) - sensorValue; // Getting the temperature error between set value and the iron temperature value
	pWM = kp*error + ki*(error-lastError);//PID Equation 
	if(pWM > 100 || pWM < 0)
	{
		pWM = 0;
	}
	else
	{
		dutyCycle = pWM;	
	}
	lastError = error;
}

//Functions//
void ironBOOT()//Staring the MCU and the OLED
{
	CLKPR = 0x80;
	CLKPR = 0x00;
	void oled_intial();
	GLCD_Setup();
	GLCD_SetFont(Font5x8, 5, 8, GLCD_Overwrite);
	GLCD_GotoXY(0, 0);
	GLCD_PrintString("Soldering Station");
	GLCD_GotoXY(0,16);
	//GLCD_InvertScreen();
	GLCD_Render();
	_delay_ms(500);
}

void ironRUN()//Countinously printing values 
{
	GLCD_GotoXY(0,16);
	GLCD_PrintString("Iron Temperature:");
	GLCD_PrintInteger(sensorValue);
	GLCD_GotoXY(0,24);
	GLCD_PrintString("Duty:");
	GLCD_PrintInteger(dutyCycle);
	GLCD_GotoXY(0,32);
	GLCD_PrintString("Set Temperature:");
	GLCD_PrintInteger(-1*enc);
	GLCD_Render();
	GLCD_Clear();
}

void analogRead(void)//mapping temperature values
{
	
	if (adc_flag)
	{
		sensorValue = adc_value;
		adc_flag = 0;
	}
	sensorValue = (sensorValue);//*300 ;//+ 25;//Room temperature Correction 
	//return sensorValue;
}