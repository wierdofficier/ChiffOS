#ifndef VIDEO_H_
#define VIDEO_H_
#include <types.h>
enum vga_color
{
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};
void clear(void);
void init_video_term(void);
void putch(char c);
void set_term_color(u8 color);
//void puts(const char *text);
extern void kputhex(u32 n);
extern void kputint(u32 n);
int stdio_read(int fd, void *buf, int length);
void printk(const char *fmt, ...);
void put_dec(u32 n);
extern u32 c_y;
extern u32 c_x;
#endif
