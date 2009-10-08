#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> 
#include <gccore.h>
#include <fat.h>
#include <iso9660.h>

#include "filestuff.h"
#include "tool.h"
                         
static fatdev * inuse = NULL;
static char bPath[MAXPATHLEN];    /* Browsing path.        */                         
static item * list = NULL; 	  /* Global list of files. */
static int  fCount = 0;    	  /* Files count.          */      

char *devtext[] = {
        "Internal SD slot",
        "Usb stick",
        "Dvd drive (requires DVDX)",
        "SD Gecko slot A",
        "SD Gecko slot B"
        };

item *getItem (int n)
{
        return &list[n];
}

int getFilesCount ()
{
        return fCount;
}

char *getCurrentPath ()
{
        char ret[MAXPATHLEN];
        
        memset(ret, 0, MAXPATHLEN);
        
        if (strlen(bPath) >= 30)
        {
                strncpy(ret, bPath, 12);
                strcat(ret, "...");
                strncat(ret, bPath + 12, 12);
        }
        else
        {
                strcpy(ret, bPath);
        }
        
        return strdup(ret);
}

char *formatSize (int size)
{
	char formattedStr[10];
        
	memset(formattedStr, 0, 10);
        
	if (size < 1024)
	{
		sprintf(formattedStr, "%i b", size);
	}
	else if (size > 1024 && size < 1024 * 1024)
	{
		sprintf(formattedStr, "%i Kb", size / 1024);
	}
	else if (size > 1024 * 1024 && size < 1024 * 1024 * 1024)
	{
		sprintf(formattedStr, "%i Mb", size / 1024 / 1024);
	}
        else if (size > 1024 * 1024 * 1024)
        {
                sprintf(formattedStr, "%i Gb", size / 1024 / 1024);
        }
                
	return strdup(formattedStr);
}

char *getNames ()
{
        char str[50];
        
        memset(str, 0, 50);
        
        if (inuse != NULL)
        {
                if (inuse->index == 0)
                {
                        sprintf("%s >>", devtext[inuse->index + 1]);
                }
                else if (inuse->index > 0 && inuse->index < maxdev - 1)
                {
                        sprintf("<< %s | %s >>", devtext[inuse->index - 1], devtext[inuse->index + 1]);
                }
                else if (inuse->index == maxdev - 1)
                {
                        sprintf("<< %s", devtext[inuse->index - 1]);
                }
        }
        
        return strdup(str);
}

int supportedFile (char *name)
{
        if (strstr(name, ".elf") ||
            strstr(name, ".dol"))
        {
                return 1;
        }
        return 0;
}

void getFiles ()
{
        int index;
        char name[MAXPATHLEN];

        DIR_ITER *iter = diropen(bPath);
        struct stat fstat;

        if (list != NULL)
        {
                free(list);
                list = NULL;
                fCount = 0;
        }

        list = malloc(sizeof(item));

        if (iter == NULL)
        {
                return;
        }

        index = 0;

        while (dirnext(iter, name, &fstat) == 0)
        {
                list = (item *)realloc(list, sizeof(item) * (index+1));
                memset(&(list[index]), 0, sizeof(item));
                sprintf(list[index].name, "%s", name);
                if (fstat.st_mode & S_IFDIR)
                {
                        list[index].size = 0;
                }
                else
                {
                        list[index].size = fstat.st_size;
                }
                if (matchStr(list[index].name, "."))
                {
                        sprintf(list[index].labl, "Current directory");
                }
		else if (matchStr(list[index].name, ".."))
                {
                        sprintf(list[index].labl, "Parent directory");
                }
                else
                {
			if (list[index].size > 0)
			{
				sprintf(list[index].labl, "%.30s \t %s", list[index].name, formatSize(list[index].size));
			}
			else
			{
				sprintf(list[index].labl, "[%.30s]", list[index].name);
			}
                }
                index++;
        }

        dirclose(iter);

        fCount = index;
}

int setDevice (fatdev device)
{
        if (inuse != NULL)
        {
                debugPrint("Unmount the old device first");
                if (matchStr(inuse->root, "dvd"))
                {
                        ISO9660_Unmount();
                }
                else
                {
                        fatUnmount(inuse->root);
                }
                inuse->io->shutdown();
                inuse = NULL;
        }

        device.io->startup();

        if (!(device.io->isInserted()))
        {
                debugPrint("Device not inserted");
                device.io->shutdown();
                return 0;
        }
        
        if (matchStr(device->root, "dvd"))
        {
                if (!(ISO9660_Mount()))
                {
                        debugPrint("Cannot mount the dvd.");
                        return 0;
                }
        }
        else
        {                       
                if (!fatMount(device->root, device->io, 0, 8, 512))
                {
                        return 0;
                }
        }
        
        inuse = &device;

        memset(&bPath, 0, 512);
        sprintf(bPath, "%s:/", inuse->root);
        getFiles();
        
        return 1;
}

void updatePath (char *update)
{
        if (matchStr(update, "."))
        {
                return;
        }
        else if (matchStr(update, ".."))
        {
                char * lastSlash = strrchr(bPath, '/');
                if (lastSlash == NULL)
                {
                        return;
                }
                *lastSlash = 0;
        }
        else
        {
                sprintf(bPath, "%s/%s", bPath, update);
        }

        getFiles();
}

u8 *memoryLoad (item *file)
{
        char ffPath[MAXPATHLEN];
        u8 *memholder = NULL;
        
        memset(&ffPath, 0, MAXPATHLEN);
        
        sprintf(ffPath, "%s/%s", getCurrentPath(), file->name);
        
        FILE * fp = fopen(ffPath, "rb");
        
        if (fp == NULL)
        {
                return NULL;
        }

        memholder = malloc(file->size);
        
        if (memholder == NULL)
        {
                return NULL;
        }
        
        if (fread(memholder, 1, file->size, fp) < file->size)
        {
                free(memholder);
                return NULL;
        }
        
        fclose(fp);
        
        return memholder;
}
