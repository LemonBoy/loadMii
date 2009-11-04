#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> 
#include <gccore.h>
#include <gctypes.h>
#include <ogc/lwp_threads.h>
#include <wiiuse/wpad.h>
#include <di/di.h>
void __exception_closeall();

#include "filestuff.h"
#include "bootstuff.h"
#include "netstuff.h"
#include "tools.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

static u32 reloadStub[] = {
	0x3c208133, // lis 1,0x8133
	0x60210000, // ori 1,1,0x0000
	0x7c2903a6, // mtctr 1
	0x4e800420  // bctr
};

void installStub ()
{
	u8 stubSign[] = {'S', 'T', 'U', 'B', 'H', 'A', 'X', 'X'};
	
	memset((void *)0x80001800, 0, 0x1800);
	//~ memcpy((void *)0x80001804, stubSign, sizeof(stubSign));
	memcpy((void *)0x80001800, reloadStub, sizeof(reloadStub));
}

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
	if(rmode->viTVMode&VI_NON_INTERLACE)
	{
		VIDEO_WaitVSync();
	}
}

void setArgs (char *path, void *ventry) {
	u32 *entry = ventry;
	struct __argv *argstruct = (struct __argv *)(entry + 2);
	if (entry[1] == ARGV_MAGIC) {
		argstruct->argvMagic = ARGV_MAGIC;
		argstruct->commandLine = path;
		argstruct->length = strlen(path) + 1;
	}
}

void reloadIOS ()
{
	int ver;

	__IOS_ShutdownSubsystems();
	if (__ES_Init() < 0) return;
	ver = IOS_GetPreferredVersion();
	if (ver < 0) {
		__ES_Close();
		return;
	}
	if (__IOS_LaunchNewIOS(ver) < 0) __ES_Close();
	return;
}

void cleanup ()
{
	/* Unmount the FAT device... */
	unmountDevice();
	/* send the shutdown command... */
	doStartup(0);
	/* shutdown WPAD... */
	WPAD_Shutdown();
	/* and reload IOS. */
	reloadIOS();
	/* Then g'bye libOGC. */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	__exception_closeall();
}

void sourceSelector ()
{
	int sel = 0;
	
	while (1)
        {
                u8 kMap = readKeys();

		printf("\x1b[2J");
                printf("\x1b[3;0H");
                printf("\t :: loadMii 0.4 REBiRTH - Device selection\n");
                printf("\t :: Press [LEFT] and [RIGHT] to choose between the devices\n");
                printf("\t :: %s\n", devlst[sel].str);
                printf("\t :: Inserted : %s\n", (devlst[sel].io->isInserted()) ? "Yes" : "No");
                
                if (kMap & KEY_RIGHT && sel < (maxdev - 1))
                {
			sel++;		
                }

                if (kMap & KEY_LEFT && sel > 0)
                {
                        sel--;
                }                
                
                if (kMap & KEY_A)
                {
			if (devlst[sel].io->isInserted())
			{
				setDevice(&devlst[sel]);
				break;
			}
		}
		
		/* Error handler. */
		handleError();
		/* Avoid flickering. */
		fflush(stdout);
		VIDEO_WaitVSync();
	}		
}

int main(int argc, char **argv) 
{
	int p = 0;
	int index = 0;
	int iosv;

	iosv = IOS_GetPreferredVersion();
	if (iosv > 0)
		IOS_ReloadIOS(iosv);

	/* Initialize the DVDX stub. */
        DI_Init();

	/* Set up the video. */
	__initializeVideo();
	
	/* Fire up the wiimotes and the GC pad. */
	PAD_Init();
	WPAD_Init();
	
	/* Send activation command to the devices. */
	doStartup(1);

#if 0
	startNetworkStuff();
	
	while (networkReady())
	{
		printf(".");
		fflush(stdout);
		VIDEO_WaitVSync();
	}
	
	printf("\t\t :: Network has been inited -> %i\n", networkReady());
	
	sleep(10);
#endif

	/* Ask the user wich device to mount. */
	sourceSelector();
        	
	installStub();	
		
        while (1)
        {
                u8 kMap = readKeys();
		char *currentPath = getCurrentPath();

		printf("\x1b[2J");
                printf("\x1b[3;0H");
		printf("\t :: loadMii 0.4 REBiRTH - Browser\n");
		printf("\t :: Press [+] to change the browsing device.\n");
                printf("\t :: [%s]\n", currentPath);
                printf("\n");
		free(currentPath);

                if (kMap & KEY_UP && index > 0)
                {
                        index--;
                }

                if (kMap & KEY_DOWN && index < (p - 1))
                {
                        index++;
                }
		
		if (kMap & KEY_LEFT)
		{
			if (index <= 5)
			{
				index = 0;
			}
			else
			{
				index -= 5;
			}
		}
		
		if (kMap & KEY_RIGHT)
		{
			if ((index + 5) >= (p - 1))
			{
				index = p - 1;
			}
			else
			{
				index += 5;
			}
		}
		
		if (kMap & KEY_PLUS)
		{
			sourceSelector();
		}
		
                if (kMap & KEY_B)
                {
			if (updatePath(".."))
			{
				index = 0;
			}
		}
		
                if (kMap & KEY_A)
                {
			if (getItem(index)->size == 0)
			{
				updatePath(getItem(index)->name);
				index = 0;
			}
			else
			{
				if (supportedFile(getItem(index)->name))
				{
					u8 *bufPtr = memoryLoad(getItem(index));
					void (*entry)() = (void*)0x80001400;
					/* Do the right relocation. */
					switch (validateHeader(bufPtr))
					{
						case 0x0:
							entry = (void *)relocateDol(bufPtr);
							break;
						case 0x1:
							entry = (void *)relocateElf(bufPtr);
							break;
						default:
							entry = NULL;
							break;
					}
					if (entry) {
						setArgs(getItem(index)->name, entry);
						cleanup();
						__lwp_thread_stopmultitasking(entry);
					}
				}
			}
		}

                for (p=index;p<18+index;p++)
	        {
		        if (getFilesCount() - p == 0)
		        {
			        break;
			}
			printf("\t %s %s\n", ((p == index) ? "::" : "  "), getItem(p)->labl);
		}
		/* Error handler. */
		handleError();
		/* Avoid flickering. */
		fflush(stdout);
		VIDEO_WaitVSync();
        }		

	return 0;
}
