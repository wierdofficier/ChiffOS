#include <types.h>
#include <irq.h>
#include <video.h>
#include <kutils.h>
#include <proc.h>

#define BCD2BIN(bcd) ((((bcd)&15) + ((bcd)>>4)*10))
const u16 TIMER_DIVISOR = 11932;
volatile u32 timerticks = 0;
volatile unsigned int useconds = 0;


#define PIT_A 0x40
#define PIT_B 0x41
#define PIT_C 0x42
#define PIT_CONTROL 0x43

#define PIT_MASK 0xFF
#define PIT_SCALE 1193180
#define PIT_SET 0x34

#define TIMER_IRQ 0

#define SUBTICKS_PER_TICK 1000
#define RESYNC_TIME 1
volatile int hours;
volatile int min;
volatile int sec;
u32 sys_get_system_time()
{
	return timerticks/100;
}
u32 uptime()
{
	return timerticks/100;
}

void timer_div(int hz)
{
	u16 divisor = 1193180 / hz;
	
	outb(0x43, 0x36);
	outb(0x40, divisor & 0xff);
	outb(0x40, divisor >> 8);
}



 
unsigned long timer_subticks = 0;
signed long timer_drift = 0;
signed long _timer_drift = 0;

static int behind = 0;

/*
 * IRQ handler for when the timer fires
 */
 extern list_t * process_list;
extern int boot_time;

 u32 timer_handle(u32 esp)
{
timerticks++;
	if (++timer_subticks == SUBTICKS_PER_TICK || (behind && ++timer_subticks == SUBTICKS_PER_TICK)) {
		;
		timer_subticks = 0;
		if (timerticks % RESYNC_TIME == 0) {
			uint32_t new_time = read_cmos();
			_timer_drift = new_time - boot_time - timerticks;
			if (_timer_drift > 0) behind = 1;
			else behind = 0;
		}
}


if( current_task->time_to_run > 0) 
{
  current_task->time_to_run--;
task_switching = 0;
}
else
   {
	
   task_switching = 1;
	//	 	   current_task->time_to_run = 10;
 //current_task=	next_ready_process();
 
			//  list_insert(process_list,   current_task);
		//	 make_process_ready(current_task);
// printk("getpid() returns = %d \n", getpid());

	


 
        }

	return esp;
	
	
    }
   
 
         
volatile u32 gettickcount(void) {

return timerticks;
}
 /*    
datetime_t getDatetime()
{
   datetime_t now;

   __asm__ __volatile__ ("cli");
   now.sec = BCD2BIN(readCMOS(0x0));
   now.min = BCD2BIN(readCMOS(0x2));
   now.hour = BCD2BIN(readCMOS(0x4));
   now.day = BCD2BIN(readCMOS(0x7));
   now.month = BCD2BIN(readCMOS(0x8));
   now.year = BCD2BIN(readCMOS(0x9));
  __asm__ __volatile__ ("sti");

   return now;
}
*/
int sleep(u32 ms) { 
	
   u32 t = ms/10;
   
    if (t == 0)t = 1;
	u32 start_ticks = gettickcount();
	while(start_ticks = gettickcount() < start_ticks + t) {}
	return start_ticks;
}



void timer_install(u32 frequency)
{
	  timer_div(frequency);
	  install_device(0,timer_handle);
	  boot_time = read_cmos();
}

 



int wait_loop0_ = 6;
int wait_loop1_ = 3;
void
sleep___( int seconds )
{   // this function needs to be finetuned for the specific microprocessor
    int i, j, k;
    for(i = 0; i < seconds; i++)
    {
        for(j = 0; j < wait_loop0_; j++)
        {
            for(k = 0; k < wait_loop1_; k++)
            {   // waste function, volatile makes sure it is not being optimized out by compiler
                int volatile t = 120 * j * i + k;
                t = t + 5;
            }
        }
    }
}

void relative_time(unsigned long seconds, unsigned long subseconds, unsigned long * out_seconds, unsigned long * out_subseconds) {
	if (subseconds + timer_subticks > SUBTICKS_PER_TICK) {
		*out_seconds    = timerticks + seconds + 1;
		*out_subseconds = (subseconds + timer_subticks) - SUBTICKS_PER_TICK;
	} else {
		*out_seconds    = timerticks + seconds;
		*out_subseconds = timer_subticks + subseconds;
	}
}

 
int process_alert_node() {}
#include <logging.h>
int assert(char *buf) {  }
