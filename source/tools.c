#include <stdio.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include "tools.h"

void blinkTray ()
{
	*(u32 *)0x0d8000c0 = *(u32 *)0x0d8000c0 ^ 0x20;
}

/* Internal errno. */

#define ERRNO_NO_ERROR 0

static int errNo = ERRNO_NO_ERROR;

char *errorTable[] = {
		0, 
		"Cannot mount the DVD, check that its a data one.",
		"Cannot mount the device, check that its FAT formatted.",
		"Cannot load the executable."
};
	
void setError (int err)
{
        errNo = err;
}

void handleError ()
{
	if (errNo)
	{
		printf("\x1b[25;10H");
		printf(":: ERROR ::\n");
		printf("\x1b[26;10H");
		printf(":: %s\n", errorTable[errNo]);
		
		sleep(1);
		blinkTray();
		sleep(1);
		blinkTray();
		sleep(1);
		blinkTray();
		sleep(1);
		blinkTray();
		sleep(1);				
		
		errNo = ERRNO_NO_ERROR;
	}
}

u8 readKeys ()
{
	u8 bitmap;
	int pad;
	
	PAD_ScanPads();
	WPAD_ScanPads();
	
	bitmap = 0;
	
	for (pad=0;pad<4;pad++)
	{
		if (PAD_ButtonsDown(pad) & PAD_BUTTON_UP || WPAD_ButtonsDown(pad) & WPAD_BUTTON_UP)
		{
			bitmap |= KEY_UP;
		}
		
		if (PAD_ButtonsDown(pad) & PAD_BUTTON_DOWN || WPAD_ButtonsDown(pad) & WPAD_BUTTON_DOWN)
		{
			bitmap |= KEY_DOWN;
		}
		
		if (PAD_ButtonsDown(pad) & PAD_BUTTON_LEFT || WPAD_ButtonsDown(pad) & WPAD_BUTTON_LEFT)
		{
			bitmap |= KEY_LEFT;
		}
		
		if (PAD_ButtonsDown(pad) & PAD_BUTTON_RIGHT || WPAD_ButtonsDown(pad) & WPAD_BUTTON_RIGHT)
		{
			bitmap |= KEY_RIGHT;
		}
		
		if (PAD_ButtonsDown(pad) & PAD_BUTTON_X || WPAD_ButtonsDown(pad) & WPAD_BUTTON_PLUS)
		{
			bitmap |= KEY_PLUS;
		}
		
		if (PAD_ButtonsDown(pad) & PAD_BUTTON_Y || WPAD_ButtonsDown(pad) & WPAD_BUTTON_MINUS)
		{
			bitmap |= KEY_MINUS;
		}	
		
		if (PAD_ButtonsDown(pad) & PAD_BUTTON_A || WPAD_ButtonsDown(pad) & WPAD_BUTTON_A)
		{
			bitmap |= KEY_A;
		}
		
		if (PAD_ButtonsDown(pad) & PAD_BUTTON_B || WPAD_ButtonsDown(pad) & WPAD_BUTTON_B)
		{
			bitmap |= KEY_B;
		}													
	}
	
	return bitmap;
}
