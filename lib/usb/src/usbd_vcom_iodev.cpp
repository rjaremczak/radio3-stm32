#include "usbd_api.h"
#include "usbd_callbacks.h"
#include "usbd_error.h"
#include "iodev.h"

#include <stdio.h>

/** Descriptors **/

const auto BLK_BUFF_SIZE = MAX_FS_BULK_PACKET_SIZE;
const auto INT_BUFF_SIZE = 16;
const auto USB_BM_ATTRIBUTES = D7_RESERVED;
const auto USB_B_MAX_POWER = (400 / 2);
const auto VID = 0x0483;
const auto PID = 0x5750;

static UsbDeviceDescriptor const device_descriptor = {
        sizeof(UsbDeviceDescriptor), /* bLength */
        DEVICE_DESCRIPTOR,               /* bDescriptorType */
        0x0200,                          /* bcdUSB */
        COMMUNICATION_DEVICE_CLASS,      /* bDeviceClass */
        0,                               /* bDeviceSubClass */
        0,                               /* bDeviceProtocol */
        64,                              /* bMaxPacketSize0 */
        VID,                             /* idVendor */
        PID + 2,                         /* idProduct */
        0x0100,                          /* bcdDevice */
        1,                               /* iManufacturer */
        2,                               /* iProduct */
        3,                               /* iSerialNumber */
        1                                /* bNumConfigurations */
};

struct UsbComConfiguration {
    UsbConfigurationDescriptor cnf_descr;
    UsbInterfaceDescriptor if0_descr;
    UsbCdcHeaderDescriptor cdc_h_descr;
    UsbCdcCallManagementDescriptor cdc_cm_descr;
    UsbCdcAcmDescriptor cdc_acm_descr;
    UsbCdcUnionDescriptor cdc_u_descr;
    UsbEndpointDescriptor ep2in_descr;
    UsbInterfaceDescriptor if1_descr;
    UsbEndpointDescriptor ep1out_descr;
    UsbEndpointDescriptor ep1in_descr;
} __packed;

static UsbComConfiguration const com_configuration = {
        {
                .bLength = sizeof(UsbConfigurationDescriptor),
                .bDescriptorType = CONFIGURATION_DESCRIPTOR,
                .wTotalLength = sizeof(UsbComConfiguration),
                .bNumInterfaces = 2,
                .bConfigurationValue = 1,
                .iConfiguration = 0,
                .bmAttributes = USB_BM_ATTRIBUTES,
                .bMaxPower = USB_B_MAX_POWER
        },{
                .bLength = sizeof(UsbInterfaceDescriptor),
                .bDescriptorType = INTERFACE_DESCRIPTOR,
                .bInterfaceNumber = 0,
                .bAlternateSetting = 0,
                .bNumEndpoints = 1,
                .bInterfaceClass = COMMUNICATION_INTERFACE_CLASS,
                .bInterfaceSubClass = ABSTRACT_CONTROL_MODEL_SUBCLASS,
                .bInterfaceProtocol = 0,
                .iInterface = 0
        },{
                .bFunctionLength = sizeof(UsbCdcHeaderDescriptor),
                .bDescriptorType = CS_INTERFACE_DESCRIPTOR,
                .bDescriptorSubtype = CDC_HEADER_DESCRIPTOR,
                .bcdCDC = 0x120
        },{
                .bFunctionLength = sizeof(UsbCdcCallManagementDescriptor),
                .bDescriptorType = CS_INTERFACE_DESCRIPTOR,
                .bDescriptorSubtype = CDC_CALL_MANAGEMENT_DESCRIPTOR,
                .bmCapabilities = 3,
                .bDataInterface = 1
        },{
                .bFunctionLength = sizeof(UsbCdcAcmDescriptor),
                .bDescriptorType = CS_INTERFACE_DESCRIPTOR,
                .bDescriptorSubtype = CDC_ACM_DESCRIPTOR,
                .bmCapabilities = 2
        },{
                .bFunctionLength = sizeof(UsbCdcUnionDescriptor),
                .bDescriptorType = CS_INTERFACE_DESCRIPTOR,
                .bDescriptorSubtype = CDC_UNION_DESCRIPTOR,
                .bControlInterface = 0,
                .bSubordinateInterface0 = 1
        },{
                .bLength = sizeof(UsbEndpointDescriptor),
                .bDescriptorType = ENDPOINT_DESCRIPTOR,
                .bEndpointAddress = ENDP2 | ENDP_IN,
                .bmAttributes = INTERRUPT_TRANSFER,
                .wMaxPacketSize = INT_BUFF_SIZE,
                .bInterval = 3
        },{
                .bLength = sizeof(UsbInterfaceDescriptor),
                .bDescriptorType = INTERFACE_DESCRIPTOR,
                .bInterfaceNumber = 1,
                .bAlternateSetting = 0,
                .bNumEndpoints = 2,
                .bInterfaceClass = DATA_INTERFACE_CLASS,
                .bInterfaceSubClass = 0,
                .bInterfaceProtocol = 0,
                .iInterface = 0
        },{
                .bLength = sizeof(UsbEndpointDescriptor),
                .bDescriptorType = ENDPOINT_DESCRIPTOR,
                .bEndpointAddress = ENDP1 | ENDP_OUT,
                .bmAttributes = BULK_TRANSFER,
                .wMaxPacketSize = BLK_BUFF_SIZE,
                .bInterval = 0
        },{
                .bLength = sizeof(UsbEndpointDescriptor),
                .bDescriptorType = ENDPOINT_DESCRIPTOR,
                .bEndpointAddress = ENDP1 | ENDP_IN,
                .bmAttributes = BULK_TRANSFER,
                .wMaxPacketSize = BLK_BUFF_SIZE,
                .bInterval = 0
        }
};

static UsbStringDescriptor<1> const string_lang = {
        sizeof(UsbStringDescriptor<1>),
        STRING_DESCRIPTOR, {LANG_US_ENGLISH}
};

static UsbStringDescriptor<12> const string_manufacturer = {
        sizeof(UsbStringDescriptor<12>),
        STRING_DESCRIPTOR, {'m', 'i', 'n', 'd', 'p', 'a', 'r', 't', '.', 'c', 'o', 'm'}
};

static UsbStringDescriptor<16> const string_product = {
        sizeof(UsbStringDescriptor<16>),
        STRING_DESCRIPTOR, {'r', 'a', 'd', 'i', 'o', '3', ' ', 'b', 'y', ' ', 'S', 'Q', '6', 'D', 'G', 'T'}
};

static UsbStringDescriptor<10> const string_serial = {
        sizeof(UsbStringDescriptor<10>),
        STRING_DESCRIPTOR, {'0', '0', '0', '0', '0', '0', '0', '0', '0', '1'}
};

struct StringTable {
    uint8_t const *data;
    uint16_t length;
};

static StringTable const strings[] = {
        {(uint8_t const *) &string_lang,         sizeof string_lang},
        {(uint8_t const *) &string_manufacturer, sizeof string_manufacturer},
        {(uint8_t const *) &string_product,      sizeof string_product},
        {(uint8_t const *) &string_serial,       sizeof string_serial}
};

static uint32_t const stringCount = sizeof(strings) / sizeof(strings[0]);

/** Callbacks **/

static int Configure(void);
static uint8_t Reset(usb_speed_t);
static UsbResult GetDescriptor(uint16_t, uint16_t, uint8_t const **, uint16_t *);
static uint8_t GetConfiguration(void);
static UsbResult SetConfiguration(uint16_t);
static uint16_t GetStatus(void);
static UsbResult ClassNoDataSetup(UsbSetupPacket const *);
static UsbResult ClassInDataSetup(UsbSetupPacket const *, uint8_t const **, uint16_t *);
static UsbResult ClassOutDataSetup(UsbSetupPacket const *, uint8_t **);
static void ClassStatusIn(UsbSetupPacket const *);

static void EP1IN(void);
static void EP1OUT(void);
static void EP2IN(void);

static UsbdCallbackList const ApplicationCallBacks = {
        Configure, Reset, 0, GetDescriptor, 0,
        GetConfiguration, SetConfiguration, GetStatus, 0, 0,
        0, 0,
        ClassNoDataSetup, ClassInDataSetup,
        ClassOutDataSetup, ClassStatusIn,
        {EP1IN, EP2IN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {EP1OUT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        0, 0
};

/** COM implementation **/

static uint16_t ep2queue;
static uint8_t configuration, rs232state;
static UsbCdcLineCoding rs232coding;

/* rs232state bits */
const uint8_t DTR = 0x01;
const uint8_t DCD = 0x02;
const uint8_t DSR = 0x04;
const uint8_t RTS = 0x08;
const uint8_t CTS = 0x10;

static void ResetState(void) {
    ep2queue = 0;
    configuration = 0;
    rs232coding.dwDTERate = 38400;
    rs232coding.bCharFormat = ONE_STOP_BIT;
    rs232coding.bParityType = NO_PARITY;
    rs232coding.bDataBits = 8;
    rs232state = 0;
}

UsbdCallbackList const *USBDgetApplicationCallbacks() {
    return &ApplicationCallBacks;
}

int Configure() {
    ResetState();
    return 0;
}

uint8_t Reset(usb_speed_t speed) {
    if (speed != FULL_SPEED) { usbd_resetableError(6); }
    ResetState();

    /* Default control endpoint must be configured here. */
    if (USBDendPointConfigure(ENDP0, CONTROL_TRANSFER,
                              device_descriptor.bMaxPacketSize0,
                              device_descriptor.bMaxPacketSize0) != REQUEST_SUCCESS) {
        usbd_resetableError(7);
    }

    return device_descriptor.bMaxPacketSize0;
}

UsbResult GetDescriptor(uint16_t wValue, uint16_t wIndex, uint8_t const **data, uint16_t *length) {
    uint32_t index = wValue & 0xff;

    switch (wValue >> 8) {
        case DEVICE_DESCRIPTOR:
            if (index == 0 && wIndex == 0) {
                *data = (uint8_t const *) &device_descriptor;
                *length = sizeof(device_descriptor);
                return REQUEST_SUCCESS;
            }
            return REQUEST_ERROR;
        case CONFIGURATION_DESCRIPTOR:
            if (index == 0 && wIndex == 0) {
                *data = (uint8_t const *) &com_configuration;
                *length = sizeof(com_configuration);
                return REQUEST_SUCCESS;
            }
            return REQUEST_ERROR;
        case STRING_DESCRIPTOR:
            if (index < stringCount) {
                *data = strings[index].data;
                *length = strings[index].length;
                return REQUEST_SUCCESS;
            }
            return REQUEST_ERROR;
        default:
            return REQUEST_ERROR;
    }
}

uint8_t GetConfiguration() {
    return configuration;
}

UsbResult SetConfiguration(uint16_t confValue) {
    if (confValue > device_descriptor.bNumConfigurations)
        return REQUEST_ERROR;

    configuration = confValue;
    USBDdisableAllNonControlEndPoints();
    if (confValue == com_configuration.cnf_descr.bConfigurationValue) {
        UsbResult r1, r2;

        r1 = USBDendPointConfigure(ENDP1, BULK_TRANSFER, BLK_BUFF_SIZE, BLK_BUFF_SIZE);
        r2 = USBDendPointConfigure(ENDP2, INTERRUPT_TRANSFER, 0, INT_BUFF_SIZE);
        if (r1 == REQUEST_SUCCESS && r2 == REQUEST_SUCCESS) {
            return REQUEST_SUCCESS;
        } else {
            return REQUEST_ERROR;
        }
    }

    return REQUEST_SUCCESS; /* confValue == 0 */
}

uint16_t GetStatus() {
    /* Current power setting should be reported. */
    if (com_configuration.cnf_descr.bmAttributes & SELF_POWERED) {
        return STATUS_SELF_POWERED;
    } else {
        return 0;
    }
}

static UsbCdcSerialState state = {
        {
                DEVICE_TO_HOST | CLASS_REQUEST | INTERFACE_RECIPIENT,
                SERIAL_STATE, 0, 0, 2
        }, 0
};

UsbResult ClassNoDataSetup(UsbSetupPacket const *setup) {
    if (setup->bmRequestType == (HOST_TO_DEVICE | CLASS_REQUEST | INTERFACE_RECIPIENT) &&
        setup->bRequest == SET_CONTROL_LINE_STATE &&
        setup->wIndex == 0 &&
        setup->wLength == 0) {
        uint8_t new_rs232state;

        /* Host to device: DTR or RTS notification */
        new_rs232state = rs232state;
        if (setup->wValue & 1) /* DTR set */
            new_rs232state |= (DTR | DSR | DCD);
        else
            new_rs232state &= ~(DTR | DSR | DCD);
        if (setup->wValue & 2) /* RTS set */
            new_rs232state |= RTS | CTS;
        else
            new_rs232state &= ~(RTS | CTS);

        /* Device to host: DCD or DSR notification */
        if ((rs232state ^ new_rs232state) & (DCD | DSR)) {
            state.wData = 0;
            if (new_rs232state & DCD)
                state.wData |= 1;
            if (new_rs232state & DSR)
                state.wData |= 2;
            if (ep2queue == 0)
                USBDwrite(ENDP2, (uint8_t const *) &state, sizeof(state));
            if (ep2queue < 2)
                ++ep2queue;
        }

        /* Set new state. */
        if (rs232state != new_rs232state) {
            rs232state = new_rs232state;
        }

        return REQUEST_SUCCESS;
    }
    return REQUEST_ERROR;
}

UsbResult ClassInDataSetup(UsbSetupPacket const *setup, uint8_t const **data, uint16_t *length) {
    if (setup->bmRequestType == (DEVICE_TO_HOST | CLASS_REQUEST | INTERFACE_RECIPIENT) &&
        setup->bRequest == GET_LINE_CODING &&
        setup->wValue == 0 &&
        setup->wIndex == 0) {
        *data = (const uint8_t *) &rs232coding;
        *length = sizeof(rs232coding);
        return REQUEST_SUCCESS;
    }
    return REQUEST_ERROR;
}

UsbResult ClassOutDataSetup(UsbSetupPacket const *setup, uint8_t **data) {
    if (setup->bmRequestType == (HOST_TO_DEVICE | CLASS_REQUEST | INTERFACE_RECIPIENT) &&
        setup->bRequest == SET_LINE_CODING &&
        setup->wValue == 0 &&
        setup->wIndex == 0 &&
        setup->wLength == sizeof(rs232coding)) {
        *data = (uint8_t *) &rs232coding;
        return REQUEST_SUCCESS;
    }
    return REQUEST_ERROR;
}

void ClassStatusIn(UsbSetupPacket const *setup) {
    if (setup->bmRequestType == (HOST_TO_DEVICE | CLASS_REQUEST | INTERFACE_RECIPIENT) &&
        setup->bRequest == SET_LINE_CODING &&
        setup->wValue == 0 &&
        setup->wIndex == 0 &&
        setup->wLength == sizeof(rs232coding)) {
    }
}

void EP2IN() {
    if (ep2queue > 0) { --ep2queue; }
    if (ep2queue > 0) { USBDwrite(ENDP2, (uint8_t const *) &state, sizeof(state)); }
}

void EP1IN() {
    usbd_write_finished(ENDP1);
}

// -------------------- iodev implementation --------------------------

static uint8_t buffer[BLK_BUFF_SIZE];
static volatile uint8_t buffer_write_pos = 0;
static volatile uint8_t buffer_read_pos = 0;

static uint8_t error = 0;
static uint32_t timeout;
static const uint8_t read_timeout_ms = 200;

extern volatile uint32_t currentTime;

void EP1OUT() {
    if (buffer_read_pos == buffer_write_pos) {
        buffer_read_pos = 0;
        buffer_write_pos = 0;
    }

    uint8_t len = (uint8_t) USBDread(ENDP1, buffer + buffer_write_pos, BLK_BUFF_SIZE - buffer_write_pos);
    buffer_write_pos += len;
}

void usbd_init();

void iodev_init() {
    usbd_init();
}

uint8_t iodev_incomingData() {
    return buffer_read_pos < buffer_write_pos;
}

uint8_t iodev_error() {
    return error;
}

void iodev_write(uint8_t byte) {
    while (usbd_isWriteInProgress(ENDP1)) {}
    USBDwrite(ENDP1, &byte, 1);
}

uint8_t iodev_read() {
    timeout = currentTime + read_timeout_ms;

    while (!iodev_incomingData()) {
        if (currentTime > timeout) {
            error = 1;
            return 0;
        }
    }

    return buffer[buffer_read_pos++];
}

uint16_t iodev_readWord() {
    uint8_t low = iodev_read();
    if (iodev_error()) { return 0; }

    uint8_t high = iodev_read();
    if (iodev_error()) { return 0; }

    return (high << 8) + low;
}

void iodev_write_word(uint16_t word) {
    iodev_write(word & 0xFF);
    iodev_write((word >> 8) & 0xFF);
}

void iodev_write_buf(void *buf, uint16_t size) {
    while (usbd_isWriteInProgress(ENDP1)) {}
    USBDwriteEx(ENDP1, (const uint8_t *) buf, size);
}

void iodev_read_buf(void *buf, uint16_t size) {
    uint8_t *bytePtr = (uint8_t *) buf;
    while (size--) {
        *bytePtr++ = iodev_read();
        if (iodev_error()) { return; }
    }
}
