#include "cleaner.h"

void stop(int signal)
{
	keyboard_destroy_request();
	
	keyboard_destroy();
	
	exit(EXIT_FAILURE);
}

