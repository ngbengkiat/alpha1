// Author: nbk
// Desc  :
// Handler_SysTick() is a 1ms regular ISR. It is used for motor PID, speed profile etc.
// ---------------------------------------------------------------------------------

#include "project.h"

u8 	wait_timer_flag=0;
volatile u32 waitDelay;
uint16_t elapsedTime;
int encoderChangeCnt;

// SysTick timer is setup in stm32_NVIC_Configuration()
// It is a regular 1ms interrupt
void Handler_SysTick(void){
	waitDelay--;
	elapsedTime++;

	// Start IR sensor interrupt
	if (bSensorEnableFlag)
		StartSensorISR();

	DoSpeedProfile();

	MotorPID();

	// alphanumeric display service routine
	Disp_dma_service();

	// User switch service routine
	User_sw_service();

	// Process CMUCAM data
	//CAMRxISR();

	if (bUseEncoderClickFlag) {
		// Check encoder count for user interface
		// Only the right wheel is used
		// This allows the user to change a variable conveniently.
		// Very useful for testing.

		encoderChangeCnt+=encoderSpeed[1];

		if (encoderChangeCnt>20) {
			bEncoderClickFlag = TRUE;
			encoderClickType=0;
			encoderChangeCnt=0;
		}
		if (encoderChangeCnt<-20) {
			bEncoderClickFlag = TRUE;
			encoderClickType=1;
			encoderChangeCnt=0;
		}
		// Provide tactile feedback to user
		// Change the gain according to your preference
		SetPWM1(-encoderChangeCnt/2);
	}

}

#define SysTick_Counter_Disable        ((u32)0xFFFFFFFE)
#define SysTick_Counter_Enable         ((u32)0x00000001)

//SysTick
#define CTRL_TICKINT_Set      ((u32)0x00000002)
#define CTRL_TICKINT_Reset    ((u32)0xFFFFFFFD)

// @brief  delay function
// @param  delay in msec
// @retval None
void DelaymSec(uint32_t time_ms){
	wait_timer_flag=1;
	waitDelay = time_ms;
	while(waitDelay>0)
		if (bSWFlag==1 ) {
			break;
		}
}

void StartElapsedTimer() {
	elapsedTime = 0;
}


uint16_t GetElapsedTime() {
	return elapsedTime;
}
