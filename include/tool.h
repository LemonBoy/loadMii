#ifndef loadMiiTools
#define loadMiiTools

#define matchStr(x,y) ((strcmp(x,y) == 0) ? 1 : 0)

inline void debugPrint (char *msg)
{
#ifndef SHOW_NO_MERCY
	printf("\t :: DEBUG -> %s\n", msg);
	sleep(2);
#endif
}

#endif
