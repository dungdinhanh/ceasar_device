#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h> // help user to read and write to file system that communicate with kernal
#include <linux/cdev.h> // this is a char driver; makes cdev available map user space to kernel space
#include <linux/semaphore.h> //used to access semaphores; synchronization behaviors
#include <linux/uaccess.h> // copy_to_user; copy_from_user
#include <linux/string.h>



#define MAXI 1000
#define DEFAULT_C 0

int ceasar = DEFAULT_C;

//(1) Create a structure for our fake device
struct fake_device{
    char data[1000];
    struct semaphore sem;
} virtual_device;


//(2) to later register our device we need a cdev object and some other variables
struct cdev *mcdev; // my character device
int major_number; // store our major number - extracted from dev_t 
int ret; // hold return values for functions: ret < 0 -> error


dev_t dev_num; // hold the major number that kernel gives us

#define DEVICE_NAME "DinhAnhDung"
#define CEAS "Ceasar"

module_param(ceasar, int, S_IRUSR | S_IWUSR);

void display_c_info(void)
{
    printk(KERN_INFO "Ceasear now is %d\n", ceasar);
}

//(7) called on device_file open
//     inode reference to the file on disk
//      and contains information about that file
//      struct file is represents an abstract open file
int device_open(struct inode *inode, struct file *filp) // inode is actual physical file on the hard disk.
{
    //only alow one process to open this device by using a semaphore as mutual exclusive lock - mutex
    if(down_interruptible(&virtual_device.sem) != 0) // prevent other using resource
    {
        printk(KERN_ALERT "DAD: could not lock device during open");
        return -1;
    }

    printk(KERN_INFO "DAD: opened device");
    return 0;
}

//(8) called when user wants to get information from the device
ssize_t device_read(struct file* filp, char *buf_data, size_t buf_count, loff_t* cur_offset)
{
    // take data from kernel space(device) to user space(process)
    // copy_to_user (destination, source, sizeToTransfer)
    printk(KERN_INFO "DAD: Reading from device");
    ret = copy_to_user(buf_data, virtual_device.data, MAXI);
    return ret;
}

//(9) called when user wants to send information to the device

ssize_t device_write(struct file *filp, const char* buf_source, size_t buf_count, loff_t* cur_offset)
{
    int i;
    int decode = 0;
    i = 0;
    printk(KERN_INFO "DAD: writing to device");   
    ret = copy_from_user(virtual_device.data, buf_source, MAXI);
    while(1)
    {
        if(virtual_device.data[i] != '\0'){
            if(virtual_device.data[i] == '\n')decode = 1;
            if(decode)virtual_device.data[i] += ceasar;
        }
        else break;
        i++;
    }
    return ret;
}

//(10) called upon user close
int device_close(struct inode *inode, struct file *filp)
{
    //by calling up, which is opposite of down for semaphore, we release the mutex that we obained at device opn
    //this has the effect of allowing other process to use the device now
    up(&virtual_device.sem);
    printk(KERN_INFO "DAD: closed device");
    return 0;
}


//(6) define file operation file structure
struct file_operations fops = {
    .owner = THIS_MODULE,  //prevent unloading of this module when operations are in use
    .open = device_open,  // points to the method that open the device
    .release = device_close, // points to the method to call when closing the device
    .write = device_write,  // points to the method to call when writing to the device
    .read = device_read     //points to the method to call when reading from the device
};


static int driver_entry(void){
    //(3) Register our device with the system: a two step process
    //step(1) use dynamic allocation to assign our device
    //a major number-- alloc_chrdev_region(dev_t*, uint fminor, uint count, char *name)
    display_c_info();
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if(ret < 0)
    {
        printk(KERN_ALERT "DAD: failed to allocation a major number");
        return ret;
    }

    major_number = MAJOR(dev_num); // extracts the major number and stor
    printk(KERN_INFO "DAD: major number is %d", major_number);
    printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\" for device file\n", DEVICE_NAME, major_number);
    
    
    
    //step(2)
    mcdev = cdev_alloc(); // create our cdev structure, initialized our cdev (char device)
    mcdev->ops = &fops; // point to file operation of the char device
    mcdev->owner = THIS_MODULE;

    //now that we created cdev, we have to add it to the kernel
    //int cdev_add(struct cdev* dev, dev_t num, unsigned int count) 

    ret = cdev_add(mcdev, dev_num, 1);  // add the created char device to kernel

    if(ret<0)
    {
        printk(KERN_ALERT "DAD: unable to add cdev to kernel");
        return ret;
    }

    sema_init(&virtual_device.sem, 1); //initial value of one
    return 0;
}


static void driver_exit(void)
{
    //(5) unregister everything in reverse order
    //(a)
    cdev_del(mcdev);

    //(b)
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_ALERT "DAD: unloaded module");
}


module_init(driver_entry);
module_exit(driver_exit);
