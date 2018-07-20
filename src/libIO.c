// ---------------------------------------------------------------------------------
// File        :libIO.c
// Date        :06Feb2011
// Author      :NgBK
// Description :All other IO initialisation functions are called from InitIO();
// InitIO() should be the first function to be called from main()
// ---------------------------------------------------------------------------------
#include "project.h"

// @brief : Initialise all IO periperhals and port pins
// @param : none
// @retval: none
void InitIO() {
	DI;
	stm32_init();

	// All general input and output port pins
	GPIO_Init_Mode(LED0_PIN, GPIO_Mode_Out_PP);
	GPIO_Init_Mode(LED1_PIN, GPIO_Mode_Out_PP);
	GPIO_Init_Mode(LED2_PIN, GPIO_Mode_Out_PP);
	IO_Write(LED0_PIN,1);
	IO_Write(LED1_PIN,1);
	IO_Write(LED2_PIN,1);


	InitMotor();

	SensorInit();

	// serial port for downloading program and debugging
	UART_init(0, 57600);

	// dotmatrix display or serial display on winM3
   	DispInit();

   	// User switch input
	InitSwitch();

	InitTimer3_servoPWM(TRUE, TRUE, TRUE, TRUE);
	SetTimer3PWM(1,1000);
	SetTimer3PWM(2,1200);
	SetTimer3PWM(3,1500);
	SetTimer3PWM(4,2000);

	EI;


	IO_Write(LED0_PIN,0);
	DelaymSec(200);
	IO_Write(LED1_PIN,0);
	DelaymSec(200);
	IO_Write(LED2_PIN,0);
	DelaymSec(200);
	IO_Write(LED0_PIN,1);
	DelaymSec(200);
	IO_Write(LED1_PIN,1);
	DelaymSec(200);
	IO_Write(LED2_PIN,1);

}
