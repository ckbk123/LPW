/*
 * gpio.c pour nucleo L476
 */
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_gpio.h"
#include "gpio.h"




void GPIO_init(void)
{
// PORT A
LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOA );
// LED verte
LL_GPIO_SetPinMode(       LED_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT );
LL_GPIO_SetPinOutputType( LED_PORT, LED_PIN, LL_GPIO_OUTPUT_PUSHPULL );

// PORT C
LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOC );
// bouton bleu pin 13
LL_GPIO_SetPinMode( BUT_PORT, BUT_PIN, LL_GPIO_MODE_INPUT );

// SQUAREWAVE 50HZ
LL_GPIO_SetPinMode( OUT_50HZ_PORT, OUT_50HZ_PIN, LL_GPIO_MODE_OUTPUT);
}


void LED_GREEN( int val )
{
if	( val )
	LL_GPIO_SetOutputPin(   LED_PORT, LED_PIN );
else	LL_GPIO_ResetOutputPin( LED_PORT, LED_PIN );
}

int BLUE_BUTTON()
{
return ( !LL_GPIO_IsInputPinSet( BUT_PORT, BUT_PIN ) );
}






