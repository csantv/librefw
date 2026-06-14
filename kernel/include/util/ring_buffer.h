// SPDX-License-Identifier: GPL-2.0
/*
 * Generic ring buffer
 *
 * Copyright (C) 2008 Steven Rostedt <srostedt@redhat.com>
 */

// bunch of stuff copied over from source/kernel/trace/ring_buffer.c

#pragma once

#include <asm/local.h>
#include <linux/ring_buffer.h>

/*
 * The "absolute" timestamp in the buffer is only 59 bits.
 * If a clock has the 5 MSBs set, it needs to be saved and
 * reinserted.
 */
#define TS_MSB		(0xf8ULL << 56)
#define ABS_TS_MASK	(~TS_MSB)
#define TS_SHIFT 27
#define RINGBUF_TYPE_DATA 0 ... RINGBUF_TYPE_DATA_TYPE_LEN_MAX
#define RB_ALIGNMENT 4U
#ifndef CONFIG_HAVE_64BIT_ALIGNED_ACCESS
#  define RB_ARCH_ALIGNMENT RB_ALIGNMENT
#else
#  define RB_ARCH_ALIGNMENT 8U
#endif
#define RB_ALIGN_DATA __aligned(RB_ARCH_ALIGNMENT)
#define RB_EVNT_HDR_SIZE (offsetof(struct ring_buffer_event, array))

struct buffer_data_page {
    u64 time_stamp;                     /* page time stamp */
    local_t commit;                     /* write committed index */
    unsigned char data[] RB_ALIGN_DATA; /* data of buffer page */
};

u64 rb_event_time_stamp(struct ring_buffer_event *event);
u64 rb_fix_abs_ts(u64 abs, u64 save_ts);
unsigned rb_event_data_length(struct ring_buffer_event *event);
unsigned rb_event_length(struct ring_buffer_event *event);
bool rb_null_event(struct ring_buffer_event *event);
