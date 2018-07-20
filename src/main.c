// ---------------------------------------------------------------------------------
//   File        :main.c
//   Version     :v1.0.1
//   Date        :06Feb2011
//   Author      :NgBK
//   Description :main()
// ---------------------------------------------------------------------------------

//......................................................................................
// To do list (reminder on what to do in this project)
// test input port pins
// buzzer
// flash data
//......................................................................................

#include "project.h"

int main(void){
	InitIO();	// Must be initialised before any printf()

	clrscr();

   	puts("I'm is alive!");
   	printf("\np%d %d %d %d", CNT_PER_CM, CNT_PER_CM_PER_MS, CNT_PER_360DEG, DIST_deg_oc(360));
   	printf("\na%d %d %d %d", SPEED_deg_oc(360), ACC_deg_oc(360), SPEED_mm_oc(1000), ACC_mm_oc(1000));

   	InitDataCold();

	InitDataWarm();

   	//ReadDataFromFlash();

    //StartCMUCAM(4, 57600);

  //  PrintCamValues();
   // CAM_to_WinM3();
    TestMotorMenu();
	MainMenu();

	return 0;
}
