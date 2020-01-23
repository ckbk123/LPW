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
#include "stm32l4xx_ll_exti.h"
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
	if	(LL_RCC_LSE_IsReady() == 0)
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
		LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE); // set LSE as clock source for RTC. IMPORTANT because only this allows RTC to work until SHUTDOWN mode

		// configure RTC
		LL_RCC_EnableRTC();   // activate
		LL_RTC_DisableWriteProtection(RTC);     // allow write config
		LL_RTC_EnableInitMode(RTC); 	    // enter init mode to set hour format and async/sync prescaler
		LL_RTC_SetHourFormat(RTC, LL_RTC_HOURFORMAT_AMPM);
		LL_RTC_SetAsynchPrescaler(RTC, RTC_ASYNCH_PREDIV);
		LL_RTC_SetSynchPrescaler(RTC, RTC_SYNCH_PREDIV);
		LL_RTC_DisableInitMode(RTC);        // end of init mode
		LL_RTC_EnableWriteProtection(RTC);     // allow write config

		while(LL_RTC_IsActiveFlag_RS(RTC) != 1);    // wait for RTC activation
	}
}

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

/* init wake up with specific delay(s) */
void RTC_wakeup_init( int delay )
{
	LL_RTC_DisableWriteProtection( RTC );
	LL_RTC_WAKEUP_Disable( RTC );
	while	( !LL_RTC_IsActiveFlag_WUTW( RTC ) ) { }
	// connecter le timer a l'horloge 1Hz de la RTC
	LL_RTC_WAKEUP_SetClock( RTC, LL_RTC_WAKEUPCLOCK_CKSPRE );
	// fixer la duree de temporisation
	LL_RTC_WAKEUP_SetAutoReload( RTC, delay );	// 16 bits
	LL_RTC_ClearFlag_WUT(RTC);
	LL_RTC_EnableIT_WUT(RTC);
	LL_RTC_WAKEUP_Enable(RTC);
	LL_RTC_EnableWriteProtection(RTC);
}

/* init wakeup mode from STANDBY/SHUTDOWN mode */
void RTC_wakeup_init_from_standby_or_shutdown( int delay )
{
	RTC_wakeup_init( delay );
	LL_PWR_EnableInternWU();	// enable the Internal Wake-up line
}

/* init wakeup mode from STOPx mode */
void RTC_wakeup_init_from_stop( int delay )
{
	RTC_wakeup_init( delay );
	// valider l'interrupt par la ligne 20 du module EXTI, qui est réservée au wakeup timer
	LL_EXTI_EnableIT_0_31( LL_EXTI_LINE_20 );
	LL_EXTI_EnableRisingTrig_0_31( LL_EXTI_LINE_20 );
	// valider l'interrupt chez NVIC
	NVIC_SetPriority( RTC_WKUP_IRQn, 1 );
	NVIC_EnableIRQ( RTC_WKUP_IRQn );
}
