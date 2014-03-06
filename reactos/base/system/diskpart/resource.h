/*
 * PROJECT:         ReactOS DiskPart
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            base/system/diskpart/lang/resource.h
 * PURPOSE:         Manages all the partitions of the OS in
 *                  an interactive way
 * PROGRAMMERS:     Lee Schroeder
 */
#ifndef RESOURCE_H
#define RESOURCE_H

#define IDS_NONE                        -1
#define IDS_APP_HEADER                  0
#define IDS_APP_USAGE                   1
#define IDS_APP_LICENSE                 2
#define IDS_APP_CURR_COMPUTER           3
#define IDS_APP_LEAVING                 4
#define IDS_APP_PROMPT                  5

#define IDS_DETAIL_INFO_DISK_ID         7
#define IDS_DETAIL_INFO_TYPE            8
#define IDS_DETAIL_INFO_STATUS          9
#define IDS_DETAIL_INFO_PATH            10
#define IDS_DETAIL_INFO_TARGET          11
#define IDS_DETAIL_INFO_LUN_ID          12
#define IDS_DETAIL_INFO_LOC_PATH        13
#define IDS_DETAIL_INFO_CURR_RO_STATE   14
#define IDS_DETAIL_INFO_RO              15
#define IDS_DETAIL_INFO_BOOT_DSK        16
#define IDS_DETAIL_INFO_PAGE_FILE_DSK   17
#define IDS_DETAIL_INFO_HIBER_FILE_DSK  18
#define IDS_DETAIL_INFO_CRASH_DSK       19
#define IDS_DETAIL_INFO_CLST_DSK        20

#define IDS_LIST_DISK_HEAD              21
#define IDS_LIST_DISK_LINE              22
#define IDS_LIST_VOLUME_HEAD            23

#define IDS_STATUS_YES                  31
#define IDS_STATUS_NO                   32
#define IDS_STATUS_DISK_HEALTHY         33
#define IDS_STATUS_DISK_SICK            34
#define IDS_STATUS_UNAVAILABLE          35
#define IDS_STATUS_ONLINE               36
#define IDS_STATUS_OFFLINE              37
#define IDS_STATUS_NO_MEDIA             38

#define IDS_MSG_CURRENT_DSK_STATUS      39
#define IDS_MSG_NO_DISK                 40
#define IDS_MSG_ARG_SYNTAX_ERROR        41

#define IDS_HELP_CMD_DESC_ACTIVE            58
#define IDS_HELP_CMD_DESC_ADD               59
#define IDS_HELP_CMD_DESC_ASSIGN            60
#define IDS_HELP_CMD_DESC_ATTRIBUTES        61
#define IDS_HELP_CMD_DESC_ATTACH            62
#define IDS_HELP_CMD_DESC_AUTOMOUNT         63
#define IDS_HELP_CMD_DESC_BREAK             64
#define IDS_HELP_CMD_DESC_CLEAN             65
#define IDS_HELP_CMD_DESC_COMPACT           66
#define IDS_HELP_CMD_DESC_CONVERT           67
#define IDS_HELP_CMD_DESC_CREATE            68
#define IDS_HELP_CMD_DESC_DELETE            69
#define IDS_HELP_CMD_DESC_DETAIL            70
#define IDS_HELP_CMD_DESC_DETACH            71
#define IDS_HELP_CMD_DESC_EXIT              72
#define IDS_HELP_CMD_DESC_EXTEND            73
#define IDS_HELP_CMD_DESC_EXPAND            74
#define IDS_HELP_CMD_DESC_FS                75
#define IDS_HELP_CMD_DESC_FORMAT            76
#define IDS_HELP_CMD_DESC_GPT               77
#define IDS_HELP_CMD_DESC_HELP              78
#define IDS_HELP_CMD_DESC_IMPORT            79
#define IDS_HELP_CMD_DESC_INACTIVE          80
#define IDS_HELP_CMD_DESC_LIST              81
#define IDS_HELP_CMD_DESC_MERGE             82
#define IDS_HELP_CMD_DESC_ONLINE            83
#define IDS_HELP_CMD_DESC_OFFLINE           84
#define IDS_HELP_CMD_DESC_RECOVER           85
#define IDS_HELP_CMD_DESC_REM               88
#define IDS_HELP_CMD_DESC_REMOVE            89
#define IDS_HELP_CMD_DESC_REPAIR            90
#define IDS_HELP_CMD_DESC_RESCAN            91
#define IDS_HELP_CMD_DESC_RETAIN            92
#define IDS_HELP_CMD_DESC_SAN               93
#define IDS_HELP_CMD_DESC_SELECT            94
#define IDS_HELP_CMD_DESC_SETID             95
#define IDS_HELP_CMD_DESC_SHRINK            96
#define IDS_HELP_CMD_DESC_UNIQUEID          97

#define IDS_ERROR_MSG_NO_SCRIPT             104
#define IDS_ERROR_MSG_BAD_ARG               98

#define IDS_HELP_CMD_ACTIVE                 105
#define IDS_HELP_CMD_ADD                    106
#define IDS_HELP_CMD_ASSIGN                 107
#define IDS_HELP_CMD_ATTACH                 108
#define IDS_HELP_CMD_ATTRIBUTES             109
#define IDS_HELP_CMD_AUTOMOUNT              110
#define IDS_HELP_CMD_BREAK                  111
#define IDS_HELP_CMD_CLEAN                  112
#define IDS_HELP_CMD_COMPACT                113
#define IDS_HELP_CMD_CONVERT                114
#define IDS_HELP_CMD_CREATE                 115
#define IDS_HELP_CMD_DELETE                 116
#define IDS_HELP_CMD_DETACH                 117
#define IDS_HELP_CMD_DETAIL                 118
#define IDS_HELP_CMD_EXPAND                 119
#define IDS_HELP_CMD_EXTEND                 120
#define IDS_HELP_CMD_FILESYSTEMS            121
#define IDS_HELP_CMD_FORMAT                 122
#define IDS_HELP_CMD_GPT                    123
#define IDS_HELP_CMD_HELP                   124
#define IDS_HELP_CMD_IMPORT                 125
#define IDS_HELP_CMD_INACTIVE               126
#define IDS_HELP_CMD_LIST                   127
#define IDS_HELP_CMD_MERGE                  128
#define IDS_HELP_CMD_OFFLINE                129
#define IDS_HELP_CMD_ONLINE                 130
#define IDS_HELP_CMD_RECOVER                131
#define IDS_HELP_CMD_REMOVE                 132
#define IDS_HELP_CMD_REPAIR                 133
#define IDS_HELP_CMD_RESCAN                 134
#define IDS_HELP_CMD_RETAIN                 135
#define IDS_HELP_CMD_SAN                    136
#define IDS_HELP_CMD_SELECT                 137
#define IDS_HELP_CMD_SETID                  138
#define IDS_HELP_CMD_SHRINK                 139
#define IDS_HELP_CMD_UNIQUEID               140

#endif // RESOURCE_H
