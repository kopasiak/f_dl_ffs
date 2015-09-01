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

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>

#include "udl_ffs.h"
#include "render.h"
#include "mat.hpp"

int render_init()
{
	img = cv::Mat(cv::Mat::zeros(V_HEIGHT, V_WIDTH, CV_8UC3));
	_img = img;
	cv::imshow("UDL demo app", img);
	cv::waitKey(1);
	return 0;
}

static inline void pix_set(int h, int w, uint16_t val)
{
	uint16_t r_val = be16toh(val);
	_img(h, w)[0] = (r_val << 3) & 0b11111000;
	_img(h, w)[1] = (r_val >> 3) & 0b11111100;
	_img(h, w)[2] = r_val >> 8 & 0b11111000;
}

static inline void pix_inc(int *h, int *w)
{
	(*w)++;
	if (*w == V_WIDTH) {
		*w = 0;
		(*h)++;
	}
}

void render_end()
{
	debug_print("Blit performed\n");
	cv::imshow("UDL demo app", img);
	cv::waitKey(1);
}


void render_exit(void)
{
}

void *render_start(void)
{
	return NULL;
}


int cmd_render(unsigned char *buf, void *v_buf)
{
	int read = 0, pix_read = 0;
	uint32_t offset = 0;
	int raw = 0;
	int i;
	int pixnum, rawnum, repeat;
	uint16_t *pix_pos;
	int v_h = 0, v_w = 0;

	offset |= (buf[read++] << 16);
	offset |= (buf[read++] << 8);
	offset |= (buf[read++]);
	offset /= 2; /* bytes per pixel */
	v_h = offset / V_WIDTH;
	v_w = offset - (V_WIDTH * v_h);

	pixnum = (buf[read] ? buf[read] : 256);
	read++;

	while (pix_read < pixnum) {
		raw = 1;
		rawnum = (buf[read] ? buf[read] : 256);
		pix_read += rawnum;
		read++;

		pix_pos = (uint16_t *) (buf + read);
		for (i = 0; i < rawnum; i++) {
			pix_set(v_h, v_w, *pix_pos);
			pix_inc(&v_h, &v_w);
			pix_pos++;
			read += 2;
		}

		if (pix_read >= pixnum)
			break;

		raw = 0;
		pix_pos = (uint16_t *) (buf + read - 2);
		repeat = buf[read];
		pix_read += repeat;
		read++;

		for (i = 0; i < repeat ; i++) {
			pix_set(v_h, v_w, *pix_pos);
			pix_inc(&v_h, &v_w);
		}
	}

	if (!raw)
		read++; /* Junk byte - empty raw span */
	return read;
}
