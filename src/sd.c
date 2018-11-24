#include "sd.h"

//others
#include <stdio.h>
#include <string.h>

static FATFS g_sFatFs; //obiekt FATFs
FIL file;
FRESULT fresult;
uint8_t detected;
uint8_t need_mount;

void sd_init(void) {
	// default values
	need_mount = 1;

	// check if card is in slot
	detected = sd_card_detected();
}
//
//uint8_t sd_card_detected(void) {
//	uint8_t current = (1-GPIO_ReadInputDataBit(SD_PORT_DETECT, SD_BIT_DETECT));
//
//	if (current != detected) {
//
//		if (current == 1) {
//			need_mount = 1;
//		}
//	}
//
//	return current;
//}

FRESULT sd_mount(void) {

    fresult = f_mount(0, &g_sFatFs);

	//exit if error
    if (fresult) {
		// return error code
		return fresult;
	}

	// change flag
	need_mount = 0;

	return 0;
}

FRESULT sd_open_file(const char* path) {

	// get card state
	detected = sd_card_detected();

	// exit if no card in slot
	if (detected == 0) {
		return 80;
	}

	if (need_mount == 1) {
		sd_mount();
	}

	// open file
	fresult = f_open(&file, path, FA_WRITE | FA_OPEN_ALWAYS);
	
	//exit if error
    if (fresult) {
		// return error code
		return fresult;
	}

	return 0;
}

FRESULT sd_close_file(void) {
	// close file
 	return f_close(&file);
}

uint8_t sd_write_line(const char* path, char* text, uint8_t size, uint8_t append) {

	UINT savedBytes = 0;

	// open file
	fresult = sd_open_file(path);

	if (fresult) {
		return fresult;
	}

	if (append & 0x1 == 1) {
		fresult = f_lseek(&file, file.fsize);
    } 
	
	else {
		fresult = f_lseek(&file, 0);
        fresult = f_truncate(&file);
	}
	
	// write text fo file
    fresult = f_write(&file, text, size, &savedBytes);	           
	
	// if any problems
    if (fresult) {
		// close file
	 	fresult = f_close (&file);

		// return error code
		return fresult;
	}

	// close file
	fresult = sd_close_file();

	if (fresult) {
		return fresult;
	}

	// return no error
	return 0;
}

