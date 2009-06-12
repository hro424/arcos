#include <stdio.h>
#include <stdint.h>

static inline void
myoutb(unsigned int port, uint8_t value )
{
	__asm__ __volatile__ ("outb %1, %%dx" : :"d"(port), "a"(value));
}

static inline uint8_t
myinb(unsigned int port)
{
	uint8_t tmp;
	__asm__ __volatile__ ("inb %%dx, %0" :"=a"(tmp) :"d"(port));
	return tmp;
}

#define COMPORT 0x3f8

static int
ser_out(int c)
{
	/* Do file output! */
	while (!(myinb(COMPORT+5) & 0x60));
	myoutb(COMPORT, (uint8_t)c);
	if (c == '\n')
		ser_out('\r');
	return 0;
}

static size_t
l4kdb_write(const void *data, long int position, size_t count, void *handle /*unused*/)
{
	size_t i;
	char *real_data = (char *)data;
	for (i = 0; i < count; i++)
		ser_out(real_data[i]);
	return count;
}

struct __file __stdin = {
	0,
	0,
	0,
	0,
	0,
	_IONBF,
	0,
	0,
	0
};


struct __file __stdout = {
	0,
	0,
	l4kdb_write,
	0,
	0,
	_IONBF,
	0,
	0,
	0
};


struct __file __stderr = {
	0,
	0,
	l4kdb_write,
	0,
	0,
	_IONBF,
	0,
	0,
	0
};

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;
