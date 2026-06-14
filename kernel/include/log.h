// SPDX-License-Identifier: GPL-2.0-only
/*
 * librefw: a free as in freedom firewall
 * 
 * Copyright (C) 2026 Carlos Santos Toro Vera
 */

#pragma once

#include <linux/types.h>

struct sk_buff;
struct log_ctx;

int lfw_log(u8 level, const char *fmt, ...);

int lfw_build_log_msg(struct sk_buff *skb, void *data);

// taken from kernel levels
#define LOGLEVEL_EMERG 0   /* system is unusable */
#define LOGLEVEL_ALERT 1   /* action must be taken immediately */
#define LOGLEVEL_CRIT 2    /* critical conditions */
#define LOGLEVEL_ERR 3     /* error conditions */
#define LOGLEVEL_WARNING 4 /* warning conditions */
#define LOGLEVEL_NOTICE 5  /* normal but significant condition */
#define LOGLEVEL_INFO 6    /* informational */
#define LOGLEVEL_DEBUG 7   /* debug-level messages */
