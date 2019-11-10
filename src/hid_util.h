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

#ifndef HID_UTIL_H
#define HID_UTIL_H

using namespace std;

int uhid_write(int fd, const struct uhid_event *ev);

struct uhid_event uhid_create(string &name, __u32 vendor, __u32 product, unsigned char *report_description);

#endif // HID_UTIL_H
