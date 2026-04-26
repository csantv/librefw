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

    __LFW_NLA_MAX,
};

#define LFW_NLA_MAX (__LFW_NLA_MAX - 1)

enum lfw_nl_cmds {
    LFW_NL_CMD_UNSPEC = 0,

    LFW_NL_CMD_SET_BOGON,
    LFW_NL_CMD_SET_UNDER_ATTACK,

    LFW_NL_CMD_LOG,
    LFW_NL_CMD_HCF,
};

enum lfw_nl_groups {
    LFW_NL_GROUP_LOG,
    LFW_NL_GROUP_HCF,
};

