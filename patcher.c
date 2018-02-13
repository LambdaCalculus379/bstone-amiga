#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define XD3_USE_LARGEFILE64 0
#if SIZE_MAX == UINT64_MAX
#define XD3_USE_LARGESIZET 1
#define SIZEOF_SIZE_T 4
#define SIZEOF_UNSIGNED_LONG 4
#define SIZEOF_UNSIGNED_LONG_LONG 8
#else
#define XD3_USE_LARGESIZET 0
#define SIZEOF_SIZE_T 4
#define SIZEOF_UNSIGNED_LONG 4
#define SIZEOF_UNSIGNED_LONG_LONG 8
#endif
#define XD3_ENCODER 0
//#define XD3_DEBUG 3
#define XD3_DEBUG 0
#include "xdelta3/xdelta3.h"
#include "xdelta3/xdelta3.c"

// Describes a required file from a specific game version
typedef struct {
	const char *filename;
	int filesize;
	uint32_t crc32;
} BE_GameFileDetails_T;

#define AOG_EXT "BS6"
#define XD3_EXT "VCDIFF"

static const BE_GameFileDetails_T aog200[] =
{
	{"AUDIOHED", 1280, 0x9397D376},
	{"AUDIOT", 267636, 0xB33A17D2},
	{"EANIM", 186221, 0x057B9573},
	{"GANIM", 219918, 0x8B0781A2},
	{"IANIM", 18977, 0xBE1CD872},
	{"MAPHEAD", 834, 0x60BD7A14},
	{"MAPTEMP", 522914, 0x80F30F09},
	{"SANIM", 276784, 0x0E6B7A68},
	{"SVSWAP", 2878696, 0x9F0928E1},
	{"VGADICT", 1024, 0x093EB7C4},
	{"VGAGRAPH", 597557, 0xFE21558C},
	{"VGAHEAD", 669, 0x83195034},
	{"VSWAP", 2767592, 0xBB222814},
	{NULL, 0, 0}
};

static const BE_GameFileDetails_T aog210[] =
{
	{"AUDIOHED", 1280, 0x9397D376},
	{"AUDIOT", 267636, 0xB33A17D2},
	{"EANIM", 186221, 0x057B9573},
	{"GANIM", 219918, 0x8B0781A2},
	{"IANIM", 18977, 0xBE1CD872},
	{"MAPHEAD", 834, 0xD8E86D2C},
	{"MAPTEMP", 526196, 0x73D47D3D},
	{"SANIM", 276784, 0x0E6B7A68},
	{"SVSWAP", 2878696, 0x0B8A2614},
	{"VGADICT", 1024, 0x05701230},
	{"VGAGRAPH", 599397, 0xAD97370E},
	{"VGAHEAD", 672, 0x27023C24},
	{"VSWAP", 2767080, 0x069F1D34},
	{NULL, 0, 0}
};

static const BE_GameFileDetails_T aog300[] =
{
	{"AUDIOHED", 1280, 0x9397D376},
	{"AUDIOT", 267636, 0xB33A17D2},
	{"EANIM", 186221, 0x057B9573},
	{"GANIM", 219918, 0x8B0781A2},
	{"IANIM", 18977, 0xBE1CD872},
	{"MAPHEAD", 834, 0xD8E86D2C},
	{"MAPTEMP", 526196, 0x73D47D3D},
	{"SANIM", 276784, 0x0E6B7A68},
	{"SVSWAP", 2878696, 0x0B8A2614},
	{"VGADICT", 1024, 0xFD93D1F8},
	{"VGAGRAPH", 709440, 0x9A6397D6},
	{"VGAHEAD", 678, 0xA25C3BE6},
	{"VSWAP", 2767080, 0x069F1D34},
	{NULL, 0, 0}
};

/*
 (C) 1995-2017 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu
*/

#define local static
#define z_crc_t uint32_t
#define z_size_t size_t
#define Z_NULL NULL
#define FAR
#define TBLS 1
#define ZEXPORT

local const z_crc_t FAR crc_table[TBLS][256] =
{
  {
    0x00000000UL, 0x77073096UL, 0xee0e612cUL, 0x990951baUL, 0x076dc419UL,
    0x706af48fUL, 0xe963a535UL, 0x9e6495a3UL, 0x0edb8832UL, 0x79dcb8a4UL,
    0xe0d5e91eUL, 0x97d2d988UL, 0x09b64c2bUL, 0x7eb17cbdUL, 0xe7b82d07UL,
    0x90bf1d91UL, 0x1db71064UL, 0x6ab020f2UL, 0xf3b97148UL, 0x84be41deUL,
    0x1adad47dUL, 0x6ddde4ebUL, 0xf4d4b551UL, 0x83d385c7UL, 0x136c9856UL,
    0x646ba8c0UL, 0xfd62f97aUL, 0x8a65c9ecUL, 0x14015c4fUL, 0x63066cd9UL,
    0xfa0f3d63UL, 0x8d080df5UL, 0x3b6e20c8UL, 0x4c69105eUL, 0xd56041e4UL,
    0xa2677172UL, 0x3c03e4d1UL, 0x4b04d447UL, 0xd20d85fdUL, 0xa50ab56bUL,
    0x35b5a8faUL, 0x42b2986cUL, 0xdbbbc9d6UL, 0xacbcf940UL, 0x32d86ce3UL,
    0x45df5c75UL, 0xdcd60dcfUL, 0xabd13d59UL, 0x26d930acUL, 0x51de003aUL,
    0xc8d75180UL, 0xbfd06116UL, 0x21b4f4b5UL, 0x56b3c423UL, 0xcfba9599UL,
    0xb8bda50fUL, 0x2802b89eUL, 0x5f058808UL, 0xc60cd9b2UL, 0xb10be924UL,
    0x2f6f7c87UL, 0x58684c11UL, 0xc1611dabUL, 0xb6662d3dUL, 0x76dc4190UL,
    0x01db7106UL, 0x98d220bcUL, 0xefd5102aUL, 0x71b18589UL, 0x06b6b51fUL,
    0x9fbfe4a5UL, 0xe8b8d433UL, 0x7807c9a2UL, 0x0f00f934UL, 0x9609a88eUL,
    0xe10e9818UL, 0x7f6a0dbbUL, 0x086d3d2dUL, 0x91646c97UL, 0xe6635c01UL,
    0x6b6b51f4UL, 0x1c6c6162UL, 0x856530d8UL, 0xf262004eUL, 0x6c0695edUL,
    0x1b01a57bUL, 0x8208f4c1UL, 0xf50fc457UL, 0x65b0d9c6UL, 0x12b7e950UL,
    0x8bbeb8eaUL, 0xfcb9887cUL, 0x62dd1ddfUL, 0x15da2d49UL, 0x8cd37cf3UL,
    0xfbd44c65UL, 0x4db26158UL, 0x3ab551ceUL, 0xa3bc0074UL, 0xd4bb30e2UL,
    0x4adfa541UL, 0x3dd895d7UL, 0xa4d1c46dUL, 0xd3d6f4fbUL, 0x4369e96aUL,
    0x346ed9fcUL, 0xad678846UL, 0xda60b8d0UL, 0x44042d73UL, 0x33031de5UL,
    0xaa0a4c5fUL, 0xdd0d7cc9UL, 0x5005713cUL, 0x270241aaUL, 0xbe0b1010UL,
    0xc90c2086UL, 0x5768b525UL, 0x206f85b3UL, 0xb966d409UL, 0xce61e49fUL,
    0x5edef90eUL, 0x29d9c998UL, 0xb0d09822UL, 0xc7d7a8b4UL, 0x59b33d17UL,
    0x2eb40d81UL, 0xb7bd5c3bUL, 0xc0ba6cadUL, 0xedb88320UL, 0x9abfb3b6UL,
    0x03b6e20cUL, 0x74b1d29aUL, 0xead54739UL, 0x9dd277afUL, 0x04db2615UL,
    0x73dc1683UL, 0xe3630b12UL, 0x94643b84UL, 0x0d6d6a3eUL, 0x7a6a5aa8UL,
    0xe40ecf0bUL, 0x9309ff9dUL, 0x0a00ae27UL, 0x7d079eb1UL, 0xf00f9344UL,
    0x8708a3d2UL, 0x1e01f268UL, 0x6906c2feUL, 0xf762575dUL, 0x806567cbUL,
    0x196c3671UL, 0x6e6b06e7UL, 0xfed41b76UL, 0x89d32be0UL, 0x10da7a5aUL,
    0x67dd4accUL, 0xf9b9df6fUL, 0x8ebeeff9UL, 0x17b7be43UL, 0x60b08ed5UL,
    0xd6d6a3e8UL, 0xa1d1937eUL, 0x38d8c2c4UL, 0x4fdff252UL, 0xd1bb67f1UL,
    0xa6bc5767UL, 0x3fb506ddUL, 0x48b2364bUL, 0xd80d2bdaUL, 0xaf0a1b4cUL,
    0x36034af6UL, 0x41047a60UL, 0xdf60efc3UL, 0xa867df55UL, 0x316e8eefUL,
    0x4669be79UL, 0xcb61b38cUL, 0xbc66831aUL, 0x256fd2a0UL, 0x5268e236UL,
    0xcc0c7795UL, 0xbb0b4703UL, 0x220216b9UL, 0x5505262fUL, 0xc5ba3bbeUL,
    0xb2bd0b28UL, 0x2bb45a92UL, 0x5cb36a04UL, 0xc2d7ffa7UL, 0xb5d0cf31UL,
    0x2cd99e8bUL, 0x5bdeae1dUL, 0x9b64c2b0UL, 0xec63f226UL, 0x756aa39cUL,
    0x026d930aUL, 0x9c0906a9UL, 0xeb0e363fUL, 0x72076785UL, 0x05005713UL,
    0x95bf4a82UL, 0xe2b87a14UL, 0x7bb12baeUL, 0x0cb61b38UL, 0x92d28e9bUL,
    0xe5d5be0dUL, 0x7cdcefb7UL, 0x0bdbdf21UL, 0x86d3d2d4UL, 0xf1d4e242UL,
    0x68ddb3f8UL, 0x1fda836eUL, 0x81be16cdUL, 0xf6b9265bUL, 0x6fb077e1UL,
    0x18b74777UL, 0x88085ae6UL, 0xff0f6a70UL, 0x66063bcaUL, 0x11010b5cUL,
    0x8f659effUL, 0xf862ae69UL, 0x616bffd3UL, 0x166ccf45UL, 0xa00ae278UL,
    0xd70dd2eeUL, 0x4e048354UL, 0x3903b3c2UL, 0xa7672661UL, 0xd06016f7UL,
    0x4969474dUL, 0x3e6e77dbUL, 0xaed16a4aUL, 0xd9d65adcUL, 0x40df0b66UL,
    0x37d83bf0UL, 0xa9bcae53UL, 0xdebb9ec5UL, 0x47b2cf7fUL, 0x30b5ffe9UL,
    0xbdbdf21cUL, 0xcabac28aUL, 0x53b39330UL, 0x24b4a3a6UL, 0xbad03605UL,
    0xcdd70693UL, 0x54de5729UL, 0x23d967bfUL, 0xb3667a2eUL, 0xc4614ab8UL,
    0x5d681b02UL, 0x2a6f2b94UL, 0xb40bbe37UL, 0xc30c8ea1UL, 0x5a05df1bUL,
    0x2d02ef8dUL
  }
};

#define DO1 crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8)
#define DO8 DO1; DO1; DO1; DO1; DO1; DO1; DO1; DO1

unsigned long ZEXPORT crc32_z(crc, buf, len)
    unsigned long crc;
    const unsigned char FAR *buf;
    z_size_t len;
{
    if (buf == Z_NULL) return 0UL;

    crc = crc ^ 0xffffffffUL;
    while (len >= 8) {
        DO8;
        len -= 8;
    }
    if (len) do {
        DO1;
    } while (--len);
    return crc ^ 0xffffffffUL;
}

uint32_t crc32(FILE *stream)
{
	uint32_t a32;
	struct stat statbuf;
	int pos;
	void *buffer;
	int nread;
	int length;

	if (fstat(fileno(stream), &statbuf))
		return 0;

	length = statbuf.st_size;

	if (!(buffer = malloc(XD3_ALLOCSIZE)))
		return 0;

	pos = fseek(stream, 0, SEEK_SET);

	a32 = 0;
	while (length)
	{
		nread = fread(buffer, 1, XD3_ALLOCSIZE, stream);
		if (nread == 0)
			break;
		//a32 = adler32(a32, buffer, nread);
		a32 = crc32_z(a32, buffer, nread);
//printf("%d %d %08x\n", length, nread, a32);
		length -= nread;
	}

	fseek(stream, pos, SEEK_SET);

	return a32;
}

int code(FILE* InFile, FILE* SrcFile, FILE* OutFile)
{
  int r, ret;
  struct stat statbuf;
  xd3_stream stream;
  xd3_config config;
  xd3_source source;
  void* Input_Buf;
  int Input_Buf_Read;

  int BufSize = XD3_ALLOCSIZE; // TODO remove

  memset (&stream, 0, sizeof (stream));
  memset (&source, 0, sizeof (source));

  xd3_init_config(&config, XD3_ADLER32);
  xd3_config_stream(&stream, &config);

  if (SrcFile)
  {
    r = fstat(fileno(SrcFile), &statbuf);
    if (r)
      return r;

    source.blksize = BufSize;
    source.curblk = malloc(source.blksize);

    /* Load 1st block of stream. */
    r = fseek(SrcFile, 0, SEEK_SET);
    if (r)
      return r;
    source.onblk = fread((void*)source.curblk, 1, source.blksize, SrcFile);
    source.curblkno = 0;
    /* Set the stream. */
    xd3_set_source(&stream, &source);
  }

  Input_Buf = malloc(BufSize);

  fseek(InFile, 0, SEEK_SET);
  do
  {
	putchar('.'); fflush(stdout); // progress display
    Input_Buf_Read = fread(Input_Buf, 1, BufSize, InFile);
    if (Input_Buf_Read < BufSize)
    {
      xd3_set_flags(&stream, XD3_FLUSH | stream.flags);
    }
    xd3_avail_input(&stream, Input_Buf, Input_Buf_Read);

process:
    /*if (encode)
      ret = xd3_encode_input(&stream);
    else*/
      ret = xd3_decode_input(&stream);

    switch (ret)
    {
    case XD3_INPUT:
      {
        //fprintf (stderr,"XD3_INPUT\n");
        continue;
      }

    case XD3_OUTPUT:
      {
        //fprintf (stderr,"XD3_OUTPUT\n");
        r = fwrite(stream.next_out, 1, stream.avail_out, OutFile);
        if (r != (int)stream.avail_out)
          return r;
	xd3_consume_output(&stream);
        goto process;
      }

    case XD3_GETSRCBLK:
      {
        //fprintf (stderr,"XD3_GETSRCBLK %qd\n", source.getblkno);
        if (SrcFile)
        {
          r = fseek(SrcFile, source.blksize * source.getblkno, SEEK_SET);
          if (r)
            return r;
          source.onblk = fread((void*)source.curblk, 1,
			       source.blksize, SrcFile);
          source.curblkno = source.getblkno;
        }
        goto process;
      }

    case XD3_GOTHEADER:
      {
        //fprintf (stderr,"XD3_GOTHEADER\n");
        goto process;
      }

    case XD3_WINSTART:
      {
        //fprintf (stderr,"XD3_WINSTART\n");
        goto process;
      }

    case XD3_WINFINISH:
      {
        //fprintf (stderr,"XD3_WINFINISH\n");
        goto process;
      }

    default:
      {
        fprintf (stderr,"!!! INVALID %s %d !!!\n",
		stream.msg, ret);
        return ret;
      }

    }

  }
  while (Input_Buf_Read == BufSize);

  free(Input_Buf);

  free((void*)source.curblk);
  xd3_close_stream(&stream);
  xd3_free_stream(&stream);

  return 0;
}

void xprintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

// based on ioquake3
char *va(const char *format, ...)
{
	va_list		argptr;
	static char string[2][256];
	static int	index = 0;
	char		*buf;

	buf = string[index & 1];
	index++;

	va_start(argptr, format);
	vsnprintf(buf, sizeof(*string), format, argptr);
	va_end(argptr);

	return buf;
}

// GCC 3.4.0 optimizer bug workaround
static int compare_crc(uint32_t a1, uint32_t a2)
{
	//printf("%s(%d,%d)\n", __FUNCTION__, a1, a2);
	return a1 == a2;
}

int main(void)
{
	//const BE_GameFileDetails_T *details = aog210[0];
	int i;
	char filename[13];
	char patchname[256];
	char newname[13];
	char oldname[13];
	uint32_t adler;
	//int size;
	FILE *src, *in, *out;
	char *patchdir = NULL;

	int res = 0;
	for (i = 0; aog210[i].filename != NULL; i++)
	{
		// get the checksum
		snprintf(filename, sizeof(filename), "%s.%s", aog210[i].filename, AOG_EXT);
		printf("checking %s... ", filename);
		if (!(src = fopen(filename, "rb")))
		{
			printf("can't open the file %s\n", strerror(errno));
			break;
		}
		adler = crc32(src);
		fclose(src);
		if (adler == aog210[i].crc32)
		{
			printf("OK\n");
			continue;
		}
		else if (compare_crc(aog200[i].crc32, adler))
		{
			printf("v2.00R\n");
			patchdir = "aog200";
		}
		else if (compare_crc(aog300[i].crc32, adler))
		{
			printf("v3.00R\n");
			patchdir = "aog300";
		}
		else
		{
			printf("unknown crc %08x, contact the developer!\n", adler);
			break;
		}

		// BS6 + VCDIFF -> NEW
		printf("patching the file");
		if (!(src = fopen(filename, "rb")))
		{
			printf("can't open the source file %s\n", strerror(errno));
			break;
		}
		snprintf(patchname, sizeof(patchname), "%s/%s.%s", patchdir, aog210[i].filename, XD3_EXT);
		if (!(in = fopen(patchname, "rb")))
		{
			fclose(src);
			printf("can't open the patch file %s\n", strerror(errno));
			break;
		}
		snprintf(newname, sizeof(newname), "%s.%s", aog210[i].filename, "NEW");
		if (!(out = fopen(newname, "wb")))
		{
			fclose(in);
			fclose(src);
			printf("can't open the output file %s\n", strerror(errno));
			break;
		}
		res = code(in, src, out);
		fclose(out);
		fclose(src);
		fclose(in);
		//printf("\n");
		if (res)
		{
			printf("patching failed\n");
			break;
		}

		// BS6 -> OLD
		snprintf(oldname, sizeof(oldname), "%s.%s", aog210[i].filename, "OLD");
		//printf("filename %s oldname %s\n", filename, oldname);
		//if (rename(filename, va("%s.%s", aog210[i].filename, "OLD")))
		unlink(oldname);
		if (rename(filename, oldname))
		{
			printf("cant't rename the old file: %s\n", strerror(errno));
			break;
		}
		// NEW -> BS6
		if (rename(newname, filename))
		{
			printf("cant't rename the patched file: %s\n", strerror(errno));
			break;
		}
		printf(" OK\n");
	}

	return res;
}
