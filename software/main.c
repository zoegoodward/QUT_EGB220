#define F_CPU 16000000UL //16 MHz
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <math.h>

//Definitions for code readability 
#define CW 1
#define CCW 0

//Declaring functions
void setupMotors();
void motor1Direction();
void motor2Direction();
void motor1Speed();
void motor2Speed();

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
	OCR1A = 255;	
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
	OCR0A = 255;
	OCR0B = 0;
}

ISR(TIMER0_OVF_vect)
{
	//Timer0 overflow interrupt
	//sei(); in main to enable
}

ISR(TIMER1_OVF_vect)
{
	//Timer1 overflow interrupt
	//sei(); in main to enable
}

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

void main()
{
	//DEMO 1A code (runs forward at half speed for 3s then stops)
	setupMotors();
	
	motor1Speed(0.5); //0.0 to 1.0
	motor2Speed(0.5); //0.0 to 1.0
	
	motor1Direction(CW); //CW or CCW
	motor2Direction(CW); //CW or CCW
	
	_delay_ms(3000);
	
	motor1Speed(0);
	motor2Speed(0);
}