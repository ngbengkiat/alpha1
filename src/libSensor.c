// ---------------------------------------------------------------------------------
// libAdc.c
// Created on: 15-Jan-2010
// Author: nbk
// Description :
// SensorInit() : To initialise for sensor use
// StartSensorISR() : To trigger sensor ISR routine. This should be called regularly every msec.
// DispAllSensorValues() : Display all sensors value for debugging
// DisableSensor() & EnableSensor() - Enable or disable sensor ISR through flag
// DisplaySensorOnDotMatrix() - To display a sensor value on dot matrix display
// ReadBatteryVolt() - Read and convert battery adc input
// DMA2_Channel4_5_IRQHandler() - 	Sensor ISR, which is actually a DMA interrupt. Need to modify
// 									according to your robot needs.
// ---------------------------------------------------------------------------------

#include "project.h"

volatile uint16_t adc[2][NUM_ADC3_INPUT];
volatile uint16_t sensor[NUM_SENSOR];
volatile uint16_t sensorOld[NUM_SENSOR];
volatile uint16_t sensorMin[NUM_SENSOR];
volatile uint16_t sensorMax[NUM_SENSOR];
volatile uint16_t adc1_dma_buf[16];
volatile uint16_t adc2_dma_buf[16];
volatile uint16_t adc3_dma_buf[16];


volatile int state, substate;
volatile int adcCnt;
bool bDispSensorValue;
bool bEndSensorISRFlag;


bool bSensorEnableFlag;
void CalibrateADC(ADC_TypeDef* ADCx);

// ---------------------------------------------------------------------------------
// @brief : To init ADC and port pins for analog sensors
// @param : none
// @retval: none
// ---------------------------------------------------------------------------------
void SensorInit(void) {

	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	//ADC i/p port pin initialisation
	//Change this according to your design
	//MODIFY HERE

	//PC0,PC1,PC2,PC3 (ADC123_IN10,11,12,13)
	GPIO_Init_Mode(GPIOC, GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3, GPIO_Mode_AIN);
	//PA0,PA1,PA2 (ADC123_IN0,1,2,3)
	GPIO_Init_Mode(GPIOA, GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3, GPIO_Mode_AIN);

	// Transmit pins init
	GPIO_Init_Mode(SEN_TX0, GPIO_Mode_Out_PP );
	GPIO_Init_Mode(SEN_TX12, GPIO_Mode_Out_PP);
	GPIO_Init_Mode(SEN_TX12_H, GPIO_Mode_Out_PP);
	GPIO_Init_Mode(SEN_TX3, GPIO_Mode_Out_PP);

	IO_Write(SEN_TX0, LOW);			// Set transmit pin to OFF
	IO_Write(SEN_TX12, LOW);
	IO_Write(SEN_TX12_H, LOW);
	IO_Write(SEN_TX3, LOW);

	//------------------------------------------------------------------
	// ADC1 is tied to DMA1 channel1
	//------------------------------------------------------------------
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);   /* Enable ADC1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (ADC1->DR);	//JDR1
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) adc1_dma_buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = NUM_ADC1_INPUT;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; // Convert a few channels
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 	//ADC_ExternalTrigInjecConv_None
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = NUM_ADC1_INPUT;
	ADC_Init(ADC1, &ADC_InitStructure);

	// Add all the non-pulsed adc inputs here
	//MODIFY HERE
	ADC_RegularChannelConfig(ADC1, ADC_CH_SEN_FLASH, 1, ADC_SampleTime_28Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_CH_SEN_BATTERY, 2, ADC_SampleTime_28Cycles5);

	ADC_Cmd(ADC1, ENABLE);
	CalibrateADC(ADC1);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		// ADC actively converting all the time
												// result in adc1_dma_buf

	//------------------------------------------------------------------
	// ADC3 is tied to DMA2 channel5
	//------------------------------------------------------------------
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE); /* Enable ADC3 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	DMA_DeInit(DMA2_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (ADC3->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) adc3_dma_buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = NUM_ADC3_INPUT;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA2_Channel5, &DMA_InitStructure);

	DMA_Cmd(DMA2_Channel5, ENABLE);

	ADC_DMACmd(ADC3, ENABLE);

	// Enable dma interrupt on TC5 (transfer complete)
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SENSOR_ISR_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_ITConfig(DMA2_Channel5, DMA_IT_TC, ENABLE);

	/* ADC3 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; // Convert a few channels
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T5_CC3; //TIM5 triggered
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = NUM_ADC3_INPUT;
	ADC_Init(ADC3, &ADC_InitStructure);

	/* ADC3 regular channels configuration */
	// Add all the pulsed adc inputs here
	// MODIFY HERE
	ADC_RegularChannelConfig(ADC3, ADC_CH_SEN_LF, 1, ADC_SampleTime_28Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_CH_SEN_LD, 2, ADC_SampleTime_28Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_CH_SEN_RD, 3, ADC_SampleTime_28Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_CH_SEN_RF, 4, ADC_SampleTime_28Cycles5);

	/* Enable ADC & Calibrate */
	ADC_Cmd(ADC3, ENABLE);
	CalibrateADC(ADC3);

	// No adc interrupt. Use dma interrupt.
	// ADC_ITConfig(ADC3, ADC_IT_EOC, ENABLE);
	ADC_ExternalTrigConvCmd(ADC3, ENABLE);

	InitTimer5_SensorISR();	// For triggering adc conversion
							// DMA will transfer adc result into adc3_dma_buf
							// When the cycle is completed, DMA generates an interrupt

}

// ---------------------------------------------------------------------------------
// @brief : To start sensor isr. ADC3 is triggered by TIM_SensorISR
//		  : ADC1 is software started
// @param : none
// @retval: none
// ---------------------------------------------------------------------------------
void StartSensorISR() {
	state = 0;
	substate = 0;
	bEndSensorISRFlag = FALSE;

	TIM_Cmd(TIM_SensorISR, ENABLE);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//Start ADC1 conversion
}

// StartSensorISR() starts the sensor read-in process.
// TIM_SensorISR(TIM5) is setup to trigger ADC3 conversion.
// ADC3 is setup to convert a few channels at a time. scan mode ON.
// ADC3 is served by dma2 channel-5
// Upon converting all adc channels, the dma will trigger this interrupt.
void DMA2_Channel4_5_IRQHandler(void) {
	DMA_ClearITPendingBit(DMA2_IT_TC5);

	adcCnt++;

	if (substate == 0) {

		// Turn on transmitter
		substate++;
		switch (state) {
		case 0:
			// store dc values of IR sensors
			adc[0][0] = adc3_dma_buf[0];
			adc[0][1] = adc3_dma_buf[1];
			adc[0][2] = adc3_dma_buf[2];
			adc[0][3] = adc3_dma_buf[3];
			IO_Write(SEN_TX0, HIGH);	// turn transmitter ON
			break;
		case 1:
			IO_Write(SEN_TX12, HIGH);
			break;
		case 2:
			IO_Write(SEN_TX3, HIGH);
			break;

		}
	}
	else {
		// turn off transmitter
		substate=0;
		switch (state) {
		case 0:
			// store adc values with tx ON
			adc[1][0] = adc3_dma_buf[0];
			IO_Write(SEN_TX0, LOW);		// turn transmitter OFF
			break;
		case 1:
			adc[1][1] = adc3_dma_buf[1];
			adc[1][2] = adc3_dma_buf[2];
			IO_Write(SEN_TX12, LOW);
			IO_Write(SEN_TX12_H, LOW);

			break;
		case 2:
			adc[1][3] = adc3_dma_buf[3];
			IO_Write(SEN_TX3, LOW);
			bEndSensorISRFlag = TRUE;
			break;

		}
		state++;

	}

	if (bEndSensorISRFlag) {
		int i;

		// Save conversion result from adc1
		senFlash = adc1_dma_buf[0]; 		// flash
		senBattery = adc1_dma_buf[1]; 		// battery

		TIM_Cmd(TIM_SensorISR, DISABLE);
		if (bDispSensorValue == TRUE)
			CollectSensorStatistic();

		for (i = 0; i < NUM_ADC3_INPUT; i++) {
			sensorOld[i] = sensor[i];
			// Subtract DC values
			sensor[i] = adc[1][i] > adc[0][i] ? (adc[1][i] - adc[0][i]) : 0;
		}
	}

}

// ---------------------------------------------------------------------------------
// @brief : To calibrate ADC during initialisation
// @param ADCx: which adc
// @retval: none
// ---------------------------------------------------------------------------------
void CalibrateADC(ADC_TypeDef* ADCx) {

	/* Enable ADCx reset calibaration register */
	ADC_ResetCalibration(ADCx);

	/* Check the end of ADCx reset calibration register */
	while (ADC_GetResetCalibrationStatus(ADCx))
		;

	/* Start ADCx calibaration */
	ADC_StartCalibration(ADCx);

	/* Check the end of ADCx calibration */
	while (ADC_GetCalibrationStatus(ADCx))
		;
}

// @brief : To collect sensors' minimum and maximum values
// @param : none
// @retval: none
void CollectSensorStatistic() {
	uint16_t i;

	for (i = 0; i < NUM_SENSOR; i++) {
		if (sensor[i] > sensorMax[i])
			sensorMax[i] = sensor[i];
		if (sensor[i] < sensorMin[i])
			sensorMin[i] = sensor[i];
	}

}

// ---------------------------------------------------------------------------------
// @brief : To display all sensor values on UART
// @param : none
// @retval: none
// ---------------------------------------------------------------------------------
#define ROW1	4
#define COL1	8
void DispAllSensorValues() {
	int i;

	bDispSensorValue = TRUE;
	clrscr();
	DispDotMatrixRaw(waitMsg, waitMsgSize);
	DelaymSec(500);

	//	SetMotorCtrl(ENABLE_MOTOR);
	//	SetMotorCtrl(FORWARDDIR);
	//	targetXSpeed = 1000;

	//	SetNextCommand( CMD_STRAIGHT_RUN, 0);
	//	curTargetPosX = 0;
	//	finalTargetPosX = CD(1800);


	for (i = 0; i < NUM_SENSOR; i++) {
		sensorMax[i] = 0;
		sensorMin[i] = 4096;
	}



	while (1) {

		gotoxy(COL1, ROW1+2);
		printf("%4u(%4u,%4u,%4u)", sensor[0], sensorMin[0], sensorMax[0],
				adc[0][0]);		// dc value

		gotoxy(COL1+4, ROW1);
		printf("%4u(%4u,%4u,%4u)", sensor[1], sensorMin[1], sensorMax[1],
				adc[0][1]);

		gotoxy(COL1+24, ROW1);
		printf("%4u(%4u,%4u,%4u)", sensor[2], sensorMin[2], sensorMax[2],
				adc[0][2]);

		gotoxy(COL1+28, ROW1+2);
		printf("%4u(%4u,%4u,%4u)", sensor[3], sensorMin[3], sensorMax[3],
				adc[0][3]);

		gotoxy(COL1, ROW1+6);
		printf("Battery %4uV flash=%4d", ReadBatteryVolt(), senFlash);


		if (bSWFlag) {
			bSWFlag = 0;
			DelaymSec(200);
			break;
		}
	}
	//	SetMotorCtrl(DISABLE_MOTOR);
	bDispSensorValue = FALSE;
}

// @brief : To disable IR sensor isr()
void DisableSensor() {
	bSensorEnableFlag = 0;
}

// @brief : To enable IR sensor isr()
void EnableSensor() {
	bSensorEnableFlag = 1;
}

void DisplaySensorOnDotMatrix(int sensorNum) {

	uint16_t value;
	uint16_t *ptr = NULL;

	switch (sensorNum) {
	case 1:
		ptr = (uint16_t *) &sensor[0];
		break;
	case 2:
		ptr = (uint16_t *) &sensor[1];
		break;
	case 3:
		ptr = (uint16_t *) &sensor[2];
		break;
	case 4:
		ptr = (uint16_t *) &sensor[3];
		break;
	case 5:
		ptr = (uint16_t *) &sensor[4];
		break;
	case 6:
		ptr = (uint16_t *) &sensor[5];
		break;
	case 7:
		ptr = (uint16_t *) &sensor[6];
		break;
	case 8:
		ptr = (uint16_t *) &sensor[7];
		break;


	}

	while (1) {
		char s[10];
		value = *ptr;

		sprintf(s, "%04d", value);
		DispDotMatrix(s);
		DelaymSec(50);

		if (bSWFlag) {
			//Beep(SOUND_ACK);
			bSWFlag = 0;
			break;
		}
	}
}

void DisplayBatteryVoltOnDotMatrix() {
	while (1) {
		char s[10];

		sprintf(s, "%04d", ReadBatteryVolt());
		DispDotMatrix(s);
		DelaymSec(100);

		if (bSWFlag) {
			bSWFlag = 0;
			break;
		}
	}
}

// Convert battery adc value into 0.01 volts. So 8.4 volts is returned as 840.
uint16_t ReadBatteryVolt() {
	// R series = 10k & 20k
	// ratio = 0.333
	// adc is 4096 for 3.3V
	// 8.4V -> adc = 8.4V*0.3333/3.3V*4096 = 3476
	// The 2nd parameter needs to be calibrated
	return (senBattery * 840) / 3476;
}

