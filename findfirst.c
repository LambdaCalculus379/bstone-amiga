/*
  dosdir.c:

  DOSDIR V2.1a: A Portable DOS/UNIX/VMS Directory Interface

  Implementation of the DOS directory functions (findfirst and findnext)
  on MS-DOS, UNIX and VMS platforms using the appropriate file & directory
  structure.

  Provides the DOS directory framework for MS-DOS/UNIX/VMS application
  portability.

  Supports MS-DOS with Borland C++, Turbo C, or Microsoft C V6.0/7.0,
  Sun with GNU C compiler, DEC Alpha (OSF-1), VAX/VMS C,
  and other comparable platforms.

  Written by: Jason Mathews <mathews@mitre.org>

  ---------------------------------------------------------------------------

Copyright (C) 1996 Jason Mathews

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

  ---------------------------------------------------------------------------

 Modification history:
   V1.0  02-May-91,  Original version.
   V2.0  13-May-94,  Reimplemented findfirst/findnext with ffblk structure
                     to match MS-DOS externally, fixed wildcard evaluation
                     function.
   V2.1  08-Jun-94,  Replaced wildcard evaluation function with recursive
                     function provided by Info-ZIP's portable UnZip,
                     added dd_ prefix to most functions & constants,
                     added VMS functions + MSC/TURBOC macros.
   V2.1a 16-Oct-96,  Call lstat() instead of stat() to avoid expanding on
                     symbolic linked directories.
   Jan 3 2000, Erwin Waterlander, update for Mingw32 compiler.
   Apr 29 2002, Erwin Waterlander, update for LCC windows compiler.
   Jul 14 2008, Erwin Waterlander, update for OS/2 using gcc.
   Jul 28 2009, Erwin Waterlander, support UTF-16 Unicode on Windows.
                UTF-16 wide character names are converted to UTF-8 multi-byte strings.
   Aug 2012, Major cleanup macros:
             * Use only C99 predefined macros.
             * Use __MSDOS__ only when it's real for MS-DOS.
             * UNIX is not defined on OS/2 EMX (fixed tailor.h, UNIX got defined,
               because _BSD_SOURCE is defined with EMX (GCC on OS/2)).
             * Make it compile with Watcom C for OS/2.
             * Borland C and LCC may have been broken, because these are not supported
               anymore.
        Erwin Waterlander
  */

#include <string.h>
//#ifdef __AMIGA__
#if !defined(__linux__) && !defined(__CYGWIN__)
#include <sys/syslimits.h>
#else
#include <limits.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fnmatch.h>
#include "findfirst.h"

int findnext(struct ffblk *fb)
{
	struct stat dd_sstat;
	if (!fb->dd_dirp) goto findnext_err;
	while ((fb->dd_dp = readdir(fb->dd_dirp)) != NULL)
	{
		//printf("%s %s %s\n", __FUNCTION__, fb->dd_filespec, fb->dd_dp->d_name);
		if (stat(fb->dd_dp->d_name, &dd_sstat))
			continue;
		if (dd_sstat.st_mode & S_IFDIR && !(fb->dd_attrib & FA_DIREC))
			continue;
		//if (dd_match(fb->dd_dp->d_name, fb->dd_filespec, 0))
		if (!fnmatch(fb->dd_filespec, fb->dd_dp->d_name, FNM_PATHNAME))
		{
			/* fill in file info */
			strncpy(fb->ff_name, fb->dd_dp->d_name, sizeof(fb->ff_name));
			fb->ff_fsize = dd_sstat.st_size;
			return 0;       /* successful match */
		}
	}  /* while */

	closedir(fb->dd_dirp);

findnext_err:

	memset(fb, 0, sizeof(struct ffblk)); /* invalidate structure */
	errno = ENOENT;       /* no file found */
	return -1;
}

int findfirst(const char *path, struct ffblk *fb, int attrib)
{
	char dir[PATH_MAX];		/* directory path */
	char *s = strrchr(path, '/');
	if (!s)
	{
		s = strrchr(path, ':');
	}
	if (s)
	{
		strcpy(fb->dd_filespec, s+1);
		strncpy(dir, path, (size_t)(s-path));
	}
	else
	{
		//strcpy(dir, ".");		/* use current directory */
		getcwd(dir, sizeof(dir));
		strcpy(fb->dd_filespec, path);
	}
	fb->dd_attrib = (char)attrib;
	fb->dd_dirp   = opendir(dir);
	return findnext(fb);
}

