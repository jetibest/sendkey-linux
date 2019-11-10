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

#include "usb_hid_keys.h"
#include "hid_util.h"
#include "keyboard.h"

using namespace std;

int keyboard_create();
int keyboard_receive_event();
int keyboard_send_event();
void keyboard_destroy();

static unsigned char RDESC_USAGE_PAGE = 0x05;
static unsigned char RDESC_USAGE_PAGE_GENERIC_DESKTOP = 0x01;
static unsigned char RDESC_USAGE_PAGE_KEY_CODES = 0x07;
static unsigned char RDESC_USAGE_PAGE_LED = 0x08;

static unsigned char RDESC_USAGE = 0x09;
static unsigned char RDESC_USAGE_KEYBOARD = 0x06;
static unsigned char RDESC_USAGE_MINIMUM = 0x19;
static unsigned char RDESC_USAGE_MAXIMUM = 0x29;
static unsigned char RDESC_USAGE_VENDOR_DEFINED = 0x05;

static unsigned char RDESC_COLLECTION = 0xa1;
static unsigned char RDESC_COLLECTION_APPLICATION = 0x01;

static unsigned char RDESC_REPORT_ID = 0x85;
static unsigned char RDESC_REPORT_SIZE = 0x75;
static unsigned char RDESC_REPORT_COUNT = 0x95;

static unsigned char RDESC_LOGICAL_MINIMUM = 0x15;
static unsigned char RDESC_LOGICAL_MAXIMUM = 0x25; // 26?

static unsigned char RDESC_INPUT = 0x81;
static unsigned char RDESC_INPUT_DATA_ARRAY_ABSOLUTE = 0x00;
static unsigned char RDESC_INPUT_CONSTANT_VARIABLE_ABSOLUTE = 0x01;
static unsigned char RDESC_INPUT_DATA_VARIABLE_ABSOLUTE = 0x02;

static unsigned char RDESC_OUTPUT = 0x91;
static unsigned char RDESC_OUTPUT_DATA_VARIABLE_ABSOLUTE = 0x02;
static unsigned char RDESC_OUTPUT_CONSTANT_VARIABLE_ABSOLUTE = 0x01;

static unsigned char RDESC_FEATURE = 0xB1;
static unsigned char RDESC_FEATURE_DATA_VARIABLE_ABSOLUTE = 0x02;

static unsigned char RDESC_END_COLLECTION = 0xc0;

static unsigned char rdesc[] = {
	RDESC_USAGE_PAGE, RDESC_USAGE_PAGE_GENERIC_DESKTOP,
	RDESC_USAGE, RDESC_USAGE_KEYBOARD,
	
	RDESC_COLLECTION, RDESC_COLLECTION_APPLICATION,
		
		// Collection report ID: 1
		// RDESC_REPORT_ID, 0x01,
		
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
		RDESC_REPORT_COUNT, 0x05,
		RDESC_REPORT_SIZE, 0x01,
		RDESC_USAGE_PAGE, RDESC_USAGE_PAGE_LED,
		RDESC_USAGE_MINIMUM, 0x01,
		RDESC_USAGE_MAXIMUM, 0x05,
		RDESC_OUTPUT_DATA_VARIABLE_ABSOLUTE, RDESC_OUTPUT_DATA_VARIABLE_ABSOLUTE,
		// Num Lock, Caps Lock, Scroll Lock, Compose, Kana
		
		// Output LED byte padding (1x 3 bits)
		RDESC_REPORT_COUNT, 0x01,
		RDESC_REPORT_SIZE, 0x03,
		RDESC_OUTPUT_DATA_VARIABLE_ABSOLUTE, RDESC_OUTPUT_CONSTANT_VARIABLE_ABSOLUTE,
		
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

int keyboard_fd = 0;
int keyboard_status = 0;
unsigned char keyboard_modkeys = 0;
unsigned char keyboard_scancodes[6] = {0};

int keyboard_create()
{
	if(keyboard_fd)
	{
		keyboard_destroy();
	}
	
	// O_RDONLY, O_RDWR, O_WRONLY -> receive events for led events, then O_RDWR
	keyboard_fd = open("/dev/uhid", O_RDWR | O_CLOEXEC);
	if(keyboard_fd < 0)
	{
		cerr << "Cannot open uhid-cdev at: /dev/uhid" << endl;
		return 0; // error on opening UHID
	}
	
	__u32 vendor = 0x046B;
	__u32 product = 0xFF10;
	string name("sendkey virtual-keyboard");
	
	struct uhid_event ev = {}; // uhid_create(name, vendor, product, &rdesc[0]);
	// memset(&ev, 0, sizeof(ev));
	ev.type = UHID_CREATE;
	strcpy((char*)ev.u.create.name, name.c_str());
	ev.u.create.rd_data = rdesc;
	ev.u.create.rd_size = sizeof(rdesc);
	ev.u.create.bus = BUS_USB;
	ev.u.create.vendor = vendor; // 0x046B; // American Megatrends
	ev.u.create.product = product; // 0xFF10; // Virtual Keyboard and Mouse
	ev.u.create.version = 0;
	ev.u.create.country = 0;

	if(uhid_write(keyboard_fd, &ev))
	{
		cerr << "Cannot register UHID." << endl;
		return 0; // error on UHID write
	}
	
	return 1;
}

int keyboard_receive_event()
{
	struct uhid_event ev;
	ssize_t ret;
	
	memset(&ev, 0, sizeof(ev));
	ret = read(keyboard_fd, &ev, sizeof(ev));
	if(ret != sizeof(ev))
	{
		cerr << "error: Error reading from uhid-cdev." << endl;
		// if <0, cannot read uhid-cdev
		// if 0, read HUP on uhid-cdev
		// else: invalid size read from uhid-dev
		return 1;
	}
	
	if(ev.type == UHID_START)
	{
		cerr << "debug: UHID_START" << endl;
		keyboard_status = 1;
	}
	else if(ev.type == UHID_STOP)
	{
		cerr << "debug: UHID_STOP" << endl;
	}
	else if(ev.type == UHID_OPEN)
	{
		cerr << "debug: UHID_OPEN" << endl;
		++keyboard_status;
	}
	else if(ev.type == UHID_CLOSE)
	{
		cerr << "debug: UHID_CLOSE" << endl;
		if(keyboard_status > 1)
		{
			--keyboard_status;
		}
	}
	else if(ev.type == UHID_OUTPUT)
	{
		cerr << "debug: UHID_OUTPUT" << endl;
		// keyboard_handle_output(struct uhid_event *ev)
		// only output are LED events
		// check ev->u.output.size
		// ev->u.output.data[0], first byte is report-ID
		// ev->u.output.rtype is UHID_OUTPUT_REPORT
		// data[1] has LED state for Num Lock, Caps Lock, Scroll Lock, Compose, Kana
	}
	else if(ev.type == UHID_OUTPUT_EV)
	{
		cerr << "debug: UHID_OUTPUT_EV" << endl;
	}
	else
	{
		cerr << "warning: Invalid UHID event." << endl;
	}
	
	return 0;
}

int keyboard_send_event()
{
	struct uhid_event ev;
	
	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_INPUT;
	ev.u.input.size = 8;
	
	// set report ID
	// ev.u.input.data[0] = 0x1;
	
	// set mod keys state (8 bits)
	ev.u.input.data[0] = keyboard_modkeys;
	
	// set reserved byte
	ev.u.input.data[1] = 0;
	
	// set 6x byte-key
	ev.u.input.data[2] = keyboard_scancodes[0];
	ev.u.input.data[3] = keyboard_scancodes[1];
	ev.u.input.data[4] = keyboard_scancodes[2];
	ev.u.input.data[5] = keyboard_scancodes[3];
	ev.u.input.data[6] = keyboard_scancodes[4];
	ev.u.input.data[7] = keyboard_scancodes[5];
	
	return uhid_write(keyboard_fd, &ev);
}

void keyboard_destroy()
{
	if(keyboard_fd)
	{
		struct uhid_event ev;
		memset(&ev, 0, sizeof(ev));
		ev.type = UHID_DESTROY;
		
		if(uhid_write(keyboard_fd, &ev))
		{
			cerr << "Failed to deregister UHID." << endl;
		}
		
		close(keyboard_fd);
		
		keyboard_fd = 0;
	}
}

struct event
{
	static const unsigned char UNDEFINED = 0u;
	static const unsigned char SLEEP = 1u;
	static const unsigned char KEY_PRESS = 2u;
	static const unsigned char KEY_RELEASE = 3u;
	
	unsigned char type;
	unsigned char scankey;
	unsigned int time_ms;
	
	event(unsigned char type, unsigned char scankey)
	{
		this->type = type;
		this->scankey = scankey;
	}
	event(unsigned char type, unsigned int time_ms)
	{
		this->type = type;
		this->scankey = scankey;
		this->time_ms = time_ms;
	}
};
deque<event> eventqueue;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void stop(int signal)
{
	keyboard_destroy();
	
	exit(EXIT_FAILURE);
}

map<string, unsigned char> scankeymap = {
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
	{"Z", HID_KEY_Z}
};

void parse_keyword(string ln)
{
	if(ln.length() == 0)
	{
		return;
	}
	
	bool action_press = ln[0] == 'p';
	bool action_release = ln[0] == 'r';
	bool action_tap = ln[0] == 't';
	
	string kw_extracted = "";
	for(int i=3;i+1<ln.length();++i)
	{
		if(ln[i-2] == 'V' && ln[i-1] == 'K' && ln[i] == '_')
		{
			kw_extracted = ln.substr(i + 1);
			break;
		}
	}
	
	// find scankey from keyword
	unsigned char key = scankeymap[kw_extracted];
	
	if(action_tap)
	{
		eventqueue.push_back(event(event::KEY_PRESS, key));
		eventqueue.push_back(event(event::SLEEP, 200));
		eventqueue.push_back(event(event::KEY_RELEASE, key));
	}
	else if(action_press)
	{
		// if last one is eventqueue is sleep, and that is key press, then put it there.. otherwise push to back
		// but the sleep duration should be the same
		if(eventqueue.size() > 1 && eventqueue[eventqueue.size() - 1],type == event::SLEEP && eventqueue[eventqueue.size() - 2].type == event::KEY_PRESS && eventqueue[eventqueue.size() - 1].time_ms == 100)
		{
			eventqueue.insert(eventqueue.size() - 1, event(event::KEY_PRESS, key));
		}
		else
		{
			eventqueue.push_back(event(event::KEY_PRESS, key));
			eventqueue.push_back(event(event::SLEEP, 100));
		}
		
		pthread_mutex_lock(&lock);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock);
	}
	else if(action_release)
	{
		// if last one in eventqueue is sleep, add in front of sleep, to combine multiple keypresses
		if(eventqueue.size() > 1 && eventqueue[eventqueue.size() - 1],type == event::SLEEP && eventqueue[eventqueue.size() - 2].type == event::KEY_RELEASE && eventqueue[eventqueue.size() - 1].time_ms == 100)
		{
			eventqueue.insert(eventqueue.size() - 1, event(event::KEY_RELEASE, key));
		}
		else
		{
			eventqueue.push_back(event(event::KEY_RELEASE, key));
			eventqueue.push_back(event(event::SLEEP, 100));
		}
		
		pthread_mutex_lock(&lock);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock);
	}
}

void poll_uhid()
{
	struct pollfd pfds[1];
	
	// pfds[0].fd = STDIN_FILENO;
	// pfds[0].events = POLLIN;
	pfds[0].fd = keyboard_fd;
	pfds[0].events = POLLIN;
	
	while(1)
	{
		int ret = poll(pfds, 1, -1);
		if(ret < 0)
		{
			cerr << "error: Failed to poll pfds." << endl;
			break;
		}
		if(pfds[0].revents & POLLHUP)
		{
			cerr << "warning: Received HUP on uhid-cdev." << endl;
		}
		if(pfds[0].revents & POLLIN)
		{
			keyboard_receive_event();
			pthread_mutex_lock(&lock);
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&lock);
		}
	}
	
	stop(1);
}

void read_stdin()
{
	string ln;
	while(getline(cin, ln))
	{
		// pause parsing until keyboard_status becomes >1 (UHID is opened by some client)
		parse_keyword(ln);
	}
	
	stop(1);
}

void controller()
{
	unsigned char prev_event_type = event::UNKNOWN;
	while(true)
	{
		// Wait until UHID is opened and the event queue is not empty
		cout << "Waiting for condition" << endl;
		pthread_mutex_lock(&lock);
		while(keyboard_status <= 1 || eventqueue.empty())
		{
			pthread_cond_wait(&cond, &lock);
			cout << "Checking condition: " << keyboard_status << endl;
		}
		pthread_mutex_unlock(&lock);
		cout << "Condition satisfied" << endl;
		
		// Get next entry from queue
		while(keyboard_status > 1 && !eventqueue.empty())
		{
			event e = eventqueue.pop_front();
			
			if(prev_event_type != event::UNKNOWN && prev_event_type != e.type)
			{
				prev_event_type = e.type;
				keyboard_send_event();
			}
			
			if(e.type == event::KEY_PRESS)
			{
				int offset = -1;
				for(int i=0;i<6;++i)
				{
					unsigned char sc = keyboard_scancodes[i];
					if(sc == key)
					{
						return;
					}
					else if(sc == 0u)
					{
						offset = i;
					}
				}
				
				if(offset >= 0)
				{
					keyboard_scancodes[offset] = key;
					cout << "setting scancode at " << offset << " to " << (uint32_t) key << endl;
					keyboard_send_event();
				}
			}
			else if(e.type == event::KEY_RELEASE)
			{
				int offset = -1;
				for(int i=0;i<6;++i)
				{
					unsigned char sc = keyboard_scancodes[i];
					if(sc == key)
					{
						offset = i;
						keyboard_scancodes[offset] = 0;
						cout << "resetting scancode at " << offset << endl;
					}
				}
				if(offset >= 0)
				{
					keyboard_send_event();
				}
			}
			else if(e.type == event::SLEEP)
			{
				// chrono sleep_for(e.time_ms);
			}
			// if entry is a pause, then chrono sleep_for
			// if entry is a key event, apply and send uhid event
		}
		
		prev_event_type = event::UNKNOWN;
		keyboard_send_event();
	}
}

int main(int argc, char **argv)
{
	// usage: echo -en 'echo "Hello World"\n' | sendkey --char --press-ms 50 --release-ms 100
	// usage: echo -en 'press 37\nrelease 37\np39\nr39\n' | sendkey --keycode --press-ms 400 --release-ms 200
	// usage: { echo -en 'press VK_ARROW_RIGHT\n' && sleep 1 && echo -en 'rVK_ARROW_RIGHT\n'; } | sendkey --keyword
	// omitting press or release means toggle, except when --char, it is not possible to control press/release
	
	signal(SIGINT, stop);
	
	if(keyboard_create())
	{
		thread t1(poll_uhid);
		thread t2(read_stdin);
		thread t3(controller);
		
		t1.join();
		t2.join();
		t3.join();
		
	}
	keyboard_destroy();
	
	return EXIT_SUCCESS;
}

