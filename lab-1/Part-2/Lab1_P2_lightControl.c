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

#define GPIOBASE 0x3F200000
#define GPSET0 0x3F20001C
#define GPSET1 0x3F200020
#define GPCLR0 0x3F200028
#define GPCLR1 0x3F20002c

MODULE_LICENSE("GPL");		// to give the permission to install module
unsigned long *vaddr;


// Old Style
int init_module(){
	printk("Load the module\n");
	vaddr = (unsigned long*)ioremap(GPIOBASE, 4096);
	//   vaddr: 0xf3200000
	// vaddr+1: 0xf3200004
	//GPFSEL0  set RED 	PIN 2 as output 0b_          0100 0000	0x40
	//GPFSEL0  set YELLOW	PIN 3 as output 0b_     0010 0000 0000	0x200
	//GPFSEL0  set BLUE	PIN 4 as output 0b_0001 0000 0000 0000	0x1000
	//GPFSEL0  set GREEN	PIN 5 as output 0b_1000 0000 0000 0000	0x8000
	//			       SET ALL: ob_1001 0010 0100 0000	0x9240
	iowrite32(0x9240, vaddr);	//GPFSEL0
	
	
	printk("%x \n",vaddr);
	printk("%x \n", vaddr + 7);

	// 0000 0000 0000 0000 0000 0000 0011 1100 --> 0x3c
	iowrite32(0x3c, vaddr + 7);	//GPSET0
	
	
	
	printk("light it up\n");
	return 0;
}

void cleanup_module(){
	iowrite32(0x3c, vaddr + 10);	//GPCLR0
	printk("Unloaded the module\n");
}

/*
// New Style
static int __init myStart(void){
    printk(KERN_INFO "Loading the moduel\n");
    return 0;
}

static void __exit myEnd(void){
    printk(KERN_INFO "Goodbye Mr.\n");
}

module_init(myStart);
module_exit(myEnd);
*/
