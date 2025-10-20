// SPDX-License-Identifier: Apache-2.0

#ifndef _SEREN_PRINTK_H
#define _SEREN_PRINTK_H

#include <lib/stdarg.h>

#define KERN_EMERG  "<0>" /* System is unusable */
#define KERN_ALERT  "<1>" /* Action must be taken immediately */
#define KERN_CRIT   "<2>" /* Critical conditions */
#define KERN_ERR    "<3>" /* Error conditions */
#define KERN_WARN   "<4>" /* Warning conditions */
#define KERN_NOTICE "<5>" /* Normal but significant condition */
#define KERN_INFO   "<6>" /* Informational */
#define KERN_DEBUG  "<7>" /* Debug-level messages */

#define LOGLEVEL_EMERG	0
#define LOGLEVEL_ALERT	1
#define LOGLEVEL_CRIT	2
#define LOGLEVEL_ERR	3
#define LOGLEVEL_WARN	4
#define LOGLEVEL_NOTICE 5
#define LOGLEVEL_INFO	6
#define LOGLEVEL_DEBUG	7

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

/**
 * printk - The core kernel printing function.
 *
 * Prints a formatted string to the kernel log buffer and console.
 * It is recommended to use the pr_* convenience macros instead of
 * calling printk() directly.
 */
int printk(const char *fmt, ...);

#define pr_emerg(fmt, ...)  printk(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pr_alert(fmt, ...)  printk(KERN_ALERT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_crit(fmt, ...)   printk(KERN_CRIT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err(fmt, ...)    printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn(fmt, ...)   printk(KERN_WARN pr_fmt(fmt), ##__VA_ARGS__)
#define pr_notice(fmt, ...) printk(KERN_NOTICE pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info(fmt, ...)   printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#define pr_debug(fmt, ...)  printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)

#define DEFAULT_MESSAGE_LOGLEVEL KERN_INFO

#endif // _SEREN_PRINTK_H