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
};

struct pkt_filter_event {
    __be32 source_ip;
    __be32 dest_ip;
    __be16 source_port;
    __be16 dest_port;
    u8 protocol;
    u8 ttl;
};

static DEFINE_PER_CPU_ALIGNED(unsigned int, packet_counter);
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

    st->workqueue = alloc_workqueue("lfw_pkt_filter_log", WQ_PERCPU, 1);
    if (unlikely(!st->workqueue)) {
        goto err_free_state;
    }

    int cpu;
    for_each_cpu(cpu, cpu_online_mask)
    {
        struct pkt_filter_flush_task *task = per_cpu_ptr(&pkt_filter_flush_tracker, cpu);
        task->cpu_id = cpu;
        INIT_WORK(&task->real_work, flush_pkt_filter_events);
    }

    st->events = ring_buffer_alloc(256 * 1024, RB_FL_OVERWRITE);
    if (unlikely(!st->events)) {
        goto err_destroy_wq;
    }
    return 0;

err_destroy_wq:
    destroy_workqueue(st->workqueue);
    st->workqueue = NULL;
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

    if (st->workqueue) {
        destroy_workqueue(st->workqueue);
    }
    ring_buffer_free(st->events);
    kfree(st);
}

// must only be executed in a soft-irq context
int log_pkt_filter_event(struct iphdr *iph)
{
    if (unlikely(this_cpu_inc_return(packet_counter) >= 128)) {
        this_cpu_write(packet_counter, 0);

        // wake up workqueue to start flushing every 128 packets
        struct pkt_filter_flush_task *task = this_cpu_ptr(&pkt_filter_flush_tracker);
        queue_work(state->workqueue, &task->real_work);
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

    if (iph->protocol == IPPROTO_TCP) {
        struct tcphdr *th = (struct tcphdr *)((__u32 *)iph + iph->ihl);
        entry->source_port = th->source;
        entry->dest_port = th->dest;
    }

    if (iph->protocol == IPPROTO_UDP) {
        struct udphdr *uh = (struct udphdr *)((__u32 *)iph + iph->ihl);
        entry->source_port = uh->source;
        entry->dest_port = uh->dest;
    }

    ring_buffer_unlock_commit(state->events);
    return 0;
}

void flush_pkt_filter_events(struct work_struct *work)
{
}
