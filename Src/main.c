
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "core_cm4.h"
#include "stm32l4xx_ll_cortex.h"
// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */

#include "gpio.h"

void    SystemClock_Config (void);
void 	LL_Init10msTick (uint32_t HCLKFrequency);

int bbleu;
int expe;
int impulsion_base;
int led_counter;

int main(void)
{
/* Configure the system clock */
SystemClock_Config();

// config GPIO
GPIO_init();

SysTick_Config(SystemCoreClock/100);

// init timer pour utiliser la fonction LL_mDelay() de stm32l4xx_ll_utils.c
//LL_Init10msTick( SystemCoreClock );

bbleu = 0;
expe = 1;
led_counter = 0;
impulsion_base = 5;   // in 10ms

LL_RCC_MSI_Disable();



while (1);
/*
while (1)
 	{
	if	( BLUE_BUTTON() )
		LED_GREEN(1);
	else {
		LED_GREEN(0);
		LL_mDelay(950);
		LED_GREEN(1);
		LL_mDelay(50);
		}
	}
*/
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
// Figure 15 page 208 Schema clock
void SystemClock_Config(void)
{
/* MSI configuration and activation */
LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

// if we want sum calib: page 213
/* Main PLL configuration and activation */
LL_RCC_MSI_DisablePLLMode();
LL_RCC_MSI_EnableRangeSelection();
if (LL_RCC_MSI_IsEnabledRangeSelect()) LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);

LL_RCC_MSI_Enable();
while	(LL_RCC_MSI_IsReady() != 1)
	{ };

// to calibrate with LSE
LL_RCC_LSE_Enable();
if (LL_RCC_LSE_IsReady()) LL_RCC_MSI_EnablePLLMode();

/* Sysclk activation on the MSI */
LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);
while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI)
	{ };
  
/* Set APB1 & APB2 prescaler*/
LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

/* Update the global variable called SystemCoreClock */
SystemCoreClockUpdate();
}

// SysTick IRQ
void SysTick_Handler() {
	if (expe == 1) {
		if (led_counter < impulsion_base*expe) {
			LED_GREEN(1);
			++led_counter;
		}else if (led_counter < 100) {
			LED_GREEN(0);
			++led_counter;
		}else {
			led_counter = 0;
		}

		if (BLUE_BUTTON()) {
			// change mode
			bbleu ^= 1;  // toggle bbleu
		}

		if (bbleu == 0) {
			LL_LPM_DisableSleepOnExit();
		}else {
			LL_LPM_EnableSleepOnExit();
		}
	}else if (expe == 2) {

	}
}

void LL_Init10msTick(uint32_t HCLKFrequency)
{
  /* Use frequency provided in argument */
  LL_InitTick(HCLKFrequency, 10000U);
}
