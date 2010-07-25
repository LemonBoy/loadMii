// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include "loader.h"

static u8 *const code_buffer = (u8 *)0x90100000;

static void dsp_reset(void)
{
	write16(0x0c00500a, read16(0x0c00500a) & ~0x01f8);
	write16(0x0c00500a, read16(0x0c00500a) | 0x0010);
	write16(0x0c005036, 0);
}

int try_sd_load(void)
{
	extern u32 fat_file_size;
	
	if (sd_init())
		return 0;

	if (fat_init())
		return 0;

	if (fat_open("boot.dol"))
		return 0;

	if (fat_read(code_buffer, fat_file_size))
		return 0;

	return 1;
}

int main(void)
{
	int tries = 0;
	
	write32(0x0c003004, 0); /* Clear interrupt mask */
	dsp_reset();
	reset_ios();

	while(tries++ < 10)
	{
		write32(0x0d8000e0, read32(0x0d8000e0) | 0x20);
		
		if (try_sd_load())
		{
			void (*entry)() = load_dol_image(code_buffer);
			entry();
		}
		
		write32(0x0d8000e0, read32(0x0d8000e0) & (~0x20));
		udelay(5 * 1000);
	}
	
	write32(0x0d8000e0, read32(0x0d8000e0) | 0x40);
	
	return 1;
}
