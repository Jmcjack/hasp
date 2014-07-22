/*
* UMN HASP Peak Detector Interrupt Handler for Bealgebone Black.
*
* Author:
* John Jackson (jacks974@umn.edu)
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

int eventABCounter = 0;
int eventACounter = 0;
int eventBCounter = 0;

char* dump_filename = "/home/root/interruptFlag";

/* Define GPIOs for output(s) (debugging) */

static struct gpio outputs[] = 
{
{88, GPIOF_OUT_INIT_LOW, "LED 1"},
};

/* Define GPIOs for detector events */
static struct gpio events[] = {
{46, GPIOF_IN, "EVENT AB"},	// Event AB Interrupt
{27, GPIOF_IN, "EVENT A"},	// Event A Interrupt
{65, GPIOF_IN, "EVENT B"}	// Event B Interrupt
};

/* The assigned IRQ numbers for the events are stored here */
static int event_irqs[] = {-1, -1, -1};

/*
* The interrupt service routine called on event occurences
*/
static irqreturn_t event_isr(int irq, void *data)
{
//if(irq == button_irqs[0] && !gpio_get_value(leds[0].gpio)) {
//gpio_set_value(leds[0].gpio, 1);
//}
//else if(irq == button_irqs[1] && gpio_get_value(leds[0].gpio)) {
//gpio_set_value(leds[0].gpio, 0);
//

	if (irq == event_irqs[0])
	{
		// read AB voltage from ADC
		eventABCounter++;		
		//printk("AB:%d:VOLTAGE\n", eventABCounter);
		printk("$\n");	
	}
	if (irq == event_irqs[1])
	{
		// read A voltage from ADC
		eventACounter++;
		//printk("A:%d:VOLTAGE\n", eventACounter);
		printk("#\n");
	}
	if (irq == event_irqs[2])
	{
		// read B voltage from ADC
		eventBCounter++;
		//printk("B:%d:VOLTAGE\n", eventBCounter);
		printk("&\n");
	}
	
	return IRQ_HANDLED;
}

/*
* Module init function
*/
static int __init gpiomode_init(void)
{
	int ret = 0;
	printk(KERN_INFO "%s\n", __func__);

	// register LED gpios
	ret = gpio_request_array(outputs, ARRAY_SIZE(outputs));

	if (ret) {
		printk(KERN_ERR "Unable to request GPIOs for OUTPUT: %d\n", ret);
		return ret;
	}

	// register BUTTON gpios
	ret = gpio_request_array(events, ARRAY_SIZE(events));

	if (ret) {
		printk(KERN_ERR "Unable to request GPIOs for EVENTs: %d\n", ret);
		goto fail1;
	}

	//printk(KERN_INFO "Current event AB value: %d\n", gpio_get_value(buttons[0].gpio));
	//printk(KERN_INFO "Current event A value: %d\n", gpio_get_value(buttons[1].gpio));
	//printk(KERN_INFO "Current event B value: %d\n", gpio_get_value(buttons[2].gpio));

	// Get IRQ AB
	ret = gpio_to_irq(events[0].gpio);

	if(ret < 0) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}

	event_irqs[0] = ret;

	printk(KERN_INFO "Successfully requested EVENT AB IRQ # %d\n", event_irqs[0]);

	// Get IRQ A
	ret = gpio_to_irq(events[1].gpio);

	if(ret < 0) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}

	event_irqs[1] = ret;

	printk(KERN_INFO "Successfully requested EVENT A IRQ # %d\n", event_irqs[1]);

	// Get IRQ B
	ret = gpio_to_irq(events[2].gpio);

	if(ret < 0) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}

	event_irqs[2] = ret;

	printk(KERN_INFO "Successfully requested EVENT B IRQ # %d\n", event_irqs[2]);
	
	// Request IRQ for event AB
	ret = request_irq(event_irqs[0], event_isr, IRQF_TRIGGER_RISING, "gpiomod#eventAB", NULL);

	if(ret) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}

	// Request IRQ for event A
	ret = request_irq(event_irqs[1], event_isr, IRQF_TRIGGER_RISING, "gpiomod#eventA", NULL);

	if(ret) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}

	// Request IRQ for event B
	ret = request_irq(event_irqs[2], event_isr, IRQF_TRIGGER_RISING, "gpiomod#eventB", NULL);

	if(ret) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}

	return 0;

	// cleanup what has been setup so far
	fail2:
	gpio_free_array(events, ARRAY_SIZE(events));

	fail1:
	gpio_free_array(outputs, ARRAY_SIZE(outputs));

	return ret;	
}

/**
* Module exit function
*/
static void __exit gpiomode_exit(void)
{
	int i;

	printk(KERN_INFO "%s\n", __func__);

	// free irqs
	free_irq(event_irqs[0], NULL);
	free_irq(event_irqs[1], NULL);
	free_irq(event_irqs[2], NULL);

	// turn all outputs off
	for(i = 0; i < ARRAY_SIZE(outputs); i++) {
		gpio_set_value(outputs[i].gpio, 0);
	}

	// unregister GPIOs
	gpio_free_array(outputs, ARRAY_SIZE(outputs));
	gpio_free_array(events, ARRAY_SIZE(events));
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Jackson");
MODULE_DESCRIPTION("UMN HASP Peak Detector Interrupt Handler for Bealgebone Black.");

module_init(gpiomode_init);
module_exit(gpiomode_exit);

