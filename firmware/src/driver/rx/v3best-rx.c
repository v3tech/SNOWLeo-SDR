#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>

//#define DEBUG
#ifdef DEBUG
#define dma_dbg(format, ...) \
    do { \
        printk(KERN_INFO format, ## __VA_ARGS__); \
    } while (0)
#else
#define dma_dbg(format, ...)
#endif

/*AXI DMA IPcore memory map address*/
#define AXI_DMA_BASE_WR 0x41210000

/*AXI DMA IPcore register offset*/
#define S2MM_START   0x0
#define S2MM_DA      0x4
#define S2MM_LENGTH  0x8
#define S2MM_CYC     0xc

/*AXI DMA IPcore mask*/
#define S2MM_DONE   0x0

#define S2MM_CHANNEL  0

#define IP_RESET  	 0
#define FIFO_IP_RESET	 1

#define DEVICE_NAME         "v3best-rx"
#define AXI_DMA_S2MM_IRQ    90
#define  XDMA_MAGIC              'D'
#define  XDMA_IOCTL_RESET          _IO(XDMA_MAGIC, 1)
#define  XDMA_START_WAIT_COMPLETE        _IO(XDMA_MAGIC, 2)
#define  XDMA_IOCTL_CONFIG          _IO(XDMA_MAGIC, 3)

static void __iomem *dma_wr_reg;
struct completion dma_completion;

struct axi_dma_config_info {
    int channel;
    unsigned long mem_addr;
    unsigned long length;
    unsigned int  cycle;
    unsigned int  reset_type;
};

/**
 * @brief Setup AXI dma IPcore.
 *
 * @param[in] channel          0 S2MM_CHANNEL, 1 MM2S_CHANNEL
 * @param[in] length           Data length for once dma transmission
 * @param[in] cycle            Number of dma transmission 
 * @param[in] mem_addr         Physical DDR address of AXI dma IPCore
 * @param[in] reg_baseaddr     Virtual register address of AXI dma IPcore
 * @retval                     0 Success
 * @retval                     -1 Failure
 *
 */
int dma_init(int channel, unsigned long length, unsigned int cycle,
        unsigned long mem_addr, void __iomem *reg_baseaddr)
{
    if(channel == 0) {
        iowrite32(mem_addr, reg_baseaddr + S2MM_DA);
        iowrite32(length, reg_baseaddr + S2MM_LENGTH);
        iowrite32(cycle, reg_baseaddr + S2MM_CYC);
    }

    return 0;
}


/**
 * @brief start AXI dma IPcore.
 *
 * @param[in] channel          0 S2MM_CHANNEL, 1 MM2S_CHANNEL
 * @param[in] mode             Operating Mode of MM2S_CHANNEL  0 Auto, 1 Manual
 * @param[in] reg_baseaddr     Virtual register address of AXI dma IPcore
 * @retval                     0 Success
 * @retval                     -1 Failure
 *
 */
int dma_start(int channel, int mode, void __iomem *reg_baseaddr)
{
    if(channel == 0)
        iowrite32(0x11, reg_baseaddr + S2MM_START);

    return 0;
}

/**
 * @brief reset AXI dma IPcore.
 *
 * @param[in] channel          0 S2MM_CHANNEL, 1 MM2S_CHANNEL
 * @param[in] length           Data length for once dma transmission
 * @param[in] reg_baseaddr     Virtual register address of AXI dma IPcore
 * @param[in] type             reset type, IP_RESET only reset IPCore, FIFO_IP_RESET reset IPcore&fifo
 * @retval                     0 Success
 * @retval                     -1 Failure
 *
 */
int dma_reset(int channel, void __iomem *reg_baseaddr, int type)
{
    if(channel == 0&&type == IP_RESET)
        iowrite32(0x10, reg_baseaddr + S2MM_START);

    if(channel == 0&&type == FIFO_IP_RESET)
        iowrite32(0x00, reg_baseaddr + S2MM_START);

    return 0;
}

static int axi_dma_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int axi_dma_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long axi_dma_ioctl(struct file *filp, unsigned int cmd,
        unsigned long arg)
{
    struct axi_dma_config_info *config_info = (struct axi_dma_config_info *)arg;
    switch (cmd) {
    case XDMA_IOCTL_RESET:
        if(config_info->channel == S2MM_CHANNEL){
            dma_reset(config_info->channel, dma_wr_reg, config_info->reset_type);
        }
        break;
    case XDMA_IOCTL_CONFIG:
        if(config_info->channel == S2MM_CHANNEL){
            dma_init(config_info->channel, config_info->length, config_info->cycle,
                    config_info->mem_addr, dma_wr_reg);
        }
        break;
    case XDMA_START_WAIT_COMPLETE:
        if(config_info->channel == S2MM_CHANNEL){
            dma_start(config_info->channel, 0, dma_wr_reg);
            wait_for_completion(&dma_completion);
        }
        break;
    default:
        return -EINVAL;
        break;
    }
    return 0;
}

static struct file_operations axi_dma_fops = {
    .owner = THIS_MODULE,
    .open = axi_dma_open,
    .release = axi_dma_release,
    .unlocked_ioctl = axi_dma_ioctl,
};

static struct miscdevice axi_dma_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &axi_dma_fops,
};

irqreturn_t axi_dma_intr_handler(int irq, void *dev_id)
{
    if(irq == AXI_DMA_S2MM_IRQ) {
        dma_dbg("S2MM_CHANNEL irq\n");
        complete(&dma_completion);
    }
    return IRQ_HANDLED;
}

static int __init axi_dma_init(void)
{
    int ret;

    dma_wr_reg = ioremap(AXI_DMA_BASE_WR, 64);

    /*DMA Interrupt enable */
    ret = request_irq(AXI_DMA_S2MM_IRQ, axi_dma_intr_handler, 
            IRQF_TRIGGER_RISING, DEVICE_NAME, NULL);
    if (ret < 0) {
        dma_dbg(KERN_ERR "unable to request IRQ%d : %d\n", AXI_DMA_S2MM_IRQ, ret);
        return ret;
    }

    init_completion(&dma_completion);
    ret = misc_register(&axi_dma_dev);
    return ret;
}

static void __exit axi_dma_exit(void)
{
    iounmap(dma_wr_reg);
    free_irq(AXI_DMA_S2MM_IRQ, NULL);
    misc_deregister(&axi_dma_dev);
    dma_dbg("AXI_DMA module exit.\n");
}

module_init(axi_dma_init);
module_exit(axi_dma_exit);

MODULE_DESCRIPTION("AXI DMA driver");
MODULE_VERSION("v1.0");
MODULE_LICENSE("GPL");
