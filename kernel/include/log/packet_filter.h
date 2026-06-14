// SPDX-License-Identifier: GPL-2.0-only
/*
 * librefw: a free as in freedom firewall
 * 
 * Copyright (C) 2026 Carlos Santos Toro Vera
 */

/*
 * Functions to log packet filtering events to user space
 * using ring buffers
 */

#pragma once

struct pkt_filter_log_state;
struct iphdr;
struct sk_buff;
struct work_struct;
struct timer_list;
struct buffer_data_page;

int init_pkt_filter_log_state(void);
void free_pkt_filter_log_state(void);

int log_pkt_filter_event(struct iphdr *iph, struct sk_buff *skb);
void flush_pkt_filter_events(struct work_struct *work);
void sched_flush_pkt_filter_events(struct timer_list *timer);
int process_rb_page(struct sk_buff *skb, void *data);
