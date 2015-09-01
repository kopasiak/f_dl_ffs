/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <endian.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>

#include <libaio.h>
#include <linux/usb/functionfs.h>
#include "udl_ffs.h"
#include "edid.h"
#include "render.h"

static void display_event(struct usb_functionfs_event *event)
{
	static const char *const names[] = {
		[FUNCTIONFS_BIND] = "BIND",
		[FUNCTIONFS_UNBIND] = "UNBIND",
		[FUNCTIONFS_ENABLE] = "ENABLE",
		[FUNCTIONFS_DISABLE] = "DISABLE",
		[FUNCTIONFS_SETUP] = "SETUP",
		[FUNCTIONFS_SUSPEND] = "SUSPEND",
		[FUNCTIONFS_RESUME] = "RESUME",
	};
	switch (event->type) {
	case FUNCTIONFS_BIND:
	case FUNCTIONFS_UNBIND:
	case FUNCTIONFS_ENABLE:
	case FUNCTIONFS_DISABLE:
	case FUNCTIONFS_SETUP:
	case FUNCTIONFS_SUSPEND:
	case FUNCTIONFS_RESUME:
		debug_print("Event %s\n", names[event->type]);
	}
}

void handle_ep0(int ep0, bool *ready)
{
	struct usb_functionfs_event event;
	struct usb_ctrlrequest *req;
	int ret;
	char *buf = NULL;
	int sz = 0;
	int ind;

	ret = read(ep0, &event, sizeof(event));
	if (!ret) {
		perror("unable to read event from ep0");
		return;
	}
	display_event(&event);
	switch (event.type) {
	case FUNCTIONFS_SETUP:
		req = &(event.u.setup);
		if (req->bRequestType & USB_DIR_IN) {
			if ((req->bRequestType & USB_RECIP_MASK) ==
			    USB_RECIP_DEVICE &&
			    (req->bRequestType & USB_TYPE_MASK) ==
			    USB_TYPE_VENDOR &&
			    le16toh(req->wIndex) == 0x00A1) {
				/* EDID request */
				ind = (le16toh(req->wValue) >> 8);
				sz = 2;
				buf = malloc(sz);
				buf[0] = 0x00;
				buf[1] = dummy_edid[ind];
			}
			ret = write(ep0, buf, sz);
			free(buf);
		} else {
			ret = read(ep0, NULL, 0);
		}
		break;

	case FUNCTIONFS_ENABLE:
		*ready = true;
		break;

	case FUNCTIONFS_DISABLE:
		*ready = false;
		break;

	default:
		break;
	}
}

int handle_bulk_out_1(unsigned char *buf, int sz)
{
	return cmd_stream(buf, sz);
}

int handle_bulk_out_2(unsigned char *buf, int sz)
{
	/* UNUSED */
	return 0;
}

int handle_int_in(unsigned char *buf, int sz)
{
	/* UNUSED */
	return 0;
}
