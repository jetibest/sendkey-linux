#include "eventqueue.h"

using namespace std;

deque<event*> eventqueue;

void eventqueue_controller()
{
	// cerr << "eventqueue_controller started\n";
	bool prev_event_is_key_press = false;
	bool prev_event_is_key_release = false;
	bool send_event = false;
		
	pthread_mutex_lock(&lock);
	
	while(running)
	{
		// Wait until UHID is opened and the event queue is not empty
		// cerr << "Waiting for condition: " << keyboard_status << ", " << eventqueue.size() << endl;
		while(running && (keyboard_status <= 1 || eventqueue.empty()))
		{
			// cout << "eventqueue_controller: Waiting for condition: " << keyboard_status << ", " << eventqueue.size() << endl;
			pthread_cond_wait(&cond, &lock);
			// cout << "eventqueue_controller: Signal received. Checking condition: " << keyboard_status << ", " << eventqueue.size() << endl;
		}
		// cerr << "Condition satisfied\n";
		// cout << "eventqueue_controller: Waking up, keyboard_status or eventqueue size changed: " << running << ", " << keyboard_status << ", " << eventqueue.size() << endl;
		
		// Get next entry from queue
		while(running && keyboard_status > 1 && !eventqueue.empty())
		{
			event* e = eventqueue.front();
			// cout << "eventqueue_controller: grabbing from eventqueue: " << (int) e->type << endl;
			
			if(send_event)
			{
				if(!(
					    (prev_event_is_key_press   && e->type == event::KEY_PRESS)
					 || (prev_event_is_key_release && e->type == event::KEY_RELEASE)
				))
				{
					// cout << "eventqueue_controller: sending intermediate keyboard event, change of type" << endl;
					keyboard_send_event();
					send_event = false;
				}
			}
			
			if(e->type == event::KEY_PRESS)
			{
				key_event* e_key = (key_event*) e;
				// cout << "found keypress event (" << &e << ") with scankey: " << (uint32_t) ((key_event*) e)->scankey << endl;
				// cout << "check: " << (uint32_t) ((key_event) eventqueue[eventqueue.size() - 1]).scankey << endl;
				
				int offset = -1;
				for(int i=0;i<KEYBOARD_MAX_KEYPRESSES;++i)
				{
					unsigned char sc = keyboard_scancodes[i];
					if(sc == e_key->scankey)
					{
						offset = -1;
						break;
					}
					else if(sc == 0u)
					{
						offset = i;
					}
				}
				
				if(offset >= 0)
				{
					keyboard_scancodes[offset] = e_key->scankey;
					// cout << "setting scancode at " << offset << " to " << (uint32_t) e_key->scankey << endl;
					send_event = true;
				}
				prev_event_is_key_press = true;
				prev_event_is_key_release = false;
			}
			else if(e->type == event::KEY_RELEASE)
			{
				key_event* e_key = (key_event*) e;
				// cout << "keyrelease: " << (uint32_t) e_key->scankey << endl;
				
				int offset = -1;
				for(int i=0;i<KEYBOARD_MAX_KEYPRESSES;++i)
				{
					unsigned char sc = keyboard_scancodes[i];
					if(sc == e_key->scankey)
					{
						offset = i;
						keyboard_scancodes[offset] = 0;
						// cout << "resetting scancode at " << offset << endl;
					}
				}
				if(offset >= 0)
				{
					send_event = true;
				}
				prev_event_is_key_release = true;
				prev_event_is_key_press = false;
			}
			else if(e->type == event::SLEEP)
			{
				// chrono sleep_for(e.time_ms);
				time_event* e_time = (time_event*) e;
				// cout << "sleep: " << e_time.time_ms << endl;
				this_thread::sleep_for(chrono::milliseconds(e_time->time_ms));
				
				prev_event_is_key_press = false;
				prev_event_is_key_release = false;
			}
			else
			{
				cerr << "error: Unknown event type." << endl;
				prev_event_is_key_press = false;
				prev_event_is_key_release = false;
			}
			
			delete e;
			eventqueue.pop_front();
			--running;
		}
		
		if(send_event)
		{
			// cout << "sending keyboard event" << endl;
			keyboard_send_event();
			prev_event_is_key_press = false;
			prev_event_is_key_release = false;
		}
		send_event = false;
		
		// cout << "eventqueue_controller: eventqueue emptied" << endl;
		
		// cerr << "eventqueue should be empty now.. " << eventqueue.size() << endl;
	}
	
	pthread_mutex_unlock(&lock);
	
	// cout << "eventqueue_controller: stopped\n";
}

