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
#include "tools.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void reloadIOS ()
{
	int ver;

	__IOS_ShutdownSubsystems();
	if (__ES_Init() < 0) 
		return;
	ver = IOS_GetPreferredVersion();
	if (ver < 0)
	{
		__ES_Close();
		return;
	}
	if (__IOS_LaunchNewIOS(ver) < 0) 
		__ES_Close();
	return;
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

extern void __exception_closeall();

void cleanup ()
{
	/* Unmount the FAT device... */
	unmountDevice();
	/* and send the shutdown command. */
	doStartup(0);
	/* We dont need WPAD anymore */
	WPAD_Shutdown();
	/* Reload an ios */
	reloadIOS();
	/* Then g'bye libOGC. */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	/* Disable exceptions */
    __exception_closeall();
}

void installStub ()
{
	FILE * rstub = fopen("sd:/loader.bin", "rb");
	u32 rstubSz;
	
	fseek(rstub, 0, SEEK_END);
	rstubSz = ftell(rstub);
	fseek(rstub, 0, SEEK_SET);
	
	fread((void *)0x80001800, 1, rstubSz, rstub);
	
	fclose(rstub);
}

void buildArgs (char *path, struct __argv *args)
{
    bzero(args, sizeof(*args));
    args->argvMagic = ARGV_MAGIC;
    args->length = strlen(path) + 2;
    args->commandLine = (char*)malloc(args->length);
    if (!args->commandLine)
	{
		args->argvMagic = 0xdeadbeef;
		return;
	}
    strcpy(args->commandLine, path);
    args->commandLine[args->length - 1] = '\0';
    args->argc = 1;
    args->argv = &args->commandLine;
    args->endARGV = args->argv + 1;
}

void sourceSelector ()
{
	int sel = 0;
	
	while (1)
    {
        u8 kMap = readKeys();

		printf("\x1b[2J");
        printf("\x1b[3;0H");
        printf("\t :: loadMii 0.4 REBiRTH - Device selector\n\t :: (C) 2009-2010 The Lemon Man\n");
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
				setDevice(devlst[sel]);
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

	/* Initialize the DVDX stub. */
    DI_Init();

	/* Set up the video. */
	__initializeVideo();
	
	/* Fire up the wiimotes and the GC pad. */
	PAD_Init();
	WPAD_Init();
	
	/* Send activation command to the devices. */
	doStartup(1);
	/* Ask the user wich device to mount. */
	sourceSelector();
	/* Install the reloading stub. */
		
    while (1)
    {
        u8 kMap = readKeys();
		char *currentPath = getCurrentPath(1);

		printf("\x1b[2J");
        printf("\x1b[3;0H");
		printf("\t :: loadMii 0.4 REBiRTH - Browser\n\t :: (C) 2009-2010 The Lemon Man\n");
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
					char *argPath = getItemFullpath(getItem(index));
					struct __argv args;
					void (*entry)();
					/* Check for loading fail */
					if (bufPtr)
					{				
						/* Do the right relocation. */
						switch (validateHeader(bufPtr))
						{
							case 0x0:
								buildArgs(argPath, &args);
								entry = (void (*)())(relocateDol(bufPtr, &args));
								break;
							case 0x1:
								entry = (void (*)())(relocateElf(bufPtr));
								break;
							default:
								entry = NULL;
						}
						
						if (entry)
						{
							free(bufPtr);
							/* Set CPU/BUS clock as Nintendo SDK apps require so. */
							*(vu32*)0x800000F8 = 0x0E7BE2C0;
							*(vu32*)0x800000FC = 0x2B73A840;
							cleanup();
							__lwp_thread_stopmultitasking(entry);
							entry();
						}
					} else {
						setError(3);
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
