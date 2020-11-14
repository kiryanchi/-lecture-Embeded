#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/mio.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <mach/platform.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

#define GPIO_MAJOR 243
#define GPIO_MINOR 243
#define GPIO_DEVICE 243
#define GPIO_LED 243
#define GPIO_SW 243

volatile unsigned *gpio;
static char msg[BLOCK_SIZE] = {0};

static int gpio_open(struct inode*, struct file*);
static ssize_t gpio_read(struct file*, char*, size_t, loff_t*);
static ssize_t gpio_write(struct file*, const char*, size_t, loff_t*);
static int gpio_close(struct inode*, struct file*);

static struct file_operation gpio_fops = {
    .owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write,
    .open = gpio_open,
    .release = gpio_close,
};

struct cdev gpio_cdev;
static int switch_irq;

static irqreturn_t isr_func(int irq, void* data) {
    if (irq == switch_irq && !gpio_get_value(GPIO_LED)) {
        gpio_set_value(GPIO_LED, 1);
    } else if (irq == switch_irq && gpio_get_value(GPIO_LED)) {
        gpio_set_value(GPIO_LED, 0);
    }
    return IRQ_HANDLED;
}

int start_module(void) {
    dev_t devno;
    unsigned int count;
    int err;

    printk(KERN_INFO "Hello module\n");

    devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    register_chrdev_region(devno, 1, GPIO_DEVICE);

    cdev_init(&gpio_cdev, &gpio_fops);

    gpio_cdev.owner = THIS_MODULE;
    count = 1;
    err = cdev_add(&gpio_cdev, devno, count);
    if (err < 0) {
        printk("ERROR : Device Add\n");
        return -1;
    }

    printk("'mknod /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
    printk("'chmod 666 /dev/%s'\n", GPIO_DEVICE);

    gpio_request(GPIO_LED, "LED");
    gpio_direction_output(GPIO_LED, 0);
    gpio_request(GPIO_SW, "SWITCH");
    switch_irq = gpio_to_irq(GPIO_SW);
    request_irq(switch_irq, isr_func, IRQF_TRIGGER_RISING  | IRQF_DISABLE, "switch", NULL);
    return 0;
}

void end_module(void) {
    dev_t devno = MKDDV(GPIO_MAJOR, GPIO_MINOR);
    unregister_chrdev_region(devno, 1);

    cdev_del(&gpio_cdev);
    free_irq(switch_irq, NULL);

    gpio_free(GPIO_LED);
    gpio_free(GPIO_SW);

    printk(KERN_INFO "Good bye module!!\n");
}

module_init(start_module);
module_exit(end_module);
static int gpio_open(struct indoe* inod, struct file* fil) {
    printk("GPIO Device opened(%d/%d)\n", imajor(inod), iminor(inod));
    try_module_get(THIS_MODULE);
    return 0;
}

static int gpio_close(struct indoe* inod, struct file* fil) {
    printk("GPIO Device closed(%d/%d)\n", imajor(inod), iminor(inod));
    module_put(THIS_MODULE);
    return 0;
}
static ssize_t gpio_read(struct file* indoe, char* buff, sizt_t len, loff_t* off) {
    int count;
    strcat(msg, " from kernel");
    count = copy_to_user(buff, msg, strlen(msg)+1);
    printk("GPIO Device(%d) read : %s(%d)\n", 243, msg, count);

    return count;
}
static ssize_t gpio_write(struct file* indoe, const char* buff, size_t len, loff_t* off) {
    short count;
    memset(msg, 0, BLOCK_SIZE);
    count = copy_from_user(msg, buff, len);

    gpio_set_value(GPIO_LED, (!strcmp(msg, "0")) ? 0 : 1);
    printk("GPIO Device(%d) write : %s(%d)\n", 243, msg, len);
    return count;
}