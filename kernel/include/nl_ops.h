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

    __LFW_NLA_MAX,
};

#define LFW_NLA_MAX (__LFW_NLA_MAX - 1)

enum lfw_nl_cmds {
    LFW_NL_CMD_UNSPEC = 0,

    LFW_NL_CMD_SET_BOGON,
    LFW_NL_CMD_SET_UNDER_ATTACK,
    LFW_NL_CMD_LOG,
};

enum lfw_genl_groups {
    LFW_GROUP_LOG,
    LFW_GROUP_FILTER,
    LFW_GROUP_HCF,
};

// taken from kernel levels
#define LOGLEVEL_EMERG 0   /* system is unusable */
#define LOGLEVEL_ALERT 1   /* action must be taken immediately */
#define LOGLEVEL_CRIT 2    /* critical conditions */
#define LOGLEVEL_ERR 3     /* error conditions */
#define LOGLEVEL_WARNING 4 /* warning conditions */
#define LOGLEVEL_NOTICE 5  /* normal but significant condition */
#define LOGLEVEL_INFO 6    /* informational */
#define LOGLEVEL_DEBUG 7   /* debug-level messages */
