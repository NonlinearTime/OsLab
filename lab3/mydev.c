#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#ifndef MY_DATA_SIZE
#define MY_DATA_SIZE 512
#endif

/*struct to describe cdev*/
struct MycDev {
	struct cdev mycDev;
	char databuf[MY_DATA_SIZE];
};

static struct MycDev * my_cdev;
static dev_t dev;

/*open dev function*/
int my_module_open(struct inode * inode, struct file *pfile) {
	// printk("open module success!\n");
	//return 1;
}

/*close dev function*/
int my_module_close(struct inode * inode, struct file *pfile) {
	// printk("close module success!\n");
}

ssize_t my_module_read(struct file *pfile, char __user *buf, size_t size, loff_t *ppos) {
	unsigned long pos = *ppos;
	unsigned int count = size;
	int ret = 0;
	int i;

	// struct MycDev *pDev = pfile->private_data;

	if (pos >= MY_DATA_SIZE) {
		return 0;
	}
	if (pos + count > MY_DATA_SIZE) {
		count = MY_DATA_SIZE - pos ;
	}

	if (copy_to_user(buf, (void*)(my_cdev->databuf+pos),count)) {
		ret = -EFAULT;
	} else {
		*ppos+=count;
		ret = count;
		printk("my_module_read() : read %d bytes from %lu\n : %s",count, pos, buf);
		for (i = 0 ; i <= count ; ++i) {
			printk("read(): %c",buf[i]);
		}
	}
	printk("device: %s", my_cdev->databuf);
	return ret;
}

static ssize_t my_module_write(struct file *pfile, const char __user *buf, size_t size, loff_t *ppos)
{
	printk("writing...");
    unsigned long pos =  *ppos;
    printk("%lu",pos);
    unsigned int count = size;
    printk("%d",count);
    int ret = 0;
    // struct MycDev * pDev = (struct MycDev *)pfile->private_data; /*获得设备结构体指针*/
    
    /*分析和获取有效的写长度*/
    if(pos>=MY_DATA_SIZE)
    {
        return 0;
    }
    if(pos+count>MY_DATA_SIZE)
    {
        count=MY_DATA_SIZE-pos;
    }
    
    printk("copy_from_user...");
    printk("%p", my_cdev);
    printk("%p", my_cdev->databuf);
    /*从用户空间写入数据*/
    if(copy_from_user(my_cdev->databuf+pos,buf,count))
    {
        ret = -EFAULT;
    }
    else
    {
        *ppos+=count;
        ret=count;
        
        printk(KERN_EMERG "my_module_write():write %d byte(s) from %d\n",count,pos);      
    }
    printk("copy_from_user success !");
    
    return ret;
}


static loff_t my_module_llseek(struct file *pfile, loff_t offset, int whence) {
	// printk("module lseek success!\n");
}


static const struct file_operations my_cdev_ops = 
{
	.owner = THIS_MODULE,
	.open = my_module_open,
	.release = my_module_close,
	.read = my_module_read,
	.write = my_module_write,
	.llseek = my_module_llseek
	// .ioctl = NULL,
	// .select = NULL
};

/*initial for module*/
int my_module_init(void) {
	int ret;

	printk("running init_module...\n");

	/*alloc dev*/
	ret = alloc_chrdev_region(&dev,0,1,"MycDev");
	if (ret < 0) return ret;

	printk("init_module : alloc_chrdev_region() success!\n");

	printk("major:%d, minor:%d\n",MAJOR(dev),MINOR(dev));

	/*alloc memeory*/
	my_cdev = kmalloc(sizeof(struct MycDev), GFP_KERNEL);
	if (!my_cdev) {
		ret = -ENOMEM;
		unregister_chrdev_region(dev,1);
		printk("kmalloc fail !\n");
		return ret;
	}

	memset(my_cdev,0,sizeof(struct MycDev));
	printk("init_module : kmalloc() success !\n");

	/*register*/
	cdev_init(&(my_cdev->mycDev),&my_cdev_ops);
	cdev_add(&(my_cdev->mycDev),dev,1);
	printk("init_module : cdev_init() success!\n");
	printk("init_module : cdev_add() success!\n");
	
	return 0;
}

/*clean module*/
void my_module_clean(void) {
	printk("clean_module is running... \n");

	cdev_del(&(my_cdev->mycDev));
	kfree(my_cdev);
	unregister_chrdev_region(dev,1);

	printk("clean_module : exit success!\n");
	
}

module_init(my_module_init);
module_exit(my_module_clean);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("cdev by haines");