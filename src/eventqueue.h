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
#include <chrono>
#include <map>
#include <deque>

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "overseer.h"
#include "cleaner.h"
#include "keyboard.h"

using namespace std;

struct event
{
	static const unsigned char UNDEFINED = 0u;
	static const unsigned char SLEEP = 1u;
	static const unsigned char KEY_PRESS = 2u;
	static const unsigned char KEY_RELEASE = 3u;
	
	unsigned char type;
};
struct key_event : event
{
	unsigned char scankey;
	
	key_event(event e)
	{
		this->type = e.type;
	}
	key_event(unsigned char type, unsigned char scankey)
	{
		this->type = type;
		this->scankey = scankey;
	}
};
struct time_event : event
{
	unsigned int time_ms;
	
	time_event(event e)
	{
		this->type = e.type;
	}
	time_event(unsigned char type, unsigned int time_ms)
	{
		this->type = type;
		this->time_ms = time_ms;
	}
};

extern deque<event*> eventqueue;

void eventqueue_controller();

#endif // EVENTQUEUE_H

