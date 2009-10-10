#ifndef loadMiiFilestuff
#define loadMiiFilestuff

#include <sdcard/wiisd_io.h>
#include <di/di.h>
#include <sdcard/gcsd.h>

typedef struct _fatdev
{
        int index;
        char root[4];
        const DISC_INTERFACE* io;
} fatdev;
                         
typedef struct _item
{
        char name[MAXPATHLEN];
        char labl[60];
        int  size;
} item;      

static fatdev devlst[] = {
                        {0, "sd", &__io_wiisd},
                        {1, "usb", &__io_usbstorage},
                        {2, "dvd", &__io_wiidvd},
                        {3, "sda", &__io_gcsda},
                        {4, "sdb", &__io_gcsdb}
                         };                   
                         
#define maxdev        (sizeof(devlst) / sizeof(fatdev))       

        
u8 *memoryLoad (item *file);
int supportedFile (char *name);
int setDevice (fatdev device);
void unmountDevice ();
int updatePath (char *update);   
char *getCurrentPath ();                
item *getItem (int n);
int getFilesCount ();
char *getNames ();

#endif
