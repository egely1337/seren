#include <nucleus/printk.h>
#include <nucleus/console.h>
#include <stdarg.h>

#define PRINTK_BUFFER_SIZE 256
#define KERNEL_LOG_BUFFER_SIZE (4 * 1024)

static char temp_printk_buffer[PRINTK_BUFFER_SIZE];

static int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    if (!buf || !fmt || size == 0) return -1;

    char *p = buf;
    const char *s;
    // char temp_num_buf[22];
    int min_width;
    // char pad_char;

    while (*fmt && (size_t)(p - buf) < (size - 1)) {
        if (*fmt != '%') {
            *p++ = *fmt++;
            continue;
        }

        fmt++;
        min_width = 0;
        // pad_char = ' ';

        if (*fmt == '0') {
            // pad_char = '0';
            fmt++;
        }

        while (*fmt >= '0' && *fmt <= '9') {
            min_width = min_width * 10 + (*fmt - '0');
            fmt++;
        }

        switch (*fmt) {
            case 'c':
                *p++ = (char)va_arg(args, int);
                break;
            case 's':
                s = va_arg(args, const char*);
                if (!s) s = "(null)";
                while (*s && (size_t)(p - buf) < (size - 1)) {
                    *p++ = *s++;
                }

                break;
            case '%':
                *p++ = '%';
                break;
            default:
                *p++ = '%';

                if (*fmt && (size_t)(p - buf) < (size - 1)) {
                    *p++ = *fmt;
                } else {
                    p--;
                    goto end_loop;
                }
                break;
        }
        fmt++;
    }

end_loop:
    *p = '\0';
    return p - buf;
}

int printk(const char *fmt, ...) {
    va_list args;
    int count;

    va_start(args, fmt);
    count = kvsnprintf(temp_printk_buffer, PRINTK_BUFFER_SIZE, fmt, args);
    va_end(args);

    if (count > 0) {
        console_writestring(temp_printk_buffer);
    }

    return count;
}