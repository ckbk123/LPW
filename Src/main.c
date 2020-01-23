


/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_exti.h"
#include "core_cm4.h"
#include "stm32l4xx_ll_cortex.h"
// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */
#include "global_variables.h"
#include "clock.h"
#include "gpio.h"

void    SystemClock_Config (void);
void 	LL_Init10msTick (uint32_t HCLKFrequency);

int main(void)
                                                                                      {
/* this only get executed if LSE is not running */
SystemClock_LSE_and_RTC_Config();  // GOT SOME PROBLEM WITH THIS: LSE ALWAYS ON, NOT SURE HOW TO FIX THIS

/* retrieve then increment the expe value stored in rtc bkup register 0 */
LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);   // enable clock to backup domain
LL_PWR_EnableBkUpAccess();							// enable access to backup domain
LL_RTC_DisableWriteProtection(RTC);					// disable write protection on RTC
expe = LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0); // read value on bkup register (expe incremented from last reset)
if (expe == 0) 	expe = 1;
LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, (expe == 8) ? 1 : expe + 1);		// write back into bkup domain expe incremented
LL_RTC_EnableWriteProtection(RTC);		// reactivate write protection on RTC

/* depending on which expe we are on, choose a correct clock configuration */
if (expe == 1) {
	SystemClock_80MHz_HSE_Range1_Config();
}else if (expe == 2) {
	SystemClock_24MHz_MSI_Range1_Config();
}else if (expe > 2) {
	SystemClock_24MHz_MSI_Range2_Config();
	if (expe > 4) { // for modes 5 to 8, always turn on calibration
		LL_RCC_MSI_EnablePLLMode();
	}
}

/* init variables */
bbleu = 0;
led_counter = 0;
impulsion_base = 5;   // in 10ms
high = 0;
debounce = 0;

/* configure SysTick clock to have exactly 10ms per interrupt */
SysTick_Config(SystemCoreClock/100);

/* configure GPIO */
GPIO_init();

__WFI();

/* infinite loop to avoid program exit */
while (1) ;

}

// SysTick IRQ
void SysTick_Handler() {
	// turn on the LED with a variable impulsion width corresponding to the correct expe mode
	if (led_counter < impulsion_base*expe) {
		LED_GREEN(1);
		++led_counter;
	}else if (led_counter < 100) {
		LED_GREEN(0);
		++led_counter;
	}else {
		led_counter = 0;
	}

	// blue button pressed detection with 100ms debouncing
	if (debounce > 0 && debounce < 10) {
		debounce++;
	}else {
		debounce = 0;
	}
	if (BLUE_BUTTON()) {
		// change mode
		if (debounce == 0) {
			bbleu ^= 1;  // toggle bbleu. So it is possible to return to last state
			debounce = 1;
		}
	}

	// toggle pin PC10 to analyse systick freq, therefore evaluating the core clock precision
	if (high) LL_GPIO_SetOutputPin(OUT_50HZ_PORT, OUT_50HZ_PIN);
	else LL_GPIO_ResetOutputPin(OUT_50HZ_PORT, OUT_50HZ_PIN);
	high ^= 1;

	// cases to use expe
	if (expe == 1 || expe == 3) {
		// push of button will toggle enable sleep on exit functionality
		if (bbleu == 0) {
			LL_LPM_DisableSleepOnExit();   // ensure that MCU stays awake after interrupt
		}else {
			LL_LPM_EnableSleepOnExit();    // send MCU to sleep after interrupt
		}
	}
	else if (expe == 2 || expe == 4) {
		// push of button will toggle MSI calibration with LSE
		if (bbleu == 0) {
			LL_RCC_MSI_DisablePLLMode();	// no calibration of MSI
		}else {
			LL_RCC_MSI_EnablePLLMode();		// calibration with MSI
		}
	}else if (expe > 4) {
		if (bbleu == 0) {
			LL_LPM_EnableSleepOnExit();
		}else {
			/* configure low power modes */
			if (expe == 5) {
				LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
				RTC_wakeup_init_from_stop(5);	// enable RTC wakeup init from stop0 mode
				bbleu = 0;
				__WFI();
			}else if (expe == 6) {
				LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
				RTC_wakeup_init_from_stop(5);	// enable RTC wakeup init from stop1 mode
				bbleu = 0;
				__WFI();
			}else if (expe == 7) {
				LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
				RTC_wakeup_init_from_stop(5);	// enable RTC wakeup init from stop2 mode
				bbleu = 0;
				__WFI();
			}else if (expe == 8) {
				LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
				RTC_wakeup_init_from_standby_or_shutdown(5);	// enable RTC wakeup init from stop mode
			}
			LL_LPM_EnableDeepSleep(); 		// enable deep sleep with STOPx and SHUTDOWN
			LL_LPM_EnableSleepOnExit();
		}
	}
}

// RTC wakeup handler
void RTC_WKUP_IRQHandler()
{
	LL_EXTI_ClearFlag_0_31( LL_EXTI_LINE_20 );
}

