#include <types.h>
#include <video.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <timer.h>
#include <multiboot.h>
#include <vmmngr.h>
#include <heapmngr.h>
#include <kbd.h>
#include <fat.h>
#include <proc.h>
#include <logging.h>
#include <timer.h>
log_type_t debug_level = NOTICE;
void * debug_file = NULL;
void (*debug_hook)(void *, char *) = NULL;
void (*debug_video_crash)(char **) = NULL;

static char * c_messages[] = {
	" \033[1;34mINFO\033[0m:",
	" \033[1;35mNOTICE\033[0m:",
	" \033[1;33mWARNING\033[0m:",
	" \033[1;31mERROR\033[0m:",
	" \033[1;37;41mCRITICAL\033[0m:",
	" \033[1;31;44mINSANE\033[0m:"
};

static char buffer[1024];

void _debug_print(char * title, int line_no, log_type_t level, char *fmt, ...) {
	//if (level >= debug_level && debug_file) {
		va_list args;
		va_start(args, fmt);
		vasprintf(buffer, fmt, args);
		va_end(args);

		char * type;
		if (level > INSANE) {
			type = "";
		} else {
			type = c_messages[level];
		}

	 	printk(  "[%10d.%3d:%s:%d]%s %s\n", timerticks, timerticks, title, line_no, type, buffer);
 
	//}
	/* else ignore */
}
