// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2025 Arda Yetistiren
 */

#include <lib/format.h>
#include <lib/string.h>
#include <seren/log.h>
#include <seren/pit.h>
#include <seren/spinlock.h>

#define LOG_BUF_SIZE (16 * 1024)
#define LOG_ALIGN    8

static char log_buf[LOG_BUF_SIZE] __attribute__((aligned(LOG_ALIGN)));
static u64 log_head = 0;
static u64 log_tail = 0;
static u64 log_seq = 0;
static spinlock_t log_lock = SPIN_LOCK_UNLOCKED;

struct log_entry {
	struct log_msg_header hdr;
	char text[0];
} __attribute__((packed));

static inline u64 __log_len(u16 text_len) {
	return (sizeof(struct log_entry) + text_len + LOG_ALIGN - 1) &
	       ~(LOG_ALIGN - 1);
}

static inline struct log_entry *__log_from_idx(u64 idx) {
	return (struct log_entry *)(log_buf + (idx & (LOG_BUF_SIZE - 1)));
}

static void __log_make_free(u16 size) {
	// Loop while there is not enough space for the new message
	while ((log_head - log_tail) + size > (u64)LOG_BUF_SIZE) {
		struct log_entry *e = __log_from_idx(log_tail);
		u64 len = __log_len(e->hdr.len);

		if (!e->hdr.len)
			break;

		log_tail += len;
	}
}

int klog_write(int level, const char *msg) {
	u64 flags;
	u16 text_len;
	u64 size;
	struct log_entry *e;
	u64 ts;

	if (!msg)
		return -1;

	text_len = strlen(msg);
	if (text_len == 0)
		return 0;

	size = __log_len(text_len);

	spin_lock_irqsave(&log_lock, flags);

	__log_make_free(size);

	e = __log_from_idx(log_head);
	e->hdr.len = text_len;
	e->hdr.level = (u8)(level & 0xFF);
	e->hdr.__reserved = 0;

	ts = timer_get_uptime_ms();
	e->hdr.ts = ts;

	memcpy(e->text, msg, text_len);

	log_head += size;
	log_seq++;

	spin_unlock_irqrestore(&log_lock, flags);

	return text_len;
}

int klog_read(char *buffer, size_t size, struct log_msg_header *out_header,
	      u64 *sequence) {
	u64 flags;
	u64 idx;
	struct log_entry *e;
	u16 text_len;

	if (!buffer || !out_header || !sequence)
		return -1;

	spin_lock_irqsave(&log_lock, flags);

	if (*sequence >= log_seq) {
		spin_unlock_irqrestore(&log_lock, flags);
		return 0;
	}

	idx = log_tail;

	for (u64 i = 0; i < (*sequence - (log_seq - (log_head - log_tail)));
	     i++) {
		e = __log_from_idx(idx);
		if (!e->hdr.len)
			break;
		idx += __log_len(e->hdr.len);
	}

	e = __log_from_idx(idx);

	if (!e->hdr.len) {
		spin_unlock_irqrestore(&log_lock, flags);
		return 0;
	}

	*out_header = e->hdr;
	text_len = e->hdr.len;

	if (text_len > size - 1)
		text_len = size - 1;

	memcpy(buffer, e->text, text_len);
	buffer[text_len] = '\0';

	(*sequence)++;

	spin_unlock_irqrestore(&log_lock, flags);

	return text_len;
}