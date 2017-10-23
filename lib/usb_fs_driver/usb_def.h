#pragma once

enum RECIPIENT_TYPE {
    DEVICE_RECIPIENT,     /* Recipient device */
    INTERFACE_RECIPIENT,  /* Recipient interface */
    ENDPOINT_RECIPIENT,   /* Recipient endpoint */
    OTHER_RECIPIENT
};

enum STANDARD_REQUESTS {
    GET_STATUS = 0,
    CLEAR_FEATURE,
    RESERVED1,
    SET_FEATURE,
    RESERVED2,
    SET_ADDRESS,
    GET_DESCRIPTOR,
    SET_DESCRIPTOR,
    GET_CONFIGURATION,
    SET_CONFIGURATION,
    GET_INTERFACE,
    SET_INTERFACE,
    TOTAL_sREQUEST,  /* Total number of Standard request */
    SYNCH_FRAME = 12
};

/* Definition of "USBwValue" */
enum DESCRIPTOR_TYPE {
    DEVICE_DESCRIPTOR = 1,
    CONFIG_DESCRIPTOR,
    STRING_DESCRIPTOR,
    INTERFACE_DESCRIPTOR,
    ENDPOINT_DESCRIPTOR,
    DEVICE_BOS_DESCRIPTOR = 0xF
};

/* Feature selector of a SET_FEATURE or CLEAR_FEATURE */
enum FEATURE_SELECTOR {
    ENDPOINT_STALL,
    DEVICE_REMOTE_WAKEUP
};

/* Exported constants --------------------------------------------------------*/
/* Definition of "USBbmRequestType" */
#define REQUEST_TYPE      0x60  /* Mask to get request type */
#define STANDARD_REQUEST  0x00  /* Standard request */
#define CLASS_REQUEST     0x20  /* Class request */
#define VENDOR_REQUEST    0x40  /* Vendor request */

#define RECIPIENT         0x1F  /* Mask to get recipient */
