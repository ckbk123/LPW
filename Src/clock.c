/*
 * clock.c
 *
 *  Created on: 5 nov. 2019
 *      Author: kbkcao
 */

#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_rtc.h"
#include "core_cm4.h"
#include "stm32l4xx_ll_cortex.h"

#include "global_variables.h"
#include "gpio.h"
#include "clock.h"

/* ck_apre=LSEFreq/(ASYNC prediv + 1) = 256Hz with LSEFreq=32768Hz */
#define RTC_ASYNCH_PREDIV          ((uint32_t)0x7F)
/* ck_spre=ck_apre/(SYNC prediv + 1) = 1 Hz */
#define RTC_SYNCH_PREDIV           ((uint32_t)0x00FF)

/* configure the LSE and activate the RTC */
void SystemClock_LSE_and_RTC_Config() {
	if	(LL_RCC_LSE_IsReady()==0)
	{
		// enable peripheral PWR to reset entire backup domain
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

		// enable backup domain access
		LL_PWR_EnableBkUpAccess();

		// force a bkup domain reset
		LL_RCC_ForceBackupDomainReset();
		LL_RCC_ReleaseBackupDomainReset();

		// activate LSE and then wait for its activation
		LL_RCC_LSE_Enable();
		while (LL_RCC_LSE_IsReady() != 1);

		// channel clock 32.678khz to RTC module
		LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);

		// configure RTC
		LL_RCC_EnableRTC();   // activate
		LL_RTC_DisableWriteProtection(RTC);     // allow write config
		LL_RTC_EnableInitMode(RTC); 	    // enter init mode to set hour format and async/sync prescaler
		LL_RTC_SetHourFormat(RTC, LL_RTC_HOURFORMAT_AMPM);
		LL_RTC_SetAsynchPrescaler(RTC, RTC_ASYNCH_PREDIV);
		LL_RTC_SetSynchPrescaler(RTC, RTC_SYNCH_PREDIV);
		LL_RTC_DisableInitMode(RTC);        // end of init mode

		while(LL_RTC_IsActiveFlag_RS(RTC) != 1);    // wait for RTC activation
	}
}

/* configure HSE clock at 80MHz */
void SystemClock_80MHz_HSE_Range1_Config() {
	// MSI configuration and activation
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
	LL_RCC_MSI_Enable();
	while	(LL_RCC_MSI_IsReady() != 1)
		{ };

	// Main PLL configuration and activation
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
	LL_RCC_PLL_Enable();
	LL_RCC_PLL_EnableDomain_SYS();
	while(LL_RCC_PLL_IsReady() != 1)
		{ };

	// Sysclk activation on the main PLL
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
		{ };

	// Set APB1 & APB2 prescaler
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	// Update the global variable called SystemCoreClock
	SystemCoreClockUpdate();
}

void SystemClock_24MHz_MSI_Range1_Config() {
	// enable clock to module PWR to set Voltage Range (range 1 for this function)
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

	// set flast latency or wait states to 1 (table 11, p100, ref manual)
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

	// select range for MSI (prog manual, p1695 or ref manual, p213) then enable MSI clock
	LL_RCC_MSI_EnableRangeSelection();
	if (LL_RCC_MSI_IsEnabledRangeSelect()) LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9);
	LL_RCC_MSI_Enable();
	while	(LL_RCC_MSI_IsReady() != 1)
		{ };

	// disable MSI calibration with LSE
	LL_RCC_MSI_DisablePLLMode();

	// Sysclk activation with MSI clock
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI)
		{ };

	// Set APB1 & APB2 prescaler
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	// Update the global variable called SystemCoreClock
	SystemCoreClockUpdate();
}

void SystemClock_24MHz_MSI_Range2_Config() {
	// enable clock to module PWR to set Voltage Range (range 2 for this function)
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);

	// set flast latency or wait states to 1 (table 11, p100, ref manual)
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);

	// select range for MSI (prog manual, p1695 or ref manual, p213) then enable MSI clock
	LL_RCC_MSI_EnableRangeSelection();
	if (LL_RCC_MSI_IsEnabledRangeSelect()) LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9);
	LL_RCC_MSI_Enable();
	while	(LL_RCC_MSI_IsReady() != 1)
		{ };

	// disable MSI calibration with LSE
	LL_RCC_MSI_DisablePLLMode();

	// Sysclk activation with MSI clock
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI)
		{ };

	// Set APB1 & APB2 prescaler
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	// Update the global variable called SystemCoreClock
	SystemCoreClockUpdate();
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
			bbleu ^= 1;  // toggle bbleu
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
			LL_LPM_DisableSleepOnExit();
		}else {
			LL_LPM_EnableSleepOnExit();
		}
	}
	else if (expe == 2 || expe == 4) {
		// push of button will toggle MSI calibration with LSE
		if (bbleu == 0) {
			LL_RCC_MSI_DisablePLLMode();
		}else {
			LL_RCC_MSI_EnablePLLMode();
		}
	}
}
