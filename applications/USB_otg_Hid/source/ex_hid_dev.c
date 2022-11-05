
#include <stdio.h>
#include <usbd_api.h>
#include <usbd_callbacks.h>

/** Descriptors **/
#define VID 0x0483
#define PID 0x5750

static usb_device_descriptor_t const device_descriptor = {
  sizeof(usb_device_descriptor_t), /* bLength */
  DEVICE_DESCRIPTOR,               /* bDescriptorType */
  (0x0200),                        /* bcdUSB */
  0,                               /* bDeviceClass */
  0,                               /* bDeviceSubClass */
  0,                               /* bDeviceProtocol */
  8,                               /* bMaxPacketSize0 */
  (VID),                           /* idVendor */
  (PID + 1),                       /* idProduct */
  (0x0100),                         /* bcdDevice */
  1,                               /* iManufacturer */
  2,                               /* iProduct */
  3,                               /* iSerialNumber */
  1                                /* bNumConfigurations */
};

static uint8_t usb_hid_report_descriptor[] = {
  0x05, 0x01, /* Usage Page (Generic Desktop) */
  0x09, 0x02, /* Usage (Mouse) */
  0xa1, 0x01, /* Collection (Application) */
  0x09, 0x01,   /* Usage (Pointer) */
  0xa1, 0x00,   /* Collection (Physical) */
  0x05, 0x09,     /* Usage Page (Buttons) */
  0x19, 0x01,     /* Usage Minimum (01) */
  0x29, 0x03,     /* Usage Maximum (03) */
  0x15, 0x00,     /* Logical Minimum (0) */
  0x25, 0x01,     /* Logical Maximum (1) */
  0x95, 0x03,     /* Report Count (3), 3 buttons bits */
  0x75, 0x01,     /* Report Size (1) */
  0x81, 0x02,     /* Input (Data, Variable, Absolute) */
  0x95, 0x01,     /* Report Count (1) */
  0x75, 0x05,     /* Report Size (5) */
  0x81, 0x01,     /* Input (Constant), 5 bit padding */
  0x05, 0x01,     /* Usage Page (Generic Desktop) */
  0x09, 0x30,     /* Usage (X) */
  0x09, 0x31,     /* Usage (Y) */
  0x15, 0x81,     /* Logical Minimum (-127) */
  0x25, 0x7f,     /* Logical Maximum (127) */
  0x95, 0x02,     /* Report Count (2), 2 position bytes */
  0x75, 0x08,     /* Report Size (8) */
  0x81, 0x06,     /* Input (Data, Variable, Relative) */
  0xc0,         /* End Collection */
  0xc0        /* End Collection */
};

typedef struct {
  usb_configuration_descriptor_t cnf_descr;
  usb_interface_descriptor_t     if_descr;
  usb_hid_main_descriptor_t      hid_descr;
  usb_endpoint_descriptor_t      ep_descr;
} __packed usb_hid_configuration_t;

#ifndef USB_BM_ATTRIBUTES
  #define USB_BM_ATTRIBUTES  (SELF_POWERED | D7_RESERVED)
#endif
#ifndef USB_B_MAX_POWER
  #define USB_B_MAX_POWER  1
#endif

static usb_hid_configuration_t const hid_configuration = {
  {
    sizeof(usb_configuration_descriptor_t),  /* bLength */
    CONFIGURATION_DESCRIPTOR,                /* bDescriptorType */
    (sizeof(usb_hid_configuration_t)),/* wTotalLength */
    1,                                       /* bNumInterfaces */
    1,                                       /* bConfigurationValue */
    0,                                       /* iConfiguration */
    USB_BM_ATTRIBUTES,                       /* bmAttributes */
    USB_B_MAX_POWER                          /* bMaxPower */
  },
  {
    sizeof(usb_interface_descriptor_t), /* bLength */
    INTERFACE_DESCRIPTOR,               /* bDescriptorType */
    0,                                  /* bInterfaceNumber */
    0,                                  /* bAlternateSetting */
    1,                                  /* bNumEndpoints */
    HUMAN_INTERFACE_DEVICE_CLASS,       /* bInterfaceClass */
    BOOT_INTERFACE_SUBCLASS,            /* bInterfaceSubClass */
    MOUSE_PROTOCOL,                     /* bInterfaceProtocol */
    0                                   /* iInterface */
  },
  {
    sizeof(usb_hid_main_descriptor_t),         /* bLength */
    HID_MAIN_DESCRIPTOR,                       /* bDescriptorType */
    (0x0111),                           /* bcdHID */
    0,                                         /* bCountryCode */
    1,                                         /* bNumDescriptors */
    HID_REPORT_DESCRIPTOR,                     /* bDescriptorType */
    (sizeof(usb_hid_report_descriptor)) /* wDescriptorLength */
  },
  {
    sizeof(usb_endpoint_descriptor_t),        /* bLength */
    ENDPOINT_DESCRIPTOR,                      /* bDescriptorType */
    ENDP3 | ENDP_IN,                          /* bEndpointAddress */
    INTERRUPT_TRANSFER,                       /* bmAttributes */
    (sizeof(hid_mouse_boot_report_t)), /* wMaxPacketSize */
    10                                        /* bInterval */
  }
};

static usb_string_descriptor_t(1) const string_lang = {
  sizeof(usb_string_descriptor_t(1)),
  STRING_DESCRIPTOR,
  {(LANG_US_ENGLISH)}
};

static usb_string_descriptor_t(16) const string_manufacturer = {
  sizeof(usb_string_descriptor_t(16)),
  STRING_DESCRIPTOR,
  {
    ('T'), ('e'), ('o'), ('d'),
    ('o'), ('r'), (' '), ('P'),
    ('o'), ('z'), ('d'), ('r'),
    ('r'), ('a'), ('w'), ('i'), ('a') 
  }
};

static usb_string_descriptor_t(26) const string_product = {
  sizeof(usb_string_descriptor_t(26)),
  STRING_DESCRIPTOR,
  {
    ('U'), ('S'), ('B'), (' '),
    ('j'), ('o'), ('y'), ('s'),
    ('t'), ('i'), ('c'), ('k'),
    ('-'), ('m'), ('o'), ('u'),
    ('s'), ('e'), (' '), ('e'),
    ('x'), ('a'), ('m'), ('p'),
    ('l'), ('e')
  }
};

static usb_string_descriptor_t(10) const string_serial = {
  sizeof(usb_string_descriptor_t(10)),
  STRING_DESCRIPTOR,
  {
    ('0'), ('0'), ('0'), ('0'),
    ('0'), ('0'), ('0'), ('0'),
    ('0'), ('1')
  }
};

typedef struct {
  uint8_t const *data;
  uint16_t      length;
} string_table_t;

static string_table_t const strings[] = {
  {(uint8_t const*)&string_lang,         sizeof string_lang},
  {(uint8_t const*)&string_manufacturer, sizeof string_manufacturer},
  {(uint8_t const*)&string_product,      sizeof string_product},
  {(uint8_t const*)&string_serial,       sizeof string_serial}
};
static uint32_t const stringCount = sizeof(strings)/sizeof(strings[0]);

/** Callbacks **/

static int          Configure(void);
static uint8_t      Reset(usb_speed_t);
static void         SoF(uint16_t frameNumber);
static usb_result_t GetDescriptor(uint16_t, uint16_t,
                                  uint8_t const **, uint16_t *);
static uint8_t      GetConfiguration(void);
static usb_result_t SetConfiguration(uint16_t);
static uint16_t     GetStatus(void);
static usb_result_t ClassNoDataSetup(usb_setup_packet_t const *);
static usb_result_t ClassInDataSetup(usb_setup_packet_t const *,
                                     uint8_t const **, uint16_t *);
static void         EP3IN(void);

static usbd_callback_list_t const ApplicationCallBacks = {
  Configure, Reset, SoF, GetDescriptor, 0, GetConfiguration,
  SetConfiguration, GetStatus, 0, 0, 0, 0, ClassNoDataSetup,
  ClassInDataSetup, 0, 0,
  {0, 0, EP3IN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  0, 0
};

/** Mouse implementation **/

#define DEBOUNCE    5
#define MIN_REPEAT  3
#define MAX_REPEAT  180

static hid_mouse_boot_report_t report;
static uint8_t configuration, protocol, busy;
static int button1, button2, button3, x, y, xrepeat, yrepeat;
static int idle_value, idle_timer;

static void ResetState(void) {
  report.buttons = 0;
  report.x = report.y = 0;
  configuration = 0;
  protocol = 1; /* report rrotocol */
  busy = 0;
  button1 = button2 = button3 = x = y = 0;
  xrepeat = yrepeat = MAX_REPEAT;
  idle_value = idle_timer = -1; /* infinity */
}

usbd_callback_list_t const * USBDgetApplicationCallbacks() {
  return &ApplicationCallBacks;
}

int Configure() {
  ResetState();

  return 0;
}

uint8_t Reset(usb_speed_t speed) {

  ResetState();

  /* Default control endpoint must be configured here. */
  if (USBDendPointConfigure(ENDP0, CONTROL_TRANSFER,
                            device_descriptor.bMaxPacketSize0,
                            device_descriptor.bMaxPacketSize0) !=
                                                      REQUEST_SUCCESS)
  {

  }

  return device_descriptor.bMaxPacketSize0;
}

usb_result_t GetDescriptor(uint16_t wValue, uint16_t wIndex,
                           uint8_t const **data, uint16_t *length) {
  uint32_t index = wValue & 0xff;

  switch (wValue >> 8) {
    case DEVICE_DESCRIPTOR:
      if (index == 0 && wIndex == 0) {
        *data = (uint8_t const *)&device_descriptor;
        *length = sizeof(device_descriptor);
        return REQUEST_SUCCESS;
      }
      return REQUEST_ERROR;
    case CONFIGURATION_DESCRIPTOR:
      if (index == 0 && wIndex == 0) {
        *data = (uint8_t const *)&hid_configuration;
        *length = sizeof(hid_configuration);
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
    case HID_MAIN_DESCRIPTOR: /* Requested when boot up. */
      if (index == 0 && wIndex == 0) {
        *data = (uint8_t const *)&hid_configuration.hid_descr;
        *length = sizeof(hid_configuration.hid_descr);
        return REQUEST_SUCCESS;
      }
      return REQUEST_ERROR;
    case HID_REPORT_DESCRIPTOR:
      if (index == 0 && wIndex == 0) {
        *data = usb_hid_report_descriptor;
        *length = sizeof(usb_hid_report_descriptor);
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

usb_result_t SetConfiguration(uint16_t confValue) {
  if (confValue > device_descriptor.bNumConfigurations)
    return REQUEST_ERROR;

  configuration = confValue;
  USBDdisableAllNonControlEndPoints();
  if (confValue == hid_configuration.cnf_descr.bConfigurationValue)
    return USBDendPointConfigure(ENDP3, INTERRUPT_TRANSFER, 0,
                                 sizeof(hid_mouse_boot_report_t));

  return REQUEST_SUCCESS; /* confValue == 0 */
}

uint16_t GetStatus() {
  /* Current power setting should be reported. */
  if (hid_configuration.cnf_descr.bmAttributes & SELF_POWERED)
    return STATUS_SELF_POWERED;
  else
    return 0;
}

usb_result_t ClassNoDataSetup(usb_setup_packet_t const *setup) {
  if (setup->bmRequestType == (HOST_TO_DEVICE |
                               CLASS_REQUEST |
                               INTERFACE_RECIPIENT)) {
    if (setup->bRequest == SET_IDLE &&
        setup->wIndex == 0 &&
        setup->wLength == 0 &&
        (setup->wValue & 0xff) == 0 /* report ID == 0 */) {
      if (setup->wValue & 0xff00)
        idle_value = idle_timer = (setup->wValue & 0xff00) >> 6;
      else
        idle_value = idle_timer = -1; /* infinity */
      return REQUEST_SUCCESS;
    }
    else if (setup->bRequest == SET_PROTOCOL &&
             setup->wIndex == 0 &&
             setup->wLength == 0) {
      protocol = setup->wValue;
      return REQUEST_SUCCESS;
    }
  }
  return REQUEST_ERROR;
}

usb_result_t ClassInDataSetup(usb_setup_packet_t const *setup,
                              uint8_t const **data,
                              uint16_t *length) {
  if (setup->bmRequestType == (DEVICE_TO_HOST |
                               CLASS_REQUEST |
                               INTERFACE_RECIPIENT)) {
    if (setup->bRequest == GET_REPORT &&
        setup->wIndex == 0 &&
        setup->wValue == 0x0100 /* input report, report ID == 0 */) {
      *data = (uint8_t const *)&report;
      *length = sizeof(report);
      return REQUEST_SUCCESS;
    }
    else if (setup->bRequest == GET_PROTOCOL &&
             setup->wValue == 0 &&
             setup->wIndex == 0 &&
             setup->wLength == 1) {
      *data = &protocol;
      *length = 1;
      return REQUEST_SUCCESS;
    }
  }
  return REQUEST_ERROR;
}

void EP3IN() {
  busy = 0;
}

void SoF(uint16_t frameNumber) {
  unsigned state;

  if (configuration > 0 && busy == 0) {
    /* We assume copy semantics. */
    // report.y = 5;

    USBDwrite(ENDP3, (uint8_t const *)&report, sizeof(report));
    report.x = report.y = 0;
    busy = 1;
    idle_timer = idle_value;
  }
}
