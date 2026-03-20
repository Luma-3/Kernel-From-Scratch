#ifndef __KFS_PRINTK_H
#define __KFS_PRINTK_H

enum kernel_log_level {
    KERNEL_LOG_LEVEL_DEBUG,
    KERNEL_LOG_LEVEL_INFO,
    KERNEL_LOG_LEVEL_WARN,
    KERNEL_LOG_LEVEL_ERROR
};


int printk(enum kernel_log_level log_level, const char *format, ...);

#endif //__KFS_PRINTK_H