/*
 * clock.h
 *
 *  Created on: 5 nov. 2019
 *      Author: kbkcao
 */

#ifndef SRC_CLOCK_H_
#define SRC_CLOCK_H_

/* LSE and RTC config, need to be done only once */
void SystemClock_LSE_and_RTC_Config();

/* 3 clock modes needed for the experiments */
void SystemClock_80MHz_HSE_Range1_Config();
void SystemClock_24MHz_MSI_Range1_Config();
void SystemClock_24MHz_MSI_Range2_Config();

/* RTC specific functions needed to configure RTC wakeup line */
void RTC_wakeup_init( int delay );
void RTC_wakeup_init_from_standby_or_shutdown( int delay );
void RTC_wakeup_init_from_stop( int delay );

#endif /* SRC_CLOCK_H_ */
