#ifndef loadMiiFilestuff
#define loadMiiFilestuff

#include <sdcard/wiisd_io.h>
#include <di/di.h>
#include <sdcard/gcsd.h>

typedef struct _fatdev
{
        char *str;
        char *root;
        const DISC_INTERFACE* io;
} fatdev;
                         
typedef struct _item
{
        char name[MAXPATHLEN];
        char labl[40];
        int  size;
} item;      

static fatdev devlst[] = {
                        {"Internal SD slot"   , "sd" , &__io_wiisd     },
                        {"Usb stick"          , "usb", &__io_usbstorage},
                        {"Dvd (Requires DVDX)", "dvd", &__io_wiidvd    },
                        {"SD Gecko in slot A" , "sda", &__io_gcsda     },
                        {"SD Gecko in slot B" , "sdb", &__io_gcsdb     }
                         };                   
                         
#define maxdev        (sizeof(devlst) / sizeof(fatdev))       

u8 *memoryLoad (item *file);
int supportedFile (char *name);
int setDevice (const fatdev *device);
void unmountDevice ();
int updatePath (char *update);   
char *getCurrentPath ();                
item *getItem (int n);
int getFilesCount ();
char *getNames ();
int isDeviceInserted ();
void doStartup (int activate);

#endif
