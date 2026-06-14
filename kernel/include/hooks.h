// SPDX-License-Identifier: GPL-2.0-only
/*
 * librefw: a free as in freedom firewall
 * 
 * Copyright (C) 2026 Carlos Santos Toro Vera
 */

#pragma once

#include <linux/netfilter.h>

int lfw_register_hooks(void);
void lfw_unregister_hooks(void);

unsigned int lfw_filter_ipv4_hook_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);
unsigned int lfw_hc_learn_ipv4_hook_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);
