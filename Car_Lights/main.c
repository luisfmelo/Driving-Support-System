#define F_CPU 16000000L

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#define pwm_0 0
#define pwm_10 25
#define pwm_25 70
#define pwm_50 150
#define pwm_75 200
#define pwm_100 255

#define steering_offset_low 4000
#define steering_offset_medium 8000
#define steering_offset_high 12000

#define i2cwatchdog_limit 4000
#define rs232watchdog_limit 8000

#define i2creset_limit 2
#define statuswait 4000


#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>


#include <math.h>  //include libm
#include "mpu6050.h"
#include "mcp23017.h"


/*
-----------------------------------------------------------------------------------------
*/
/*
Bit macros
*/
#define sbi(port,bit) (port)|=(1<<(bit))
#define cbi(port,bit) (port)&= ~(1<<(bit))
/***** Configure IO *****/
#define configure_as_input(bit)             {bit ## _DDR &= ~(1 << bit);}
#define configure_as_output(bit)            {bit ## _DDR |= (1 << bit);}

#define pullup_on(bit)                      {bit ## _PORT |= (1 << bit);}
#define pullup_off(bit)                     {bit ## _PORT &= ~(1 << bit);}

#define disable_digital_input(channel)      {DIDR0 |= (1 << channel);}
#define enable_digital_input(channel)       {DIDR0 &= ~(1 << channel);}

#define enable_pin_change_interrupt(pin)    {pin ## _PCMSK |= (1 << pin ## _PCINT);}
#define disable_pin_change_interrupt(pin)   {pin ## _PCMSK &= ~(1<< pin ## _PCINT);}


/***** Manipulate Outputs *****/
#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_read(port,bit) (port & (1<<bit))
#define bit_read_pull(port,bit) (!(port & (1<<bit)))


/*
-----------------------------------------------------------------------------------------
*/

static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


/*
variaveis globais
*/


/*IO
in front of the car
out_highbeam[5]
HL5L4 HL3L2 HL1R5 HR4R3 HR2R1

out_lowbeam[6]
ML5 ML4L3 ML2L1 MR5R4 MR3R2 MR1

HL5L4 -> PORTB,0
HL3L2 -> PORTB,1 
HL1R5 -> PORTB,2
HR4R3 -> PORTB,3
HR2R1 -> PORTB,4


*/

char ReceivedByte;
char video_data;

uint8_t out_highbeam[5];
uint8_t out_lowbeam[6];

uint8_t slide_highbeam[11]={0,0,0,0,75,255,75,0,0,0,0};
uint8_t slide_lowbeam[12]={0,0,0,0,75,255,255,75,0,0,0,0};

int8_t offset_steering =0;

bool sensor_light=0;
bool sensor_speed=0;

bool sw_auto=0;

bool video_new=0;

bool rx_ok=0;

bool reseti2c=0;
bool tick_100ms=0;

uint16_t tm0=0;

uint16_t resetcount=0;

uint16_t i2cwatchdog=0;
uint16_t delay_tm0=0;

uint16_t timeout_232=0;

char statusmsg=0;
uint16_t statusdelay=0;

bool video_error=false;

/*
Auto off   -> 0
Auto DRL   -> 1
A_Lowbeam  -> 2
A_Highbeam -> 3
*/
uint8_t mode_auto=0;
uint8_t mode_sw=0;

/*
i2c @ GY OK?
*/
int16_t OKax = 0;
int16_t OKay = 0;
int16_t OKaz = 0;
int16_t OKgx = 0;
int16_t OKgy = 0;
int16_t OKgz = 0;

//GY521
#if (MPU6050_GETATTITUDE == 0)
int16_t ax = 0;
int16_t ay = 0;
int16_t az = 0;
int16_t gx = 0;
int16_t gy = 0;
int16_t gz = 0;
double axg = 0;
double ayg = 0;
double azg = 0;
double gxds = 0;
double gyds = 0;
double gzds = 0;
#endif

#if ((MPU6050_GETATTITUDE == 1) ||( MPU6050_GETATTITUDE == 2))
long *ptr = 0;
double qw = 1.0f;
double qx = 0.0f;
double qy = 0.0f;
double qz = 0.0f;
double roll = 0.0f;
double pitch = 0.0f;
double yaw = 0.0f;
#endif


/*
prototipos
*/
void InitHW();
void Init232();
void InitExt();
void set_drl();
void set_lowbeam();
void set_highbeam();
void refreshOUTvalues();
void read_input();
void restarti2c();
void set_status();

/////////////////////////////////////////////////////////////////////////////////////////////////////




/*
Inits
*/



void Init232()
{
	//USART Baud rate: 9600
	UBRR0H = (MYUBRR >> 8);
	UBRR0L = MYUBRR;
	UCSR0B = (1<<RXEN0)|(1<<RXCIE0)|(1<<TXEN0);
	
	stdout = &mystdout; //Required for printf init
}

void InitExt()
{
	cli();
	
	//init mpu6050
	mpu6050_init();
	//printf("\n mpu6050 OK\n");
	
/*	//init mcp23017
	mcp23017_init(&device, 0b000);
		
	printf("\n mcp23017 OK\n");
	*/
	sei();
	
}


void InitHW()
{
	cli();
	//PortB as Output
	DDRB = 0xFF;
	PORTB = 0x00;
	
	//PortC as Output
	DDRC = 0xFF;
	PORTC = 0x00;

/*	
	//PortD as Input/Output
	DDRD = 0b00011110; //PORTD (RX on PD0);
	PORTD = 0b11100000;*/

	//PortD as Output
	DDRD = 0b00001110; //PORTD (RX on PD0);
	PORTD = 0xF0;
	
	// initialize Timer0

	//set timer0 as PWM, Phase Correct mode
	TCCR0A = 0;// set entire TCCRxA register to 0
	TCCR0B = 0;// same for TCCRxB
	TCNT0  = 0;//initialize counter value to 0
	// turn on PWM, Phase Correct mode
	TCCR0A |= (1 << WGM00);
	// Set CS02 and CS00 bits for 256 prescaler
/*	//Focr=Fclk/(Nx510)=16000000/(256*510)=122.5Hz / 8.16ms
	TCCR0B |= (1 << CS02);*/
/*	//Focr=Fclk/(Nx510)=16000000/(64*510)=490Hz / 2.04ms
	TCCR0B |= (1 << CS01)|(1 << CS00);*/
	//Focr=Fclk/(Nx510)=16000000/(8*510)=3.9kHz / 255us
	TCCR0B |= (1 << CS01);

	// enable timer compare interrupt
	TIMSK0 |= (1 << OCIE0A);


	// initialize Timer2

	//set timer0 interrupt at 1kHz
	TCCR2A = 0;// set entire TCCR2A register to 0
	TCCR2B = 0;// same for TCCR2B
	TCNT2  = 0;//initialize counter value to 0
	// set compare match register for 2khz increments
	//freq=(16*10^6) / (2*prescaller*(1+OCR)
	//OCR2A = 249;// = 125Hz@pre=256
	OCR2A = 124;// = 250Hz@pre=256
	//OCR2A = 32;// = 1KHz@pre=256
	// turn on CTC mode
	TCCR2A |= (1 << WGM21);
	// Set CS21 and CS20 bits for 32 prescaler
	TCCR2B |= (1 << CS21)|(1 << CS20);
	// enable timer compare interrupt
	TIMSK2 |= (1 << OCIE2A);
	
	//InitADC();
	
	//init uart
	Init232();

	InitExt();
	
	sei();
	
	//printf("\n Init OK\n");
	
}


/*
Serial print
*/

static int uart_putchar(char c, FILE *stream)
{
	if (c == '\n') uart_putchar('\r', stream);
	
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	
	return 0;
}

uint8_t uart_getchar(void)
{
	while( !(UCSR0A & (1<<RXC0)) );
	return(UDR0);
}


ISR(USART_RX_vect)
{
	ReceivedByte = UDR0;
	rx_ok=true;
	timeout_232=0;
	/*ReceivedByte = UDR0; // Fetch the received byte value into the variable "ByteReceived"
	printf("\nRecieved: %c\n", ReceivedByte); // Echo back the received byte back to the computer
	*/
}

ISR(TIMER0_COMPA_vect)
{	
	
	if (i2cwatchdog>i2cwatchdog_limit)
	{
		reseti2c=true;
	} 
	else
	{
		i2cwatchdog++;
//		resetcount=0;
	}
	
	if (reseti2c)
	{
		if (tm0>400)
		{
			resetcount++;
			tm0=0;
		}
		else
		{
			tm0++;
		}		
	}
	
	
	delay_tm0++;
	timeout_232++;
	statusdelay++;
}



ISR(TIMER2_COMPA_vect)
{	//timer2 interrupt 10kHz 

	if (TCNT0<out_highbeam[0])
	{
		sbi(PORTB,0);
	}
	else
	{
		cbi(PORTB,0);
	}
	
	if (TCNT0<out_highbeam[1])
	{
		sbi(PORTB,1);
	}
	else
	{
		cbi(PORTB,1);
	}
	if (TCNT0<out_highbeam[2])
	{
		sbi(PORTB,2);
	}
	else
	{
		cbi(PORTB,2);
	}
	
	if (TCNT0<out_highbeam[3])
	{
		sbi(PORTB,3);
	}
	else
	{
		cbi(PORTB,3);
	}
	if (TCNT0<out_highbeam[4])
	{
		sbi(PORTB,4);
	}
	else
	{
		cbi(PORTB,4);
	}
	
	if (TCNT0<out_lowbeam[0])
	{
		sbi(PORTC,0);
	}
	else
	{
		cbi(PORTC,0);
	}
	
	if (TCNT0<out_lowbeam[1])
	{
		sbi(PORTC,1);
	}
	else
	{
		cbi(PORTC,1);
	}
	if (TCNT0<out_lowbeam[2])
	{
		sbi(PORTC,2);
	}
	else
	{
		cbi(PORTC,2);
	}
	
	if (TCNT0<out_lowbeam[3])
	{
		sbi(PORTC,3);
	}
	else
	{
		cbi(PORTC,3);
	}
	if (TCNT0<out_lowbeam[4])
	{
		sbi(PORTD,2);
	}
	else
	{
		cbi(PORTD,2);
	}
	if (TCNT0<out_lowbeam[5])
	{
		sbi(PORTD,3);
	}
	else
	{
		cbi(PORTD,3);
	}	

}


void read_input()
{
	
	if (!(bit_read_pull(PIND,7)))
	{
		//auto on
		sw_auto=1;
		//printf("\nauto on");
	}
	else
	{
		//auto off
		sw_auto=0;
		mode_auto=0;
		//printf("\nauto off");
	}
	
	
	if (sw_auto)
	{
		if (sensor_light)
		{
			if (sensor_speed)
			{
				mode_auto=3;
				set_highbeam();
			} 
			else
			{
				mode_auto=2;
				set_lowbeam();
			}
		} 
		else
		{
			mode_auto=1;
			set_drl();
		}
	}
	else
	{
		/*  6 5
		1 0 -> high beam
		0 0 -> low beam
		0 1 -> drl
		*/
		
		video_error=true;
		video_data=0b00100000;
		sensor_speed=(((video_data>>(6))&0b00000001));
		sensor_light=(((video_data>>(5))&0b00000001));
		video_new=1;
		
		if (!(bit_read_pull(PIND,6)))
		{
			if (!(bit_read_pull(PIND,5)))
			{
				//low beam
				set_lowbeam();
			} 
			else
			{
				//drl
				set_drl();
			}
		} 
		else
		{
			//highbeam
			set_highbeam();
		}
	}

// Sensor luminosidade fisico
/*	if (!(bit_read_pull(PIND,4)))
	{
		//printf("\nnight");	
		sensor_light=1;
	}
	else
	{
		//printf("\nday");	
		sensor_light=0;
	}*/
}

void set_drl()
{
	int i;
	for(i=0; i<12; i++)
	{
		slide_lowbeam[i]=pwm_25;
	}
	
	for( i=0; i<11; i++)
	{
		slide_highbeam[i]=pwm_0;
	}
}

void set_lowbeam()
{
	int i;
	for(i=0; i<3; i++)
	{
		slide_lowbeam[i]=pwm_25;
	}
	for(i=3; i<7; i++)
	{
		slide_lowbeam[i]=pwm_50;
	}
	for(i=7; i<9; i++)
	{
		slide_lowbeam[i]=pwm_100;
	}
	for(i=9; i<12; i++)
	{
 		slide_lowbeam[i]=pwm_50;
	}
	
	for( i=0; i<11; i++)
	{
		slide_highbeam[i]=pwm_0;
	}
}

void set_highbeam()
{
	int i;
	for(i=0; i<12; i++)
	{
		slide_lowbeam[i]=pwm_0;
	}
	
	for( i=0; i<1; i++)
	{
		slide_highbeam[i]=pwm_25;
	}
	for( i=1; i<4; i++)
	{
		slide_highbeam[i]=pwm_50;
	}
	for( i=4; i<9; i++)
	{
		slide_highbeam[i]=pwm_100;
	}
	for( i=9; i<10; i++)
	{
		slide_highbeam[i]=pwm_50;
	}
	for( i=10; i<11; i++)
	{
		slide_highbeam[i]=pwm_25;
	}
}

void set_steering()
{
	mpu6050_getRawData(&ax, &ay, &az, &gx, &gy, &gz);
	
	//if ((OKax==ax)&&(OKay==ay)&&(OKaz==az)&&(OKgx==gx)&&(OKgy==gy)&&(OKgz==gz))
	if ((OKax==ax)&&(OKay==az))
	{
		OKay=ay;
		ay=0;
		
		reseti2c=1;	
		//i2cwatchdog=(i2cwatchdog_limit)*2;	
	}
	else
	{
		OKax=ax;
		OKay=ay;
		OKaz=az;
		OKgx=gx;
		OKgy=gy;
		OKgz=gz;
	}
				
	
	
	if ((ay<steering_offset_low)&&(ay>-steering_offset_low))
	{
		offset_steering=0;
	}
	else
	{
		if (ay>steering_offset_low)
		{
			if (ay>steering_offset_medium)
			{
				if (ay>steering_offset_high)
				{
					offset_steering=-3;
				} 
				else
				{
					offset_steering=-2;
				}
			} 
			else
			{
				offset_steering=-1;
			}
		} 
		else
		{
			if (ay<-steering_offset_low)
			{
				if (ay<-steering_offset_medium)
				{
					if (ay<-steering_offset_high)
					{
						offset_steering=3;
					}
					else
					{
						offset_steering=2;
					}
				}
				else
				{
					offset_steering=1;
				}
			}
		}
	}
}

void set_out_offset()
{
	int i;
	for(i=0; i<6; i++)
	{
		out_lowbeam[i]=slide_lowbeam[3+i+offset_steering];
	}
	
	if (3==mode_auto)
	{
		for( i=0; i<5; i++)
		{
			out_highbeam[i]=(slide_highbeam[3+i+offset_steering])*(1-(((video_data>>(4-i))&0b00000001)));
			out_lowbeam[i+1]=(slide_highbeam[3+i+offset_steering])*((((video_data>>(4-i))&0b00000001)));
		}
	} 
	else
	{
		for( i=0; i<5; i++)
		{
			out_highbeam[i]=slide_highbeam[3+i+offset_steering];
		}
	}
		
}

void restarti2c()
{

	if (resetcount<i2creset_limit)
	{
		//printf("\nRESET!!!\n");
		if ((3==mode_auto)&&(1==sw_auto))
		{
			set_lowbeam();
		}
		
		TWCR =0;
		InitExt();
		i2cwatchdog=0;
		reseti2c=false;	
		resetcount++;
	}
	
	if (resetcount>250)
	{
		resetcount=0;
	}
	
	ay=0;
	OKay=0;
	offset_steering=0;

}


void set_status()
{
	if (statusdelay>statuswait)
	{
		statusmsg=0;
		if (sw_auto)
		{
			
			
			if (1==mode_auto)
			{
				//drl
				statusmsg=statusmsg|0b00000010;
			} 
			else
			{
				if (2==mode_auto)
				{
					//low beam
					statusmsg=statusmsg|0b00000100;
				}
				else
				{
					if (3==mode_auto)
					{
						//highbeam
						statusmsg=statusmsg|0b00001000;
					}
				}
			}	
		}
		else
		{
			
		statusmsg=statusmsg|0b00000001;
		/*  6 5
		1 0 -> high beam
		0 0 -> low beam
		0 1 -> drl
		*/
			if (!(bit_read_pull(PIND,6)))
			{
				if (!(bit_read_pull(PIND,5)))
				{
					//low beam
					statusmsg=statusmsg|0b00000100;
				} 
				else
				{
					//drl
					statusmsg=statusmsg|0b00000010;
				}
			} 
			else
			{
				//highbeam
				statusmsg=statusmsg|0b00001000;
			}
		}
		
		if (reseti2c)
		{
			//i2c problem
			statusmsg=statusmsg|0b00010000;
		} 
		
		if (video_error)
		{
			//i2c problem
			statusmsg=statusmsg|0b00100000;
		}
		
		//uart_putc(statusmsg);
		printf("%c\n",statusmsg);
		
		statusdelay=0;
	}
}

int main(void) 
{
	InitHW();
	
	
	while(1)
	{
		
		if (reseti2c)
		{
			restarti2c();
		}
		
		read_input();
		if (rx_ok)
		{
			rx_ok=false;
			//ReceivedByte = UDR0;
			if (0==video_new)
			{
				video_error=false;
				video_data=ReceivedByte;
				sensor_speed=(((video_data>>(6))&0b00000001));
				sensor_light=(((video_data>>(5))&0b00000001));
				video_new=1;
				//printf("\n%c",ReceivedByte);
			}
		}
		else
		{
			if ((0==video_new)&&(timeout_232>rs232watchdog_limit))
			{
				video_error=true;
				video_data=0b00100000;
				sensor_speed=(((video_data>>(6))&0b00000001));
				sensor_light=(((video_data>>(5))&0b00000001));
				video_new=1;
			}
		}
		
		
		
		
		if (delay_tm0>(i2cwatchdog_limit/4))
		{
			set_steering();
			video_new=0;
			delay_tm0=0;
			i2cwatchdog=0;
			set_out_offset();
			set_status();
		} 
		
		
	}
	
}
