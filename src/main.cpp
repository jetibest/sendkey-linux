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

#include "config.h"
#include "overseer.h"
#include "cleaner.h"
#include "keyboard.h"
#include "eventqueue.h"
#include "stdinparser.h"

using namespace std;

int running = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char **argv)
{
	// usage: echo -en 'echo "Hello World"\n' | sendkey --char --press-ms 50 --release-ms 100
	// usage: echo -en 'press 37\nrelease 37\np39\nr39\n' | sendkey --keycode --press-ms 400 --release-ms 200
	// usage: { echo -en 'press VK_ARROW_RIGHT\n' && sleep 1 && echo -en 'rVK_ARROW_RIGHT\n'; } | sendkey --keyword
	// omitting press or release means toggle, except when --char, it is not possible to control press/release
	
	// TODO: add CFG_DEFAULT options, to change it here with argv/argc parameters
	// TODO: create the help text with examples
	// TODO: create examples directory with test scripts that test if the output is correct when run in terminal (see if key is really pressed)
	// -> instead of the open delay, we might need to check with linux kernel if the device is listened to
	
	signal(SIGINT, stop);
	
	++running; // stdinparser_reader
	
	if(keyboard_create())
	{
		thread t1(stdinparser_reader);
		thread t2(eventqueue_controller);
		thread t3(keyboard_poll);
		
		t1.join();
		t2.join();
		
		struct timespec timeout;
		struct timeval time;
		gettimeofday(&time, nullptr);
		timeout.tv_sec = time.tv_sec;
		timeout.tv_nsec = time.tv_usec * 1000;
		timeout.tv_sec += (CFG_UHID_STOP_TIMEOUT_MS/1000);
		timeout.tv_nsec += (CFG_UHID_STOP_TIMEOUT_MS%1000) * 1000 * 1000;
		
		pthread_mutex_lock(&lock);
		{
			// cerr << "Waiting for keyboard to destroy, requesting.." << endl;
			keyboard_destroy_request();
			
			// wait until keyboard has stopped
			while(keyboard_fd && keyboard_status)
			{
				// or timeout epoch is reached, sometimes EINVAL returns, but why?
				if(pthread_cond_timedwait(&cond, &lock, &timeout))
				//if(res) // == ETIMEDOUT) -> there is also EINVAL and EPERM which may occur, after which we need to break
				{
					cerr << "Forced destroy after timeout" << endl;
					keyboard_destroy();
					break;
				}
				
				// cerr << "Timelock unlocked: " << keyboard_fd << ", " << keyboard_status << endl;
			}
		}
		pthread_mutex_unlock(&lock);
		
		// cerr << "Keyboard gracefully joining with main.." << endl;
		
		t3.join();
	}
	
	// cerr << "Final graceful keyboard destroy.." << endl;
	keyboard_destroy();
	
	return EXIT_SUCCESS;
}

