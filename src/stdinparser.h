#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <linux/uhid.h>
#include <iostream>
#include <thread>
#include <map>
#include <deque>

#ifndef STDINPARSER_H
#define STDINPARSER_H

#include "overseer.h"
#include "cleaner.h"
#include "usb_hid_keys.h"
#include "eventqueue.h"

using namespace std;

// not good to use this, since the actual key depends on the keyboard-layout
// set layout on Xorg system like: setxkbmap -device "$(xinput -list | grep 'sendkey virtual-keyboard' | sed -r -e '/sendkey virtual-keyboard/!d' -e 's/^.*id=([0-9]+).*$/\1/g')" -layout <layout>
// where <layout> is for instance "us" or "dvorak", etc.
static map<string, unsigned char> scankeymap = {
	{"CTRL", HID_KEY_LEFTCTRL},
	{"SHIFT", HID_KEY_LEFTSHIFT},
	{"ALT", HID_KEY_LEFTALT},
	{"META", HID_KEY_LEFTMETA},
	{"SUPER", HID_KEY_LEFTMETA},
	{"CTRL_LEFT", HID_KEY_LEFTCTRL},
	{"SHIFT_LEFT", HID_KEY_LEFTSHIFT},
	{"ALT_LEFT", HID_KEY_LEFTALT},
	{"META_LEFT", HID_KEY_LEFTMETA},
	{"SUPER_LEFT", HID_KEY_LEFTMETA},
	{"CTRL_RIGHT", HID_KEY_RIGHTCTRL},
	{"SHIFT_RIGHT", HID_KEY_RIGHTSHIFT},
	{"ALT_RIGHT", HID_KEY_RIGHTALT},
	{"META_RIGHT", HID_KEY_RIGHTMETA},
	{"SUPER_RIGHT", HID_KEY_RIGHTMETA},
	{"A", HID_KEY_A},
	{"B", HID_KEY_B},
	{"C", HID_KEY_C},
	{"D", HID_KEY_D},
	{"E", HID_KEY_E},
	{"F", HID_KEY_F},
	{"G", HID_KEY_G},
	{"H", HID_KEY_H},
	{"I", HID_KEY_I},
	{"J", HID_KEY_J},
	{"K", HID_KEY_K},
	{"L", HID_KEY_L},
	{"M", HID_KEY_M},
	{"N", HID_KEY_N},
	{"O", HID_KEY_O},
	{"P", HID_KEY_P},
	{"Q", HID_KEY_Q},
	{"R", HID_KEY_R},
	{"S", HID_KEY_S},
	{"T", HID_KEY_T},
	{"U", HID_KEY_U},
	{"V", HID_KEY_V},
	{"W", HID_KEY_W},
	{"X", HID_KEY_X},
	{"Y", HID_KEY_Y},
	{"Z", HID_KEY_Z},
    {"1", HID_KEY_1},
    {"2", HID_KEY_2},
    {"3", HID_KEY_3},
    {"4", HID_KEY_4},
    {"5", HID_KEY_5},
    {"6", HID_KEY_6},
    {"7", HID_KEY_7},
    {"8", HID_KEY_8},
    {"9", HID_KEY_9},
    {"0", HID_KEY_0},
    {"ENTER", HID_KEY_ENTER},
    {"ESC", HID_KEY_ESC},
    {"BACKSPACE", HID_KEY_BACKSPACE},
    {"TAB", HID_KEY_TAB}
};

void parse_keyword(string ln);

void stdinparser_reader();

#endif // STDINPARSER_H
