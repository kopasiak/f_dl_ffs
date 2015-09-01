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

#include <libaio.h>
#define IOCB_FLAG_RESFD         (1 << 0)

#include "udl_ffs.h"

void prepare_aio(
		struct ep_data *ep,
		struct iocb *cb,
		io_context_t *ctx,
		int evfd)
{
	struct aio_buf *buf = container_of(cb, struct aio_buf, cb);

	if (ep->read)
		io_prep_pread(cb, ep->fd, buf->buf, buf->size, 0);
	else
		io_prep_pwrite(cb, ep->fd, buf->buf, buf->size, 0);
	cb->u.c.flags |= IOCB_FLAG_RESFD;
	cb->u.c.resfd = evfd;
}

int handle_aio(struct ep_data *ep, struct iocb *cb, int len)
{
	struct aio_buf *buf = container_of(cb, struct aio_buf, cb);

	return ep->handler((unsigned char *) buf->buf, len);
}
