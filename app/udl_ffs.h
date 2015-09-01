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

#ifndef _UDL_FFS_H
#define _UDL_FFS_H

#include "render.h"

#include <libaio.h>
#include <stdbool.h>
#include <stddef.h>

#define WINDOW_SIZE (V_WIDTH * V_HEIGHT)

#ifdef DEBUG
#define debug_print printf
#else
#define debug_print(...) {}
#endif


#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})


struct aio_buf {
	struct iocb cb;
	int size;
	char *buf;
};

struct ep_data {
	int read;
	int fd;
	int (*handler)(unsigned char*, int);
	unsigned aio_buf_size;
	unsigned aio_num;
	struct aio_buf *aio_p;
};

void prepare_aio(
	struct ep_data *ep,
	struct iocb *cb,
	io_context_t *ctx,
	int evfd);
int handle_aio(struct ep_data *ep, struct iocb *cb, int len);

void handle_ep0(int ep0, bool *ready);

int handle_bulk_out_1(unsigned char *buf, int sz);
int handle_bulk_out_2(unsigned char *buf, int sz);
int handle_int_in(unsigned char *buf, int sz);
int cmd_stream(unsigned char *buf, int sz);

#endif /* _UDL_FFS_H */
