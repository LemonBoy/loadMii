#ifndef loadMiiTools
#define loadMiiTools

/* Key bitmasks. */

#define KEY_UP		0x01
#define KEY_DOWN 	0x02
#define KEY_LEFT 	0x04
#define KEY_RIGHT 	0x08
#define KEY_PLUS 	0x10
#define KEY_MINUS 	0x20
#define KEY_A 		0x40
#define KEY_B 		0x80

void setError (int err);
void handleError ();
u8 readKeys ();
void blinkTray ();

#define matchStr(x,y) ((strcmp(x,y) == 0) ? 1 : 0)

#endif
