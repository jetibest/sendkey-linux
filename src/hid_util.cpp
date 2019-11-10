#include "hid_util.h"

using namespace std;

int uhid_write(int fd, const struct uhid_event *ev)
{
	ssize_t ret = write(fd, ev, sizeof(*ev));
	
	if(ret != sizeof(*ev))
	{
		return 1; // -1, then failed to write.. otherwise wrong size was written
	}
	return 0;
}

struct uhid_event uhid_create(string &name, __u32 vendor, __u32 product, unsigned char *report_description)
{
	struct uhid_event ev = {}; // memset(&ev, 0, sizeof(ev));
	
	ev.type = UHID_CREATE;
	strcpy((char*)ev.u.create.name, name.c_str());
	ev.u.create.rd_data = report_description;
	ev.u.create.rd_size = sizeof(report_description);
	ev.u.create.bus = BUS_USB;
	ev.u.create.vendor = vendor; // 0x046B; // American Megatrends
	ev.u.create.product = product; // 0xFF10; // Virtual Keyboard and Mouse
	ev.u.create.version = 0;
	ev.u.create.country = 0;
	
	return ev;
}

