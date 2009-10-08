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

int main(int argc, char **argv) 
{
        int device = 0;
	int p = 0;
	int i = 0;

        DI_Init();

	__initializeVideo();
	WPAD_Init();
	
	//~ initializeNet();
	
	setDevice(devlst[device]);
        	
	installStub();	
		
        while (1)
        {
                WPAD_ScanPads();

		printf("\x1b[2J");
                printf("\x1b[4;0H");
		printf("\t :: loadMii 0.4 - REBiRTH\n");
                printf("\t :: [%s] %s\n", getCurrentPath(), getNames());
		printf("\n\n");

                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_UP && i > 0)
                {
                        i--;
                }

                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_DOWN && i < (p - 1))
                {
                        i++;
                }
		
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_LEFT)
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
		
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_RIGHT)
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

                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS && device < (maxdev - 1))
                {
                        device++;
                        setDevice(devlst[device]);
			i = 0;
                }

                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_MINUS && device > 0)
                {
                        device--;
                        setDevice(devlst[device]);
			i = 0;
                }
		
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
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
					
					entry();
				}
			}
		}

                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
                {
			exit(0);
                }

                for (p=i;p<15 + i;p++)
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
