#include "util/ring_buffer.h"

u64 rb_event_time_stamp(struct ring_buffer_event *event)
{
    u64 ts;

    ts = event->array[0];
    ts <<= TS_SHIFT;
    ts += event->time_delta;

    return ts;
}

