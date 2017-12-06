#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>
#include <sysctl.h>
#include <os.h>
#include <tasks.h>

void blink1()
{

	GPIO_PORTF_DATA_R = 0x2;
	os_delay_ms(5);
	GPIO_PORTF_DATA_R = 0x0;
}

void blink3()
{
	GPIO_PORTC_DATA_R = 0x40;
	os_delay_ms(5);
	GPIO_PORTC_DATA_R = 0x00;
}

void blink5()
{
	GPIO_PORTC_DATA_R ^= 0x80;
	os_delay_ms(5);
	GPIO_PORTC_DATA_R = 0x00;
}
