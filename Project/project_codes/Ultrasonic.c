#include "Ultrasonic.h"
#include "tm4c123gh6pm.h"
#include "Systick.h"
#include "OmniControl.h"
// ----------------------------------
//  ID        | TRIG_PIN | ECHO_PIN |
// ----------------------------------
// HC-SR04    |  PB2     | PE0      |
// ----------------------------------

#define GPIO_PORTB_PB5_M 0x00000020   // PB5 mask
#define GPIO_PORTE_PE0_M 0x00000001   // PE0 mask

ultrasonic_t ult1,ult2;
/** @brief  This routine intitializes HC-SR04 ultrasonic sensor1
  * @input  None
  * @output None
  */
void Ultrasonic1_Init(void)
{
   volatile unsigned long delay;
   SYSCTL_RCGC2_R|=0x0000001A;            // activate clock for Port B, Port D and Port E 
   delay=SYSCTL_RCGC2_R;                  // dummy delay
   
   /* Make PB2 OUTPUT */
   GPIO_PORTB_CR_R|=0x04;                 // allow changes to PB2   
   GPIO_PORTB_AMSEL_R&=~0x04;             // disable analog func for PB2       
   GPIO_PORTB_PCTL_R&=~0x00000100;        // no alternative func    
   GPIO_PORTB_DIR_R|=0x04;                // make PB2 output
   GPIO_PORTB_AFSEL_R&=~0x04;             // not alternative func   
   GPIO_PORTB_DEN_R|=0x04;                // enable digital I/0 for PB2 
	
	ult1.done = 1;  // to start sensor measurements for the first time
}


/** @brief  Send trigger to activate first ultrasonic sensor for distance measuring
  * @input  None
  * @output None
  * @description HC-SR04 provides 2cm - 400cm distance measurement. Measuring 400 cm takes nearly 23.5 ms. 
	* When there is no obstacle in front of the sensor within the 400 cm range, echo stays high approx. 200 ms.
  * Triggering sensor while echo is high, does not cause any interrupt on the sensor. Therefore one can
  * call this triggering function every ms or faster. 
  */
void Ultrasonic1_sendTrigger(void)
{
	GPIO_PORTB_DATA_R&=~0x04;  // Trigpin: LOW
	delay_us(2);               // wait 2 us
	GPIO_PORTB_DATA_R|=0x04;   // Trigpin: HIGH
	delay_us(10);              // wait 10 us for triggering
	GPIO_PORTB_DATA_R&=~0x04;  // Trigpin: LOW
}

/** @brief  Send trigger to activate second ultrasonic sensor for distance measuring
  * @input  None
  * @output None
  * @description HC-SR04 provides 2cm - 400cm distance measurement. Measuring 400 cm takes nearly 23.5 ms. 
	* When there is no obstacle in front of the sensor within the 400 cm range, echo stays high approx. 200 ms.
  * Triggering sensor while echo is high, does not cause any interrupt on the sensor. Therefore one can
  * call this triggering function every ms or faster. 
  */
void Ultrasonic2_sendTrigger(void)
{
	GPIO_PORTE_DATA_R&=~0x10;  // Trigpin: LOW
	delay_us(2);               // wait 2 us
	GPIO_PORTE_DATA_R|=0x10;   // Trigpin: HIGH
	delay_us(10);              // wait 10 us for triggering
	GPIO_PORTE_DATA_R&=~0x10;  // Trigpin: LOW 
}

	
/** @brief  Measure time difference between echo high and low, then calculate distance
  * @input  None
  * @output None
  * @description The speed of sound is 340m/s or 29.41us/cm.
	* Sound wave travels out and back, so in order to find the distance we should divide by 2.
	* Therefore distance(cm)=time(us)/58.82;
  */
void GPIOPortE_UltrasonicTask(void)
{ 
	if(GPIO_PORTE_RIS_R & GPIO_PORTE_PE0_M)    // PE0 interrupt occurred
	{
		GPIO_PORTE_ICR_R |= GPIO_PORTE_PE0_M;    // ack flag0
		ult1.flag++;
		
		if(GPIO_PORTE_DATA_R & GPIO_PORTE_PE0_M) // if PE0 is high
			ult1.first_time = Counts;              // measure first time
		else                                     // if PE0 is low
		{
			ult1.second_time = Counts;             // measure second time
			ult1.distMeasure = 1;
		}
		//ult1.play = ult1.distMeasure;
		if(ult1.distMeasure == 1)  // calculate distance only after echo pin goes low  
		{
			ult1.change = (ult1.second_time - ult1.first_time) / 1000.0; // time change in ms
			ult1.dist = (ult1.second_time - ult1.first_time) / 58.82;    // calculate distance in cm
			ult1.distMeasure = 0;
			ult1.done = 1;         // measurement is completed
			
			if(ult1.dist < 10) {}			
			else if(ult1.dist > 50) {};
			else{
				ult1.play = 1;
				TIMER3_CTL_R = 0x00000001; // enable TIMER3A
			}
		}
		
	} 
}

/** @brief  This routine controls ultrasonic sensor1's power
  * @input  on: Determines if ultrasonic sensor is on or off
  * @output None
  * @description This routine is neccessary because HC-SR04 is lack of timeout and echo stucks at high after some time.
  */
void Ultrasonic1_power(bool on)
{
	if(on)
		GPIO_PORTD_DATA_R &= ~0x02;
	else
		GPIO_PORTD_DATA_R |= 0x02;
}


/** @brief  This routine controls ultrasonic sensor2's power
  * @input  on: Determines if ultrasonic sensor is on or off
  * @output None
  * @description This routine is neccessary because HC-SR04 is lack of timeout and echo stucks at high after some time.
  */
void Ultrasonic2_power(bool on)
{
	if(on)
		GPIO_PORTD_DATA_R &= ~0x01;
	else
		GPIO_PORTD_DATA_R |= 0x01;
}

