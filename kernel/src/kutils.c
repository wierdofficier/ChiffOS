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
/* Find the first occurrence of C in S or the final NUL byte.  */
char *
strchrnul (const char *s, int c_in)
{
  const unsigned char *char_ptr;
  const unsigned long int *longword_ptr;
  unsigned long int longword, magic_bits, charmask;
  unsigned char c;
  c = (unsigned char) c_in;
  /* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = (const unsigned char *) s;
       ((unsigned long int) char_ptr & (sizeof (longword) - 1)) != 0;
       ++char_ptr)
    if (*char_ptr == c || *char_ptr == '\0')
      return (void *) char_ptr;
  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to 8-byte longwords.  */
  longword_ptr = (unsigned long int *) char_ptr;
  /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:
     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD
     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
  magic_bits = -1;
  magic_bits = magic_bits / 0xff * 0xfe << 1 >> 1 | 1;
  /* Set up a longword, each of whose bytes is C.  */
  charmask = c | (c << 8);
  charmask |= charmask << 16;
  if (sizeof (longword) > 4)
    /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
    charmask |= (charmask << 16) << 16;
  if (sizeof (longword) > 8)
    abort ();
  /* Instead of the traditional loop which tests each character,
     we will test a longword at a time.  The tricky part is testing
     if *any of the four* bytes in the longword in question are zero.  */
  for (;;)
    {
      /* We tentatively exit the loop if adding MAGIC_BITS to
         LONGWORD fails to change any of the hole bits of LONGWORD.
         1) Is this safe?  Will it catch all the zero bytes?
         Suppose there is a byte with all zeros.  Any carry bits
         propagating from its left will fall into the hole at its
         least significant bit and stop.  Since there will be no
         carry from its most significant bit, the LSB of the
         byte to the left will be unchanged, and the zero will be
         detected.
         2) Is this worthwhile?  Will it ignore everything except
         zero bytes?  Suppose every byte of LONGWORD has a bit set
         somewhere.  There will be a carry into bit 8.  If bit 8
         is set, this will carry into bit 16.  If bit 8 is clear,
         one of bits 9-15 must be set, so there will be a carry
         into bit 16.  Similarly, there will be a carry into bit
         24.  If one of bits 24-30 is set, there will be a carry
         into bit 31, so all of the hole bits will be changed.
         The one misfire occurs when bits 24-30 are clear and bit
         31 is set; in this case, the hole at bit 31 is not
         changed.  If we had access to the processor carry flag,
         we could close this loophole by putting the fourth hole
         at bit 32!
         So it ignores everything except 128's, when they're aligned
         properly.
         3) But wait!  Aren't we looking for C as well as zero?
         Good point.  So what we do is XOR LONGWORD with a longword,
         each of whose bytes is C.  This turns each byte that is C
         into a zero.  */
      longword = *longword_ptr++;
      /* Add MAGIC_BITS to LONGWORD.  */
      if ((((longword + magic_bits)
            /* Set those bits that were unchanged by the addition.  */
            ^ ~longword)
           /* Look at only the hole bits.  If any of the hole bits
              are unchanged, most likely one of the bytes was a
              zero.  */
           & ~magic_bits) != 0 ||
          /* That caught zeroes.  Now test for C.  */
          ((((longword ^ charmask) + magic_bits) ^ ~(longword ^ charmask))
           & ~magic_bits) != 0)
        {
          /* Which of the bytes was C or zero?
             If none of them were, it was a misfire; continue the search.  */
          const unsigned char *cp = (const unsigned char *) (longword_ptr - 1);
          if (*cp == c || *cp == '\0')
            return (char *) cp;
          if (*++cp == c || *cp == '\0')
            return (char *) cp;
          if (*++cp == c || *cp == '\0')
            return (char *) cp;
          if (*++cp == c || *cp == '\0')
            return (char *) cp;
          if (sizeof (longword) > 4)
            {
              if (*++cp == c || *cp == '\0')
                return (char *) cp;
              if (*++cp == c || *cp == '\0')
                return (char *) cp;
              if (*++cp == c || *cp == '\0')
                return (char *) cp;
              if (*++cp == c || *cp == '\0')
                return (char *) cp;
            }
        }
    }
  /* This should never happen.  */
  return NULL;
}

char * strchrnul_(const char * s, int c) {
	size_t * w;
	size_t k;

	c = (unsigned char)c;
	if (!c) {
		return (char *)s + strlen(s);
	}
printk("strchrnul\n");
	for (; (uintptr_t)s % ALIGN; s++) {
		if (!*s || *(unsigned char *)s == c) {
			return (char *)s;
		}
	}
printk("strchrnul2\n");
	k = ONES * c;
	for (w = (void *)s; !HASZERO(*w) && !HASZERO(*w^k); w++);
printk("strchrnul3\n");
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
