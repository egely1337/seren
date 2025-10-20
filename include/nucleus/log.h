#ifndef _NUCLEUS_LOG_H
#define _NUCLEUS_LOG_H

#include <nucleus/types.h>

struct log_msg_header {
	u16 len;
	u8 level;
	u8 __reserved;
	u64 ts;
} __attribute__((packed));

/**
 * klog_write - Write a fully formatted message into kernel log buffer.
 * @level: The log level
 * @msg: The null-terminated message string
 */
int klog_write(int level, const char *msg);

/**
 * klog_read - Read the next message from the kernel log buffer.
 * @buffer:	Buffer to store the message text.
 * @size:	Size of the user-provided buffer.
 * @out_header:	Pointer to store the message header.
 * @sequence:	Pointer to a sequence number, used to read subsequent messages.
 * 		Initialize to 0 to read the first message.
 *
 * This function is for console drivers.
 */
int klog_read(char *buffer, size_t size, struct log_msg_header *out_header,
	      u64 *sequence);

#endif // _NUCLEUS_LOG_H