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

#define _BSD_SOURCE /* for endian.h */

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/eventfd.h>

#include <libaio.h>
#define IOCB_FLAG_RESFD         (1 << 0)

#include <linux/usb/functionfs.h>

#include "udl_ffs.h"
#include "descriptors.h"
#include "render.h"

static struct ep_data eps[] = {
	{
		.read = 1,
		.handler = handle_bulk_out_1,
		/* udl uses PAGE_SIZE * 16, but things break on larger
		 * displays, this doesn't break for 640x480 */
		.aio_buf_size = 1024 * 1024 * 2,
		.aio_num = 4,
		.aio_p = NULL,
	},
	{
		.read = 0,
		.handler = handle_int_in,
		.aio_buf_size = 0,
		.aio_num = 0,
		.aio_p = NULL,
	},
	{
		.read = 1,
		.handler = handle_bulk_out_2,
		.aio_buf_size = 0,
		.aio_num = 0,
		.aio_p = NULL,
	},
};

static int open_eps(int *ep0, const char *path) {
	char *ep_path;
	int i, j;

	ep_path = malloc(strlen(path) + 4 /* "/ep#" */ + 1 /* '\0' */);
	if (!ep_path) {
		perror("malloc");
		return 1;
	}

	/* open endpoint files */
	sprintf(ep_path, "%s/ep0", path);
	*ep0 = open(ep_path, O_RDWR);
	if (*ep0 < 0) {
		perror("unable to open ep0");
		return 1;
	}
	if (write(*ep0, &descriptors, sizeof(descriptors)) < 0) {
		perror("unable do write descriptors");
		return 1;
	}
	if (write(*ep0, &strings, sizeof(strings)) < 0) {
		perror("unable to write strings");
		return 1;
	}
	for (i = 0; i < 3; ++i) {
		sprintf(ep_path, "%s/ep%d", path, i+1);
		eps[i].fd = open(ep_path, O_RDWR);
		if (eps[i].fd < 0) {
			printf("unable to open ep%d: %s\n", i+1,
			       strerror(errno));
			for (j = 0; j < i; j++)
				close(eps[j].fd);
			close(*ep0);
			return 1;
		}
	}

	free(ep_path);
	return 0;
}

int main(int argc, char *argv[])
{
	int i, j, ret = 0;
	int res = 0;

	int ep0;

	int max_events = 0;
	io_context_t ctx;

	int evfd;
	fd_set rfds;

	bool ready = 0;
	int cb_num = 0;
	struct iocb **cbs_to_submit;

	struct aio_buf *buf;

	if (argc != 2) {
		printf("ffs directory not specified!\n");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		max_events += eps[i].aio_num;
		eps[i].aio_p = calloc(eps[i].aio_num, sizeof(*eps[i].aio_p));
		if (!eps[i].aio_p) {
			perror("malloc");
			res = 1;
			goto free;
		}
		for (j = 0; j < eps[i].aio_num ; j++) {
			buf = eps[i].aio_p + j;
			buf->size = eps[i].aio_buf_size;
			buf->buf = malloc(buf->size);
			if (!buf->buf) {
				perror("malloc");
				res = 1;
				goto free;
			}
		}
	}

	if (render_init()) {
		printf("Render init failed!\n");
		printf("Most likely X server was not found.\n");
		res = 1;
		goto free;
	}

	if (open_eps(&ep0, argv[1])) {
		res = 1;
		goto ep_fail;
	}

	memset(&ctx, 0, sizeof(ctx));

	if (io_setup(max_events, &ctx) < 0) {
		perror("unable to setup aio");
		res = 1;
		goto ctx_fail;
	}

	cbs_to_submit = calloc(max_events, sizeof(struct iocb *));
	if (!cbs_to_submit) {
		perror("malloc");
		res = 1;
		goto cbs_fail;
	}

	evfd = eventfd(0, 0);
	if (evfd < 0) {
		perror("unable to open eventfd");
		res = 1;
		goto end;
	}

	for (i = 0; i < 3 ; i++) {
		for (j = 0; j < eps[i].aio_num ; j++) {
			buf = eps[i].aio_p + j;
			prepare_aio(eps + i, &buf->cb, &ctx, evfd);
			cbs_to_submit[cb_num] = &buf->cb;
			cb_num++;
		}
	}

	printf("Running. Type 'exit' to exit.\n");
	while (1) {
		FD_ZERO(&rfds);
		FD_SET(ep0, &rfds);
		FD_SET(evfd, &rfds);
		FD_SET(STDIN_FILENO, &rfds);

		ret = select(((ep0 > evfd) ? ep0 : evfd)+1,
			     &rfds, NULL, NULL, NULL);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			perror("select");
			break;
		}

		if (FD_ISSET(ep0, &rfds))
			handle_ep0(ep0, &ready);

		if (FD_ISSET(STDIN_FILENO, &rfds)) {
			char response[5];
			fgets(response, 5, stdin);
			if (strcmp("exit", response) == 0)
				goto end;
			fseek(stdin, 0, SEEK_END);
		}

		/* we are waiting for function ENABLE */
		if (!ready)
			continue;

		/* if something was submitted we wait for event */
		if (FD_ISSET(evfd, &rfds)) {
			uint64_t ev_cnt;
			struct io_event e[max_events];

			ret = read(evfd, &ev_cnt, sizeof(ev_cnt));
			if (ret < 0) {
				perror("unable to read eventfd");
				break;
			}

			/* we wait for one event */
			ret = io_getevents(ctx, 1, max_events, e, NULL);
			/* if we got event */
			for (i = 0; i < ret; i++) {
				for (j = 0; j < 3; j++) {
					if (e[i].obj->aio_fildes == eps[j].fd)
						break;
				}
				ret = e[i].res;
				if (ret < 0)
					goto end;
				ret = handle_aio(eps + j, e[i].obj, ret);
				if (ret < 0)
					goto end;
				prepare_aio(eps + j, e[i].obj, &ctx, evfd);
				cbs_to_submit[cb_num] = e[i].obj;
				cb_num++;
			}
		}
		if (cb_num > 0) {
			ret = io_submit(ctx, cb_num, cbs_to_submit);
			cb_num = 0;
		}
	}

	/* free resources */
end:
	free(cbs_to_submit);
cbs_fail:
	io_destroy(ctx);
ctx_fail:
	for (i = 0; i < 3; i++)
		close(eps[i].fd);
	close(ep0);
ep_fail:
	render_exit();
free:
	for (i = 0; i < 3; i++) {
		if (eps[i].aio_p) {
			for (j = 0; j < eps[i].aio_num ; j++) {
				buf = eps[i].aio_p + j;
				free(buf->buf);
			}
		}
		free(eps[i].aio_p);
	}

	return res;
}
