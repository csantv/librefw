#pragma once

#define LFW_NL_FAMILY_NAME "librefw"
#define LFW_NL_MC_GRP_NAME "lfwmc"
#define LFW_NL_FAMILY_VER 1

enum lfw_nl_attrs {
    LFW_NLA_UNSPEC,

    LFW_NLA_NUM_IP_PREFIX,
    LFW_NLA_IP_PREFIX,
    LFW_NLA_N_IP_ADDR,
    LFW_NLA_N_IP_PREFIX_LEN,

    LFW_NLA_LOG_TS,
    LFW_NLA_LOG_LVL,
    LFW_NLA_LOG_MSG,

    LFW_NLA_END,
};

#define LFW_NL_A_MAX (LFW_NLA_END - 1)

enum lfw_nl_cmds {
    LFW_NLX_UNSPEC,
    LFW_NLX_ECHO,
    LFW_NLX_BOGON_SET,
    LFW_NLX_LOG,
    LFW_NLX_END,
};

#define LFW_NL_CMD_MAX (LFW_NLX_END - 1)

// taken from kernel levels
#define LOGLEVEL_EMERG		0	/* system is unusable */
#define LOGLEVEL_ALERT		1	/* action must be taken immediately */
#define LOGLEVEL_CRIT		2	/* critical conditions */
#define LOGLEVEL_ERR		3	/* error conditions */
#define LOGLEVEL_WARNING	4	/* warning conditions */
#define LOGLEVEL_NOTICE		5	/* normal but significant condition */
#define LOGLEVEL_INFO		6	/* informational */
#define LOGLEVEL_DEBUG		7	/* debug-level messages */
