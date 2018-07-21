// ---------------------------------------------------------------------------------
// libSwitch.c
// Created on: 29-Jan-2010
// Author: nbk
// Desc:
// InitSwitch() to initialise the switch port pin before using it.
// WaitSW() to wait for user to do a switch press
// ReadSWSpecial() to detect how long the switch is pressed by user.
// ---------------------------------------------------------------------------------
#include "project.h"

// @brief : To initialise port pin for user switch input
// @param : none
// @retval
void InitSwitch() {
	GPIO_InitTypeDef GPIO_InitStructure;

	//Switch(SW_USER) init
	//USER_SW_PORT is defined as PC15
	GPIO_InitStructure.GPIO_Pin = USER_SW_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(USER_SW_PORT, &GPIO_InitStructure);
}

#define SW_RELEASE			0
#define SW_DEBOUNCE_PRESS	1
#define SW_PRESS			2
#define SW_DEBOUNCE_RELEASE	3
#define SW_DEBOUNCE_TIME	10
volatile int16_t sw_State;
volatile int16_t sw_press_time;
int16_t sw_debounce_time;
volatile bool bSWFlag;

// This routine must be called from a regular 1ms ISR.
// Currently called from Handler_SysTick();
// It's written in state machine.
void User_sw_service(void) {

	if (sw_debounce_time) {
		// debouncing. So do nothing.
		sw_debounce_time--;
		return;
	}
	if (USER_SW == USER_SW_CLOSED) {
		// switch is pressed
		if (sw_State==SW_PRESS) {
			sw_State = SW_DEBOUNCE_PRESS;
			sw_debounce_time = SW_DEBOUNCE_TIME;
		}
		else if (sw_State == SW_DEBOUNCE_PRESS){
			// after debouncing, switch is still high. So the high state is true.
			sw_State = SW_RELEASE;
		}
	} else {
		// switch is released

		if (sw_State==SW_RELEASE) {
			sw_State = SW_DEBOUNCE_RELEASE;
			sw_debounce_time = SW_DEBOUNCE_TIME;
		}
		else if (sw_State == SW_DEBOUNCE_RELEASE){
			// after debouncing, switch is still low. So the low state is true.
			sw_State = SW_PRESS;
			sw_press_time=0;
			bSWFlag = TRUE;
		}
		else {
			// increment the low state time.
			// This is use to check how long the switch is pressed.
			sw_press_time++;
		}
	}
}

void WaitSW(void) {
	while(sw_State!=SW_PRESS)
		;
	DelaymSec(10);
	while(sw_State!=SW_RELEASE)
		;
	DelaymSec(10);

	bSWFlag=FALSE;
}
// @brief : To read switch press status
// @param : none
// @retval: 0 (<500ms) 1(<1000ms) 2 (<1500ms) else 3
int16_t ReadSWSpecial(void) {
	int16_t ret=0;

	bEncoderClickFlag=FALSE;

	while(sw_State!=SW_RELEASE)
		;
	while(sw_State!=SW_PRESS) {
		if (bEncoderClickFlag) {
			// Use encoder to change values
			return encoderClickType;
		}
	}

	bSWFlag=0;
	DispScroll(FALSE);
	bBlinkFlag = FALSE;


	ret = 0;
	while(sw_State==SW_PRESS) {
		if (sw_press_time<400)
			// short press
			ret = 0;
		else if (sw_press_time<800) {
			// long press, 1 blink
			if (ret == 0) {
				ret = 1;
				DispBlink();
			}
		}
		else if (sw_press_time<1200) {
			if (ret == 1) {
				// very long press, 2 blinks
				ret = 2;
				DispBlink();
			}
		}
		else if (sw_press_time<1600) {
			if (ret == 2) {
				// very long press, 3 blinks
				ret = 3;
				DispBlink();
			}
		}
		else {
			if (ret == 3) {
				ret = 4;
				DispBlink();
			}
		}

	}

	DispScroll(TRUE);
	bSWFlag=FALSE;

	bBlinkFlag = TRUE;
	return ret;
}


