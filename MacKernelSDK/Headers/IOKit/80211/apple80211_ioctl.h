/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef _APPLE80211_IOCTL_H_
#define _APPLE80211_IOCTL_H_

#include <Availability.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <sys/param.h>
#include <sys/ioctl.h>

// This is necessary, because even the latest Xcode does not support properly targeting 11.0.
#ifndef __IO80211_TARGET
#error "Please define __IO80211_TARGET to the requested version"
#endif

#include <IOKit/80211/apple80211_var.h>

struct apple80211req
{
    char        req_if_name[IFNAMSIZ];    // 16 bytes
    int         req_type;                 // 4 bytes
    int         req_val;                  // 4 bytes
    u_int32_t   req_len;                  // 4 bytes
    void        *req_data;                // 4 bytes
};

#define SIOCSA80211 2150656456
#define SIOCGA80211 3224398281

// req_type

#define APPLE80211_IOC_SSID                     1    // req_type

#define APPLE80211_IOC_AUTH_TYPE                2    // req_type
#define     APPLE80211_AUTH_TYPE_UNICAST        1    // req_val, SIOCGA80211 only
#define     APPLE80211_AUTH_TYPE_MULTICAST      2    // req_val, SIOCGA80211 only

#define APPLE80211_IOC_CIPHER_KEY                3   // req_type
#define     APPLE80211_CIPHER_KEY_UNICAST        1   // req_val
#define     APPLE80211_CIPHER_KEY_MULICAST       2   // req_val

#define APPLE80211_IOC_CHANNEL                   4   // req_type

#define APPLE80211_IOC_POWERSAVE                 5   // req_type

#define APPLE80211_IOC_PROTMODE                  6   // req_type

#define APPLE80211_IOC_TXPOWER                   7   // req_type
#define APPLE80211_IOC_RATE                      8   // req_type
#define APPLE80211_IOC_BSSID                     9   // req_type

#define APPLE80211_IOC_SCAN_REQ                 10   // req_type

#define APPLE80211_IOC_SCAN_RESULT              11   // req_type

#define APPLE80211_IOC_CARD_CAPABILITIES        12   // req_type

#define APPLE80211_IOC_STATE                    13   // req_type (apple80211_state)
#define APPLE80211_IOC_PHY_MODE                 14   // req_type (apple80211_phymode)

#define APPLE80211_IOC_OP_MODE                  15   // req_type (apple80211_opmode)
#define APPLE80211_IOC_RSSI                     16   // req_type
#define APPLE80211_IOC_NOISE                    17   // req_type

#define APPLE80211_IOC_INT_MIT                  18
#define APPLE80211_IOC_INT_MIT_OFF               1   // req_val
#define APPLE80211_IOC_INT_MIT_ON                2   // req_val

// card power
#define APPLE80211_IOC_POWER                    19   // req_type

#define APPLE80211_IOC_ASSOCIATE                20   // req_type
#define APPLE80211_IOC_ASSOCIATE_RESULT         21   // req_type
#define APPLE80211_IOC_DISASSOCIATE             22   // req_type
#define APPLE80211_IOC_STATUS_DEV_NAME          23   // req_type

#define APPLE80211_IOC_IBSS_MODE                24   // req_type
#define APPLE80211_IOC_IBSS_MODE_START           1   // req_val
#define APPLE80211_IOC_IBSS_MODE_STOP            2   // req_val

#define APPLE80211_IOC_HOST_AP_MODE             25   // req_type
#define APPLE80211_IOC_HOST_AP_MODE_START        1   // req_val
#define APPLE80211_IOC_HOST_AP_MODE_STOP         2   // req_val

#define APPLE80211_IOC_AP_MODE                   26  // req_type (apple80211_apmode)
#define APPLE80211_IOC_SUPPORTED_CHANNELS        27  // req_type
#define APPLE80211_IOC_LOCALE                    28  // req_type
#define APPLE80211_IOC_DEAUTH                    29  // req_type
#define APPLE80211_IOC_COUNTERMEASURES           30  // req_type
#define APPLE80211_IOC_FRAG_THRESHOLD            31  // req_type
#define APPLE80211_IOC_RATE_SET                  32  // req_type
#define APPLE80211_IOC_SHORT_SLOT                33  // req_type
#define APPLE80211_IOC_MULTICAST_RATE            34  // req_type
#define APPLE80211_IOC_SHORT_RETRY_LIMIT         35  // req_type
#define APPLE80211_IOC_LONG_RETRY_LIMIT          36  // req_type
#define APPLE80211_IOC_TX_ANTENNA                37  // req_type
#define APPLE80211_IOC_RX_ANTENNA                38  // req_type
#define APPLE80211_IOC_ANTENNA_DIVERSITY         39  // req_type
#define APPLE80211_IOC_ROM                       40  // req_type
#define APPLE80211_IOC_DTIM_INT                  41  // req_type
#define APPLE80211_IOC_STATION_LIST              42  // req_type
#define APPLE80211_IOC_DRIVER_VERSION            43  // req_type
#define APPLE80211_IOC_HARDWARE_VERSION          44  // req_type
#define APPLE80211_IOC_RAND                      45  // req_type
#define APPLE80211_IOC_RSN_IE                    46  // req_type
#define APPLE80211_IOC_BACKGROUND_SCAN           47  // req_type
#define APPLE80211_IOC_AP_IE_LIST                48  // req_type
#define APPLE80211_IOC_STATS                     49  // req_type
#define APPLE80211_IOC_ASSOCIATION_STATUS        50  // req_type
#define APPLE80211_IOC_COUNTRY_CODE              51  // req_type
#define APPLE80211_IOC_DEBUG_FLAGS               52  // req_type
#define APPLE80211_IOC_LAST_RX_PKT_DATA          53  // req_type
#define APPLE80211_IOC_RADIO_INFO                54  // req_type
#define APPLE80211_IOC_GUARD_INTERVAL            55  // req_type
#define APPLE80211_IOC_MIMO_POWERSAVE            56  // req_type
#define APPLE80211_IOC_MCS                       57  // req_type
#define APPLE80211_IOC_RIFS                      58  // req_type
#define APPLE80211_IOC_LDPC                      59  // req_type
#define APPLE80211_IOC_MSDU                      60  // req_type
#define APPLE80211_IOC_MPDU                      61  // req_type
#define APPLE80211_IOC_BLOCK_ACK                 62  // req_type
#define APPLE80211_IOC_PLS                       63  // req_type
#define APPLE80211_IOC_PSMP                      64  // req_type
#define APPLE80211_IOC_PHY_SUB_MODE              65  // req_type
#define APPLE80211_IOC_MCS_INDEX_SET             66  // req_type
#define APPLE80211_IOC_CACHE_THRESH_BCAST        67  // req_type
#define APPLE80211_IOC_CACHE_THRESH_DIRECT       68  // req_type
#define APPLE80211_IOC_WOW_PARAMETERS            69  // req_type
#define APPLE80211_IOC_WOW_ENABLED               70  // req_type
#define APPLE80211_IOC_40MHZ_INTOLERANT          71  // req_type
#define APPLE80211_IOC_PID_LOCK                  72
#define APPLE80211_IOC_STA_IE_LIST               73
#define APPLE80211_IOC_STA_AUTHORIZE             74
#define APPLE80211_IOC_STA_DISASSOCIATE          75
#define APPLE80211_IOC_STA_DEAUTH                76
#define APPLE80211_IOC_RSN_CONF                  77
#define APPLE80211_IOC_KEY_RSC                   78
#define APPLE80211_IOC_STA_STATS                 79
#define APPLE80211_IOC_ROAM_THRESH               80
#define APPLE80211_IOC_VENDOR_DBG_FLAGS          81
#define APPLE80211_IOC_CACHE_AGE_THRESH          82
#define APPLE80211_IOC_PMK_CACHE                 83
#define APPLE80211_IOC_LINK_QUAL_EVENT_PARAMS    84
#define APPLE80211_IOC_IE                        85
#define APPLE80211_IOC_SCAN_REQ_MULTIPLE         86
#define APPLE80211_IOC_BTCOEX_MODE               87
#define APPLE80211_IOC_WOW_TEST                  88
#define APPLE80211_IOC_CLEAR_PMK_CACHE           89
#define APPLE80211_IOC_SCANCACHE_CLEAR           90
#define APPLE80211_IOC_P2P_ENABLE                91
#define APPLE80211_IOC_P2P_LISTEN                92
#define APPLE80211_IOC_P2P_SCAN                  93
#define APPLE80211_IOC_VIRTUAL_IF_CREATE         94
#define APPLE80211_IOC_VIRTUAL_IF_DELETE         95
#define APPLE80211_IOC_VIRTUAL_IF_ROLE           96
#define APPLE80211_IOC_VIRTUAL_IF_PARENT         97
#define APPLE80211_IOC_P2P_GO_CONF               98
#define APPLE80211_IOC_P2P_NOA_LIST              99
#define APPLE80211_IOC_P2P_OPP_PS                100
#define APPLE80211_IOC_P2P_CT_WINDOW             101
#define APPLE80211_IOC_BT_COEX_FLAGS             102
#define APPLE80211_IOC_CURRENT_NETWORK           103
#define APPLE80211_IOC_BT_POWER                  104
#define APPLE80211_IOC_AVAILABILITY              105
#define APPLE80211_IOC_RSSI_BOUNDS               106
#define APPLE80211_IOC_ROAM                      107
#define APPLE80211_IOC_TX_CHAIN_POWER            108
#define APPLE80211_IOC_CDD_MODE                  109
#define APPLE80211_IOC_LAST_BCAST_SCAN_TIME      110
#define APPLE80211_IOC_THERMAL_THROTTLING        111
#define APPLE80211_IOC_FACTORY_MODE              112
#define APPLE80211_IOC_REASSOCIATE               113

#define APPLE80211_IOC_POWER_DEBUG_INFO 115
#define APPLE80211_IOC_AWDL_SYNC_PARAMS 116
#define APPLE80211_IOC_AWDL_SYNC_ENABLED 117
#define APPLE80211_IOC_AWDL_EXTENSION_STATE_MACHINE_PARAMETERS 118
#define APPLE80211_IOC_AWDL_SERVICE_PARAMS 119
#define APPLE80211_IOC_AWDL_PEER_SERVICE_REQUEST 120
#define APPLE80211_IOC_AWDL_ELECTION_ALGORITHM_ENABLED 121
#define APPLE80211_IOC_AWDL_ELECTION_ID 122
#define APPLE80211_IOC_AWDL_MAX_TREE_DEPTH 123
#define APPLE80211_IOC_AWDL_GUARD_TIME 124
#define APPLE80211_IOC_AWDL_BSSID 125
#define APPLE80211_IOC_AWDL_ELECTION_METRIC 126
#define APPLE80211_IOC_AWDL_AVAILABILITY_WINDOW_AP_ALIGNMENT 127
#define APPLE80211_IOC_AWDL_SYNC_FRAME_AP_BEACON_ALIGNMENT 128
#define APPLE80211_IOC_AWDL_SYNCHRONIZATION_CHANNEL_SEQUENCE 129
#define APPLE80211_IOC_PEER_CACHE_MAXIMUM_SIZE 130
#define APPLE80211_IOC_AWDL_OUI 131
#define APPLE80211_IOC_AWDL_MASTER_CHANNEL 132
#define APPLE80211_IOC_AWDL_TOP_MASTER 133
#define APPLE80211_IOC_AWDL_SYNC_STATE 134
#define APPLE80211_IOC_AWDL_ELECTION_RSSI_THRESHOLDS 135
#define APPLE80211_IOC_AWDL_PRESENCE_MODE 136
#define APPLE80211_IOC_AWDL_ELECTION_MASTER_COUNTS 137
#define APPLE80211_IOC_AWDL_PERIODIC_SYNC_FRAME_PACKET_LIFETIME 138
#define APPLE80211_IOC_AWDL_MASTER_MODE_SYNC_FRAME_PERIOD 139
#define APPLE80211_IOC_AWDL_NON_ELECTION_MASTER_MODE_SYNC_FRAME_PERIOD 140
#define APPLE80211_IOC_AWDL_EXPLICIT_AVAILABILITY_WINDOW_EXTENSION_OPT_OUT 141
#define APPLE80211_IOC_AWDL_GET_AWDL_MASTER_DATABASE 142
#define APPLE80211_IOC_PEER_CACHE_CONTROL 143
#define APPLE80211_IOC_AWDL_BATTERY_LEVEL 144
#define APPLE80211_IOC_AWDL_BT_COEX_AW_PROTECTED_PERIOD_LENGTH 145
#define APPLE80211_IOC_AWDL_BT_COEX_AGREEMENT 146
#define APPLE80211_IOC_AWDL_BT_COEX_AGREEMENT_ENABLED 147
#define APPLE80211_IOC_AWDL_STRATEGY 148
#define APPLE80211_IOC_AWDL_OOB_REQUEST 149
#define APPLE80211_IOC_AWDL_MAX_NO_MASTER_PERIODS 150
#define APPLE80211_IOC_AWDL_SYNC_FRAME_TEMPLATE 151
#define APPLE80211_IOC_LOG_FLAGS 152
#define APPLE80211_IOC_PEER_STATS 153
#define APPLE80211_IOC_HT_CAPABILITY 154
#define APPLE80211_IOC_AWDL_ELECTION_PARAMS 155
#define APPLE80211_IOC_LINK_CHANGED_EVENT_DATA 156
#define APPLE80211_IOC_GET_DEBUG_INFO 157
#define APPLE80211_IOC_AWDL_DEVICE_CAPABILITIES 158
#define APPLE80211_IOC_AWDL_RSSI_MEASUREMENT_REQUEST 159
#define APPLE80211_IOC_AWDL_AES_KEY 160
#define APPLE80211_IOC_AWDL_SCAN_RESERVED_TIME 161
#define APPLE80211_IOC_AWDL_CTL 162
#define APPLE80211_IOC_AWDL_SOCIAL_TIME_SLOTS 163
#define APPLE80211_IOC_AWDL_PEER_TRAFFIC_REGISTRATION 164
#define APPLE80211_IOC_EXTENDED_STATS 165
#define APPLE80211_IOC_BEACON_PERIOD 166
#define APPLE80211_IOC_AWDL_FORCED_ROAM_CONFIG 167
#define APPLE80211_IOC_AWDL_QUIET 168
#define APPLE80211_IOC_ACL_POLICY 169
#define APPLE80211_IOC_ACL_ADD 170
#define APPLE80211_IOC_ACL_REMOVE 171
#define APPLE80211_IOC_ACL_FLUSH 172
#define APPLE80211_IOC_ACL_LIST 173
#define APPLE80211_IOC_CHAIN_ACK 174
#define APPLE80211_IOC_DESENSE 175
#define APPLE80211_IOC_OFFLOAD_SCANNING 176
#define APPLE80211_IOC_OFFLOAD_RSN 177
#define APPLE80211_IOC_OFFLOAD_COUNTRY_CODE 178
#define APPLE80211_IOC_OFFLOAD_KEEPALIVE_L2 179
#define APPLE80211_IOC_OFFLOAD_ARP_NDP 180
#define APPLE80211_IOC_VHT_MCS_INDEX_SET 181
#define APPLE80211_IOC_DWDS 182
#define APPLE80211_IOC_INTERRUPT_STATS 183
#define APPLE80211_IOC_INTERRUPT_STATS_RESET 184
#define APPLE80211_IOC_TIMER_STATS 185
#define APPLE80211_IOC_TIMER_STATS_RESET 186
#define APPLE80211_IOC_OFFLOAD_STATS 187
#define APPLE80211_IOC_OFFLOAD_STATS_RESET 188
#define APPLE80211_IOC_OFFLOAD_BEACONS 189
#define APPLE80211_IOC_ROAMING 190
#define APPLE80211_IOC_OFFLOAD_ARP 191
#define APPLE80211_IOC_OFFLOAD_NDP 192
#define APPLE80211_IOC_OFFLOAD_SCAN 193
#define APPLE80211_IOC_DESENSE_LEVEL 194
#define APPLE80211_IOC_MCS_VHT 195
#define APPLE80211_IOC_TX_NSS 196
#define APPLE80211_IOC_GAS_REQ 197
#define APPLE80211_IOC_GAS_START 198
#define APPLE80211_IOC_GAS_SET_PEER 199
#define APPLE80211_IOC_GAS_RESULTS 200
#define APPLE80211_IOC_AWDL_BTLE_PEER_INDICATION 201
#define APPLE80211_IOC_AWDL_BTLE_STATE_PARAMS 202
#define APPLE80211_IOC_AWDL_PEER_DATABASE 203
#define APPLE80211_IOC_AWDL_BTLE_ENABLE_SYNC_WITH_PARAMS 204
#define APPLE80211_IOC_AWDL_SECONDARY_MASTER_CHANNEL 205
#define APPLE80211_IOC_PHY_STATS 206
#define APPLE80211_IOC_CHANNELS_INFO 207
#define APPLE80211_IOC_AWDL_AF_TX_MODE 208
#define APPLE80211_IOC_ERROR_STRING 209
#define APPLE80211_IOC_ERROR_NO 210
#define APPLE80211_IOC_AWDL_PIGGYBACK_SCAN_REQ 211
#define APPLE80211_IOC_AWDL_PRIVATE_ELECTION_ID 212
#define APPLE80211_IOC_AWDL_MIN_RATE 213
#define APPLE80211_IOC_VHT_CAPABILITY 214
#define APPLE80211_IOC_BGSCAN_CACHE_RESULTS 215
#define APPLE80211_IOC_ROAM_PROFILE 216
#define APPLE80211_IOC_AWDL_OPER_MODE 217
#define APPLE80211_IOC_RESTORE_DEFAULTS 218
#define APPLE80211_IOC_AWDL_ENCRYPTION_KEYS 219
#define APPLE80211_IOC_AWDL_ENCRYPTION_TYPE 220
#define APPLE80211_IOC_BTCOEX_PROFILES 221
#define APPLE80211_IOC_BTCOEX_CONFIG 222
#define APPLE80211_IOC_AWDL_STATISTICS 223
#define APPLE80211_IOC_AWDL_ENABLE_ROAMING 224
#define APPLE80211_IOC_AWDL_OOB_AUTO_REQUEST 225
#define APPLE80211_IOC_AWDL_TXCAL_PERIOD 226
#define APPLE80211_IOC_CHIP_COUNTER_STATS 227
#define APPLE80211_IOC_DBG_GUARD_TIME_PARAMS 228
#define APPLE80211_IOC_AWDL_AWDL_ADVERTISERS 229
#define APPLE80211_IOC_LEAKY_AP_STATS_MODE 230
#define APPLE80211_IOC_CAPTURE 231
#define APPLE80211_IOC_LEAKY_AP_STATS 232
#define APPLE80211_IOC_AWDL_BLOCK_SET_COMMANDS 233
#define APPLE80211_IOC_LEAKY_AP_AWD_MODE 234
#define APPLE80211_IOC_BTCOEX_OPTIONS 235
#define APPLE80211_IOC_FORCE_SYNC_TO_PEER 236
#define APPLE80211_IOC_COUNTRY_CHANNELS 237
#define APPLE80211_IOC_PRIVATE_MAC 238
#define APPLE80211_IOC_RESET_CHIP 239
#define APPLE80211_IOC_CRASH 240
#define APPLE80211_IOC_RANGING_ENABLE 241
#define APPLE80211_IOC_RANGING_START 242
#define APPLE80211_IOC_RANGING_AUTHENTICATE 243
#define APPLE80211_IOC_AWDL_PREFERRED_CHANNELS 244
#define APPLE80211_IOC_LEAKY_AP_SSID_STATS 245
#define APPLE80211_IOC_AWDL_RSDB_CAPS 246
#define APPLE80211_IOC_AWDL_DEV_STATS 247
#define APPLE80211_IOC_LAST_ASSOC_HISTORY 248
#define APPLE80211_IOC_AWDL_COMMON_CHANNEL 249
#define APPLE80211_IOC_AWDL_PEERS_INFO 250
#define APPLE80211_IOC_TKO_PARAMS 251
#define APPLE80211_IOC_TKO_DUMP 252
#define APPLE80211_IOC_AWDL_NEARBY_LOG_TRIGGER 253
#define APPLE80211_IOC_HW_SUPPORTED_CHANNELS 254
#define APPLE80211_IOC_BTCOEX_PROFILE 255
#define APPLE80211_IOC_BTCOEX_PROFILE_ACTIVE 256
#define APPLE80211_IOC_TRAP_INFO 257
#define APPLE80211_IOC_THERMAL_INDEX 258
#define APPLE80211_IOC_MAX_NSS_FOR_AP 259
#define APPLE80211_IOC_BTCOEX_2G_CHAIN_DISABLE 260
#define APPLE80211_IOC_POWER_BUDGET 261
#define APPLE80211_IOC_AWDL_DFSP_CONFIG 262
#define APPLE80211_IOC_AWDL_DFSP_UCSA_CONFIG 263
#define APPLE80211_IOC_SCAN_BACKOFF_REPORT 264
#define APPLE80211_IOC_OFFLOAD_TCPKA_ENABLE 265
#define APPLE80211_IOC_RANGING_CAPS 266
#define APPLE80211_IOC_PER_CORE_RSSI_REPORT 267
#define APPLE80211_IOC_NSS 353

#define APPLE80211_IOC_CARD_SPECIFIC            0xffffffff    // req_type

// Kernel interface

// Bump this value when structures change
#define APPLE80211_VERSION    1

struct apple80211_ssid_data
{
    u_int32_t    version;
    u_int32_t    ssid_len;
    u_int8_t     ssid_bytes[APPLE80211_MAX_SSID_LEN];
};

struct apple80211_virt_if_create_data {
    uint32_t    version;
    uint8_t     mac[APPLE80211_ADDR_LEN];
    uint16_t    unk1;
    uint32_t    role;
    uint8_t     bsd_name[15];
} __attribute__((packed));

struct apple80211_virt_if_delete_data {
    uint32_t    version;
    uint8_t     bsd_name[15];
} __attribute__((packed));

struct apple80211_ht_capability {
    uint32_t    version;
    uint8_t     unk1;
    uint8_t     unk2;
    uint16_t    unk3;
    uint8_t     unk4;
    uint8_t     unk5[16];
    uint16_t    unk6;
    uint        unk7;
    uint8_t     unk8;
} __attribute__((packed));

struct apple80211_channel_data
{
    u_int32_t                    version;
    struct apple80211_channel    channel;
};

struct apple80211_bssid_data
{
    u_int32_t            version;
    struct ether_addr    bssid;
};

struct apple80211_capability_data
{
    u_int32_t    version;
    u_int8_t     capabilities[11];
};

struct apple80211_state_data
{
    u_int32_t    version;
    u_int32_t    state;
};

struct apple80211_rssi_data
{
    u_int32_t    version;
    u_int32_t    num_radios;
    u_int32_t    rssi_unit;
    int32_t      rssi[APPLE80211_MAX_RADIO];        // control channel
    int32_t      aggregate_rssi;                    // aggregate control channel rssi
    int32_t      rssi_ext[APPLE80211_MAX_RADIO];    // extension channel rssi
    int32_t      aggregate_rssi_ext;                // aggregate extension channel rssi
};

struct apple80211_power_data
{
    u_int32_t    version;
    u_int32_t    num_radios;
    u_int32_t    power_state[APPLE80211_MAX_RADIO];
};

struct apple80211_assoc_result_data
{
    u_int32_t    version;
    u_int32_t    result;
};

struct apple80211_assoc_status_data
{
    u_int32_t    version;
    u_int32_t    status;
};

struct apple80211_rate_data
{
    u_int32_t    version;
    u_int32_t    num_radios;
    u_int32_t    rate[APPLE80211_MAX_RADIO];
};

struct apple80211_status_dev_data
{
    u_int32_t    version;
    u_int8_t     dev_name[MAXPATHLEN];
};

struct apple80211_powersave_data
{
    u_int32_t    version;
    u_int32_t    powersave_level;
};

struct apple80211_protmode_data
{
    u_int32_t    version;
    u_int32_t    protmode;
    u_int32_t    threshold;        // bytes
};

struct apple80211_txpower_data
{
    u_int32_t    version;
    u_int32_t    txpower_unit;
    int32_t      txpower;
};

struct apple80211_phymode_data
{
    u_int32_t    version;
    u_int32_t    phy_mode;            // vector of supported phy modes
    u_int32_t    active_phy_mode;     // current active phy mode
};

struct apple80211_opmode_data
{
    u_int32_t    version;
    u_int32_t    op_mode;
};

struct apple80211_noise_data
{
    u_int32_t    version;
    u_int32_t    num_radios;
    u_int32_t    noise_unit;
    int32_t      noise[APPLE80211_MAX_RADIO];        // control channel
    int32_t      aggregate_noise;                    // aggregate control channel noise
    int32_t      noise_ext[APPLE80211_MAX_RADIO];    // extension channel noise
    int32_t      aggregate_noise_ext;                // aggregate extension channel noise
};

struct apple80211_intmit_data
{
    u_int32_t    version;
    u_int32_t    int_mit;
};

struct apple80211_authtype_data
{
    u_int32_t    version;
    u_int32_t    authtype_lower;    //    apple80211_authtype_lower
    u_int32_t    authtype_upper;    //    apple80211_authtype_upper
};

struct apple80211_sup_channel_data
{
    u_int32_t                    version;
    u_int32_t                    num_channels;
    struct apple80211_channel    supported_channels[APPLE80211_MAX_CHANNELS];
};


struct apple80211_roam_threshold_data
{
    u_int32_t threshold;
    u_int32_t count;
};

struct apple80211_locale_data
{
    u_int32_t    version;
    u_int32_t    locale;
};

struct apple80211_scan_data
{
    u_int32_t                    version;
    u_int32_t                    bss_type;                            // apple80211_apmode
    struct ether_addr            bssid;                               // target BSSID
    u_int32_t                    ssid_len;                            // length of the SSID
    u_int8_t                     ssid[APPLE80211_MAX_SSID_LEN];       // direct scan ssid or AirDrop scan ssid like "Air-xxxx"
    u_int32_t                    scan_type;                           // apple80211_scan_type
    u_int32_t                    phy_mode;                            // apple80211_phymode vector
    u_int16_t                    dwell_time;                          // time to spend on each channel (ms)
    u_int32_t                    rest_time;                           // time between scanning each channel (ms)
    u_int32_t                    num_channels;                        // 0 if not passing in channels
    struct apple80211_channel    channels[APPLE80211_MAX_CHANNELS];   // channel list
};

struct apple80211_scan_multiple_data
{
    uint32_t                  version;
    uint32_t                  ap_mode; // apple80211_apmode
    uint32_t                  ssid_count;
    apple80211_ssid_data      ssids[16];
    uint32_t                  bssid_count;
    ether_addr                bssids[16];
    uint32_t                  scan_type;
    uint32_t                  phy_mode;
    uint32_t                  dwell_time;
    uint32_t                  rest_time;
    uint32_t                  num_channels;
    struct apple80211_channel channels[128];
    uint16_t                  unk_2;
};

struct apple80211_link_changed_event_data
{
    bool       isLinkDown; // 0
    uint32_t   rssi;       // 4
    uint16_t   snr;        // 8
    uint16_t   nf;         // 10
    char       cca;        // 12
    bool       voluntary;  // 16
    uint32_t   reason;     // 20
};

struct apple80211_apmode_data
{
    u_int32_t    version;
    u_int32_t    apmode;
};

struct apple80211_assoc_data
{
    u_int32_t                version;
    u_int16_t                ad_mode;          // apple80211_apmode
    u_int16_t                ad_auth_lower;    // apple80211_authtype_lower
    u_int16_t                ad_auth_upper;    // apple80211_authtype_upper
    u_int32_t                ad_ssid_len;
    u_int8_t                 ad_ssid[ APPLE80211_MAX_SSID_LEN ];
    struct ether_addr        ad_bssid;         // prefer over ssid if not zeroed
    struct apple80211_key    ad_key;
    uint16_t                 pad;
    u_int8_t                 ad_rsn_ie[ APPLE80211_MAX_RSN_IE_LEN ];
    u_int32_t                ad_flags;         // apple80211_assoc_flags
};

static_assert(offsetof(apple80211_assoc_data, ad_rsn_ie) == 206, "offsetof(apple80211_assoc_data, ad_rsn_ie)");

struct apple80211_deauth_data
{
    u_int32_t            version;
    u_int32_t            deauth_reason;    // reason code
    struct ether_addr    deauth_ea;        // BSSID of AP
};

struct apple80211_countermeasures_data
{
    u_int32_t    version;
    u_int32_t    enabled;
};

struct apple80211_frag_threshold_data
{
    u_int32_t    version;
    u_int32_t    threshold;    // bytes
};

struct apple80211_rate_set_data
{
    u_int32_t                version;
    u_int16_t                num_rates;
    struct apple80211_rate   rates[APPLE80211_MAX_RATES];
};

struct apple80211_short_slot_data
{
    u_int32_t    version;
    u_int8_t     mode;
};

struct apple80211_retry_limit_data
{
    u_int32_t    version;
    u_int32_t    limit;
};

struct apple80211_antenna_data
{
    u_int32_t    version;
    u_int32_t    num_radios;
    int32_t      antenna_index[APPLE80211_MAX_RADIO];
};

struct apple80211_dtim_int_data
{
    u_int32_t    version;
    u_int32_t    interval;
};

struct apple80211_sta_data
{
    u_int32_t                    version;
    u_int32_t                    num_stations;
    struct apple80211_station    station_list[APPLE80211_MAX_STATIONS];
};

struct apple80211_version_data
{
    u_int32_t    version;
    u_int16_t    string_len;
    char         string[APPLE80211_MAX_VERSION_LEN];
};

struct apple80211_rom_data
{
    u_int32_t    version;
    u_int32_t    rom_len;
    u_int8_t     rom[1];    // variable length
};

struct apple80211_rand_data
{
    u_int32_t    version;
    u_int32_t    rand;
};

struct apple80211_rsn_ie_data
{
    u_int32_t    version;
    u_int16_t    len;
    u_int8_t     ie[ APPLE80211_MAX_RSN_IE_LEN ];
};

struct apple80211_ap_ie_data
{
    u_int32_t    version;
    u_int32_t    len;
    u_int8_t     *ie_data;
};

struct apple80211_stats_data
{
    u_int32_t    version;
    u_int32_t    tx_frame_count;
    u_int32_t    tx_errors;
    u_int32_t    rx_frame_count;
    u_int32_t    rx_errors;
};

struct apple80211_country_code_data
{
    u_int32_t    version;
    u_int8_t     cc[APPLE80211_MAX_CC_LEN];
};

struct apple80211_last_rx_pkt_data
{
    u_int32_t         version;
    u_int32_t         rate;
    int32_t           rssi;
    u_int32_t         num_streams;    // number of spatial streams
    struct ether_addr sa;             // source address
};

struct apple80211_radio_info_data
{
    u_int32_t    version;
    u_int32_t    count;        // number of rf chains
};

struct apple80211_guard_interval_data
{
    u_int32_t    version;
    u_int32_t    interval;    // apple80211_guard_interval
};

struct apple80211_mcs_data
{
    u_int32_t    version;
    u_int32_t    index;        // 0 to APPLE80211_MAX_MCS_INDEX
};

struct apple80211_rifs_data
{
    u_int32_t    version;
    u_int32_t    enabled;
};

struct apple80211_ldpc_data
{
    u_int32_t    version;
    u_int32_t    enabled;
};

struct apple80211_msdu_data
{
    u_int32_t    version;
    u_int32_t    max_length;        // 3839 or 7935 bytes
};

struct apple80211_mpdu_data
{
    u_int32_t    version;
    u_int32_t    max_factor;        // 0 - APPLE80211_MAX_MPDU_FACTOR
    u_int32_t    max_density;       // 0 - APPLE80211_MAX_MPDU_DENSITY
};

struct apple80211_block_ack_data
{
    u_int32_t   version;
    u_int8_t    ba_enabled;             // block ack enabled
    u_int8_t    immediate_ba_enabled;   // immediate block ack enabled
    u_int8_t    cbba_enabled;           // compressed bitmap block ack enabled
    u_int8_t    implicit_ba_enabled;    // implicit block ack enabled
};

struct apple80211_pls_data
{
    u_int32_t    version;
    u_int32_t    enabled;    // phy level spoofing enabled
};

struct apple80211_psmp_data
{
    u_int32_t    version;
    u_int32_t    enabled;
};

struct apple80211_physubmode_data
{
    u_int32_t    version;
    u_int32_t    phy_mode;       // one apple80211_phymode
    u_int32_t    phy_submode;    // one apple80211_physubmode
    u_int32_t    flags;          // apple80211_channel_flag vector
};

struct apple80211_mcs_index_set_data
{
    u_int32_t   version;
    u_int8_t    mcs_set_map[APPLE80211_MAP_SIZE( APPLE80211_MAX_MCS_INDEX + 1 )];
};

struct apple80211_wow_parameter_data
{
    u_int32_t                     version;
    u_int8_t                      wake_cond_map[APPLE80211_MAP_SIZE( APPLE80211_MAX_WAKE_COND + 1 )];
    u_int32_t                     beacon_loss_time;
    u_int32_t                     pattern_count;
    struct apple80211_wow_pattern patterns[APPLE80211_MAX_WOW_PATTERNS];
};

struct apple80211_40mhz_intolerant_data
{
    u_int32_t    version;
    u_int32_t    enabled;    // bit enabled or not
};

struct apple80211_tx_nss_data
{
    uint32_t    version;
    uint8_t     nss;
};

struct apple80211_nss_data
{
    uint32_t    version;
    uint8_t     nss;
};

struct apple80211_awdl_peer_traffic_registration
{
    uint32_t    version;
    void        *addr;
    uint32_t    name_len;
    char        name[152];
    uint32_t    active;
} __attribute__((packed));

struct apple80211_awdl_election_metric
{
    uint32_t    version;
    uint32_t    metric;
} __attribute__((packed));

struct apple80211_awdl_sync_enabled
{
    uint32_t    version;
    uint32_t    unk1;
    uint32_t    enabled;
} __attribute__((packed));

struct apple80211_awdl_sync_frame_template
{
    uint32_t    version;
    uint32_t    payload_len;
    void        *payload;
} __attribute__((packed));

#endif // _APPLE80211_IOCTL_H_

