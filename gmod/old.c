/** @todo should block if no data available (from [bigbook]?)
    @todo just give back chuncks of data that are convenient (think of
    IAV implementation)
    
    Currently does nothing as we do not intend to read much from here.
*/
/* static ssize_t read (struct file * f , char __user * u, size_t s, loff_t * l) { */
 
/*   printk(KERN_INFO DEVICE_NAME " read attempt.\n"); */
/*   return -EINVAL; */
/* } */

/** 
    @todo
    - must be reentrant due to copy_user which can wait.
    - could I block? Yes, no problems with that?
    - If I accepted less bytes than count the caller will most
    probably retry immediately -- ie the driver will be stuck as it is
    expecting more bytes,  
    @todo perhaps just raise a warning if the data to be send is not a
    multiple of the dma channel width.
    @todo was there a convention to first return 0 bytes and the an error?
*/

//#define ROUNDUP(_size, _width) ( ( ( (_size) + (_width) - 1) / (_width) ) * (_width) )
