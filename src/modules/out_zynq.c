// Dummy output.
//
// Copyright (c) 2019, Adrian "vifino" Pistol <vifino@tty.sh>
// 
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <types.h>
#include <timers.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

// Matrix size
#ifndef MATRIX_X
#error Define MATRIX_X as the matrixes X size.
#endif

#ifndef MATRIX_Y
#error Define MATRIX_Y as the matrixes Y size.
#endif

#define GPIO_MAP_SIZE 		0x10000
#define GPIO_DATA_OFFSET 	0x00
#define GPIO_TRI_OFFSET 	0x04
#define GPIO2_DATA_OFFSET 	0x00
#define GPIO2_TRI_OFFSET 	0x04

#define CONTROL_OFFSET 0
#define N_ROWS_OFFSET 4
#define N_COLS_OFFSET 8
#define BITDEPTH_OFFSET 12
#define LSB_LENGTH_OFFSET 16

void *control_base_addr;
void *data_base_addr;

int init(void) {
	int 		value 		= 0;
	int 		period 		= 0;
	int 		brightness 	= 0;
	int 		fd;

	fprintf(stderr,"Opening /dev/uio1\n");
	fd = open("/dev/uio1", O_RDWR);
	if (fd < 1) {
		fprintf(stderr,"Invalid UIO device file.\n");
		return -1;
	}
	control_base_addr = mmap(NULL, GPIO_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);


	fprintf(stderr,"Opening /dev/uio0\n");
	fd = open("/dev/uio0", O_RDWR);
	if (fd < 1) {
		fprintf(stderr,"Invalid UIO device file.\n");
		return -1;
	}
	// mmap the UIO device
	data_base_addr = mmap(NULL, GPIO_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	// Clear LED
	fprintf(stdout,"\nInitilize Panel\n");
	// *((unsigned *)(ptr + GPIO_TRI_OFFSET)) = 0;
	*((unsigned *)(control_base_addr + CONTROL_OFFSET)) = 0x00000003; // Clear control register
	*((unsigned *)(control_base_addr + N_ROWS_OFFSET)) = 64; // Set to 64 for a 64x64 1:32 panel. It will output 2xRGB data for 32 rows
	*((unsigned *)(control_base_addr + N_COLS_OFFSET)) = 64;
	*((unsigned *)(control_base_addr + LSB_LENGTH_OFFSET)) = 20;


	return 0;
}

int getx(int _modno) {
	return MATRIX_X;
}
int gety(int _modno) {
	return MATRIX_Y;
}

int set(int _modno, int x, int y, RGB color) {
	assert(x >= 0);
	assert(y >= 0);
	assert(x < MATRIX_X);
	assert(y < MATRIX_Y);

	*((unsigned *)(data_base_addr + (x + y*MATRIX_X) * 4)) = (color.red << 8*2) | (color.green << 8*1) | (color.blue << 8*0); // Need to set to a hex 3 to enable the led_panel_driver
	return 0;
}

RGB get(int _modno, int x, int y) {
	// Nice. We're batman.
	return RGB(0, 0, 0);
}

int clear(int _modno) {
	for(unsigned int j = 0; j < MATRIX_X*MATRIX_Y*4; j = j + 4) {
	    *((unsigned *)(data_base_addr + j)) = 0;
	}	
	return 0;
};

int render(void) {
	// Meh, don't feel like it.
	return 0;
}

oscore_time wait_until(int _modno, oscore_time desired_usec) {
	// Hey, we can just delegate work to someone else. Yay!
#ifdef CIMODE
	return desired_usec;
#else
	return timers_wait_until_core(desired_usec);
#endif
}

void wait_until_break(int _modno) {
#ifndef CIMODE
	timers_wait_until_break_core();
#endif
}

void deinit(int _modno) {
	// Can we just.. chill for a moment, please?
	// Unmap the address range
	munmap(data_base_addr, GPIO_MAP_SIZE);
	munmap(control_base_addr, GPIO_MAP_SIZE);
}
