#ifndef FINDFIRST_H
#define FINDFIRST_H

#include <sys/types.h>
#include <dirent.h>

//#define FA_RDONLY	0x01	/* Read only attribute */
#define FA_HIDDEN	0x02	/* Hidden file */
#define FA_SYSTEM	0x04	/* System file */
//#define FA_LABEL	0x08	/* Volume label */
#define FA_DIREC	0x10	/* Directory */
#define FA_ARCH		0x20	/* Archive */

struct ffblk
{
	//char     ff_reserved[21];
	//char     ff_attrib;
	//unsigned ff_ftime;
	//unsigned ff_fdate;
	long     ff_fsize;
	char     ff_name[256];
	// reserved
	DIR     *dd_dirp;
	struct   dirent *dd_dp;
	char     dd_filespec[256];
	char     dd_attrib;
};

int findfirst(const char *pathname, struct ffblk *ffblk, int attrib);
int findnext(struct ffblk *ffblk);

#endif
