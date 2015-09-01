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

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>

#include "udl_ffs.h"
#include "render.h"

/* The driver is nice to us and never makes a command cross buffer boundary */
int cmd_stream(unsigned char *buf, int len)
{
	int render = 0;
	unsigned int i = 0, j;
	static void *v_buf;
	static struct timeval last_render = {};
	struct timeval now, diff;
	int prev_i = 0;

	gettimeofday(&now, NULL);
	timersub(&now, &last_render, &diff);
	if (diff.tv_sec != 0 || diff.tv_usec >= 1000 * 30) {
		last_render = now;
		render = 1;
		debug_print("Refreshing\n");
	}
	if (v_buf == NULL)
		v_buf = render_start();

	debug_print("stream start\n");

	while (i < len) {
		if (buf[i] != 0xaf) {
			debug_print("ERR: bad cmd header! i = %d, prev = %d\n",
			       i, prev_i);
			debug_print("Previous cmd dump:\n");
			debug_print("len %d\n", len);
			for (j = prev_i; j < i ; j++)
				debug_print("%x ", buf[j]);
			debug_print("\n");
			return -1;
		}
		prev_i = i;
		i++;	/* AF */
		if (i == len)
			break;
		switch (buf[i]) {
		case 0x6b:
			i++;
			i += cmd_render(buf + i, v_buf);
			break;

		case 0xaf:
			/* no-op */
			break;

		case 0x20:
			/* Config & modeset ; skip everything */
			i += 3;
			break;

		case 0x6a:
			/* Dummy */
			i += 8;
			break;

		default:
			/* Unused by udl driver */
			printf("ERR: unknown opcode %x!\n", buf[i]);
			return -1;
		}
	}
	if (render) {
		render_end();
		v_buf = NULL;
	}
	debug_print("stream end\n");
	return 0;
}
