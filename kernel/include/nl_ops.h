#ifndef LFW_NL_OPS_H
#define LFW_NL_OPS_H

#define LFW_NL_FAMILY_NAME "librefw"
#define LFW_NL_MC_GRP_NAME "lfwmc"
#define LFW_NL_FAMILY_VER 1

enum lfw_nl_attrs {
    LFW_NL_A_UNSPEC,
    LFW_NL_A_MSG,
    LFW_NL_A_END,
};

#define LFW_NL_A_MAX (LFW_NL_A_END - 1)

enum lfw_nl_cmds {
    LFW_NL_CMD_UNSPEC,
    LFW_NL_CMD_ECHO,
    LFW_NL_CMD_END,
};

#define LFW_NL_CMD_MAX (LFW_NL_CMD_END - 1)

#endif //LFW_NL_OPS_H