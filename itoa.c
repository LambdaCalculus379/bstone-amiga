#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// loosely based on OpenQNX

static const char xlat[] = "0123456789abcdefghijklmnopqrstuvwxyz";

char *ultoa(unsigned long value, char *str, int radix)
{
	char tmp[32 + 1];
	char *p1 = tmp, *p2;
	do
	{
		*p1++ = xlat[value % (unsigned)radix];
	} while ((value /= (unsigned)radix) > 0);
	for (p2 = str; p1 != tmp; *p2++ = *--p1);
	*p2 = '\0';
	return str;
}

char *ltoa(long value, char *str, int radix)
{
	char *ptr = str;
	if (radix == 10 && value < 0)
	{
		*ptr++ = '-';
		value = -value;
	}
	ultoa(value, ptr, radix);
	return str;
}

char *itoa(int value, char *str, int radix)
{
	return ltoa((long)value, str, radix);
}

#if 0
int main(int argc, char** argv)
{
	char str[256];

	for (int i = 0; i < 20; i++)
	{
		int number = rand();
		printf("%d %s\n", -number, itoa(-number, str, 10));
		printf("%ld %s\n", (long)number, ltoa((long)number, str, 10));
		printf("%lu %s\n", number, ultoa((unsigned long)number, str, 10));
	}

	return 0;
}
#endif
