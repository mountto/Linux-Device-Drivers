/*
*	test_blockdev.c
*   le duc hoang hai
*
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
//#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/blk-mq.h>
#include <linux/hdreg.h>

#ifndef SECTOR_SIZE
#define SECTOR_SIZE 512
#endif

static int dev_major = 0;
static int dev_major_os_init;

/* Just internal representation of the our block device
 * can hold any useful data */
struct block_dev {
    sector_t capacity;
    u8 *data;   /* Data buffer to emulate real storage device */
    struct blk_mq_tag_set tag_set;
    struct request_queue *queue;
    struct gendisk *gdisk;
};

/* Device instance */
static struct block_dev *block_device = NULL;

static int blockdev_open(struct block_device *dev, fmode_t mode)
{
    printk(">>> blockdev_open\n");

    return 0;
}

static void blockdev_release(struct gendisk *gdisk, fmode_t mode)
{
    printk(">>> blockdev_release\n");
}

int blockdev_ioctl(struct block_device *bdev, fmode_t mode, unsigned cmd, unsigned long arg)
{
    printk("ioctl cmd 0x%08x\n", cmd);

    return -ENOTTY;
}

/* Set block device file I/O */
static struct block_device_operations blockdev_ops = {
    .owner = THIS_MODULE,
    .open = blockdev_open,
    .release = blockdev_release,
    .ioctl = blockdev_ioctl
};

/* Serve requests */
static int do_request(struct request *rq, unsigned int *nr_bytes)
{
    int ret = 0;
    struct bio_vec bvec;
    struct req_iterator iter;
    struct block_dev *dev = rq->q->queuedata;
    loff_t pos = blk_rq_pos(rq) << SECTOR_SHIFT;
    loff_t dev_size = (loff_t)(dev->capacity << SECTOR_SHIFT);

    printk(KERN_WARNING "sblkdev: request start from sector %lld  pos = %lld  dev_size = %lld\n", blk_rq_pos(rq), pos, dev_size);

    /* Iterate over all requests segments */
    rq_for_each_segment(bvec, rq, iter)
    {
        unsigned long b_len = bvec.bv_len;

        /* Get pointer to the data */
        void* b_buf = page_address(bvec.bv_page) + bvec.bv_offset;

        /* Simple check that we are not out of the memory bounds */
        if ((pos + b_len) > dev_size) {
            b_len = (unsigned long)(dev_size - pos);
        }

        if (rq_data_dir(rq) == WRITE) {
            /* Copy data to the buffer in to required position */
            memcpy(dev->data + pos, b_buf, b_len);
        } else {
            /* Read data from the buffer's position */
            memcpy(b_buf, dev->data + pos, b_len);
        }

        /* Increment counters */
        pos += b_len;
        *nr_bytes += b_len;
    }

    return ret;
}

/* queue callback function */
static blk_status_t queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data* bd)
{
    unsigned int nr_bytes = 0;
    blk_status_t status = BLK_STS_OK;
    struct request *rq = bd->rq;

    /* Start request serving procedure */
    blk_mq_start_request(rq);

    if (do_request(rq, &nr_bytes) != 0) {
        status = BLK_STS_IOERR;
    }

    /* Notify kernel about processed nr_bytes */
    if (blk_update_request(rq, status, nr_bytes)) {
        /* Shouldn't fail */
        BUG();
    }

    /* Stop request serving procedure */
    __blk_mq_end_request(rq, status);

    return status;
}

static struct blk_mq_ops mq_ops = {
    .queue_rq = queue_rq,
};

static int __init myblock_driver_init(void)
{
    	int err = -ENOMEM;
    //struct request_queue *q;

    /* Register new block device and get device major number */
    //Vì dev_major = 0 nên kernel sẻ cấp phát đại 1 dev_major nào đó sau khi register_blkdev sẻ vào dev_major_os_init
    dev_major_os_init = register_blkdev(dev_major, "testblk");

    block_device = kmalloc(sizeof (struct block_dev), GFP_KERNEL);

    if (block_device == NULL) {
        printk("Failed to allocate struct block_dev\n");
        unregister_blkdev(dev_major_os_init, "testblk");

        return -ENOMEM;
    }

    /* Set some random capacity of the device */
    block_device->capacity = (112 * PAGE_SIZE) >> 9; /* nsectors * SECTOR_SIZE; */
    /* Allocate corresponding data buffer */
    block_device->data = kmalloc(block_device->capacity << 9, GFP_KERNEL);

    if (block_device->data == NULL) {
        printk("Failed to allocate device IO buffer\n");
        unregister_blkdev(dev_major_os_init, "testblk");
        kfree(block_device);

        return -ENOMEM;
    }

    printk("Initializing queue\n");

    memset(&block_device->tag_set, 0, sizeof(block_device->tag_set));
	block_device->tag_set.ops = &mq_ops;
	block_device->tag_set.queue_depth = 128;
	block_device->tag_set.numa_node = NUMA_NO_NODE;
	//block_device->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
	block_device->tag_set.nr_hw_queues = num_present_cpus();
    //block_device_>tag_set.driver_data = block_device;
    //block_device->tag_set.nr_maps = 1;

    err = blk_mq_alloc_tag_set(&block_device->tag_set);
    if (err)
		    return err;

    printk("OOOOOOOOOOOO o day [err] ");
    /* Set driver's structure as user data of the queue */
    //block_device->queue->queuedata = block_device;
    printk("111111111111111111111");
    /* Allocate new disk */
    block_device->gdisk = blk_mq_alloc_disk(&block_device->tag_set,block_device);
    block_device->queue = block_device->gdisk->queue;
    
    blk_queue_max_hw_sectors(block_device->queue, BLK_DEF_MAX_SECTORS);
    	/*
	 * By default, we do buffer IO, so it doesn't make sense to enable
	 * merge because the I/O submitted to backing file is handled page by
	 * page. For directio mode, merge does help to dispatch bigger request
	 * to underlayer disk. We will enable merge once directio is enabled.
	 */
	blk_queue_flag_set(QUEUE_FLAG_NOMERGES, block_device->queue);

    printk("2222222222222222222");
    /* Set all required flags and data */
    //block_device->gdisk->flags = GENHD_FL_NO_PART;
    block_device->gdisk->major = dev_major;
    block_device->gdisk->first_minor = 0;
    printk("3333333333333333333333333");
    block_device->gdisk->fops = &blockdev_ops;
    block_device->gdisk->queue = block_device->queue;
    block_device->gdisk->private_data = block_device;
    block_device->gdisk->events		= DISK_EVENT_MEDIA_CHANGE;
	block_device->gdisk->event_flags	= DISK_EVENT_FLAG_UEVENT;

    printk("4444444444444444444444");
        if (block_device->queue == NULL) {
        printk("Failed to allocate device queue\n");
        kfree(block_device->data);

        unregister_blkdev(dev_major_os_init, "testblk");
        kfree(block_device);

        return -ENOMEM;
        }


    /* Set device name as it will be represented in /dev */
    strncpy(block_device->gdisk->disk_name, "blockdev\0", 9);

    printk("Adding disk %s\n", block_device->gdisk->disk_name);

    /* Set device capacity */
    set_capacity(block_device->gdisk, block_device->capacity);
    printk("==========gan cuoi=============");
    	/*
	 * This is so fdisk will align partitions on 4k, because of
	 * direct_access API needing 4k alignment, returning a PFN
	 * (This is only a problem on very small devices <= 4M,
	 *  otherwise fdisk will align on 1M. Regardless this call
	 *  is harmless)
	 */
	//blk_queue_physical_block_size(disk->queue, PAGE_SIZE);

    	/* Tell the block layer that this is not a rotational device */
	//blk_queue_flag_set(QUEUE_FLAG_NONROT, disk->queue);
	//blk_queue_flag_clear(QUEUE_FLAG_ADD_RANDOM, disk->queue);

    /* Notify kernel about new disk device */
    err = add_disk(block_device->gdisk);
        printk("======================Adding disk==================\n");

if (err)
		goto out_cleanup_disk;

	return 0;

out_cleanup_disk:
	put_disk(block_device->gdisk);
	printk("............CUOI..........");
	return err;
}

static void __exit myblock_driver_exit(void)
{
    /* Don't forget to cleanup everything */
    if (block_device->gdisk) {
        del_gendisk(block_device->gdisk);
        put_disk(block_device->gdisk);
    }

    if (block_device->queue) {
        blk_mq_destroy_queue(block_device->queue);
    }

    kfree(block_device->data);

    unregister_blkdev(dev_major_os_init, "testblk");
    kfree(block_device);
}

module_init(myblock_driver_init);
module_exit(myblock_driver_exit);
MODULE_LICENSE("GPL");

