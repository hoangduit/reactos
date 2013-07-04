
#include <initguid.h>

//GUID_NDIS_STATUS_LINK_SPEED_CHANGE
//GUID_NDIS_STATUS_MEDIA_SPECIFIC_INDICATION
//GUID_NDIS_STATUS_MEDIA_DISCONNECT
//GUID_NDIS_STATUS_MEDIA_CONNECT
//GUID_NDIS_STATUS_RESET_END
//GUID_NDIS_STATUS_RESET_START
//GUID_NDIS_ATM_RCV_CELLS_DROPPED
//GUID_NDIS_ATM_XMIT_CELLS_OK
//GUID_NDIS_ATM_RCV_CELLS_OK
//GUID_NDIS_ATM_MAX_AAL5_PACKET_SIZE
//GUID_NDIS_ATM_MAX_AAL34_PACKET_SIZE
//GUID_NDIS_ATM_MAX_AAL1_PACKET_SIZE
//GUID_NDIS_ATM_MAX_AAL0_PACKET_SIZE
//GUID_NDIS_ATM_MAX_ACTIVE_VPI_BITS
//GUID_NDIS_ATM_MAX_ACTIVE_VCI_BITS
//GUID_NDIS_ATM_MAX_ACTIVE_VCS
//GUID_NDIS_ATM_HW_CURRENT_ADDRESS
//GUID_NDIS_ATM_SUPPORTED_AAL_TYPES
//GUID_NDIS_ATM_SUPPORTED_SERVICE_CATEGORY
//GUID_NDIS_ATM_SUPPORTED_VC_RATES
//GUID_NDIS_802_11_MEDIA_STREAM_MODE
//GUID_NDIS_802_11_TEST
//GUID_NDIS_802_11_ASSOCIATION_INFORMATION
//GUID_NDIS_802_11_REMOVE_KEY
//GUID_NDIS_802_11_ADD_KEY
//GUID_NDIS_802_11_RELOAD_DEFAULTS
//GUID_NDIS_802_11_WEP_STATUS
//GUID_NDIS_802_11_BSSID_LIST_SCAN
//GUID_NDIS_802_11_PRIVACY_FILTER
//GUID_NDIS_802_11_AUTHENTICATION_MODE
//GUID_NDIS_802_11_DISASSOCIATE
//GUID_NDIS_802_11_REMOVE_WEP
//GUID_NDIS_802_11_ADD_WEP
//GUID_NDIS_802_11_STATISTICS
//GUID_NDIS_802_11_CONFIGURATION
//GUID_NDIS_802_11_DESIRED_RATES
//GUID_NDIS_802_11_SUPPORTED_RATES
//GUID_NDIS_802_11_TX_ANTENNA_SELECTED
//GUID_NDIS_802_11_RX_ANTENNA_SELECTED
//GUID_NDIS_802_11_NUMBER_OF_ANTENNAS
//GUID_NDIS_802_11_RTS_THRESHOLD
//GUID_NDIS_802_11_FRAGMENTATION_THRESHOLD
//GUID_NDIS_802_11_INFRASTRUCTURE_MODE
//GUID_NDIS_802_11_BSSID_LIST
//GUID_NDIS_802_11_RSSI_TRIGGER
//GUID_NDIS_802_11_RSSI
//GUID_NDIS_802_11_TX_POWER_LEVEL
//GUID_NDIS_802_11_POWER_MODE
//GUID_NDIS_802_11_NETWORK_TYPE_IN_USE
//GUID_NDIS_802_11_NETWORK_TYPES_SUPPORTED
//GUID_NDIS_802_11_SSID
//GUID_NDIS_802_11_BSSID
//GUID_NDIS_FDDI_LCONNECTION_STATE
//GUID_NDIS_FDDI_LEM_REJECTS
//GUID_NDIS_FDDI_LCT_FAILURES
//GUID_NDIS_FDDI_RING_MGT_STATE
//GUID_NDIS_FDDI_FRAMES_LOST
//GUID_NDIS_FDDI_FRAME_ERRORS
//GUID_NDIS_FDDI_DOWNSTREAM_NODE_LONG
//GUID_NDIS_FDDI_UPSTREAM_NODE_LONG
//GUID_NDIS_FDDI_ATTACHMENT_TYPE
//GUID_NDIS_FDDI_SHORT_MAX_LIST_SIZE
//GUID_NDIS_FDDI_SHORT_MULTICAST_LIST
//GUID_NDIS_FDDI_SHORT_CURRENT_ADDR
//GUID_NDIS_FDDI_SHORT_PERMANENT_ADDR
//GUID_NDIS_FDDI_LONG_MAX_LIST_SIZE
//GUID_NDIS_FDDI_LONG_MULTICAST_LIST
//GUID_NDIS_FDDI_LONG_CURRENT_ADDR
//GUID_NDIS_FDDI_LONG_PERMANENT_ADDR
//GUID_NDIS_802_5_LOST_FRAMES
//GUID_NDIS_802_5_LINE_ERRORS
//GUID_NDIS_802_5_CURRENT_RING_STATE
//GUID_NDIS_802_5_CURRENT_RING_STATUS
//GUID_NDIS_802_5_LAST_OPEN_STATUS
//GUID_NDIS_802_5_CURRENT_GROUP
//GUID_NDIS_802_5_CURRENT_FUNCTIONAL
//GUID_NDIS_802_5_CURRENT_ADDRESS
//GUID_NDIS_802_5_PERMANENT_ADDRESS
//GUID_NDIS_802_3_XMIT_MORE_COLLISIONS
//GUID_NDIS_802_3_XMIT_ONE_COLLISION
//GUID_NDIS_802_3_RCV_ERROR_ALIGNMENT
//GUID_NDIS_802_3_MAC_OPTIONS
//GUID_NDIS_802_3_MAXIMUM_LIST_SIZE
//GUID_NDIS_802_3_MULTICAST_LIST
//GUID_NDIS_802_3_CURRENT_ADDRESS
//GUID_NDIS_802_3_PERMANENT_ADDRESS
//GUID_NDIS_GEN_CO_RCV_PDUS_NO_BUFFER
//GUID_NDIS_GEN_CO_RCV_PDUS_ERROR
//GUID_NDIS_GEN_CO_XMIT_PDUS_ERROR
//GUID_NDIS_GEN_CO_RCV_PDUS_OK
//GUID_NDIS_GEN_CO_XMIT_PDUS_OK
//GUID_NDIS_GEN_CO_MINIMUM_LINK_SPEED
//GUID_NDIS_GEN_CO_VENDOR_DRIVER_VERSION
//GUID_NDIS_GEN_CO_MEDIA_CONNECT_STATUS
//GUID_NDIS_GEN_CO_MAC_OPTIONS
//GUID_NDIS_GEN_CO_DRIVER_VERSION
//GUID_NDIS_GEN_CO_VENDOR_DESCRIPTION
//GUID_NDIS_GEN_CO_VENDOR_ID
//GUID_NDIS_GEN_CO_LINK_SPEED
//GUID_NDIS_GEN_CO_MEDIA_IN_USE
//GUID_NDIS_GEN_CO_MEDIA_SUPPORTED
//GUID_NDIS_GEN_CO_HARDWARE_STATUS
//GUID_NDIS_GEN_RCV_NO_BUFFER
//GUID_NDIS_GEN_RCV_ERROR
//GUID_NDIS_GEN_XMIT_ERROR
//GUID_NDIS_GEN_RCV_OK
//GUID_NDIS_GEN_XMIT_OK
//GUID_NDIS_GEN_PHYSICAL_MEDIUM
//GUID_NDIS_GEN_VLAN_ID
//GUID_NDIS_GEN_VENDOR_DRIVER_VERSION
//GUID_NDIS_GEN_MAXIMUM_SEND_PACKETS
//GUID_NDIS_GEN_MEDIA_CONNECT_STATUS
//GUID_NDIS_GEN_MAC_OPTIONS
//GUID_NDIS_GEN_MAXIMUM_TOTAL_SIZE
//GUID_NDIS_GEN_DRIVER_VERSION
//GUID_NDIS_GEN_CURRENT_LOOKAHEAD
//GUID_NDIS_GEN_CURRENT_PACKET_FILTER
//GUID_NDIS_GEN_VENDOR_DESCRIPTION
//GUID_NDIS_GEN_VENDOR_ID
//GUID_NDIS_GEN_RECEIVE_BLOCK_SIZE
//GUID_NDIS_GEN_TRANSMIT_BLOCK_SIZE
//GUID_NDIS_GEN_RECEIVE_BUFFER_SPACE
//GUID_NDIS_GEN_TRANSMIT_BUFFER_SPACE
//GUID_NDIS_GEN_LINK_SPEED
//GUID_NDIS_GEN_MAXIMUM_FRAME_SIZE
//GUID_NDIS_GEN_MAXIMUM_LOOKAHEAD
//GUID_NDIS_GEN_MEDIA_IN_USE
//GUID_NDIS_GEN_MEDIA_SUPPORTED
//GUID_NDIS_GEN_HARDWARE_STATUS
//GUID_NDIS_NOTIFY_DEVICE_POWER_OFF
//GUID_NDIS_NOTIFY_DEVICE_POWER_ON
//GUID_NDIS_WAKE_ON_MAGIC_PACKET_ONLY
//GUID_NDIS_NOTIFY_UNBIND
//GUID_NDIS_NOTIFY_BIND
//GUID_NDIS_NOTIFY_VC_ARRIVAL
//GUID_NDIS_NOTIFY_VC_REMOVAL
//GUID_NDIS_ENUMERATE_VC
//GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL
//GUID_NDIS_NOTIFY_ADAPTER_REMOVAL
//GUID_NDIS_ENUMERATE_ADAPTER
//GUID_DEVINTERFACE_NET
//GUID_NDIS_LAN_CLASS
