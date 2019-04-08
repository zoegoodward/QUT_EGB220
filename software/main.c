#define F_CPU 16000000UL //16 MHz
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <math.h>

//Definitions
#define CW 1
#define CCW 0

#define SENSOR8 (0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0)
#define SENSOR7 (0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(1<<MUX0)
#define SENSOR6 (0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(0<<MUX0)
#define SENSOR5 (0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(1<<MUX0)
#define SENSOR4 (0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)
#define SENSOR3 (0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0)
#define SENSOR2 (0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0)
#define SENSOR1 (0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(0<<MUX0)
	//MUXn (n = 5(in ADCSRB), 4, 3, 2, 1, 0)
		//ADC0 = 000000
		//ADC1 = 000001
		//ADC4 = 000100 (SENSOR 1)
		//ADC5 = 000101 (SENSOR 2)
		//ADC6 = 000110 (SENSOR 3)
		//ADC7 = 000111 (SENSOR 4)
		//ADC8 = 100000 (SENSOR 8)
		//ADC9 = 100001 (SENSOR 7)
		//ADC10 =100010 (SENSOR 6)
		//ADC11 =100011 (SENSOR 5)
		//ADC12 =100100
		//ADC13 =100101
		
//Global Variables
uint8_t sensorOutput[4]; //ADC sensor value array

//Declaring functions
void setupMotors();
void motor1Direction();
void motor2Direction();
void motor1Speed();
void motor2Speed();
void setupADC();
void getSensorReading();

void setupMotors()
{
	//Motor 1 output pins
	DDRB |= (1<<5);
	DDRB |= (1<<6);
	//Timer 1 settings (8-bit fast pwm, ignore on compare match, clear at top, 256 prescaler)
	TCCR1A |= (1<<WGM10)|(1<<COM1A1)|(1<<COM1B1);
	TCCR1B |= (1<<WGM12)|(1<<CS12); 
	//Timer 1 interrupt enable
	TIMSK1 |= (1<<0);
	//MAX PWM cycle
	OCR1A = 0;	
	OCR1B = 0;
	
	//Motor 2 output pins
	DDRD |= (1<<0);
	DDRB |= (1<<7);
	//Timer 0 settings (8-bit fast pwm, ignore on compare match, clear at top, 256 prescaler)
	TCCR0A |= (1<<COM0A1)|(1<<COM0B1)|(1<<WGM01)|(1<<WGM00);
	TCCR0B |= (1<<CS02);
	//Timer 0 interrupt enable
	TIMSK0 |= (1<<0);
	//Max PWM cycle
	OCR0A = 0;
	OCR0B = 0;
}

//ISR(TIMER0_OVF_vect)
//{
	//Timer0 overflow interrupt
	//sei(); in main to enable
//}

//ISR(TIMER1_OVF_vect)
//{
	//Timer1 overflow interrupt
	//sei(); in main to enable
//}

//ISR(ADC_vect)//ADC conversion complete interrupt
//{
	
//}

void motor1Direction(int direction)
{
	if (direction == 1)
	{
		//Braking motor 1
		PORTB |= (1<<5);
		PORTB |= (1<<6);
		//Running motor 1 CW
		PORTB |= (0<<5);
	}
	else if (direction == 0)
	{
		//Braking motor 1
		PORTB |= (1<<5);
		PORTB |= (1<<6);
		//Running motor 1 CCW
		PORTB |= (0<<6);
	}
}

void motor2Direction(int direction)
{
	if (direction == 1)
	{
		//Braking motor 2
		PORTD |= (1<<0);
		PORTB |= (1<<7);
		//Running motor 2 CW
		PORTD |= (0<<0);
	}
	else if (direction == 0)
	{
		//Braking motor 2
		PORTD |= (1<<0);
		PORTB |= (1<<7);
		//Running motor 2 CCW
		PORTB |= (0<<7);
	}
}

void motor1Speed(double speed)
{
	//Getting PWM value for speed (0-255)
	speed = round(speed * 255);
	//Checking speed is between 0% and 100%
	if (!(speed > 255 | speed < 0))
	{
		OCR1A = speed;	
		OCR1B = 0;
	}
}

void motor2Speed(double speed)
{
	//Getting PWM value for speed (0-255)
	speed = round(speed * 255);
	//Checking speed is between 0% and 100%
	if (!(speed > 255 | speed < 0))
	{
		OCR0A = speed;	
		OCR0B = 0;
	}
}

void setupADC()
{
	//Using internal 2.56V reference, left adjusted
	ADMUX |= (1<<REFS1)|(1<<REFS0)|(1<<ADLAR); 
	//Enabling ADC, 128 prescaler, no auto-triggering (1<<ADATE to auto trigger) (1<<ADIE for conversion complete interrupt enable)
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	//Free running mode
	ADCSRB = 0; 
}

void getSensorReading()
{	
	///When changing number of sensors - change length of for loop and size of global sensorOutput[]
	int sens_num;
	for (sens_num = 0; sens_num < 4; sens_num++)
	{
		//Clearing current ADC
		ADMUX |= (0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0);
		
		//Cycling through each sensor
		if (sens_num == 0) {ADMUX |= SENSOR1; ADCSRB |= (0<<MUX5);}//Does not work for sensors that require MUX5 set high (some problem with clearing this bit..?)
		if (sens_num == 1) {ADMUX |= SENSOR2; ADCSRB |= (0<<MUX5);}
		if (sens_num == 2) {ADMUX |= SENSOR3; ADCSRB |= (0<<MUX5);}
		if (sens_num == 3) {ADMUX |= SENSOR4; ADCSRB |= (0<<MUX5);}
		
		//Starting conversion
		ADCSRA |= (1<<ADSC);
		//Waiting till conversion finished
		while(~ADCSRA&(1<<ADIF)){}
		//Setting sensed value in array
		sensorOutput[sens_num] = ADCH;
	}
}

void main()
{
	setupADC();
	///sensorOutput[] tests (//higher when over black(low reflectance) //smaller when over white(high reflectance))
	//From 0.5 cm above line
	//Anything smaller than 30-40 is white (high reflectance)
	//Anything above 30-40 is grey to black (low reflectance)
		
	while(1)
	{
		getSensorReading();
		//Testing 
		if (sensorOutput[2] < 30)//30 value here needs to be calibrated with actual robot conditions
		{
			//Do something when high reflectance
		}
		else
		{
			//Do something else when low reflectance
		}
	}
	

	
	
	//DEMO 1A code (runs forward at 30% speed for 2.5s then stops)
/* 	setupMotors();
	
	motor1Speed(0.3); //0.0 to 1.0
	motor2Speed(0.3); //0.0 to 1.0
	
	motor1Direction(CW); //CW or CCW
	motor2Direction(CW); //CW or CCW
	
	_delay_ms(2500);
	
	motor1Speed(0);
	motor2Speed(0); */
}
