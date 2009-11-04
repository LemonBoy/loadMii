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

extern const fatdev devlst[];
extern const int maxdev;

u8 *memoryLoad (item *file);
int supportedFile (char *name);
int setDevice (const fatdev *device);
void unmountDevice ();
int updatePath (char *update);   
char *getCurrentPath ();                
item *getItem (int n);
int getFilesCount ();
char *getNames ();
char *getFullName (item *file);
int isDeviceInserted ();
void doStartup (int activate);

#endif
