#include <types.h>

#include <video.h>
#include <kutils.h>
size_t lfind(const char * str, const char accept);
void * memrchr(const void * m, int c, size_t n);
u16 *memsetw(u16 *dest, u16 value, int count)
{
	u16 *temp = (u16 *)dest;
	for( ;count != 0; count--) *temp++ = value;
	
	return dest;	
}
#  define UCHAR_MAX	255
#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(X) (((X)-ONES) & ~(X) & HIGHS)

#define BITOP(A, B, OP) \
((A)[(size_t)(B)/(8*sizeof *(A))] OP (size_t)1<<((size_t)(B)%(8*sizeof *(A))))
void *memcpy(void *dest,const void *src,size_t n) { 
  u32 num_dwords = n/4;
  u32 num_bytes = n%4;
  u32 *dest32 = (u32*)dest;
  u32 *src32 = (u32*)src;
  u8 *dest8 = ((u8*)dest)+num_dwords*4;
  u8 *src8 = ((u8*)src)+num_dwords*4;
  u32 i;

  for (i=0;i<num_dwords;i++) {
    dest32[i] = src32[i];
  }
  for (i=0;i<num_bytes;i++) {
    dest8[i] = src8[i];
  }
  return dest;
}

void *memset(void *dest,int val,size_t n) { 
  u32 num_dwords = n/4;
  u32 num_bytes = n%4;
  u32 *dest32 = (u32*)dest;
  u8 *dest8 = ((u8*)dest)+num_dwords*4;
  u8 val8 = (u8)val;
  u32 val32 = val|(val<<8)|(val<<16)|(val<<24);
  u32 i;

  for (i=0;i<num_dwords;i++) {
    dest32[i] = val32;
  }
  for (i=0;i<num_bytes;i++) {
    dest8[i] = val8;
  }
  return dest;
}

int strlen( char* str)
{
	int ret = 0;
	while ( str[ret] != 0 )
		ret++;
	return ret;
}

void outb(u16 port, u8 data)
{	
	__asm__ __volatile__("outb %1, %0" : : "dN" (port), "a" (data));		
}

u8 inb(u16 port)
{
   u8 ret;
   __asm__ volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}
void outw(u16 port, u16 val)
{
    __asm__ volatile ("out %%ax,%%dx" :: "a"(val), "d"(port));
}

void outl(u16 port, u32 val)
{
    __asm__ volatile ("outl %%eax,%%dx" : : "a"(val), "d"(port));
}

u16 inw(u16 port)
{
    u16 ret_val;
    __asm__ volatile ("in %%dx,%%ax" : "=a" (ret_val) : "d"(port));
    return ret_val;
}

u32 inl(u16 port)
{
    u32 ret_val;
    __asm__ volatile ("in %%dx,%%eax" : "=a" (ret_val) : "d"(port));
    return ret_val;
}
char *strcpy(char *dest, const char *src)
{
    do
    {
      *dest++ = *src++;
    }
    while (*src != 0);
}
u32 strcmp(const char *s1, const char *s2)
{
	while((*s1) && (*s1 == *s2))
	{
		++s1;
		++s2;
	}	
	return (*s1 - *s2);
}
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}
int strncmp( const char *s1,  const char *s2, size_t n) {
unsigned char uc1, uc2;
if (n == 0 || s1 == 0 || s2 == 0)
return 0;
/* Loop, comparing bytes. */
while (n-- > 0 && *s1 == *s2) {
if (n == 0 || *s1 == '\0')
return 0;
s1++, s2++;
}
}
int memcmp(const void *lhs, const void *rhs, size_t count) {
const u8 *us1 = (u8 *)lhs;
const u8 *us2 = (u8 *)rhs;
while (count-- != 0) {
if (*us1 != *us2)
return (*us1 < *us2) ? -1 : 1;
us1++, us2++;
}

return 0;
}
#define toupper(c)      ((c) - 0x20 * (((c) >= 'a') && ((c) <= 'z')))
void ToDosFileName (const char* filename,
              char* fname,
            unsigned int FNameLength) {

	unsigned int  i=0;

	if (FNameLength > 11)
		return;

	if (!fname || !filename)
		return;


	memset (fname, ' ', FNameLength);


	for (i=0; i < strlen(filename) && i < FNameLength; i++) {

		if (filename[i] == '.' || i==8 )
			break;

		fname[i] = toupper (filename[i] );
	}

	if (filename[i]=='.') {
int k;

		for ( k=0; k<3; k++) {

			++i;
			if ( filename[i] )
				fname[8+k] = filename[i];
		}
	}
	for (i = 0; i < 3; i++)
		fname[8+i] = toupper (fname[8+i]);
}

void reboot()
{
    outb (0x64, 0xFE); /* send reboot command */
}



void outportsm(unsigned short port, unsigned char * data, unsigned long size) {
	__asm__ __volatile__ ("rep outsw" : "+S" (data), "+c" (size) : "d" (port));
}


void inportsm(unsigned short port, unsigned char * data, unsigned long size) {
	__asm__ __volatile__ ("rep insw" : "+D" (data), "+c" (size) : "d" (port) : "memory");
}

void outportb(u16 port, u8 data)
{	
__asm__ __volatile__("outb %1, %0" : : "dN" (port), "a" (data));	
}

u8 inportb(u16 port)
{
   u8 ret;
   __asm__ volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}
void outports(u16 port, u16 val)
{
    __asm__ volatile ("out %%ax,%%dx" :: "a"(val), "d"(port));
}
void outportw(u16 port, u16 val)
{
    __asm__ volatile ("out %%ax,%%dx" :: "a"(val), "d"(port));
}

void outportl(u16 port, u32 val)
{
    __asm__ volatile ("outl %%eax,%%dx" : : "a"(val), "d"(port));
}

u16 inports(u16 port)
{
    u16 ret_val;
    __asm__ volatile ("in %%dx,%%ax" : "=a" (ret_val) : "d"(port));
    return ret_val;
}

u16 inportw(u16 port)
{
    u16 ret_val;
    __asm__ volatile ("in %%dx,%%ax" : "=a" (ret_val) : "d"(port));
    return ret_val;
}
u32 inportl(u16 port)
{
    u32 ret_val;
    __asm__ volatile ("in %%dx,%%eax" : "=a" (ret_val) : "d"(port));
    return ret_val;
}
size_t strspn(const char * s, const char * c) {
	const char * a = s;
	size_t byteset[32/sizeof(size_t)] = { 0 };

	if (!c[0]) {
		return 0;
	}
	if (!c[1]) {
		for (; *s == *c; s++);
		return s-a;
	}

	for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
	for (; *s && BITOP(byteset, *(unsigned char *)s, &); s++);

	return s-a;
}

char * strchrnul(const char * s, int c) {
	size_t * w;
	size_t k;

	c = (unsigned char)c;
	if (!c) {
		return (char *)s + strlen(s);
	}

	for (; (uintptr_t)s % ALIGN; s++) {
		if (!*s || *(unsigned char *)s == c) {
			return (char *)s;
		}
	}

	k = ONES * c;
	for (w = (void *)s; !HASZERO(*w) && !HASZERO(*w^k); w++);
	for (s = (void *)w; *s && *(unsigned char *)s != c; s++);
	return (char *)s;
}

char * strchr(const char * s, int c) {
	char *r = strchrnul(s, c);
	return *(unsigned char *)r == (unsigned char)c ? r : 0;
}

char * strrchr(const char * s, int c) {
	return memrchr(s, c, strlen(s) + 1);
}

size_t strcspn(const char * s, const char * c) {
	const char *a = s;
	if (c[0] && c[1]) {
		size_t byteset[32/sizeof(size_t)] = { 0 };
		for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
		for (; *s && !BITOP(byteset, *(unsigned char *)s, &); s++);
		return s-a;
	}
	return strchrnul(s, *c)-a;
}

char * strpbrk(const char * s, const char * b) {
	s += strcspn(s, b);
	return *s ? (char *)s : 0;
}
char * strdup(const char * s) {
	size_t l = strlen(s);
	return memcpy(valloc(l+1), s, l+1);
}
char * strtok_r(char * str, const char * delim, char ** saveptr) {
	char * token;
	if (str == NULL) {
		str = *saveptr;
	}
	str += strspn(str, delim);
	if (*str == '\0') {
		*saveptr = str;
		return NULL;
	}
	token = str;
	str = strpbrk(token, delim);
	if (str == NULL) {
		*saveptr = (char *)lfind(token, '\0');
	} else {
		*str = '\0';
		*saveptr = str + 1;
	}
	return token;
}
void * memmove(void * dest, const void * src, size_t n) {
	char * d = dest;
	const char * s = src;

	if (d==s) {
		return d;
	}

	if (s+n <= d || d+n <= s) {
		return memcpy(d, s, n);
	}

	if (d<s) {
		if ((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
			while ((uintptr_t)d % sizeof(size_t)) {
				if (!n--) {
					return dest;
				}
				*d++ = *s++;
			}
			for (; n >= sizeof(size_t); n -= sizeof(size_t), d += sizeof(size_t), s += sizeof(size_t)) {
				*(size_t *)d = *(size_t *)s;
			}
		}
		for (; n; n--) {
			*d++ = *s++;
		}
	} else {
		if ((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
			while ((uintptr_t)(d+n) % sizeof(size_t)) {
				if (!n--) {
					return dest;
				}
				d[n] = s[n];
			}
			while (n >= sizeof(size_t)) {
				n -= sizeof(size_t);
				*(size_t *)(d+n) = *(size_t *)(s+n);
			}
		}
		while (n) {
			n--;
			d[n] = s[n];
		}
	}

	return dest;
}

void * memrchr(const void * m, int c, size_t n) {
	const unsigned char * s = m;
	c = (unsigned char)c;
	while (n--) {
		if (s[n] == c) {
			return (void*)(s+n);
		}
	}
	return 0;
}

size_t lfind(const char * str, const char accept) {
	return (size_t)strchr(str, accept);
}

size_t rfind(const char * str, const char accept) {
	return (size_t)strrchr(str, accept);
}
