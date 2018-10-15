#include <types.h>
#include <video.h>
#include <kutils.h>
#include <irq.h>
#include <kbd.h>
u8 getchar();
volatile ringbuffer_t *keybuffer = 0;
u8 scan_code;
u8 key_result;
u8 keyboard_layout_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	
  '9', '0', '-', '=', '\b',	
  '\t',			
  'q', 'w', 'e', 'r',	
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	
    '^',		
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
 '\'', '`',   0,		
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			
  'm', ',', '.', '/',   0,				
  '*',
    0,	
  ' ',	
    0,	
    0,	
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	
    0,	
    0,	
    0,	
    0,	
    0,	
  '-',
    0,	
    0,
    0,
  '+',
    0,	
    0,	
    0,	
    0,	
    0,	
    0,   0,   0,
    0,	
    0,	
    0,	
};


u32 kbd_handler(u32 esp)
{
	
	scan_code = inb(0x60);
	if(scan_code & 0x80)
	{
		return esp;
	}
	else
	{
		key_result = keyboard_layout_us[scan_code];
		
	}
	
		if(key_result == 0)
			return esp;
	keybuffer->counter++;
	if(keybuffer->counter == 256)
		printk("keybuffer -> FULL!");

	if (keybuffer->write_ptr > keybuffer->data + 256)
	keybuffer->write_ptr = keybuffer->data;	/*wrap write_ptr back to zero to behave like a circular buffer!*/

	*(keybuffer->write_ptr++) = key_result;
	
	return esp;
}

void _kbd_initialize()
{
	keybuffer = (ringbuffer_t*)malloc_(sizeof(ringbuffer_t));
	keybuffer->counter = 0;
	keybuffer->read_ptr = keybuffer->data;	/*both pointers use data array to store its data*/
    keybuffer->write_ptr = keybuffer->data;
	install_device(1,kbd_handler);
}
int getch_char;
u8 getch_polling()
{

	 if(keyboard_layout_us[inb(0x60)] != 0) 
		outb(0x60,0xf4); 
     
     while(keyboard_layout_us[inb(0x60)] == 0); 
	 getch_char = keyboard_layout_us[inb(0x60)];
     outb(0x60,0xf4); 
	 
     return getch_char; 
	
}
void idleloop()
{
while (keybuffer->counter == 0 ){
		break;
	}

}

u8 getchar()
{
	
	while(keybuffer->counter == 0)
	 {
		__asm__ __volatile__("sti");
	}
	
	
	u8 getkey = *(keybuffer->read_ptr++);
	
	keybuffer->counter--;

	if (keybuffer->read_ptr > keybuffer->data + 256)	
	keybuffer->read_ptr = keybuffer->data; /*wrap read_ptr back to zero to behave like a circular buffer!*/
	
	return getkey;
	
}
