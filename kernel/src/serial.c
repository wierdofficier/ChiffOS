#include <system.h>
#include <types.h>
#include <irq.h>
 
#include <heapmngr.h>
/* Serial */
#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8
#define SERIAL_PORT_C 0x3E8
#define SERIAL_PORT_D 0x2E8

#define SERIAL_IRQ 4
void serial_init_hook_(void);
void serial_enable(int device) {
	outportb(device + 1, 0x00);
	outportb(device + 3, 0x80); /* Enable divisor mode */
	outportb(device + 0, 0x01); /* Div Low:  01 Set the port to 115200 bps */
	outportb(device + 1, 0x00); /* Div High: 00 */
	outportb(device + 3, 0x03);
	outportb(device + 2, 0xC7);
	outportb(device + 4, 0x0B);
}

void serial_install(void) {
	printk( "Installing serial communication driver");

	serial_enable(SERIAL_PORT_A);
	serial_enable(SERIAL_PORT_B);

	serial_init_hook_(); /* Install the serial input handler */

	outportb(SERIAL_PORT_A + 1, 0x01);      /* Enable interrupts on receive */
	outportb(SERIAL_PORT_B + 1, 0x01);      /* Enable interrupts on receive */

}
int serial_interrupt(u32 esp)
 {
	 printk("serial interrupt !");
	 return esp;
 }
void serial_init_hook_(void)
{
 
 
	    install_device(4,serial_interrupt);
    install_device(3,serial_interrupt);

	
 

 
	
}
int serial_rcvd(int device) {
	return inportb(device + 5) & 1;
}

char serial_recv(int device) {
	while (serial_rcvd(device) == 0) ;
	return inportb(device);
}

char serial_recv_async(int device) {
	return inportb(device);
}

int serial_transmit_empty(int device) {
	return inportb(device + 5) & 0x20;
}

void serial_send(int device, char out) {
	while (serial_transmit_empty(device) == 0);
	outportb(device, out);
}

void serial_string(int device, char * out) {
	for (u32 i = 0; i < strlen(out); ++i) {
		serial_send(device, out[i]);
	}
}

