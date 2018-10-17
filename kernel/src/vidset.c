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
#include <palette.h>
#define PREFERRED_W  1268
#define PREFERRED_H  1024
#define PREFERRED_VY 4096
#define PREFERRED_B 32
#define PCI_BAR0 0x10 // 4
#include <terminal-font.h>
struct procfs_entry {
	int          id;
	char *       name;
	read_type_t  func;
};
extern struct multiboot *mboot_ptr;
/* Exported to other modules */
unsigned short  lfb_resolution_x = 0;
unsigned short  lfb_resolution_y = 0;
unsigned short  lfb_resolution_b = 0;
unsigned int lfb_resolution_s = 0;
unsigned char  * lfb_vid_memory = (unsigned char  *)0xE0000000;
const char * lfb_driver_name = NULL;

/* Where to send display size change signals */
static pid_t display_change_recipient = 0;

/* Driver-specific modesetting function */
static void (*lfb_resolution_impl)(unsigned short ,unsigned short ) = NULL;

/* Called by ioctl on /dev/fb0 */
void lfb_set_resolution(unsigned short  x, unsigned short  y) {
	if (lfb_resolution_impl) {
		lfb_resolution_impl(x,y);
		if (display_change_recipient) {
			send_signal(display_change_recipient, SIGWINEVENT, 1);
			debug_print(WARNING, "Telling %d to SIGWINEVENT", display_change_recipient);
		}
	}
}

/**
 * Framebuffer control ioctls.
 * Used by the compositor to get display sizes and by the
 * resolution changer to initiate modesetting.
 */

static int ioctl_vid(fs_node_t * node, int request, void * argp) {
	/*switch (request) {
		case IO_VID_WIDTH:
			// Get framebuffer width  
			validate(argp);
			*((size_t *)argp) = lfb_resolution_x;
			return 0;
		case IO_VID_HEIGHT:
			//  Get framebuffer height  
			validate(argp);
			*((size_t *)argp) = lfb_resolution_y;
			return 0;
		case IO_VID_DEPTH:
			//  Get framebuffer bit depth  
			validate(argp);
			*((size_t *)argp) = lfb_resolution_b;
			return 0;
		case IO_VID_STRIDE:
			//  Get framebuffer scanline stride  
			validate(argp);
			*((size_t *)argp) = lfb_resolution_s;
			return 0;
		case IO_VID_ADDR:
			//  Get framebuffer address - TODO: map the framebuffer?  
			validate(argp);
			*((uintptr_t *)argp) = (uintptr_t)lfb_vid_memory;
			return 0;
		case IO_VID_SIGNAL:
			//  ioctl to register for a signal (vid device change? idk) on display change 
			display_change_recipient = getpid();
			return 0;
		case IO_VID_SET:
			//   Initiate mode setting  
			validate(argp);
			lfb_set_resolution(((struct vid_size *)argp)->width, ((struct vid_size *)argp)->height);
			return 0;
		case IO_VID_DRIVER:
			validate(argp);
			memcpy(argp, lfb_driver_name, strlen(lfb_driver_name));
			return 0;
		default:
			return -EINVAL;
	}*/
}

/* Framebuffer device file initializer */
static fs_node_t * lfb_video_device_create(void /* TODO */) {
	fs_node_t * fnode = malloc(sizeof(fs_node_t));
	memset(fnode, 0x00, sizeof(fs_node_t));
	sprintf(fnode->name, "fb0"); /* TODO */
	fnode->length  = lfb_resolution_s * lfb_resolution_y; /* Size is framebuffer size in bytes */
	fnode->flags   = FS_BLOCKDEVICE; /* Framebuffers are block devices */
	fnode->mask    = 0660; /* Only accessible to root user/group */
	fnode->ioctl   = ioctl_vid; // control function defined above  
	return fnode;
}

/**
 * Framebuffer fatal error presentation.
 *
 * This is called by a kernel hook to render fatal error messages
 * (panic / oops / bsod) to the graphraical framebuffer. Mostly,
 * that means the "out of memory" error. Bescause this is a fatal
 * error condition, we don't care much about speed, so we can do
 * silly things like ready from the framebuffer, which we do to
 * produce a vignetting and desaturation effect.
 */
static int vignette_at(int x, int y) {
	int amount = 0;
	int level = 100;
	if (x < level) amount += (level - x);
	if (x > lfb_resolution_x - level) amount += (level - (lfb_resolution_x - x));
	if (y < level) amount += (level - y);
	if (y > lfb_resolution_y - level) amount += (level - (lfb_resolution_y - y));
	return amount;
}

 

/* XXX Why is this not defined in the font header...  */
 
  unsigned short char_width     = 8;    /* Width of a cell in pixels */
  unsigned short char_height    = 16;   /* Height of a cell in pixels */
/* Set point in framebuffer */
static void set_point(int x, int y, uint32_t value) {
	uint32_t * disp = (uint32_t *)lfb_vid_memory;
	uint32_t * cell = &disp[y * (lfb_resolution_s / 4) + x];
	if(cell != NULL)
	*cell = value;
}

/* Draw text on framebuffer */
  void write_char(int x, int y, int val, uint32_t color) {
	if (val > 128) {
		val = 4;
	}
	unsigned short  * c = large_font[val];
	for (unsigned char  i = 0; i < char_height; ++i) {
		for (unsigned char  j = 0; j < char_width; ++j) {
			if (c[i] & (1 << (15-j))) {
				set_point(x+j,y+i,color);
			}
		}
	}
}

#define _RED(color) ((color & 0x00FF0000) / 0x10000)
#define _GRE(color) ((color & 0x0000FF00) / 0x100)
#define _BLU(color) ((color & 0x000000FF) / 0x1)
#define _ALP(color) ((color & 0xFF000000) / 0x1000000)
static void lfb_video_panic(char ** msgs) {
	/* Desaturate the display */
	uint32_t * disp = (uint32_t *)lfb_vid_memory;
	for (int y = 0; y < lfb_resolution_y; y++) {
		for (int x = 0; x < lfb_resolution_x; x++) {
			uint32_t * cell = &disp[y * (lfb_resolution_s / 4) + x];

			int r = _RED(*cell);
			int g = _GRE(*cell);
			int b = _BLU(*cell);

			int l = 3 * r + 6 * g + 1 * b;
			r = (l) / 10;
			g = (l) / 10;
			b = (l) / 10;

			r = r > 255 ? 255 : r;
			g = g > 255 ? 255 : g;
			b = b > 255 ? 255 : b;

			int amount = vignette_at(x,y);
			r = (r - amount < 0) ? 0 : r - amount;
			g = (g - amount < 0) ? 0 : g - amount;
			b = (b - amount < 0) ? 0 : b - amount;

			*cell = 0xFF000000 + ((0xFF & r) * 0x10000) + ((0xFF & g) * 0x100) + ((0xFF & b) * 0x1); 
		}
	}

	/* Now print the message, divided on line feeds, into the center of the screen */
	int num_entries = 0;
	for (char ** m = msgs; *m; m++, num_entries++);
	int y = (lfb_resolution_y - (num_entries * char_height)) / 2;
	for (char ** message = msgs; *message; message++) {
		int x = (lfb_resolution_x - (strlen(*message) * char_width)) / 2;
		for (char * c = *message; *c; c++) {
			write_char(x+1, y+1, *c, 0xFF000000);
			write_char(x, y, *c, 0xFFFF0000);
			x += char_width;
		}
		y += char_height;
	}
}

static uint32_t framebuffer_func(fs_node_t * node, uint32_t offset, uint32_t size, unsigned char  * buffer) {
	char * buf = malloc(4096);

	if (lfb_driver_name) {
		sprintf(buf,
			"Driver:\t%s\n"
			"XRes:\t%d\n"
			"YRes:\t%d\n"
			"BitsPerPixel:\t%d\n"
			"Stride:\t%d\n"
			"Address:\t0x%x\n",
			lfb_driver_name,
			lfb_resolution_x,
			lfb_resolution_y,
			lfb_resolution_b,
			lfb_resolution_s,
			lfb_vid_memory);
	} else {
		sprintf(buf, "Driver:\tnone\n");
	}

	size_t _bsize = strlen(buf);
	if (offset > _bsize) {
		free(buf);
		return 0;
	}
	if (size > _bsize - offset) size = _bsize - offset;

	memcpy(buffer, buf + offset, size);
	free(buf);
	return size;
}

static struct procfs_entry framebuffer_entry = {
	0,
	"framebuffer",
	framebuffer_func,
};

/* Install framebuffer device */
static void finalize_graphics(const char * driver) {
	lfb_driver_name = driver;
	fs_node_t * fb_device = lfb_video_device_create();
	vfs_mount("/dev/fb0", fb_device);
	debug_video_crash = lfb_video_panic;

	//int (*procfs_install)(struct procfs_entry *) = (int (*)(struct procfs_entry *))(uintptr_t)hashmap_get(modules_get_symbols(),"procfs_install"); //ändring

	//if (procfs_install) {
	//	procfs_install(&framebuffer_entry);
	//}
}//

/* Bochs support {{{ */
static void bochs_scan_pci(uint32_t device, unsigned short  v, unsigned short  d, void * extra) {
	if ((v == 0x1234 && d == 0x1111) ||
	    (v == 0x80EE && d == 0xBEEF) ||
	    (v == 0x10de && d == 0x0a20))  {
		uintptr_t t = pci_read_field(device, PCI_BAR0, 4);
		if (t > 0) {
			*((unsigned char  **)extra) = (unsigned char  *)(t & 0xFFFFFFF0);
		}
	}
}

static void bochs_set_resolution(unsigned short  x, unsigned short  y) {
	outports(0x1CE, 0x04);
	outports(0x1CF, 0x00);
	/* Uh oh, here we go. */
	outports(0x1CE, 0x01);
	outports(0x1CF, x);
	/* Set Y resolution to 768 */
	outports(0x1CE, 0x02);
	outports(0x1CF, y);
	/* Set bpp to 32 */
	outports(0x1CE, 0x03);
	outports(0x1CF, PREFERRED_B);
	/* Set Virtual Height to stuff */
	outports(0x1CE, 0x07);
	outports(0x1CF, PREFERRED_VY);
	/* Turn it back on */
	outports(0x1CE, 0x04);
	outports(0x1CF, 0x41);

	/* Read X to see if it's something else */
	outports(0x1CE, 0x01);
	unsigned short  new_x = inports(0x1CF);
	if (x != new_x) {
		x = new_x;
	}

	lfb_resolution_x = x;
	lfb_resolution_s = x * 4;
	lfb_resolution_y = y;
	lfb_resolution_b = 32;
}

static void graphics_install_bochs(unsigned short  resolution_x, unsigned short  resolution_y) {
	uint32_t vid_memsize;
	debug_print(NOTICE, "Setting up BOCHS/QEMU graphics controller...");

	outports(0x1CE, 0x00);
	unsigned short  i = inports(0x1CF);
	if (i < 0xB0C0 || i > 0xB0C6) {
		return;
	}
 
	outports(0x1CF, 0xB0C4);
	i = inports(0x1CF);
	bochs_set_resolution(resolution_x, resolution_y);
	resolution_x = lfb_resolution_x; /* may have changed */
 
	pci_scan(bochs_scan_pci, -1, &lfb_vid_memory);
 
	lfb_resolution_impl = &bochs_set_resolution;
 
	if (!lfb_vid_memory) {
		debug_print(ERROR, "Failed to locate video memory.");
		return;
	}
 
	/* Enable the higher memory */
	uintptr_t fb_offset = (uintptr_t)lfb_vid_memory;
	for (uintptr_t i = fb_offset; i <= fb_offset + 0xFF0000; i += 0x1000) {
		page_t * p = get_page(i, 1, kernel_directory);
		dma_frame(p, 0, 1, i);
		//p->pat = 1;
		p->writethrough = 1;
		p->cachedisable = 1;
	}
 
	outports(0x1CE, 0x0a);
	i = inports(0x1CF);
 
	if (i > 1) {
		vid_memsize = (uint32_t)i * 64 * 1268;
	} else {
		vid_memsize = inportl(0x1CF);
	}

	debug_print(WARNING, "Video memory size is 0x%x", vid_memsize);
	for (uintptr_t i = (uintptr_t)lfb_vid_memory; i <= (uintptr_t)lfb_vid_memory + vid_memsize; i += 0x1000) {
		dma_frame(get_page(i, 1, kernel_directory), 0, 1, i);
	}

	//finalize_graphics("bochs");
}

static void graphics_install_preset(unsigned short  w, unsigned short  h) {
	if (!(mboot_ptr && (mboot_ptr->flags & (1 << 12)))) {
		debug_print(ERROR, "Failed to locate preset video memory - missing multiboot header.");
		return;
	}

	/* Extract framebuffer information from multiboot */
	lfb_vid_memory = (void *)mboot_ptr->framebuffer_addr;
	lfb_resolution_x = mboot_ptr->framebuffer_width;
	lfb_resolution_y = mboot_ptr->framebuffer_height;
	lfb_resolution_s = mboot_ptr->framebuffer_pitch;
	lfb_resolution_b = 32;

	debug_print(WARNING, "Mode was set by bootloader: %dx%d bpp should be 32, framebuffer is at 0x%x", w, h, (uintptr_t)lfb_vid_memory);

	for (uintptr_t i = (uintptr_t)lfb_vid_memory; i <= (uintptr_t)lfb_vid_memory + w * h * 4; i += 0x1000) {
		page_t * p = get_page(i, 1, kernel_directory);
		dma_frame(p, 0, 1, i);
		//p->pat = 1;
		p->writethrough = 1;
		p->cachedisable = 1;
	}
	finalize_graphics("preset");
}

#define SVGA_IO_BASE (vmware_io)
#define SVGA_IO_MUL 1
#define SVGA_INDEX_PORT 0
#define SVGA_VALUE_PORT 1

#define SVGA_REG_ID 0
#define SVGA_REG_ENABLE 1
#define SVGA_REG_WIDTH 2
#define SVGA_REG_HEIGHT 3
#define SVGA_REG_BITS_PER_PIXEL 7
#define SVGA_REG_BYTES_PER_LINE 12
#define SVGA_REG_FB_START 13

static uint32_t vmware_io = 0;

static void vmware_scan_pci(uint32_t device, unsigned short  v, unsigned short  d, void * extra) {
	if ((v == 0x15ad && d == 0x0405)) {
		uintptr_t t = pci_read_field(device, PCI_BAR0, 4);
		if (t > 0) {
			*((unsigned char  **)extra) = (unsigned char  *)(t & 0xFFFFFFF0);
		}
	}
}

static void vmware_write(int reg, int value) {
	outportl(SVGA_IO_MUL * SVGA_INDEX_PORT + SVGA_IO_BASE, reg);
	outportl(SVGA_IO_MUL * SVGA_VALUE_PORT + SVGA_IO_BASE, value);
}

static uint32_t vmware_read(int reg) {
	outportl(SVGA_IO_MUL * SVGA_INDEX_PORT + SVGA_IO_BASE, reg);
	return inportl(SVGA_IO_MUL * SVGA_VALUE_PORT + SVGA_IO_BASE);
}

static void vmware_set_resolution(unsigned short  w, unsigned short  h) {
	vmware_write(SVGA_REG_ENABLE, 0);
	vmware_write(SVGA_REG_ID, 0);
	vmware_write(SVGA_REG_WIDTH, w);
	vmware_write(SVGA_REG_HEIGHT, h);
	vmware_write(SVGA_REG_BITS_PER_PIXEL, 32);
	vmware_write(SVGA_REG_ENABLE, 1);

	uint32_t bpl = vmware_read(SVGA_REG_BYTES_PER_LINE);

	lfb_resolution_x = w;
	lfb_resolution_s = bpl;
	lfb_resolution_y = h;
	lfb_resolution_b = 32;
}

static void graphics_install_vmware(unsigned short  w, unsigned short  h) {
	pci_scan(vmware_scan_pci, -1, &vmware_io);

	if (!vmware_io) {
		debug_print(ERROR, "No vmware device found?");
		return;
	} else {
		debug_print(WARNING, "vmware io base: 0x%x", vmware_io);
	}

	vmware_set_resolution(w,h);
	lfb_resolution_impl = &vmware_set_resolution;

	uint32_t fb_addr = vmware_read(SVGA_REG_FB_START);
	debug_print(WARNING, "vmware fb address: 0x%x", fb_addr);

	uint32_t fb_size = vmware_read(15);

	debug_print(WARNING, "vmware fb size: 0x%x", fb_size);

	lfb_vid_memory = (unsigned char  *)fb_addr;

	uintptr_t fb_offset = (uintptr_t)lfb_vid_memory;
	for (uintptr_t i = fb_offset; i <= fb_offset + fb_size; i += 0x1000) {
		page_t * p = get_page(i, 1, kernel_directory);
		dma_frame(p, 0, 1, i);
		//p->pat = 1;
		p->writethrough = 1;
		p->cachedisable = 1;
	}

	finalize_graphics("vmware");
}

struct disp_mode {
	signed short x;
	signed short y;
	int set;
};
 
static void auto_scan_pci(unsigned int device, unsigned short  v, unsigned short  d, void * extra) {
	struct disp_mode * mode = extra;
	if (mode->set) return;
	if ((v == 0x1234 && d == 0x1111) ||
	    (v == 0x80EE && d == 0xBEEF) ||
	    (v == 0x10de && d == 0x0a20))  {
		mode->set = 1;
		graphics_install_bochs(mode->x, mode->y);
	} else if ((v == 0x15ad && d == 0x0405)) {
		mode->set = 1;
		graphics_install_vmware(mode->x, mode->y);
	}
}
char *pmessage = "this is a pointer22\n";
int GRAPHICS_ON = 0;
  int init_graphics(void) {

	if (mboot_ptr->vbe_mode_info) {
		lfb_vid_memory = (unsigned char  *)((vbe_info_t *)(mboot_ptr->vbe_mode_info))->physbase;
	}
	unsigned short  x, y;
	x = 1268;//PREFERRED_W;
	y = 1024;//PREFERRED_H;
	debug_print(NOTICE, "Automatically detecting display driver...");
			struct disp_mode mode = {x,y,0};
			pci_scan(auto_scan_pci, -1, &mode);
			if (!mode.set) {
				graphics_install_preset(x,y);
			}
 

//lfb_video_panic(msg);
GRAPHICS_ON =1;
 
return 0;
	char * c;
	if ((c = args_value("vid"))) {
		debug_print(NOTICE, "Video mode requested: %s", c);

		char * arg = strdup(c);
		char * argv[10];
		int argc = tokenize(arg, ",", argv);

		
		if (argc < 3) {
			x = PREFERRED_W;
			y = PREFERRED_H;
		} else {
			x = atoi(argv[1]);
			y = atoi(argv[2]);
		}

		if (!strcmp(argv[0], "auto")) {
			/* Attempt autodetection */
			debug_print(NOTICE, "Automatically detecting display driver...");
			struct disp_mode mode = {x,y,0};
			pci_scan(auto_scan_pci, -1, &mode);
			if (!mode.set) {
				graphics_install_preset(x,y);
			}
		} else if (!strcmp(argv[0], "qemu")) {
			/* Bochs / Qemu Video Device */
			graphics_install_bochs(x,y);
		} else if (!strcmp(argv[0],"vmware")) {
			/* VMware SVGA */
			graphics_install_vmware(x,y);
		} else if (!strcmp(argv[0],"preset")) {
			/* Set by bootloader (UEFI) */
			graphics_install_preset(x,y);
		} else {
			debug_print(WARNING, "Unrecognized video adapter: %s", argv[0]);
		}

		free(arg);
	}

	return 0;
}
 typedef struct {
	uint32_t c;     /* codepoint */
	uint32_t fg;    /* background indexed color */
	uint32_t bg;    /* foreground indexed color */
	uint32_t flags; /* other flags */
} term_cell_t;
#define TERM_BUF_LEN 128
typedef struct {
	uint16_t x;       /* Current cursor location */
	uint16_t y;       /*    "      "       "     */
	uint16_t save_x;  /* Last cursor save */
	uint16_t save_y;
	uint32_t width;   /* Terminal width */
	uint32_t height;  /*     "    height */
	uint32_t fg;      /* Current foreground color */
	uint32_t bg;      /* Current background color */
	uint8_t  flags;   /* Bright, etc. */
	uint8_t  escape;  /* Escape status */
	uint8_t  box;
	uint8_t  buflen;  /* Buffer Length */
	char     buffer[TERM_BUF_LEN];  /* Previous buffer */
	//term_callbacks_t * callbacks;
	int volatile lock;
	uint8_t  mouse_on;
	uint32_t img_collected;
	uint32_t img_size;
	char *   img_data;
} term_state_t;
static bool cursor_on      = 1;    /* Whether or not the cursor should be rendered */
static bool _fullscreen    = 0;    /* Whether or not we are running in fullscreen mode (GUI only) */
static bool _no_frame      = 0;    /* Whether to disable decorations or not */
static bool _use_aa        = 1;    /* Whether or not to use best-available anti-aliased renderer */
static bool _have_freetype = 0;    /* Whether freetype is available */
static bool _force_no_ft   = 0;    /* Whether to force disable the freetype backend */
static bool _free_size = 1; /* Disable rounding when resized */
static int decor_left_width = 0;
static int decor_top_height = 0;
static int decor_right_width = 0;
static int decor_bottom_height = 0;
static int decor_width = 0;
static int decor_height = 0;
#define INT32_MAX 2147483647
#define PALETTE_COLORS 256
static int      scale_fonts    = 0;    /* Whether fonts should be scaled */
static float    font_scaling   = 1.0;  /* How much they should be scaled by */
static float    font_gamma     = 1.7;  /* Gamma to use for SDF library */
  unsigned short term_width     = 1268;    /* Width of the terminal (in cells) */
  unsigned short term_height    = 1024 ;    /* Height of the terminal (in cells) */
int ctx;
static term_cell_t * term_buffer = NULL; /* The terminal cell buffer */
static term_cell_t * term_buffer_a = NULL;
static term_cell_t * term_buffer_b = NULL;
static term_state_t * ansi_state = NULL;
static int menu_bar_height = 24;
#define GFX_W(ctx)  term_width 			/* Display width */
#define GFX_H(ctx)  term_height 			/* Display height */
static unsigned short font_size      = 16;   /* Font size according to SDF library */
static int32_t l_x = INT32_MAX;
static int32_t l_y = INT32_MAX;
static int32_t r_x = -1;
static int32_t r_y = -1;
static unsigned short char_offset    = 0;    /* Offset of the font within the cell */
static int      csr_x          = 0;    /* Cursor X */
static int      csr_y          = 0;    /* Cursor Y */
static unsigned int current_fg     = 7;    /* Current foreground color */
static unsigned int current_bg = 0; /* Current background color */
enum sdf_font {
    SDF_FONT_THIN,
    SDF_FONT_BOLD,
    SDF_FONT_MONO,
    SDF_FONT_MONO_BOLD,
    SDF_FONT_MONO_OBLIQUE,
    SDF_FONT_MONO_BOLD_OBLIQUE,
};
/* Triggers escape mode. */
#define ANSI_ESCAPE  27
/* Escape verify */
#define ANSI_BRACKET '['
#define ANSI_BRACKET_RIGHT ']'
#define ANSI_OPEN_PAREN '('
/* Anything in this range (should) exit escape mode. */
#define ANSI_LOW    'A'
#define ANSI_HIGH   'z'
/* Escape commands */
#define ANSI_CUU    'A' /* CUrsor Up                  */
#define ANSI_CUD    'B' /* CUrsor Down                */
#define ANSI_CUF    'C' /* CUrsor Forward             */
#define ANSI_CUB    'D' /* CUrsor Back                */
#define ANSI_CNL    'E' /* Cursor Next Line           */
#define ANSI_CPL    'F' /* Cursor Previous Line       */
#define ANSI_CHA    'G' /* Cursor Horizontal Absolute */
#define ANSI_CUP    'H' /* CUrsor Position            */
#define ANSI_ED     'J' /* Erase Data                 */
#define ANSI_EL     'K' /* Erase in Line              */
#define ANSI_SU     'S' /* Scroll Up                  */
#define ANSI_SD     'T' /* Scroll Down                */
#define ANSI_HVP    'f' /* Horizontal & Vertical Pos. */
#define ANSI_SGR    'm' /* Select Graphic Rendition   */
#define ANSI_DSR    'n' /* Device Status Report       */
#define ANSI_SCP    's' /* Save Cursor Position       */
#define ANSI_RCP    'u' /* Restore Cursor Position    */
#define ANSI_HIDE   'l' /* DECTCEM - Hide Cursor      */
#define ANSI_SHOW   'h' /* DECTCEM - Show Cursor      */
/* Display flags */
#define ANSI_BOLD      0x01
#define ANSI_UNDERLINE 0x02
#define ANSI_ITALIC    0x04
#define ANSI_ALTFONT   0x08 /* Character should use alternate font */
#define ANSI_SPECBG    0x10
#define ANSI_BORDER    0x20
#define ANSI_WIDE      0x40 /* Character is double width */
#define ANSI_CROSS     0x80 /* And that's all I'm going to support (for now) */
#define ANSI_EXT_IMG   0x100 /* Cell is actually an image, use fg color as pointer */

#define ANSI_EXT_IOCTL 'z'  /* These are special escapes only we support */

/* Default color settings */
#define TERM_DEFAULT_FG     0x07 /* Index of default foreground */
#define TERM_DEFAULT_BG     0x10 /* Index of default background */
#define TERM_DEFAULT_FLAGS  0x00 /* Default flags for a cell */
#define TERM_DEFAULT_OPAC 0xF2 /* For background, default transparency */



void cell_set(unsigned short x, unsigned short y, unsigned int c, unsigned int fg, unsigned int bg, unsigned int flags) {
	/* Avoid setting cells out of range. */
	if (x >= term_width || y >= term_height) return;

	/* Calculate the cell position in the terminal buffer */
	term_cell_t * cell = (term_cell_t *)((uint32_t *)lfb_vid_memory + (y * term_width + x) * sizeof(term_cell_t));
 
	/* Set cell attributes */
	cell->c     = c;
	cell->fg    = fg;
	cell->bg    = bg;
	cell->flags = flags;
}
void cell_redraw(unsigned short x, unsigned short y) {
	/* Avoid cells out of range. */
	if (x >= term_width || y >= term_height) return;

	/* Calculate the cell position in the terminal buffer */
	term_cell_t * cell = (term_cell_t *)((uint32_t*)lfb_vid_memory + (y * term_width + x) * sizeof(term_cell_t));

	/* If it's an image cell, redraw the image data. */
	//if (cell->flags & ANSI_EXT_IMG) {
	//	redraw_cell_image(x,y,cell);
	//	return;
	//}
 
	/* Special case empty cells. */
	if (((unsigned int *)cell)[0] == 0x00000000) {
		write_char ( x * char_width, y * char_height, "", cell->bg );
	} else {
		write_char  (x * char_width, y * char_height, cell->c, cell->bg );
	}
}
  int32_t max(int32_t a, int32_t b) {
	return (a > b) ? a : b;
}
#define GFX_W(ctx)  (term_width)			/* Display width */
#define GFX_H(ctx)  (term_height)			/* Display height */
#define GFX_B(ctx)  (32 / 8)		/* Display byte depth */
#define GFX_S(ctx)  sizeof(term_cell_t)  /* Stride */
#define GFX(ctx,x,y) *((uint32_t *)&((uint32_t*)lfb_vid_memory)[(GFX_S(ctx) * (y) + (x) * GFX_B(ctx))])


 void term_set_point(uint16_t x, uint16_t y, uint32_t color ) {
	set_point(x,y,color);
return;
	if (_fullscreen) {
		/* In full screen mode, pre-blend the color over black. */
		color = alpha_blend_rgba(premultiply(rgba(0,0,0,0xFF)), color);
	}
	if (!_no_frame) {
		GFX(ctx, (x+decor_left_width),(y+decor_top_height+menu_bar_height)) = color;
	} else {
		GFX(ctx, x,y) = color;
	}
}
 

/* Scroll the terminal up or down. */
int width___;
  void term_scroll(int how_much) {

	/* A large scroll request should just clear the screen. */
	if (how_much >= term_height || -how_much >= term_height) {
		// term_clear();
		//return;
	}

	/* A request to scroll 0... is a request not to scroll. */
	if (how_much == 0) {
		return;
	}

	/* Redraw the cursor before continuing. */
//	cell_redraw(csr_x, csr_y);

	if (how_much > 0) {
//memmove( (uintptr_t )lfb_vid_memory, (void *)(uintptr_t )lfb_vid_memory + (term_width *  char_height * how_much) * 12, char_height * (term_height - how_much) * term_width * 12);
		/* Scroll up */
		// width___ = term_width -4;
		 memmove((uintptr_t )lfb_vid_memory, (void *)((uintptr_t )lfb_vid_memory + 12 * (term_width -4)), 12 * (term_width -4) * (term_height-4 ));
 
 
 
return;
		/* Reset the "new" row to clean cells */
		memset((void *)((uint32_t*)lfb_vid_memory + 1 * term_width * (term_height - how_much)), 0x0, 1 * term_width * how_much);
		/* In graphical modes, we will shift the graphics buffer up as necessary */
		uintptr_t dst, src;

		size_t    siz = char_height * (term_height - how_much) * term_width * 12;
		if (!_no_frame) {
			/* Must include decorations */
			dst = (uintptr_t )lfb_vid_memory + (term_width * (decor_top_height+menu_bar_height)) * GFX_B(ctx);
			src = (uintptr_t )lfb_vid_memory + (term_width * (decor_top_height+menu_bar_height + char_height * how_much)) * GFX_B(ctx);
		} else {
			/* Can skip decorations */
			dst = (uintptr_t )lfb_vid_memory;
			src = (uintptr_t )lfb_vid_memory + (term_width *  char_height * how_much) * GFX_B(ctx);
		}

		/* Perform the shift */
 printk("src = %x \n", src);
 printk("src = %x \n", dst);
		
for(;;);
		/* And redraw the new rows */
		for (int i = 0; i < how_much; ++i) {
			for (uint16_t x = 0; x < term_width; ++x) {
				cell_set(x,term_height - how_much,' ', current_fg, current_bg, ansi_state->flags);
				cell_redraw(x, term_height - how_much);
			}
		}

	} else {
		how_much = -how_much;
		/* Scroll down */
		memmove((void *)((uintptr_t)lfb_vid_memory + 1 * term_width), lfb_vid_memory, 1 * term_width * (term_height - how_much));
return;
		/* Reset the "new" row to clean cells */
		memset(lfb_vid_memory, 0x0, sizeof(term_cell_t) * term_width * how_much);
		uintptr_t dst, src;
		size_t    siz = char_height * (term_height - how_much) * GFX_W(ctx) * GFX_B(ctx);
		if (!_no_frame) {
			src = (uint32_t*)lfb_vid_memory + (GFX_W(ctx) * (decor_top_height+menu_bar_height)) * GFX_B(ctx);
			dst = (uint32_t*)lfb_vid_memory + (GFX_W(ctx) * (decor_top_height+menu_bar_height + char_height * how_much)) * GFX_B(ctx);
		} else {
			src = (uint32_t*)lfb_vid_memory;
			dst = (uint32_t*)lfb_vid_memory + (GFX_W(ctx) *  char_height * how_much) * GFX_B(ctx);
		}
		/* Perform the shift */
		memmove((void *)dst, (void *)src, siz);
		/* And redraw the new rows */
		for (int i = 0; i < how_much; ++i) {
			for (uint16_t x = 0; x < term_width; ++x) {
				cell_redraw(x, i);
			}
		}
	}

	/* Remove image data for image cells that are no longer on screen. */
	//flush_unused_images();

	/* Flip the entire window. */
	//yutani_flip(yctx, window);
}
int yy = 0;
volatile ringbuffer_t *printbuffer = 0;

void put_dec_g(u32 n)
{

    if (n == 0)
    {
        //putch('0');
        return;
    }

    s32 acc = n;
    char c[32];
    int i = 0;
    while (acc > 0)
    {
        c[i] = '0' + acc%10;
        acc /= 10;
        i++;
    }
    c[i] = 0;

    char c2[32];
    c2[i--] = 0;
    int j = 0;
    while(i >= 0)
    {
        c2[i--] = c[j++];
    }
   write_char(0, yy, c2, 0x3344ffff);

}
int xxx;
extern int USING_STDIO;
void puts_g(const char **text)
{
	
int xx = 0;
int num_entries = 3;
for(int g = 0 ; g < num_entries; g++)
 {
u32 i = 0;

//int size = strlen(text[2]);
	while(i < strlen(text[g]))
	{
		//write_char(xx+1, yy+1, text[i++], 0x11440000);
		if(g == 0)
			write_char(xx, yy, text[g][i++], 0x11ffff00);
		if(g == 1)
			write_char(xx, yy, text[g][i++], 0x111ff166);
		if(g == 2)
			write_char(xx, yy, text[g][i++], 0xffffffff);
		if(g == 3)
			write_char(xx, yy, text[g][i++], 0x00000000);		
		
			if(xx > term_width)
			{
				 yy += char_height;	
			 	 xx = 0;
			}	
			
		 
		if(strlen(text[2]) == '\0' )
				break;		 
	 xx += char_width;
	 
	xxx =xx;
	 
	
	}

}
yy += char_height;

	if(yy > term_height)
	{
 
		memset((void *)(uintptr_t )lfb_vid_memory,0, term_width * term_height * 12);

		yy = 0;
		
	}
		// term_scroll(4);
}
 

void _vesa_initialize()
{
	printbuffer = (ringbuffer_t*)malloc_(sizeof(ringbuffer_t)*10000);
	printbuffer->counter = 0;
	printbuffer->read_ptr = printbuffer->data;	/*both pointers use data array to store its data*/
        printbuffer->write_ptr = printbuffer->data;
	 
}
