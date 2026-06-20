// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
/*
 * librefw: a free as in freedom firewall
 * shared netlink interface enums and defines
 * 
 * Copyright (C) 2026 Carlos Santos Toro Vera
 */

#pragma once

#define LFW_NL_FAMILY_NAME "librefw"
#define LFW_NL_FAMILY_VER 1

enum lfw_nl_attrs {
    LFW_NLA_UNSPEC = 0,

    LFW_NLA_NUM_IP_PREFIX,
    LFW_NLA_IP_PREFIX,
    LFW_NLA_N_IP_ADDR,
    LFW_NLA_N_IP_PREFIX_LEN,

    LFW_NLA_UNDER_ATTACK,

    LFW_NLA_LOG_TS,
    LFW_NLA_LOG_LVL,
    LFW_NLA_LOG_MSG,

    LFW_NLA_HCF_IP,
    LFW_NLA_HCF_HC,
    LFW_NLA_HCF_TTL,

    LFW_NLA_HCF_HISTORY,
    LFW_NLA_HCF_HISTORY_IP,
    LFW_NLA_HCF_HISTORY_HC,
    LFW_NLA_HCF_HISTORY_TTL,

    LFW_NLA_PKT_FILTER_LOG,
    LFW_NLA_PKT_FILTER_LOG_TS,
    LFW_NLA_PKT_FILTER_LOG_SRC_IP,
    LFW_NLA_PKT_FILTER_LOG_DEST_IP,
    LFW_NLA_PKT_FILTER_LOG_SRC_PORT,
    LFW_NLA_PKT_FILTER_LOG_DEST_PORT,
    LFW_NLA_PKT_FILTER_LOG_PROTO,
    LFW_NLA_PKT_FILTER_LOG_TTL,
    LFW_NLA_PKT_FILTER_LOG_ACTION,

    __LFW_NLA_MAX,
};

#define LFW_NLA_MAX (__LFW_NLA_MAX - 1)

enum lfw_nl_cmds {
    LFW_NL_CMD_UNSPEC = 0,

    LFW_NL_CMD_SET_HCF_HISTORY,
    LFW_NL_CMD_SET_BOGON,
    LFW_NL_CMD_SET_UNDER_ATTACK,

    LFW_NL_CMD_LOG,
    LFW_NL_CMD_HCF,
    LFW_NL_CMD_PKT_FILTER_LOG,
};

enum lfw_nl_groups {
    LFW_NL_GROUP_LOG,
    LFW_NL_GROUP_HCF,
    LFW_NL_GROUP_PKT_FILTER_LOG,
};
