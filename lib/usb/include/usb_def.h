#ifndef _USB_DEF_H
#define _USB_DEF_H 1

#include <stdint.h>
#include <sys/cdefs.h>

/** Definitions from the USB standards **/

/* Visible device states */


const unsigned DISCONNECTED = 0x00;
const unsigned ATTACHED = 0x01;
const unsigned POWERED = 0x03;
const unsigned DEFAULT = 0x07;
const unsigned ADDRESS = 0x0f;
const unsigned CONFIGURED = 0x1f;
const unsigned SUSPENDED = 0x80;

/* PID tokens */

enum UsbPid {
    PID_OUT = 0x1,
    PID_IN = 0x9,
    PID_SOF = 0x5,
    PID_SETUP = 0xD,
    PID_DATA0 = 0x3,
    PID_DATA1 = 0xB,
    PID_DATA2 = 0x7,
    PID_MDATA = 0xF,
    PID_ACK = 0x2,
    PID_NAK = 0xA,
    PID_STALL = 0xE,
    PID_NYET = 0x6,
    PID_PRE = 0xC,
    PID_ERR = 0xC,
    PID_SPLIT = 0x8,
    PID_PING = 0x4,
    PID_RESERVED = 0x0
};

static inline UsbPid USBtoggleDataPid(UsbPid pid) {
    if (pid == PID_DATA0)
        return PID_DATA1;
    else
        return PID_DATA0;
}

/* The setup packet format */

struct UsbSetupPacket {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __packed;

/* The notification packet format */

struct UsbNotificationPacket {
    uint8_t bmRequestType;
    uint8_t bNotification;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __packed;

/* The bmRequestType field in the setup packet */

const unsigned REQUEST_DIRECTION = 0x80; /* mask to get transfer direction */
const unsigned DEVICE_TO_HOST = 0x80;
const unsigned HOST_TO_DEVICE = 0x00;
const unsigned REQUEST_TYPE = 0x60; /* mask to get request type */
const unsigned STANDARD_REQUEST = 0x00;
const unsigned CLASS_REQUEST = 0x20;
const unsigned VENDOR_REQUEST = 0x40;
const unsigned REQUEST_RECIPIENT = 0x1f; /* mask to get recipient */
const unsigned DEVICE_RECIPIENT = 0x00;
const unsigned INTERFACE_RECIPIENT = 0x01;
const unsigned ENDPOINT_RECIPIENT = 0x02;

/* The bRequest field in the standard request setup packet */

enum UsbStandardRequest {
    GET_STATUS = 0,
    CLEAR_FEATURE = 1,
    /* Reserved for future use = 2, */
            SET_FEATURE = 3,
    /* Reserved for future use = 4, */
            SET_ADDRESS = 5,
    GET_DESCRIPTOR = 6,
    SET_DESCRIPTOR = 7,
    GET_CONFIGURATION = 8,
    SET_CONFIGURATION = 9,
    GET_INTERFACE = 10,
    SET_INTERFACE = 11,
    SYNCH_FRAME = 12
};

/* The bRequest field in the audio class specific request setup */

enum UsbAcRequest {
    SET_CUR = 0x01,
    GET_CUR = 0x81,
    SET_MIN = 0x02,
    GET_MIN = 0x82,
    SET_MAX = 0x03,
    GET_MAX = 0x83,
    SET_RES = 0x04,
    GET_RES = 0x84,
    SET_MEM = 0x05,
    GET_MEM = 0x85,
    GET_STAT = 0xff
};

/* CS value in the audio class specific request setup */

enum UsbAcFeatureUnitControlSelector {
    MUTE_CONTROL = 0x01,
    VOLUME_CONTROL = 0x02,
    BASS_CONTROL = 0x03,
    MID_CONTROL = 0x04,
    TREBLE_CONTROL = 0x05,
    GRAPHIC_EQUALIZER_CONTROL = 0x06,
    AUTOMATIC_GAIN_CONTROL = 0x07,
    DELAY_CONTROL = 0x08,
    BASS_BOOST_CONTROL = 0x09,
    LOUDNESS_CONTROL = 0x0a
};

/* The bRequest field in the CDC request setup packet */

enum UsbCdcRequest {
    SEND_ENCAPSULATED_COMMAND = 0x00,
    GET_ENCAPSULATED_COMMAND = 0x01,
    SET_COMM_FEATURE = 0x02,
    GET_COMM_FEATURE = 0x03,
    CLEAR_COMM_FEATURE = 0x04,
    SET_AUX_LINE_STATE = 0x10,
    SET_HOOK_STATE = 0x11,
    PULSE_SETUP = 0x12,
    SEND_PULSE = 0x13,
    SET_PULSE_TIME = 0x14,
    RING_AUX_JACK = 0x15,
    SET_LINE_CODING = 0x20,
    GET_LINE_CODING = 0x21,
    SET_CONTROL_LINE_STATE = 0x22,
    SEND_BREAK = 0x23,
    SET_RINGER_PARAMS = 0x30,
    GET_RINGER_PARAMS = 0x31,
    SET_OPERATION_PARAMS = 0x32,
    GET_OPERATION_PARAMS = 0x33,
    SET_LINE_PARAMS = 0x34,
    GET_LINE_PARAMS = 0x35,
    DIAL_DIGITS = 0x36
    /* ... and many more ... */
};

/* The bNotification field in the CDC notification packet */

enum UsbCdcPstnSubclassNotification {
    NETWORK_CONNECTION = 0x00,
    RESPONSE_AVAILABLE = 0x01,
    AUX_JACK_HOOK_STATE = 0x08,
    RING_DETECT = 0x09,
    SERIAL_STATE = 0x20,
    CALL_STATE_CHANGE = 0x28,
    LINE_STATE_CHANGE = 0x29
};

/* The bRequest field in the HID class request setup packet */

enum UsbHidRequest {
    GET_REPORT = 1,
    GET_IDLE = 2,
    GET_PROTOCOL = 3,
    SET_REPORT = 9,
    SET_IDLE = 10,
    SET_PROTOCOL = 11
};

enum UsbHidProtocol {
    HID_BOOT_PROTOCOL = 0,
    HID_REPORT_PROTOCOL = 1
};

/* When bRequest is CLEAR_FEATURE or SET_FEATURE, the wValue field
   contains the feature selector. */

enum UsbFeatureSelector {
    ENDPOINT_HALT = 0,
    DEVICE_REMOTE_WAKEUP = 1,
    TEST_MODE = 2
};

/* Device, interface class, subclass, etc. */

const unsigned AUDIO_CLASS = 0x01;
const unsigned AUDIOCONTROL_SUBCLASS = 0x01;
const unsigned AUDIOSTREAMING_SUBCLASS = 0x02;
const unsigned MIDISTREAMING_SUBCLASS = 0x03;

const unsigned COMMUNICATION_DEVICE_CLASS = 0x02;
const unsigned COMMUNICATION_INTERFACE_CLASS = 0x02;
const unsigned DATA_INTERFACE_CLASS = 0x0A;
const unsigned ABSTRACT_CONTROL_MODEL_SUBCLASS = 0x02;

const unsigned HUMAN_INTERFACE_DEVICE_CLASS = 0x03;
const unsigned BOOT_INTERFACE_SUBCLASS = 0x01;
const unsigned KEYBOARD_PROTOCOL = 0x01;
const unsigned MOUSE_PROTOCOL = 0x02;

const unsigned MASS_STORAGE_CLASS = 0x08;
const unsigned SCSI_TRANSPARENT_SUBCLASS = 0x06;
const unsigned BULK_ONLY_TRANSPORT_PROTOCOL = 0x50;

const unsigned VENDOR_SPECIFIC = 0xFF;

/* When bRequest is GET_DESCRIPTOR or SET_DESCRIPTOR, the wValue field
   contains the descriptor type. The same values are used in the
   bDescriptorType field in all descriptors. */

enum UsbDescriptorType {
    DEVICE_DESCRIPTOR = 1,
    CONFIGURATION_DESCRIPTOR = 2,
    STRING_DESCRIPTOR = 3,
    INTERFACE_DESCRIPTOR = 4,
    ENDPOINT_DESCRIPTOR = 5,
    DEVICE_QUALIFIER_DESCRIPTOR = 6,
    OTHER_SPEED_CONFIGURATION_DESCRIPTOR = 7,
    INTERFACE_POWER_DESCRIPTOR = 8,
    OTG_DESCRIPTOR = 9,
    DEBUG_DESCRIPTOR = 10,
    INTERFACE_ASSOCIATION_DESCRIPTOR = 11,
    CS_UNDEFINED_DESCRIPTOR = 0x20,
    CS_DEVICE_DESCRIPTOR = 0x21,
    CS_CONFIGURATION_DESCRIPTOR = 0x22,
    CS_STRING_DESCRIPTOR = 0x23,
    CS_INTERFACE_DESCRIPTOR = 0x24,
    CS_ENDPOINT_DESCRIPTOR = 0x25
};

enum UsbAudioControlInterfaceDescriptorSubtype {
    AC_HEADER_DESCRIPTOR = 0x01,
    AC_INPUT_TERMINAL_DESCRIPTOR = 0x02,
    AC_OUTPUT_TERMINAL_DESCRIPTOR = 0x03,
    AC_MIXER_UNIT_DESCRIPTOR = 0x04,
    AC_SELECTOR_UNIT_DESCRIPTOR = 0x05,
    AC_FEATURE_UNIT_DESCRIPTOR = 0x06,
    AC_PROCESSING_UNIT_DESCRIPTOR = 0x07,
    AC_EXTENSION_UNIT_DESCRIPTOR = 0x08
};

enum UsbAudioStreamingInterfaceDescriptorSubtype {
    AS_GENERAL_DESCRIPTOR = 0x01,
    AS_FORMAT_TYPE_DESCRIPTOR = 0x02,
    AS_FORMAT_SPECIFIC_DESCRIPTOR = 0x03
};

enum UsbAudioDataEndpointDescriptorSubtype {
    EP_GENERAL_DESCRIPTOR = 0x01
};

enum UsbCommunicationDataClassFunctionalDescriptorSubtype {
    CDC_HEADER_DESCRIPTOR = 0x00,
    CDC_CALL_MANAGEMENT_DESCRIPTOR = 0x01,
    CDC_ACM_DESCRIPTOR = 0x02,
    CDC_UNION_DESCRIPTOR = 0x06
};

enum UsbHidDescriptorType {
    HID_MAIN_DESCRIPTOR = 0x21,
    HID_REPORT_DESCRIPTOR = 0x22,
    HID_PHYSICAL_DESCRIPTOR = 0x23,
};

/* Descriptor formats */

struct UsbDeviceDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __packed;

struct UsbDeviceQualifierDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint8_t bNumConfigurations;
    uint8_t bReserved;
} __packed;

struct UsbConfigurationDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} __packed;

struct UsbInterfaceDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __packed;

struct UsbEndpointDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} __packed;

struct UsbHidMainDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    uint8_t bDescriptorType1;
    uint16_t wDescriptorLength1;
} __packed;

struct UsbCdcHeaderDescriptor {
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdCDC;
} __packed;

struct UsbCdcCallManagementDescriptor {
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
} __packed;

struct UsbCdcAcmDescriptor {
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
} __packed;

struct UsbCdcUnionDescriptor {
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bControlInterface;
    uint8_t bSubordinateInterface0;
} __packed;

struct UsbAcHeaderDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdADC;
    uint16_t wTotalLength;
    uint8_t bInCollection;
    uint8_t baInterfaceNr;
} __packed;

struct UsbAcInputTerminalDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalID;
    uint16_t wTerminalType;
    uint8_t bAssocTerminal;
    uint8_t bNrChannels;
    uint16_t wChannelConfig;
    uint8_t iChannelNames;
    uint8_t iTerminal;
} __packed;

struct UsbAcFeatureUnitDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bUnitID;
    uint8_t bSourceID;
    uint8_t bControlSize;
    uint16_t bmaControls0;
    uint8_t iFeature;
} __packed;

struct UsbAcOutputTerminalDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalID;
    uint16_t wTerminalType;
    uint8_t bAssocTerminal;
    uint8_t bSourceID;
    uint8_t iTerminal;
} __packed;

struct UsbAsGeneralDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalLink;
    uint8_t bDelay;
    uint16_t wFormatTag;
} __packed;

struct UsbAsFormatTypeDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bFormatType;
    uint8_t bNrChannels;
    uint8_t bSubFrameSize;
    uint8_t bBitResolution;
    uint8_t bSamFreqType;
    uint8_t tSamFreq[3];
} __packed;

struct UsbStdAudioDataEndpointDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
    uint8_t bRefresh;
    uint8_t bSynchAddress;
} __packed;

struct UsbCsAudioDataEndpointDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmAttributes;
    uint8_t bLockDelayUnits;
    uint16_t wLockDelay;
} __packed;

template<int N>
struct UsbStringDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bString[N];
} __packed;

/* Maximal length USB string descriptor */

typedef UsbStringDescriptor<126> UsbMaxStringDescriptor;

/* The bmAttributes field in the configuration descriptor */

const auto D7_RESERVED = 0x80;
const auto SELF_POWERED = 0x40;
const auto REMOTE_WAKEUP = 0x20;

/* Device status returned in GET_STATUS request */

const auto STATUS_SELF_POWERED = 0x0001;
const auto STATUS_REMOTE_WAKEUP = 0x0002;

/* The endpoint address (number and direction) */

const uint8_t ENDP0 = 0;
const uint8_t ENDP1 = 1;
const uint8_t ENDP2 = 2;
const uint8_t ENDP3 = 3;
const uint8_t ENDP4 = 4;
const uint8_t ENDP5 = 5;
const uint8_t ENDP6 = 6;
const uint8_t ENDP7 = 7;
const uint8_t ENDP8 = 8;
const uint8_t ENDP9 = 9;
const uint8_t ENDP10 = 10;
const uint8_t ENDP11 = 11;
const uint8_t ENDP12 = 12;
const uint8_t ENDP13 = 13;
const uint8_t ENDP14 = 14;
const uint8_t ENDP15 = 15;
const uint8_t ENDP_DIRECTION_MASK = 0x80;
const uint8_t ENDP_NUMBER_MASK = 0x7f;
const uint8_t ENDP_IN = 0x80;
const uint8_t ENDP_OUT = 0x00;

const uint8_t USB_EP_MAX_COUNT = 16;

/* The bmAttributes field in the endpoint descriptor */

enum usb_transfer_t {
    CONTROL_TRANSFER = 0,
    ISOCHRONOUS_TRANSFER = 1,
    BULK_TRANSFER = 2,
    INTERRUPT_TRANSFER = 3
};

const uint8_t CONTROL_TRANSFER_BM = 0x00;
const uint8_t ISOCHRONOUS_TRANSFER_BM = 0x01;
const uint8_t BULK_TRANSFER_BM = 0x02;
const uint8_t INTERRUPT_TRANSFER_BM = 0x03;
const uint8_t ASYNCHRONOUS_ENDPOINT = 0x04;
const uint8_t ADAPTIVE_ENDPOINT = 0x08;
const uint8_t SYNCHRONOUS_ENDPOINT = 0x0c;
const uint8_t FEEDBACK_ENDPOINT = 0x10;
const uint8_t IMPLICIT_FEEDBACK_DATA_ENDPOINT = 0x20;

/* Endpoint wMaxPacketSize */

const uint16_t MAX_LS_CONTROL_PACKET_SIZE = 8;
const uint16_t MAX_FS_CONTROL_PACKET_SIZE = 64;
const uint16_t MAX_HS_CONTROL_PACKET_SIZE = 64;
const uint16_t MAX_FS_ISOCHRONOUS_PACKET_SIZE = 1023;
const uint16_t MAX_HS_ISOCHRONOUS_PACKET_SIZE = 1024;
const uint16_t MAX_FS_BULK_PACKET_SIZE = 64;
const uint16_t MAX_HS_BULK_PACKET_SIZE = 12;
const uint16_t MAX_LS_INTERRUPT_PACKET_SIZE = 8;
const uint16_t MAX_FS_INTERRUPT_PACKET_SIZE = 64;
const uint16_t MAX_HS_INTERRUPT_PACKET_SIZE = 1024;

/* The language selector in the string descriptor */

const uint16_t LANG_US_ENGLISH = 0x0409;
const uint16_t LANG_POLISH = 0x0415;

/* Mouse boot protocol report */

struct HidMouseBootReport {
    uint8_t buttons;
    int8_t x;
    int8_t y;
} __packed;

const unsigned MOUSE_LEFT_BUTTON = 0x01;
const unsigned MOUSE_RIGHT_BUTTON = 0x02;
const unsigned MOUSE_MIDDLE_BUTTON = 0x04;

/* Keyboard boot protocol report */

const unsigned KEYBOARD_LEFT_CTRL = 0x01;
const unsigned KEYBOARD_LEFT_SHIFT = 0x02;
const unsigned KEYBOARD_LEFT_ALT = 0x04;
const unsigned KEYBOARD_LEFT_GUI = 0x08;
const unsigned KEYBOARD_RIGHT_CTRL = 0x10;
const unsigned KEYBOARD_RIGHT_SHIFT = 0x20;
const unsigned KEYBOARD_RIGHT_ALT = 0x40;
const unsigned KEYBOARD_RIGHT_GUI = 0x80;

const unsigned KEYBOARD_NUM_LOCK_LED = 0x01;
const unsigned KEYBOARD_CAPS_LOCK_LED = 0x02;
const unsigned KEYBOARD_SCROLL_LOCK_LED = 0x04;
const unsigned KEYBOARD_COMPOSE_LED = 0x08;
const unsigned KEYBOARD_KANA_LED = 0x10;

const unsigned KEYBOARD_MAX_PRESSED_KEYS = 6;
const unsigned CAPS_LOCK_SCAN_CODE = 57;
const unsigned NUM_LOCK_SCAN_CODE = 83;

/* RS-232 emulation data structures */

struct UsbCdcLineCoding {
    uint32_t dwDTERate;   /* bits per second */
    uint8_t bCharFormat; /* stop bit(s), see below */
    uint8_t bParityType; /* parity, see below */
    uint8_t bDataBits;
} __packed;

const unsigned ONE_STOP_BIT = 0;
const unsigned ONE_AND_HALF_STOP_BITS = 1;
const unsigned TWO_STOP_BITS = 2;

const unsigned NO_PARITY = 0;
const unsigned ODD_PARITY = 1;
const unsigned EVEN_PARITY = 2;
const unsigned MARK_PARITY = 3;
const unsigned SPACE_PARITY = 4;

struct UsbCdcSerialState {
    UsbNotificationPacket notification;
    uint16_t wData;
} __packed;

/* Mass Storage Class, Bulk-Only Transport structures and constants */

/* Mass Storage Class requests */
const unsigned MSC_GET_MAX_LUN = 0xfe;
const unsigned MSC_BULK_ONLY_RESET = 0xff;
const unsigned MSC_CBWCB_LENGTH = 16;

/* Mass Storage Class, Bulk-Only Transport, Command Block Wrapper */
struct msc_bot_cbw_t {
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t bmCBWFlags;   /* Only 7-th bit, bits 0-6 are reserved. */
    uint8_t bCBWLUN;      /* Only 4 LS bits, 4 MS bits are reserved. */
    uint8_t bCBWCBLength; /* Only 5 LS bits, 3 MS bits are reserved. */
    uint8_t CBWCB[MSC_CBWCB_LENGTH];
} __packed;

const unsigned MSC_BOT_CBW_SIGNATURE = 0x43425355;
const unsigned MSC_BOT_CBW_FLAGS_RESERVED_BITS = 0x7f;
const unsigned MSC_BOT_CBW_LUN_RESERVED_BITS = 0xf0;
const unsigned MSC_BOT_CBW_CB_LENGTH_RESERVED_BITS = 0xe0;
const unsigned MSC_BOT_CBW_DATA_IN = 0x80;

/* Mass Storage Class, Bulk-Only Transport, Command Status Wrapper */
struct msc_bot_csw_t {
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t bCSWStatus;
} __packed;

const unsigned MSC_BOT_CSW_SIGNATURE = 0x53425355;
const unsigned MSC_BOT_CSW_COMMAND_PASSED = 0x00;
const unsigned MSC_BOT_CSW_COMMAND_FAILED = 0x01;
const unsigned MSC_BOT_CSW_PHASE_ERROR = 0x02;
const unsigned MSC_BOT_UNDEF_ERROR = 0xFF;

/** Host limits **/

/* Standard timers (milliseconds) */

const unsigned STARTUP_TIME_MS = 100;
const unsigned RESET_TIME_MS = 15;
const unsigned RECOVERY_TIME_MS = 10;
const unsigned DATA_STAGE_TIMEOUT_MS = 5000;
const unsigned NODATA_STAGE_TIMEOUT_MS = 50;

/* Application specific timers (microseconds) */

const unsigned HOST_LS_SCHEDULE_US = 275;
const unsigned HOST_FS_SCHEDULE_US = 30;
const unsigned HOST_HS_SCHEDULE_US = 15;

/* Application specific timers (milliseconds) */

const unsigned DEVICE_RESET_TIME_MS = 300;

/* Others */

const unsigned TRANS_MAX_REP_COUNT = 3;

/** USB library common definitions **/

/* Device speed or maximal host speed */

enum usb_speed_t {
    HIGH_SPEED = 0,
    FULL_SPEED = 1,
    LOW_SPEED = 2,
    FULL_LOW_SPEED = 3,
    NO_SPEED = 15 /* indicates an error */
};

/* Used transceiver */

enum usb_phy_t {
    USB_PHY_A,
    USB_PHY_B,
    USB_PHY_ULPI,
    USB_PHY_I2C
};

/* Return value of device request routines */

enum UsbResult {
    REQUEST_SUCCESS,
    REQUEST_ERROR
};

/* Values reported after transaction completion */

enum usbh_transaction_result_t {
    TR_UNDEF = 0,
    TR_DONE,
    TR_NAK,
    TR_STALL,
    TR_ERROR
};

#endif
