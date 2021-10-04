#include "stdinparser.h"
#include "xkb_keycodes.h"

using namespace std;

void parse_keyword(string ln)
{
	if(ln.length() == 0)
	{
		return;
	}
	
	bool action_press = ln[0] == 'p';
	bool action_release = ln[0] == 'r';
	bool action_tap = ln[0] == 't';
	
	unsigned char key = 0x00;
	
    for(int i=3;i+1<ln.length();++i)
    {
        // VK_<KEY_NAME>
        if(ln[i-2] == 'V' && ln[i-1] == 'K' && ln[i] == '_')
        {
            key = scankeymap[ln.substr(i + 1)];
            break;
        }
    }
    
    if(key == 0x00)
    {
        for(int i=4;i+1<ln.length();++i)
        {
            // xkb#<UINT>
            if(ln[i-3] == 'x' && ln[i-2] == 'k' && ln[i-1] == 'b' && ln[i] == '#')
            {
                key = xkb_keycodes[static_cast<unsigned char>(std::strtoul(ln.substr(i + 1).c_str(), nullptr, 10))];
                break;
            }
        }
    }
    
    if(key == 0x00)
    {
        // i.e. p 0x04, dit zijn HID codes (todo: detect decimal vs hexadecimal)
        key = static_cast<unsigned char>(std::strtoul(ln.substr(1).c_str(), nullptr, 16));
    }
	
	cerr << "key: " << key << "\n";
	
	if(action_tap)
	{
		eventqueue.push_back(new key_event(event::KEY_PRESS, key));
		++running;
		// cerr << "running + 1\n";
		eventqueue.push_back(new time_event(event::SLEEP, CFG_DEFAULT_TAP_DURATION_MS));
		++running;
		// cerr << "running + 1\n";
		eventqueue.push_back(new key_event(event::KEY_RELEASE, key));
		++running;
		eventqueue.push_back(new time_event(event::SLEEP, CFG_DEFAULT_TAP_INTERVAL_MS));
		++running;
		// cerr << "running + 1\n";
		
		// cerr << "tap added\n";
	}
	else if(action_press)
	{
		// if last one is eventqueue is sleep, and that is key press, then put it there.. otherwise push to back
		// but the sleep duration should be the same
		if(eventqueue.size() > 1 && eventqueue[eventqueue.size() - 1]->type == event::SLEEP && eventqueue[eventqueue.size() - 2]->type == event::KEY_PRESS && ((time_event*) eventqueue[eventqueue.size() - 1])->time_ms == CFG_DEFAULT_PRESS_DURATION_MS)
		{
			deque<event*>::iterator it = eventqueue.end();
			--it;
			eventqueue.insert(it, new key_event(event::KEY_PRESS, key));
			++running;
			// cerr << "running + 1\n";
			// cerr << "extra press added\n";
		}
		else
		{
			eventqueue.push_back(new key_event(event::KEY_PRESS, key));
			++running;
			// cerr << "running + 1\n";
			eventqueue.push_back(new time_event(event::SLEEP, CFG_DEFAULT_PRESS_DURATION_MS));
			++running;
			// cerr << "running + 1\n";
			// cerr << "press added\n";
		}
	}
	else if(action_release)
	{
		// if last one in eventqueue is sleep, add in front of sleep, to combine multiple keypresses
		if(eventqueue.size() > 1 && eventqueue[eventqueue.size() - 1]->type == event::SLEEP && eventqueue[eventqueue.size() - 2]->type == event::KEY_RELEASE && ((time_event*) eventqueue[eventqueue.size() - 1])->time_ms == CFG_DEFAULT_RELEASE_DURATION_MS)
		{
			deque<event*>::iterator it = eventqueue.end();
			--it;
			eventqueue.insert(it, new key_event(event::KEY_RELEASE, key));
			++running;
			// cerr << "running + 1\n";
			// cerr << "extra release added\n";
		}
		else
		{
			eventqueue.push_back(new key_event(event::KEY_RELEASE, key));
			++running;
			// cerr << "running + 1\n";
			eventqueue.push_back(new time_event(event::SLEEP, CFG_DEFAULT_RELEASE_DURATION_MS));
			++running;
			// cerr << "running + 1\n";
			// cerr << "release added\n";
		}
	}
}

void stdinparser_reader()
{
	// ++running; in main
	// cerr << "stdinparser_reader started\n";
	
    // initialized keycodes conversion for USB HID usage ID's to XBD keycodes for X.org server
    xkb_keycodes_init();
    
	string ln;
	while(getline(cin, ln))
	{
		// pause parsing until keyboard_status becomes >1 (UHID is opened by some client)
		pthread_mutex_lock(&lock);
		
		// int prevcount = eventqueue.size();
		parse_keyword(ln);
		// cout << "stdinparser added " << eventqueue.size() - prevcount << " to eventqueue" << endl;
		
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock);
	}
	
	// cerr << "running - 1 (stdinparser)\n";
	--running;
	// cerr << "stdinparser_reader stopped\n";
}

