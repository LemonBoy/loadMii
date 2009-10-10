#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> 
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <di/di.h>

#include "filestuff.h"
#include "bootstuff.h"
#include "netstuff.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

static u32 reloadStub[] = {
	0x3c208133, // lis 1,0x8133
	0x60210000, // ori 1,1,0x0000
	0x7c2903a6, // mtctr 1
	0x4e800420  // bctr
};

void __initializeVideo()
{
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	CON_Init(xfb, 60, 60, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	printf("\x1b[2;0H");
}	

void installStub ()
{
	u8 stubSign[8] = {'S', 'T', 'U', 'B', 'H', 'A', 'X', 'X'};
	
	memset((void *)0x80001800, 0, 0x1800);
	//~ memcpy((void *)0x80001804, stubSign, sizeof(stubSign));
	memcpy((void *)0x80001800, reloadStub, sizeof(reloadStub));
}

#define KEY_UP		0x01
#define KEY_DOWN 	0x02
#define KEY_LEFT 	0x04
#define KEY_RIGHT 	0x08
#define KEY_PLUS 	0x10
#define KEY_MINUS 	0x20
#define KEY_A 		0x40
#define KEY_B 		0x80

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

int main(int argc, char **argv) 
{
        int device = 0;
	int p = 0;
	int i = 0;

        DI_Init();

	__initializeVideo();
	
	PAD_Init();
	WPAD_Init();
	
	//~ initializeNet();
	
	setDevice(devlst[device]);
        	
	installStub();	
		
        while (1)
        {
                u8 kMap = readKeys();

		printf("\x1b[2J");
                printf("\x1b[4;0H");
		printf("\t :: loadMii 0.4 - REBiRTH\n");
                printf("\t :: [%s]\n", getCurrentPath());

                if (kMap & KEY_UP && i > 0)
                {
                        i--;
                }

                if (kMap & KEY_DOWN && i < (p - 1))
                {
                        i++;
                }
		
		if (kMap & KEY_LEFT)
		{
			if (i <= 5)
			{
				i = 0;
			}
			else
			{
				i -= 5;
			}
		}
		
		if (kMap & KEY_RIGHT)
		{
			if ((i + 5) >= (p - 1))
			{
				i = p - 1;
			}
			else
			{
				i += 5;
			}
		}		

                if (kMap & KEY_PLUS && device < (maxdev - 1))
                {
                        setDevice(devlst[device++]);
			i = 0;
                }

                if (kMap & KEY_MINUS && device > 0)
                {
                        setDevice(devlst[device--]);
			i = 0;
                }
		
                if (kMap & KEY_B)
                {
			if (updatePath(".."))
			{
				i = 0;
			}
		}
		
                if (kMap & KEY_A)
                {
			if (getItem(i)->size == 0)
			{
				updatePath(getItem(i)->name);
				i = 0;
			}
			else
			{
				if (supportedFile(getItem(i)->name))
				{
					u8 *bufPtr = memoryLoad(getItem(i));
					void (*entry)() = (void *)0x80001800;
					
					switch (validateHeader(bufPtr))
					{
						case 0x0:
							entry = (void *)relocateDol(bufPtr);
							break;
						case 0x1:
							entry = (void *)relocateElf(bufPtr);
							break;
					}
					
					unmountDevice();
					entry();
				}
			}
		}

                for (p=i;p<20 + i;p++)
	        {
		        if (getFilesCount() - p == 0)
		        {
			        break;
			}
			printf("\t %s %s\n", ((p == i) ? "::" : "  "), getItem(p)->labl);
		}
		/* Avoid flickering. */
		fflush(stdout);
		VIDEO_WaitVSync();
        }		

	return 0;
}
