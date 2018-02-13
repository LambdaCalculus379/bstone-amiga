
#include "id_heads.h"
#include <sys/stat.h>

long tell(int handle)
{
	return lseek(handle, 0, SEEK_CUR);
}

long filelength(int handle)
{
#if 1
	long prev = tell(handle);
	lseek(handle, 0, SEEK_END);
	long sz = tell(handle);
	lseek(handle, prev, SEEK_SET);
#else
	long sz = 0;
	struct stat st;
	if (fstat(handle, &st) != -1)
		sz = st.st_size;
#endif
	return sz;
}

/*
 MinGW-w64 - for 32 and 64 bit Windows
A complete runtime environment for gcc 
*/

int memicmp(const void *s1, const void *s2, size_t n)
{
	const char *sc1 = (const char *)s1;
	const char *sc2 = (const char *)s2;
	const char *sc2end = sc2 + n;
	while (sc2 < sc2end && toupper(*sc1) == toupper(*sc2))
	{
		sc1++;
		sc2++;
	}
	if (sc2 == sc2end)
		return 0;
	return (int)(toupper(*sc1) - toupper(*sc2));
} 
