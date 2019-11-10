#include "keyboard.h"

using namespace std;

// only can use this if in header is set to extern
int keyboard_fd = 0;
int keyboard_status = 0;
unsigned char keyboard_modkeys = 0;
unsigned char keyboard_scancodes[KEYBOARD_MAX_KEYPRESSES] = {0};

bool keyboard_destroy_requested = false;

int keyboard_create()
{
	if(keyboard_fd)
	{
		keyboard_destroy_request();
		keyboard_destroy();
	}
	
	keyboard_status = 0;
	keyboard_destroy_requested = false;
	keyboard_modkeys = 0;
	for(int i=0;i<KEYBOARD_MAX_KEYPRESSES;++i)
	{
		keyboard_scancodes[i] = 0;
	}
	
	// O_RDONLY, O_RDWR, O_WRONLY -> receive events for led events, then O_RDWR
	keyboard_fd = open("/dev/uhid", O_RDWR | O_CLOEXEC);
	if(keyboard_fd < 0)
	{
		cerr << "Cannot open uhid-cdev at: /dev/uhid. Do you have root permissions? Try sudo." << endl;
		return 0; // error on opening UHID
	}
	
	__u32 vendor = CFG_KEYBOARD_VENDOR;
	__u32 product = CFG_KEYBOARD_PRODUCT;
	string name(CFG_KEYBOARD_NAME);
	
	struct uhid_event ev = {0}; // uhid_create(name, vendor, product, &rdesc[0]);
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
		keyboard_status = 0;
		
		// keyboard status changed to zero notification
		pthread_mutex_lock(&lock);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock);
	}
	else if(ev.type == UHID_OPEN)
	{
		cerr << "debug: UHID_OPEN" << endl;
		if(keyboard_status > 0)
		{
			// Wait at least ~100ms before sending events
			eventqueue.push_front(new time_event(event::SLEEP, CFG_DEFAULT_UHID_OPEN_DELAY_MS));
			++running;
			
			++keyboard_status;
		}
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
	ev.u.input.size = 2 + KEYBOARD_MAX_KEYPRESSES;
	
	// set report ID
	// ev.u.input.data[0] = 0x1;
	
	// set mod keys state (8 bits)
	ev.u.input.data[0] = keyboard_modkeys;
	
	// set reserved byte
	ev.u.input.data[1] = 0;
	
	cout << "Sending keyboard events: ";
	
	// set 6x byte-key
	for(int i=0;i<KEYBOARD_MAX_KEYPRESSES;++i)
	{
		if(i != 0)
		{
			cout << ", ";
		}
		cout << (uint32_t) keyboard_scancodes[i];
		ev.u.input.data[2 + i] = keyboard_scancodes[i];
	}
	
	cout << endl;
	
	return uhid_write(keyboard_fd, &ev);
}

void keyboard_destroy_request()
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
		
		keyboard_destroy_requested = true;
	}
}

void keyboard_destroy()
{
	if(keyboard_fd)
	{
		close(keyboard_fd);
		keyboard_fd = 0;
		keyboard_status = 0;
	}
}

void keyboard_poll()
{
	// cerr << "keyboard_poll started\n";
	
	struct pollfd pfds[1];
	
	// pfds[0].fd = STDIN_FILENO;
	// pfds[0].events = POLLIN;
	pfds[0].fd = keyboard_fd;
	pfds[0].events = POLLIN;
	
	// destroy keyboard to stop poll
	while(keyboard_fd && (keyboard_status || !keyboard_destroy_requested))
	{
		int ret = poll(pfds, 1, -1);
		if(ret < 0)
		{
			cerr << "error: Failed to poll pfds." << endl;
			break;
		}
		else if(pfds[0].revents & POLLHUP)
		{
			cerr << "warning: Received HUP on uhid-cdev." << endl;
		}
		else if(pfds[0].revents & POLLIN)
		{
			if(keyboard_receive_event())
			{
				break;
			}
			
			pthread_mutex_lock(&lock);
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&lock);
		}
		else
		{
			cerr << "error: Unexpected poll event." << endl;
			break;
		}
	}
	
	keyboard_status = 0;
	
	// cerr << "keyboard_poll stopped\n";
}


