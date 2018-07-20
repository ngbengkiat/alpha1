// ---------------------------------------------------------------------------------
// libSensor.h
// Created on: 22-Feb-2011
// Author: nbk
// ---------------------------------------------------------------------------------

#ifndef LIBSENSOR_H_
#define LIBSENSOR_H_

// ---------------------------------------------------------------------------------
//  Macros
// ---------------------------------------------------------------------------------
// These sensors need to be actively pulsed. They are converted thru ADC3
#define ADC_CH_SEN_LF		ADC_Channel_12
#define ADC_CH_SEN_RF		ADC_Channel_13
#define ADC_CH_SEN_LD		ADC_Channel_0
#define ADC_CH_SEN_RD		ADC_Channel_1
#define NUM_ADC3_INPUT		4

// These sensors provide analog output directly. They are converted thru ADC1
#define ADC_CH_SEN_FLASH	ADC_Channel_2
#define ADC_CH_SEN_BATTERY	ADC_Channel_3
#define NUM_ADC1_INPUT		4

#define NUM_SENSOR		(NUM_ADC3_INPUT+NUM_ADC1_INPUT)

// IO pins used for IR sensor transmit
#define SEN_TX0			GPIOC,GPIO_Pin_13
#define SEN_TX12		GPIOB,GPIO_Pin_8
#define SEN_TX12_H		GPIOB,GPIO_Pin_9
#define SEN_TX3			GPIOC,GPIO_Pin_14

// Macros to give meaningful names to the sensors
#define senLeftFront		sensor[0]
#define senLeftDiag			sensor[1]
#define senRightDiag		sensor[2]
#define senRightFront		sensor[3]
#define senFlash			sensor[4]
#define senBattery			sensor[5]

// ---------------------------------------------------------------------------------
//  Global variables
// ---------------------------------------------------------------------------------
extern volatile uint16_t adc1_dma_buf[16];	// For adc->dma buffer
extern volatile uint16_t adc2_dma_buf[16];	// max-16 inputs
extern volatile uint16_t adc3_dma_buf[16];	// Each adc has it's own buffer

extern volatile uint16_t adc3[2][NUM_ADC3_INPUT];		// For storing the adc values
extern volatile uint16_t sensor[NUM_SENSOR];		// actual sensor values
extern volatile uint16_t sensorOld[NUM_SENSOR];

extern char dispBuffer[256];
extern bool	bDispSensorValue;
extern bool bSensorEnableFlag;
extern bool bFlashDetectFlag;
extern bool bEndSensorISRFlag;

// ---------------------------------------------------------------------------------
//  Function prototypes
// ---------------------------------------------------------------------------------
void SensorInit(void);
void StartSensorISR(void);
void DispAllSensorValues(void);
void CollectSensorStatistic();
void EnableSensor();
void DisableSensor();
void DisplaySensorOnDotMatrix(int sensorNum);
void DisplayBatteryVoltOnDotMatrix();
uint16_t ReadBatteryVolt();

#endif /* LIBSENSOR_H_ */
