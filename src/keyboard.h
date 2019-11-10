#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/uhid.h>
#include <iostream>
#include <string>

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "config.h"
#include "cleaner.h"
#include "eventqueue.h"
#include "hid_util.h"

using namespace std;

static const unsigned int KEYBOARD_MAX_KEYPRESSES = 6;

#define rdescbyte static unsigned char

rdescbyte RDESC_USAGE_PAGE = 0x05;
rdescbyte RDESC_USAGE_PAGE_GENERIC_DESKTOP = 0x01;
rdescbyte RDESC_USAGE_PAGE_KEY_CODES = 0x07;
rdescbyte RDESC_USAGE_PAGE_LED = 0x08;

rdescbyte RDESC_USAGE = 0x09;
rdescbyte RDESC_USAGE_KEYBOARD = 0x06;
rdescbyte RDESC_USAGE_MINIMUM = 0x19;
rdescbyte RDESC_USAGE_MAXIMUM = 0x29;
rdescbyte RDESC_USAGE_VENDOR_DEFINED = 0x05;

rdescbyte RDESC_COLLECTION = 0xa1;
rdescbyte RDESC_COLLECTION_APPLICATION = 0x01;

rdescbyte RDESC_REPORT_ID = 0x85;
rdescbyte RDESC_REPORT_SIZE = 0x75;
rdescbyte RDESC_REPORT_COUNT = 0x95;

rdescbyte RDESC_LOGICAL_MINIMUM = 0x15;
rdescbyte RDESC_LOGICAL_MAXIMUM = 0x25; // 26?

rdescbyte RDESC_INPUT = 0x81;
rdescbyte RDESC_INPUT_DATA_ARRAY_ABSOLUTE = 0x00;
rdescbyte RDESC_INPUT_CONSTANT_VARIABLE_ABSOLUTE = 0x01;
rdescbyte RDESC_INPUT_DATA_VARIABLE_ABSOLUTE = 0x02;

rdescbyte RDESC_OUTPUT = 0x91;
rdescbyte RDESC_OUTPUT_DATA_VARIABLE_ABSOLUTE = 0x02;
rdescbyte RDESC_OUTPUT_CONSTANT_VARIABLE_ABSOLUTE = 0x01;

rdescbyte RDESC_FEATURE = 0xB1;
rdescbyte RDESC_FEATURE_DATA_VARIABLE_ABSOLUTE = 0x02;

rdescbyte RDESC_END_COLLECTION = 0xc0;

static unsigned char rdesc[] = {
	RDESC_USAGE_PAGE, RDESC_USAGE_PAGE_GENERIC_DESKTOP,
	RDESC_USAGE, RDESC_USAGE_KEYBOARD,
	
	RDESC_COLLECTION, RDESC_COLLECTION_APPLICATION,
		
		// Collection report ID: 1
//		RDESC_REPORT_ID, 0x01,
		
		// Input Modifier byte (8x 1 bit)
		RDESC_REPORT_SIZE, 0x01,
		RDESC_REPORT_COUNT, 0x08,
		RDESC_USAGE_PAGE, RDESC_USAGE_PAGE_KEY_CODES,
		RDESC_USAGE_MINIMUM, 0xe0, // 224
		RDESC_USAGE_MAXIMUM, 0xe7, // 231
		RDESC_LOGICAL_MINIMUM, 0x00,
		RDESC_LOGICAL_MAXIMUM, 0x01,
		RDESC_INPUT, RDESC_INPUT_DATA_VARIABLE_ABSOLUTE,
		// left control = 0xe0
		// left shift = 0xe1
		// left alt = 0xe2
		// left meta = 0xe3
		// right control = 0xe4
		// right shift = 0xe5
		// right alt = 0xe6
		// right meta = 0xe7
		
		// Input Reserved byte (1x 8 bits)
		RDESC_REPORT_COUNT, 0x01,
		RDESC_REPORT_SIZE, 0x08,
		RDESC_INPUT, RDESC_INPUT_CONSTANT_VARIABLE_ABSOLUTE,
		
		// Output LED byte (5x 1 bit)
//		RDESC_REPORT_COUNT, 0x05,
//		RDESC_REPORT_SIZE, 0x01,
//		RDESC_USAGE_PAGE, RDESC_USAGE_PAGE_LED,
//		RDESC_USAGE_MINIMUM, 0x01,
//		RDESC_USAGE_MAXIMUM, 0x05,
//		RDESC_OUTPUT_DATA_VARIABLE_ABSOLUTE, RDESC_OUTPUT_DATA_VARIABLE_ABSOLUTE,
		// Num Lock, Caps Lock, Scroll Lock, Compose, Kana
		
		// Output LED byte padding (1x 3 bits)
//		RDESC_REPORT_COUNT, 0x01,
//		RDESC_REPORT_SIZE, 0x03,
//		RDESC_OUTPUT_DATA_VARIABLE_ABSOLUTE, RDESC_OUTPUT_CONSTANT_VARIABLE_ABSOLUTE,
		
		// Input Keycodes (6x 8 bits)
		RDESC_REPORT_COUNT, 0x06,
		RDESC_REPORT_SIZE, 0x08,
		RDESC_LOGICAL_MINIMUM, 0x00,
		RDESC_LOGICAL_MAXIMUM, 0xff, // 255
		RDESC_USAGE_PAGE, RDESC_USAGE_PAGE_KEY_CODES,
		RDESC_USAGE_MINIMUM, 0x00,
		RDESC_USAGE_MAXIMUM, 0xff, // 255
		RDESC_INPUT, RDESC_INPUT_DATA_ARRAY_ABSOLUTE,
		
		// RDESC_REPORT_COUNT, 0x08,
		// RDESC_REPORT_SIZE, 0x02,
		// RDESC_USAGE, RDESC_USAGE_VENDOR_DEFINED,
		// RDESC_LOGICAL_MINIMUM, 0x00,
		// RDESC_LOGICAL_MAXIMUM, 0xff, 0x00, // 255
		// RDESC_FEATURE, RDESC_FEATURE_DATA_VARIABLE_ABSOLUTE,
		
	
	RDESC_END_COLLECTION
};

extern int keyboard_fd;
extern int keyboard_status;
extern unsigned char keyboard_modkeys;
extern unsigned char keyboard_scancodes[KEYBOARD_MAX_KEYPRESSES];

int keyboard_create();

int keyboard_receive_event();

int keyboard_send_event();

void keyboard_destroy_request();

void keyboard_destroy();

void keyboard_poll();

#endif // KEYBOARD_H

