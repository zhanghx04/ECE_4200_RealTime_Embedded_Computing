#ifndef MODULE
#define MODULE
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kthread.h>	// for kthreads
#include <linux/sched.h>	// for task_struct
#include <linux/time.h>		// for using jiffies 
#include <linux/timer.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define MSG_SIZE 50
#define CDEV_NAME "lab6_hx"	// "YourDevName"

// Address
#define GPIOBASE    0x3F200000
#define SPKR_SEL    0x40000

// Button's frequency
#define BTN_1 1150
#define BTN_2 1050
#define BTN_3 950
#define BTN_4 850
#define BTN_5 750

// Give the permission to install module
MODULE_LICENSE("GPL");		

// Masks for each part
unsigned long spkr_mask = 0x40;
unsigned long btn1_mask = 0x10000;
unsigned long btn2_mask = 0x20000;
unsigned long btn3_mask = 0x40000;
unsigned long btn4_mask = 0x80000;
unsigned long btn5_mask = 0x100000;
unsigned long btns_mask = 0x1F0000;	// btn1 to btn5
unsigned long pins_mask = 0x1F0040;	// bottons and speaker

// address
unsigned long* GPFSEL0;
unsigned long* GPSET0;
unsigned long* GPCLR0;
unsigned long* GPEDS0;
unsigned long* GPPUD;
unsigned long* GPPUDCLK0;
unsigned long* GPAREN0;


// structure for the kthread.
static struct task_struct *kthread1;
unsigned long *vaddr;
unsigned int sound = BTN_3;
// structure for isr
int mydev_id;	// to identify the handler
// structure for device
static int major; 
static char msg[MSG_SIZE];

bool pressed = false;

// function for get length of actual char number
size_t getlen(const char* str){
	const char *s;
	for (s = str; *s; ++s){
		if(strncmp(s, "/0", 2) == 0){
			break;
		}
	}
	return (s - str);
}

// Function called when the user space program reads the character device.
// Some arguments not used here.
// buffer: data will be placed there, so it can go to user space
// The global variable msg is used. Whatever is in that string will be sent to userspace.
// Notice that the variable may be changed anywhere in the module...
static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset){
	// Whatever is in msg will be placed into buffer, which will be copied into user space
	ssize_t dummy = copy_to_user(buffer, msg, length);	// dummy will be 0 if successful
	// msg should be protected (e.g. semaphore). Not implemented here, but you can add it.
	msg[0] = '\0';	// "Clear" the message, in case the device is read again.
					// This way, the same message will not be read twice.
					// Also convenient for checking if there is nothing new, in user space.
	return length;
}

// Function called when the user space program writes to the Character Device.
// Some arguments not used here.
// buff: data that was written to the Character Device will be there, so it can be used
//       in Kernel space.
// In this example, the data is placed in the same global variable msg used above.
// That is not needed. You could place the data coming from user space in a different
// string, and use it as needed...
static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off){
	ssize_t dummy;
	
	if(len > MSG_SIZE)
		return -EINVAL;
	
	// unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
	dummy = copy_from_user(msg, buff, len);	// Transfers the data from user space to kernel space
	if(len == MSG_SIZE)
		msg[len-1] = '\0';	// will ignore the last character received.
	else
		msg[len] = '\0';
	
	// You may want to remove the following printk in your final version.
	printk("Message from user space: %s\n", msg);
	if (strncmp(msg, "@A", 2) == 0){
		sound = BTN_1;
	} else if (strncmp(msg, "@B", 2) == 0){
		sound = BTN_2;
	} else if (strncmp(msg, "@C", 2) == 0){
		sound = BTN_3;
	} else if (strncmp(msg, "@D", 2) == 0){
		sound = BTN_4;
	} else if (strncmp(msg, "@E", 2) == 0){
		sound = BTN_5;
	}
	return len;		// the number of bytes that were written to the Character Device.
}

// structure needed when registering the Character Device. Members are the callback
// functions when the device is read from or written to.
static struct file_operations fops = {
	.read = device_read, 
	.write = device_write,
};

// ==============================  K-Thread  =======================================

// Function to be associated with the kthread; what the kthread executes.
int speaker_fn(void *ptr){	
	unsigned long j0, j1;
	printk("In kthread1\n");
	j0 = jiffies;		// number of clock ticks since system started;
						// current "time" in jiffies
	j1 = j0 + 10*HZ;	// HZ is the number of ticks per second, that is
						// 1 HZ is 1 second in jiffies
	while(time_before(jiffies, j1))	// true when current "time" is less than j1
        schedule();		// voluntarily tell the scheduler that it can schedule
						// some other process
	printk("Before loop\n");
	
	// The ktrhead does not need to run forever. It can execute something
	// and then leave.
	while(1)
	{
		// In an infinite loop, you should check if the kthread_stop
		// function has been called (e.g. in clean up module). If so,
		// the kthread should exit. If this is not done, the thread
		// will persist even after removing the module.		
		// comment out if your loop is going "fast". You don't want to
		// printk too often. Sporadically or every second or so, it's okay.
		if(kthread_should_stop()) {
			iowrite32(0x40, vaddr + 10);	//GPCLR0
			do_exit(0);
		}
		iowrite32((uint32_t)GPSET0 | spkr_mask, GPSET0);	//GPSET0
		udelay(sound);
		iowrite32((uint32_t)GPCLR0 | spkr_mask, GPCLR0);	//GPCLR0
		udelay(sound);	
	}
	
	return 0;
}

static irqreturn_t button_isr(int irq, void *dev_id) {
	// In general, you want to disable the interrupt while handling it.
	disable_irq_nosync(79);

	// This same handler will be called regardless of what button was pushed,
	// assuming that they were properly configured.
	// How can you determine which button was the one actually pushed?
		
	// DO STUFF (whatever you need to do, based on the button that was pushed)
	if (ioread32(GPEDS0) == btn1_mask){
		sound = BTN_1;
		sprintf(msg, "@A");
	}
	if (ioread32(GPEDS0) == btn2_mask){
		sound = BTN_2;
		sprintf(msg, "@B");
	}
	if (ioread32(GPEDS0) == btn3_mask){
		sound = BTN_3;
		sprintf(msg, "@C");
	}
	if (ioread32(GPEDS0) == btn4_mask){
		sound = BTN_4;
		sprintf(msg, "@D");
	}
	if (ioread32(GPEDS0) == btn5_mask){
		sound = BTN_5;
		sprintf(msg, "@E");
	}

	// IMPORTANT: Clear the Event Detect status register before leaving.	
	iowrite32(btns_mask, GPEDS0);
	
	printk("Interrupt handled\n");	
	enable_irq(79);		// re-enable interrupt
	
	return IRQ_HANDLED;
}

int thread_init(void){
	int dummy = 0;

	char kthread_name[11]="my_kthread";	
	// try running  ps -ef | grep my_kthread
	// when the thread is active.
	printk("In init module\n");

	// register the Characted Device and obtain the major (assigned by the system)
	major = register_chrdev(0, CDEV_NAME, &fops);
	if (major < 0) {
     		printk("Registering the character device failed with %d\n", major);
	     	return major;
	}
	printk("Lab6_cdev_kmod example, assigned major: %d\n", major);
	printk("Create Char Device (node) with: sudo mknod /dev/%s c %d 0\n", CDEV_NAME, major);
	
	// gpio set
	vaddr = (unsigned long*)ioremap(GPIOBASE, 4096);
	GPFSEL0   = vaddr + 0;
	GPSET0    = vaddr + 7;
	GPCLR0    = vaddr + 10;
	GPEDS0    = vaddr + 16;
	GPAREN0   = vaddr + 31;
	GPPUD     = vaddr + 37;
	GPPUDCLK0 = vaddr + 38;
	iowrite32(SPKR_SEL, GPFSEL0);  //GPFSEL0
	iowrite32((uint32_t)GPCLR0 | spkr_mask, GPCLR0);
	iowrite32(0x01, GPPUD);
	udelay(100);
	iowrite32(btns_mask, GPPUDCLK0);
	iowrite32(btns_mask, GPAREN0);
    	    
	kthread1 = kthread_create(speaker_fn, NULL, kthread_name);
	
	if((kthread1)){	// true if kthread creation is successful
		printk("Inside if\n"); 
		// kthread is dormant after creation. Needs to be woken up
		wake_up_process(kthread1);
	}
	dummy = request_irq(79, button_isr, IRQF_SHARED, "Button_handler", &mydev_id);

	printk("Button Detection enabled.\n");
    	return 0;
}

void thread_cleanup(void) {
	int ret;

	// disable reading from device
	unregister_chrdev(major, CDEV_NAME);
	printk("Char Device /dev/%s unregistered.\n", CDEV_NAME);

	// Good idea to clear the Event Detect status register here, just in case.
	iowrite32(0, GPEDS0);
	// Disable (Async) Rising Edge detection for all 5 GPIO ports.
	iowrite32(0, GPAREN0);
	// Remove the interrupt handler; you need to provide the same identifier
    	free_irq(79, &mydev_id);
	
	printk("Button Detection disabled.\n");

	// the following doesn't actually stop the thread, but signals that
	// the thread should stop itself (with do_exit above).
	// kthread should not be called if the thread has already stopped.
	ret = kthread_stop(kthread1);
								
	if(!ret)
		printk("Kthread stopped\n");
}








/*
// ==============================  hrtimer  =======================================
unsigned long timer_interval_ns = 1e6;	// timer interval length (nano sec part)
static struct hrtimer hr_timer;			// timer structure
static int dummy = 0;

// Timer callback function: this executes when the timer expires
enum hrtimer_restart timer_callback(struct hrtimer *timer_for_restart)
{
  	ktime_t currtime, interval;	// time type, in nanoseconds
	unsigned long overruns = 0;

	short spkr_mask = 0x40;
	vaddr = (unsigned long*)ioremap(GPIOBASE, 4096);
	iowrite32(SPKR_SEL, vaddr);  //GPFSEL0

	
	// Re-configure the timer parameters (if needed/desired)
  	currtime  = ktime_get();
  	interval = ktime_set(0, timer_interval_ns); // (long sec, long nano_sec)
	
	// Advance the expiration time to the next interval. This returns how many
	// intervals have passed. More than 1 may happen if the system load is too high.
  	overruns = hrtimer_forward(timer_for_restart, currtime, interval);
	
	
	// The following printk only executes once every 1000 cycles.
	if(dummy == 0){
		printk("mark here\n");
		//int s;		
		// comment out if your loop is going "fast". You don't want to
		// printk too often. Sporadically or every second or so, it's okay.
		//for (s = 1; s <900; s++){
		if(kthread_should_stop()) {
			iowrite32(0x40, vaddr + 10);	//GPCLR0
			do_exit(0);
		}
		//	if (s >= 895){
		//		s = 1;
		//	}
		iowrite32((uint32_t)(vaddr+7) | spkr_mask, vaddr + 7);	//GPSET0
		udelay(s);
		iowrite32((uint32_t)(vaddr+7) | spkr_mask, vaddr + 10);	//GPCLR0
		udelay(s);	
		//}
	}
	dummy = (dummy + 1)%1;	// kind of timer here
	
	
	return HRTIMER_RESTART;	// Return this value to restart the timer.
							// If you don't want/need a recurring timer, return
							// HRTIMER_NORESTART (and don't forward the timer).
}

int timer_init(void)
{
	// Configure and initialize timer
	ktime_t ktime = ktime_set(0, timer_interval_ns); // (long sec, long nano_sec)
	
	// CLOCK_MONOTONIC: always move forward in time, even if system time changes
	// HRTIMER_MODE_REL: time relative to current time.
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	
	// Attach callback function to the timer
	hr_timer.function = &timer_callback;
	
	// Start the timer
 	hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);
	
	return 0;
}

void timer_exit(void)
{
	int ret;
  	ret = hrtimer_cancel(&hr_timer);	// cancels the timer.
  	if(ret)
		printk("The timer was still in use...\n");
	else
		printk("The timer was already canceled...\n");	// if not restarted or
														// canceled before
	
  	printk("HR Timer module uninstalling\n");
	
}
*/

// Notice this alternative way to define your init_module()
// and cleanup_module(). "thread_init" will execute when you install your
// module. "thread_cleanup" will execute when you remove your module.
// You can give different names to those functions.

module_init(thread_init);
module_exit(thread_cleanup);

//module_init(timer_init);
//module_exit(timer_exit);

