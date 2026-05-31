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

/*
 * Functions to log packet filtering events to user space
 * using ring buffers
 */

#pragma once

struct pkt_filter_log_state;
struct iphdr;
struct work_struct;
struct timer_list;

int init_pkt_filter_log_state(void);
void free_pkt_filter_log_state(void);

int log_pkt_filter_event(struct iphdr *iph);
void flush_pkt_filter_events(struct work_struct *work);
void sched_flush_pkt_filter_events(struct timer_list *timer);
