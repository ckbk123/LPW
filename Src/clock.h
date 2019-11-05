/*
 * clock.h
 *
 *  Created on: 5 nov. 2019
 *      Author: kbkcao
 */

#ifndef SRC_CLOCK_H_
#define SRC_CLOCK_H_



void SystemClock_LSE_and_RTC_Config();

void SystemClock_80MHz_HSE_Range1_Config();

void SystemClock_24MHz_MSI_Range1_Config();

void SystemClock_24MHz_MSI_Range2_Config();

void SysTick_Handler();

#endif /* SRC_CLOCK_H_ */
