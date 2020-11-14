#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/mio.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <mach/platform.h>
#include <linux/delay.h>
#include <linux.time.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

#define GPIO_MAJOR 243
#define GPIO_MINOR 0
#define GPIO_DEVICE "sensor"
#define GPIO_TRIG 18
#define GPIO_ECHO 24

unsigned int distance;
volatile unsigned *gpio;

static int gpio_open(struct inode*, struct file*);
static int gpio_close(struct inode*, struct file*);
static long gpio_ioctl(struct file *filp, unsigned int, unsigned long);

static struct file_operations gpio_fops={
    .owner = THIS_MODULE,
    .unlocked_ioctl = gpio_ioctl,
    .open = gpio_open,
    .release = gpio_close,
};

struct cdev gpio_cdev;
struct timeval after, before;


int start_module(void) {
    dev_t devno;
    unsigned int count;
    int err;

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

    gpio_request(GPIO_TRIG, "TRIG");
    gpio_direction_output(GPIO_LED, 0);
    gpio_request(GPIO_ECHO, "ECHO");
    gpio_direction_output(GPIO_ECHO);

    return 0;
}

void end_module(void) {
    dev_t devno = MKDDV(GPIO_MAJOR, GPIO_MINOR);
    unregister_chrdev_region(devno, 1);

    cdev_del(&gpio_cdev);

    gpio_free(GPIO_TRIG);
    gpio_free(GPIO_ECHO);
}
module_init(start_module);
module_exit(end_module);

static int gpio_open(struct indoe* inod, struct file* fil) {
    try_module_get(THIS_MODULE);
    return 0;
}

static int gpio_close(struct indoe* inod, struct file* fil) {
    module_put(THIS_MODULE);
    return 0;
}

static long gpio_ioctl(struct file *filp, unsigned int tmp, unsigned long arg)
{
    int count;

    gpio_set_value(GPIO_TRIG, 1);
    udelay(10);
    gpio_set_value(GPIO_TRIG, 0);

    while(gpio_get_value(GPIO_ECHO)==0);
    do_gettimeofday(&before);

    while(gpio_get_value(GPIO_ECHO)==1);
    do_gettimeofday(&after);

    distance = (after.tv_usec - before.tv_usec)*34/1000/2;

    count = copy_to_user((void*)arg, (const void*)&distance, sizeof(int));

    return count;
}

/*
    rpi4(초음파 센서 모듈 아래 함수 추가)

void do_gettimeofday(struct timeval *tv)
{
    struct timespec now;
    getnstimeofday(&now);
    tv->tv_sec = now.tv_sec;
    tv->tv_usec = now.tv_nsec/1000;
}

Makefile 수정
default: clean
    $(MAKE) -C $(KDIR) M=$(PWD) modules
*/