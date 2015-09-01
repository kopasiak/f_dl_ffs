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

#ifndef _DESCRIPTORS_H
#define _DESCRIPTORS_H

#include <endian.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <linux/usb/functionfs.h>

#include "udl_ffs.h"

struct udl_vendor_data_header {
	__u8 bLength;
	__u8 bDescriptorType;
	__le16 type;
	__u8 length;
} __attribute__ ((__packed__));

struct udl_vendor_data_piece {
	__le16 key;
	__u8 length;
} __attribute__ ((__packed__));

struct udl_vendor_max_pixels {
	struct udl_vendor_data_header header;
	struct udl_vendor_data_piece vendor_header;
	__le32 max_pixels_data;
} __attribute__ ((__packed__));

#define VENDOR_DESC { \
	.header = { \
		.bLength = sizeof(struct udl_vendor_max_pixels), \
		.bDescriptorType = 0x5f, \
		.type = htole16(0x0001), \
		.length = sizeof(struct udl_vendor_max_pixels) - 2, \
	}, \
	.vendor_header = { \
		.key = htole16(0x0200),	/* max_area */ \
		.length = 4, \
	}, \
	.max_pixels_data = htole32(WINDOW_SIZE), \
}

const struct {
	struct usb_functionfs_descs_head_v2 header;
	__le32 fs_count;
	__le32 hs_count;
	struct {
		struct usb_interface_descriptor intf;
		struct udl_vendor_max_pixels vendor;
		struct usb_endpoint_descriptor_no_audio bulk_out_1;
		struct usb_endpoint_descriptor_no_audio int_in_2;
		struct usb_endpoint_descriptor_no_audio bulk_out_10;
	} __attribute__ ((__packed__)) fs_descs, hs_descs;
} __attribute__ ((__packed__)) descriptors = {
	.header = {
		.magic = htole32(FUNCTIONFS_DESCRIPTORS_MAGIC_V2),
		.flags = htole32(FUNCTIONFS_HAS_FS_DESC |
				     FUNCTIONFS_HAS_HS_DESC),
		.length = htole32(sizeof(descriptors)),
	},
	.fs_count = htole32(5),
	.fs_descs = {
		.intf = {
			.bLength = sizeof(descriptors.fs_descs.intf),
			.bDescriptorType = USB_DT_INTERFACE,
			.bInterfaceNumber = 0,
			.bAlternateSetting = 0,
			.bNumEndpoints = 4,
			.bInterfaceClass = USB_CLASS_VENDOR_SPEC,
			.iInterface = 1,
		},
		.vendor = VENDOR_DESC,
		.bulk_out_1 = {
			.bLength = sizeof(descriptors.fs_descs.bulk_out_1),
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 1 | USB_DIR_OUT,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = htole16(512),
		},
		.int_in_2 = {
			.bLength = sizeof(descriptors.fs_descs.int_in_2),
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 2 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_INT,
			.wMaxPacketSize = htole16(8),
			.bInterval = 0x04,
		},
		.bulk_out_10 = {
			.bLength = sizeof(descriptors.fs_descs.bulk_out_10),
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 10 | USB_DIR_OUT,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = htole16(512),
		},

	},
	.hs_count = htole32(5),
	.hs_descs = {
		.intf = {
			.bLength = sizeof(descriptors.hs_descs.intf),
			.bDescriptorType = USB_DT_INTERFACE,
			.bInterfaceNumber = 0,
			.bAlternateSetting = 0,
			.bNumEndpoints = 4,
			.bInterfaceClass = USB_CLASS_VENDOR_SPEC,
			.iInterface = 1,
		},
		.vendor = VENDOR_DESC,
		.bulk_out_1 = {
			.bLength = sizeof(descriptors.hs_descs.bulk_out_1),
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 1 | USB_DIR_OUT,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = htole16(512),
		},
		.int_in_2 = {
			.bLength = sizeof(descriptors.hs_descs.int_in_2),
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 2 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_INT,
			.wMaxPacketSize = htole16(8),
			.bInterval = 0x04,
		},
		.bulk_out_10 = {
			.bLength = sizeof(descriptors.hs_descs.bulk_out_10),
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 10 | USB_DIR_OUT,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = htole16(512),
		},
	},
};

#define STR_VENDOR "DisplayLink"
#define STR_DEVICE "DL-165 Adapter"
#define STR_VERSION "699495"

const struct {
	struct usb_functionfs_strings_head header;
	struct {
		__le16 code;
		const char str1[sizeof(STR_VENDOR)];
		const char str2[sizeof(STR_DEVICE)];
		/* FIXME: Sniffing reports 3rd has some extra padding */
		const char str3[sizeof(STR_VERSION)];
	} __attribute__ ((__packed__)) lang0;
} __attribute__ ((__packed__)) strings = {
	.header = {
		.magic = htole32(FUNCTIONFS_STRINGS_MAGIC),
		.length = htole32(sizeof(strings)),
		.str_count = htole32(3),
		.lang_count = htole32(1),
	},
	.lang0 = {
		htole16(0x0409), /* en-us */
		STR_VENDOR,
		STR_DEVICE,
		STR_VERSION,
	},
};

#endif /* _DESCRIPTORS_H */
