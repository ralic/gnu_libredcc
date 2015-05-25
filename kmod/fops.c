#include <linux/module.h>
#include <linux/fs.h>

static int major = DEVICE_MAJOR; // major device number
module_param(major, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(major, "Major device number used for dcc device.");

static bool opened;
static int open (struct inode *, struct file *) {
  if(opened) return -EBUSY;
  
  // we could do part of the initialisation here instead of in init (eg switching outputs)

  opened = true;
  return 0;
}

static int release (struct inode *, struct file *) {

  opened = false;
  return 0;
}

static ssize_t read (struct file *, char __user *, size_t, loff_t *) {
  return -EINVAL;
}

static ssize_t write (struct file *, const char __user *, size_t, loff_t *) {
}


static struct file_operations fops = {
  //  	.read = dcc_read,
  .owner = THIS_MODULE,
  .write = dcc_write,
  .open = dcc_open,
  .release = dcc_release
};

static enum fops_level {nothing, got_major} fops_level = nothing;

int __init fops_init(void) {

  // setup chardev 	
  int major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops);
  if (major < 0) {
    printk(KERN_ALERT "Registering device " DEVICE_NAME " failed with %d\n", major);
    fops_unwind();
    return major;
  }
  fops_level = got_major;


  printk(KERN_INFO "DCC service has major device number %d.\n", major);
  return 0;
}

void fops_unwind(void) {

  switch(fops_level) {
  default:
  case got_major:
    unregister_chrdev(major, DEVICE_NAME); 
  case nothing: {
    // nothing to do 
  }
  }
  return;
}


