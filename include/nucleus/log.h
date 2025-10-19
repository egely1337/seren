// SPDX-License-Identifier: Apache-2.0

#ifndef _NUCLEUS_LOG_H
#define _NUCLEUS_LOG_H

#include <lib/stdarg.h>
#include <nucleus/types.h>

#define KERN_EMERG  0 /* System is unusable */
#define KERN_ALERT  1 /* Action must be taken immediately */
#define KERN_CRIT   2 /* Critical conditions */
#define KERN_ERR    3 /* Error conditions */
#define KERN_WARN   4 /* Warning conditions */
#define KERN_NOTICE 5 /* Normal but significant condition */
#define KERN_INFO   6 /* Informational */
#define KERN_DEBUG  7 /* Debug-level messages */

struct log_msg_header {
	u16 len;
	u8 level;
	u8 __reserved;
	u64 timestamp_ms;
};

int klog_write(int level, const char *fmt, ...);

int klog_vwrite(int level, const char *fmt, va_list args);

int klog_read(char *buffer, size_t size, struct log_msg_header *out_header,
	      u64 *sequence);

#endif // _NUCLEUS_LOG_H