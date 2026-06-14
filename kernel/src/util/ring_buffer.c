// SPDX-License-Identifier: GPL-2.0
/*
 * Generic ring buffer
 *
 * Copyright (C) 2008 Steven Rostedt <srostedt@redhat.com>
 */

// bunch of stuff copied over from source/kernel/trace/ring_buffer.c

#include "util/ring_buffer.h"

enum {
    RB_LEN_TIME_EXTEND = 8,
    RB_LEN_TIME_STAMP = 8,
};

bool rb_null_event(struct ring_buffer_event *event)
{
    return event->type_len == RINGBUF_TYPE_PADDING && !event->time_delta;
}

u64 rb_event_time_stamp(struct ring_buffer_event *event)
{
    u64 ts;

    ts = event->array[0];
    ts <<= TS_SHIFT;
    ts += event->time_delta;

    return ts;
}

u64 rb_fix_abs_ts(u64 abs, u64 save_ts)
{
    if (save_ts & TS_MSB) {
        abs |= save_ts & TS_MSB;
        /* Check for overflow */
        if (unlikely(abs < save_ts))
            abs += 1ULL << 59;
    }
    return abs;
}

unsigned rb_event_data_length(struct ring_buffer_event *event)
{
    unsigned length;

    if (event->type_len)
        length = event->type_len * RB_ALIGNMENT;
    else
        length = event->array[0];
    return length + RB_EVNT_HDR_SIZE;
}

/*
 * Return the length of the given event. Will return
 * the length of the time extend if the event is a
 * time extend.
 */
inline unsigned rb_event_length(struct ring_buffer_event *event)
{
    switch (event->type_len) {
        case RINGBUF_TYPE_PADDING:
            if (rb_null_event(event))
                /* undefined */
                return -1;
            return event->array[0] + RB_EVNT_HDR_SIZE;

        case RINGBUF_TYPE_TIME_EXTEND:
            return RB_LEN_TIME_EXTEND;

        case RINGBUF_TYPE_TIME_STAMP:
            return RB_LEN_TIME_STAMP;

        case RINGBUF_TYPE_DATA:
            return rb_event_data_length(event);
        default:
            WARN_ON_ONCE(1);
    }
    /* not hit */
    return 0;
}
