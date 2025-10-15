#include <lib/format.h>
#include <nucleus/printk.h>
#include <nucleus/tty/console.h>
#include <nucleus/types.h>
#include <lib/stdarg.h>

#define PRINTK_BUFFER_SIZE     256
#define KERNEL_LOG_BUFFER_SIZE (4 * 1024)
#define KERN_DEFAULT_LOGLEVEL  4

static char temp_printk_buffer[PRINTK_BUFFER_SIZE];

static int g_console_loglevel = 7;

int printk(const char *fmt, ...) {
    va_list args;
    const char *fmt_body = fmt;
    int level = KERN_DEFAULT_LOGLEVEL;

    if (fmt[0] == '<' && fmt[2] == '>') {
        if (fmt[1] >= '0' && fmt[1] <= '7') {
            level = fmt[1] - '0';
            fmt_body = fmt + 3;
        }
    }

    if (level > g_console_loglevel) {
        return 0;
    }

    va_start(args, fmt);
    int count =
        kvsnprintf(temp_printk_buffer, PRINTK_BUFFER_SIZE, fmt_body, args);

    if (count >= 0) {
        console_log(level, temp_printk_buffer);
    }

    return count;
}