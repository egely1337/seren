#include <lib/format.h>
#include <nucleus/printk.h>
#include <nucleus/tty/console.h>
#include <nucleus/types.h>
#include <stdarg.h>

#define PRINTK_BUFFER_SIZE     256
#define KERNEL_LOG_BUFFER_SIZE (4 * 1024)

static char temp_printk_buffer[PRINTK_BUFFER_SIZE];

int printk(const char *fmt, ...) {
    va_list args;
    const char *fmt_body = fmt;
    bool info_flag = false;

    if (fmt[0] == '<' && fmt[2] == '>') {
        char level = fmt[1];
        const char *prefix_str = "";
        uint32_t prefix_color = CONSOLE_DEFAULT_FG_COLOR;

        switch (level) {
        case '0': // KERN_EMERG
            prefix_str = "EMRG";
            prefix_color = COLOR_BRIGHT_RED;
            break;
        case '1': // KERN_ALERT
            prefix_str = "ALRT";
            prefix_color = COLOR_BRIGHT_RED;
            break;
        case '2': // KERN_CRIT
            prefix_str = "CRIT";
            prefix_color = COLOR_BRIGHT_RED;
            break;
        case '3': // KERN_ERR
            prefix_str = "ERR ";
            prefix_color = COLOR_RED;
            break;
        case '4': // KERN_WARN
            prefix_str = "WARN";
            prefix_color = COLOR_BRIGHT_YELLOW;
            break;
        case '5': // KERN_NOTICE
            prefix_str = "NOTE";
            prefix_color = COLOR_GREEN;
            break;
        case '6': // KERN_INFO
            prefix_str = "    ";
            info_flag = true;
            break;
        case '7': // KERN_DEBUG
            prefix_str = "DBG ";
            prefix_color = COLOR_DARK_GREY;
            break;
        }

        console_reset_fg_color();
        console_putchar(info_flag ? ' ' : '[');
        console_set_fg_color(prefix_color);
        console_writestring(prefix_str);

        console_reset_fg_color();
        console_putchar(info_flag ? ' ' : ']');
        console_putchar(' ');
    }

    fmt_body = fmt + 3;

    va_start(args, fmt);
    int count =
        kvsnprintf(temp_printk_buffer, PRINTK_BUFFER_SIZE, fmt_body, args);
    va_end(args);

    if (count > 0) {
        console_reset_fg_color();
        console_writestring(temp_printk_buffer);
    }

    return count;
}