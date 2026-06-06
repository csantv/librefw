// librefw: a free as in freedom firewall
// Copyright (C) 2026  Carlos Santos Toro Vera
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <linux/ip.h>
#include <linux/percpu.h>
#include <linux/ring_buffer.h>
#include <linux/tcp.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/udp.h>
#include <linux/workqueue.h>

#include "log/packet_filter.h"

struct pkt_filter_log_state {
    struct trace_buffer *events;
    struct workqueue_struct *workqueue;
};

struct pkt_filter_flush_task {
    struct work_struct real_work;
    int cpu_id;
    int flush_incomplete;
};

struct pkt_filter_event {
    __be32 source_ip;
    __be32 dest_ip;
    __be16 source_port;
    __be16 dest_port;
    u8 protocol;
    u8 ttl;
};

struct pkt_filter_flush_ctx {
    void *data;
    int data_len;
};

static DEFINE_PER_CPU_ALIGNED(unsigned int, packet_counter);
static DEFINE_PER_CPU_ALIGNED(struct timer_list, packet_timer);
static DEFINE_PER_CPU_ALIGNED(struct pkt_filter_flush_task, pkt_filter_flush_tracker);
static struct pkt_filter_log_state *state = NULL;

int init_pkt_filter_log_state(void)
{
    int ret = -ENOMEM;
    state = kzalloc(sizeof(struct pkt_filter_log_state), GFP_KERNEL);
    if (unlikely(!state)) {
        return ret;
    }
    struct pkt_filter_log_state *st = state;

    st->events = ring_buffer_alloc(256 * 1024, RB_FL_OVERWRITE);
    if (unlikely(!st->events)) {
        goto err_free_state;
    }

    st->workqueue = alloc_workqueue("lfw_pkt_filter_log", WQ_PERCPU, 1);
    if (unlikely(!st->workqueue)) {
        goto err_free_rb;
    }

    int cpu;
    for_each_online_cpu(cpu)
    {
        per_cpu(packet_counter, cpu) = 0;
        struct pkt_filter_flush_task *task = per_cpu_ptr(&pkt_filter_flush_tracker, cpu);
        task->cpu_id = cpu;
        task->flush_incomplete = 0;
        INIT_WORK(&task->real_work, flush_pkt_filter_events);

        struct timer_list *timer = per_cpu_ptr(&packet_timer, cpu);
        timer_setup(timer, sched_flush_pkt_filter_events, TIMER_PINNED);
        timer->expires = jiffies + secs_to_jiffies(5);
        add_timer_on(timer, cpu);
    }

    return 0;

err_free_rb:
    ring_buffer_free(st->events);
    st->events = NULL;
err_free_state:
    kfree(st);
    state = NULL;
    return ret;
}

void free_pkt_filter_log_state(void)
{
    if (unlikely(!state)) {
        return;
    }
    struct pkt_filter_log_state *st = state;

    int cpu;
    for_each_online_cpu(cpu)
    {
        struct timer_list *timer = per_cpu_ptr(&packet_timer, cpu);
        timer_shutdown_sync(timer);
    }

    if (st->workqueue) {
        destroy_workqueue(st->workqueue);
    }
    ring_buffer_free(st->events);
    kfree(st);
}

// must only be executed in a soft-irq context
int log_pkt_filter_event(struct iphdr *iph, struct sk_buff *skb)
{
    if (unlikely(__this_cpu_inc_return(packet_counter) >= 128)) {
        // wake up workqueue to start flushing every 128 packets
        struct pkt_filter_flush_task *task = this_cpu_ptr(&pkt_filter_flush_tracker);
        task->flush_incomplete = 0;
        if (queue_work(state->workqueue, &task->real_work)) {
            // enable requeue-ing later
            __this_cpu_write(packet_counter, 0);
        }
    }

    struct ring_buffer_event *event = ring_buffer_lock_reserve(state->events, sizeof(struct pkt_filter_event));
    if (!event) {
        return -ENOMEM;
    }

    struct pkt_filter_event *entry = ring_buffer_event_data(event);
    entry->source_ip = iph->saddr;
    entry->dest_ip = iph->daddr;
    entry->ttl = iph->ttl;
    entry->protocol = iph->protocol;
    entry->source_port = 0;
    entry->dest_port = 0;

    int thoff = skb_network_offset(skb) + (iph->ihl * 4);
    if (iph->protocol == IPPROTO_TCP) {
        struct tcphdr _tcph, *th;
        th = skb_header_pointer(skb, thoff, sizeof(_tcph), &_tcph);
        if (likely(th)) {
            entry->source_port = th->source;
            entry->dest_port = th->dest;
        }
    } else if (iph->protocol == IPPROTO_UDP) {
        struct udphdr _udph, *uh;
        uh = skb_header_pointer(skb, thoff, sizeof(_udph), &_udph);
        if (likely(uh)) {
            entry->source_port = uh->source;
            entry->dest_port = uh->dest;
        }
    }
    ring_buffer_unlock_commit(state->events);
    return 0;
}

void flush_pkt_filter_events(struct work_struct *work)
{
    struct pkt_filter_flush_task *task = container_of(work, struct pkt_filter_flush_task, real_work);
    struct buffer_data_read_page *rpage = ring_buffer_alloc_read_page(state->events, task->cpu_id);

    int page_size = ring_buffer_subbuf_size_get(state->events);
    while (true) {
        int ret = ring_buffer_read_page(state->events, rpage, page_size, task->cpu_id, task->flush_incomplete);
        if (ret < 0) {
            break;
        }
        process_rb_page(ring_buffer_read_page_data(rpage), ret);
    }
    ring_buffer_free_read_page(state->events, task->cpu_id, rpage);
}

void process_rb_page(void *data, int data_len)
{
    int bytes_read = 0;
    while (bytes_read < data_len) {
        struct ring_buffer_event *event = data + bytes_read;
        if (event->type_len == RINGBUF_TYPE_PADDING) {
            break;
        }

        int event_len = ring_buffer_event_length(event);
        if (event_len <= 0 || bytes_read + event_len > data_len) {
            break;
        }

        struct pkt_filter_event *entry = ring_buffer_event_data(event);
        bytes_read += ring_buffer_event_length(event);
    }
    /*struct ring_buffer_event *event;
    u64 ts;
    unsigned long lost;
    while ((event = ring_buffer_consume(state->events, task->cpu_id, &ts, &lost)) != NULL) {
        struct pkt_filter_event *entry = ring_buffer_event_data(event);
        // TODO: send data to netlink multicast group
    }*/
}

void sched_flush_pkt_filter_events(struct timer_list *timer)
{
    struct pkt_filter_flush_task *task = this_cpu_ptr(&pkt_filter_flush_tracker);
    task->flush_incomplete = 1;
    if (queue_work(state->workqueue, &task->real_work)) {
        // enable requeue-ing later
        __this_cpu_write(packet_counter, 0);
    }
    mod_timer(timer, jiffies + msecs_to_jiffies(250));
}
