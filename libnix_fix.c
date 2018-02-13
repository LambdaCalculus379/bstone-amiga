#include <stdio.h>

// these come from NOTES-3.4.0 of gcc-3.4.0-diffs.tar.gz

int (getc)(FILE *stream)
{
	return fgetc(stream);
}

int (getchar)(void)
{
	return fgetc(stdin);
}

int (putc)(int c,FILE *stream)
{
	return fputc(c,stream);
}

int (putchar)(int c)
{
	return fputc(c,stdout);
}
