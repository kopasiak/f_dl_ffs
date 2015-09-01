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

#ifndef _RENDER_H
#define _RENDER_H

#include <sys/types.h>
#include <stdint.h>

#define V_WIDTH 640
#define V_HEIGHT 480

#ifdef __cplusplus
extern"C" {
#endif

int render_init(void);

void *render_start(void);

int cmd_render(unsigned char *buf, void *v_buf);

void render_end(void);

void render_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* _RENDER_H */
