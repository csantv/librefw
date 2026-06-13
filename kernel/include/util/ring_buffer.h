#include <asm/local.h>
#include <linux/ring_buffer.h>

#define TS_SHIFT 27
#define RINGBUF_TYPE_DATA 0 ... RINGBUF_TYPE_DATA_TYPE_LEN_MAX
#define RB_ALIGNMENT 4U
#ifndef CONFIG_HAVE_64BIT_ALIGNED_ACCESS
#  define RB_FORCE_8BYTE_ALIGNMENT 0
#  define RB_ARCH_ALIGNMENT RB_ALIGNMENT
#else
#  define RB_FORCE_8BYTE_ALIGNMENT 1
#  define RB_ARCH_ALIGNMENT 8U
#endif
#define RB_ALIGN_DATA __aligned(RB_ARCH_ALIGNMENT)

struct buffer_data_page {
    u64 time_stamp;                     /* page time stamp */
    local_t commit;                     /* write committed index */
    unsigned char data[] RB_ALIGN_DATA; /* data of buffer page */
};

u64 rb_event_time_stamp(struct ring_buffer_event *event);
