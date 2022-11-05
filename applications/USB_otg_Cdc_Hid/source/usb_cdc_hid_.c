

#include <stdio.h>
#include <usbd_api.h>
#include <usbd_callbacks.h>




#define USE_HID_DEVICE

#define VID 0x0483
#define PID 0x5750

/** Descriptors **/

#define BLK_BUFF_SIZE  MAX_FS_BULK_PACKET_SIZE
#define INT_BUFF_SIZE  8


static hid_mouse_boot_report_t report;
static uint8_t configuration, protocol, busy;
static int idle_value, idle_timer;

static usb_device_descriptor_t const device_descriptor = {
  sizeof(usb_device_descriptor_t), /* bLength */
  DEVICE_DESCRIPTOR,               /* bDescriptorType */
  (0x0200),                        /* bcdUSB */
  0,                               /* bDeviceClass */
  0,                               /* bDeviceSubClass */
  0,                               /* bDeviceProtocol */
  64,                              /* bMaxPacketSize0 */
  (VID),                           /* idVendor */
  (PID + 2),                       /* idProduct */
  (0x0100),                        /* bcdDevice */
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
  usb_configuration_descriptor_t       cnf_descr;
  usb_interface_descriptor_t           if0_descr;
  usb_cdc_header_descriptor_t          cdc_h_descr;
  usb_cdc_call_management_descriptor_t cdc_cm_descr;
  usb_cdc_acm_descriptor_t             cdc_acm_descr;
  usb_cdc_union_descriptor_t           cdc_u_descr;
  usb_endpoint_descriptor_t            ep2in_descr;
  usb_endpoint_descriptor_t            ep1out_descr;
  usb_endpoint_descriptor_t            ep1in_descr;
  /*HID SECTION*/
#ifdef USE_HID_DEVICE
  usb_interface_descriptor_t           hid_if_descr;
  usb_hid_main_descriptor_t            hid_descr;
  usb_endpoint_descriptor_t            hid_ep_descr;
#endif
} __packed usb_com__hid_configuration_t;

#ifndef USB_BM_ATTRIBUTES
  #define USB_BM_ATTRIBUTES  (SELF_POWERED | D7_RESERVED)
#endif
#ifndef USB_B_MAX_POWER
  #define USB_B_MAX_POWER  1
#endif

static usb_com__hid_configuration_t const com_configuration = {
  {
    sizeof(usb_configuration_descriptor_t),   /* bLength */
    CONFIGURATION_DESCRIPTOR,                 /* bDescriptorType */
    (sizeof(usb_com__hid_configuration_t)),        /* wTotalLength */
    2,                                        /* bNumInterfaces */
    1,                                        /* bConfigurationValue */
    0,                                        /* iConfiguration */
    USB_BM_ATTRIBUTES,                        /* bmAttributes */
    USB_B_MAX_POWER                           /* bMaxPower */
  },
  {
    sizeof(usb_interface_descriptor_t),   /* bLength */
    INTERFACE_DESCRIPTOR,               /* bDescriptorType */
    0,                                  /* bInterfaceNumber */
    0,                                  /* bAlternateSetting */
    3,                                  /* bNumEndpoints */
    COMMUNICATION_INTERFACE_CLASS,      /* bInterfaceClass */
    ABSTRACT_CONTROL_MODEL_SUBCLASS,    /* bInterfaceSubClass */
    1,                                  /* bInterfaceProtocol */
    0                                   /* iInterface */
  },
  {
    sizeof(usb_cdc_header_descriptor_t), /* bFunctionLength */
    CS_INTERFACE_DESCRIPTOR,             /* bDescriptorType */
    CDC_HEADER_DESCRIPTOR,               /* bDescriptorSubtype */
    (0x110)                       /* bcdCDC */
  },
  {
    sizeof(usb_cdc_call_management_descriptor_t),/* bFunctionLength */
    CS_INTERFACE_DESCRIPTOR,                     /* bDescriptorType */
    CDC_CALL_MANAGEMENT_DESCRIPTOR,           /* bDescriptorSubtype */
    0,                                        /* bmCapabilities */
    0                                         /* bDataInterface */
  },
  {
    sizeof(usb_cdc_acm_descriptor_t), /* bFunctionLength */
    CS_INTERFACE_DESCRIPTOR,          /* bDescriptorType */
    CDC_ACM_DESCRIPTOR,               /* bDescriptorSubtype */
    2                                 /* bmCapabilities */
  },
  {
    sizeof(usb_cdc_union_descriptor_t), /* bFunctionLength */
    CS_INTERFACE_DESCRIPTOR,            /* bDescriptorType */
    CDC_UNION_DESCRIPTOR,               /* bDescriptorSubtype */
    0,                                  /* bControlInterface */
    0                                   /* bSubordinateInterface0 */
  },
  {
    sizeof(usb_endpoint_descriptor_t), /* bLength */
    ENDPOINT_DESCRIPTOR,               /* bDescriptorType */
    ENDP2 | ENDP_IN,                   /* bEndpointAddress */
    INTERRUPT_TRANSFER,                /* bmAttributes */
    (INT_BUFF_SIZE),            /* wMaxPacketSize */
    10                                  /* bInterval */
  },
  {
    sizeof(usb_endpoint_descriptor_t), /* bLength */
    ENDPOINT_DESCRIPTOR,               /* bDescriptorType */
    ENDP1 | ENDP_OUT,                  /* bEndpointAddress */
    BULK_TRANSFER,                     /* bmAttributes */
    (BLK_BUFF_SIZE),            /* wMaxPacketSize */
    0                                  /* bInterval */
  },
  {
    sizeof(usb_endpoint_descriptor_t), /* bLength */
    ENDPOINT_DESCRIPTOR,               /* bDescriptorType */
    ENDP1 | ENDP_IN,                   /* bEndpointAddress */
    BULK_TRANSFER,                     /* bmAttributes */
    (BLK_BUFF_SIZE),            /* wMaxPacketSize */
    0                                  /* bInterval */
  } //**************************************

  #ifdef USE_HID_DEVICE
  ,{
    sizeof(usb_interface_descriptor_t),         /* bLength */
    INTERFACE_DESCRIPTOR,                       /* bDescriptorType */
    1,                                          /* bInterfaceNumber */
    0,                                          /* bAlternateSetting */
    1,                                          /* bNumEndpoints */
    HUMAN_INTERFACE_DEVICE_CLASS,               /* bInterfaceClass */
    BOOT_INTERFACE_SUBCLASS,                    /* bInterfaceSubClass */
    MOUSE_PROTOCOL,                             /* bInterfaceProtocol */
    0                                           /* iInterface */
  },
  {
    sizeof(usb_hid_main_descriptor_t),         /* bLength */
    HID_MAIN_DESCRIPTOR,                       /* bDescriptorType */
    (0x0111),                                  /* bcdHID */
    0,                                         /* bCountryCode */
    1,                                         /* bNumDescriptors */
    HID_REPORT_DESCRIPTOR,                     /* bDescriptorType */
    (sizeof(usb_hid_report_descriptor))        /* wDescriptorLength */
  },
  {
    sizeof(usb_endpoint_descriptor_t),        /* bLength */
    ENDPOINT_DESCRIPTOR,                      /* bDescriptorType */
    ENDP3 | ENDP_IN,                          /* bEndpointAddress */
    INTERRUPT_TRANSFER,                       /* bmAttributes */
    (sizeof(hid_mouse_boot_report_t)), /* wMaxPacketSize */
    10                                        /* bInterval */
  }
  #endif
};

static usb_string_descriptor_t(1) const string_lang = {
  sizeof(usb_string_descriptor_t(1)),
  STRING_DESCRIPTOR,
  {(LANG_US_ENGLISH)}
};

static usb_string_descriptor_t(17) const string_manufacturer = {
  sizeof(usb_string_descriptor_t(17)),
  STRING_DESCRIPTOR,
  {
    ('T'), ('e'), ('o'), ('d'),
    ('o'), ('r'), (' '), ('P'),
    ('o'), ('z'), ('d'), ('r'),
    ('r'), ('a'), ('w'), ('i'), ('m') 
  }
};

static usb_string_descriptor_t(19) const string_product = {
  sizeof(usb_string_descriptor_t(19)),
  STRING_DESCRIPTOR,
  {
    ('U'), ('S'), ('B'), (' '),
    ('H'), ('I'), ('D'), ('+'),
    ('V'), ('C'), ('P'), (' '),
    ('E'), ('X'), ('A'), ('M'),
    ('P'), ('L'), ('E')
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
static usb_result_t GetDescriptor(uint16_t, uint16_t,
                                  uint8_t const **, uint16_t *);
static uint8_t      GetConfiguration(void);
static usb_result_t SetConfiguration(uint16_t);
static uint16_t     GetStatus(void);
static usb_result_t ClassNoDataSetup(usb_setup_packet_t const *);
static usb_result_t ClassInDataSetup(usb_setup_packet_t const *,
                                     uint8_t const **, uint16_t *);
static usb_result_t ClassOutDataSetup(usb_setup_packet_t const *,
                                      uint8_t **);
static void         ClassStatusIn(usb_setup_packet_t const *);
static void         EP1OUT(void);
static void         EP2IN(void);
/*HID st*/
static void         EP3_HID_IN(void);
static void SoF(uint16_t frameNumber);
/*HID end*/

static usbd_callback_list_t const ApplicationCallBacks = {
  Configure, Reset, SoF, GetDescriptor, 0,
  GetConfiguration, SetConfiguration, GetStatus, 0, 0,
  0, 0,
  ClassNoDataSetup, ClassInDataSetup,
  ClassOutDataSetup, ClassStatusIn,
  {0, EP2IN, EP3_HID_IN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {EP1OUT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  0, 0
};

/** COM implementation **/

static uint16_t ep2queue;
static uint8_t configuration, refresh, rs232state;
static usb_cdc_line_coding_t rs232coding;

/* rs232state bits */
#define DTR  0x01
#define DCD  0x02
#define DSR  0x04
#define RTS  0x08
#define CTS  0x10

static void ResetState(void) {
  ep2queue = 0;
  configuration = 0;
  rs232coding.dwDTERate = 38400;
  rs232coding.bCharFormat = ONE_STOP_BIT;
  rs232coding.bParityType = NO_PARITY;
  rs232coding.bDataBits = 8;
  rs232state = 0;
  refresh = 1;
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
        *data = (uint8_t const *)&com_configuration;
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
      /*HID*/
    case HID_MAIN_DESCRIPTOR: /* Requested when boot up. */
      if (index == 0 && wIndex == 1) {
        *data = (uint8_t const *)&com_configuration.hid_descr;
        *length = sizeof(com_configuration.hid_descr);
        return REQUEST_SUCCESS;
      }
      return REQUEST_ERROR;
    case HID_REPORT_DESCRIPTOR:
      if (index == 0 && wIndex == 1) { /*Interfejs numer 1 to intefejs hidowy */
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
  if (confValue == com_configuration.cnf_descr.bConfigurationValue) {
    usb_result_t r1, r2;

    r1 = USBDendPointConfigure(ENDP1, BULK_TRANSFER,
                               BLK_BUFF_SIZE, BLK_BUFF_SIZE);
    r2 = USBDendPointConfigure(ENDP2, INTERRUPT_TRANSFER,
                               0, INT_BUFF_SIZE);
 #ifdef USE_HID_DEVICE       
    USBDendPointConfigure(ENDP3, INTERRUPT_TRANSFER, 0,
                                 sizeof(hid_mouse_boot_report_t));
#endif

    if (r1 == REQUEST_SUCCESS && r2 == REQUEST_SUCCESS)
      return REQUEST_SUCCESS;
    else
      return REQUEST_ERROR;
  }

  return REQUEST_SUCCESS; /* confValue == 0 */
}

uint16_t GetStatus() {
  /* Current power setting should be reported. */
  if (com_configuration.cnf_descr.bmAttributes & SELF_POWERED)
    return STATUS_SELF_POWERED;
  else
    return 0;
}

static usb_cdc_serial_state_t state = {
  {DEVICE_TO_HOST | CLASS_REQUEST | INTERFACE_RECIPIENT,
   SERIAL_STATE, 0, 0, 2}, 0
};

/*Specyficzne zadania klasy nie wymagajace przeslania danych do hosta...*/
usb_result_t ClassNoDataSetup(usb_setup_packet_t const *setup) {
  
    /*HID st*/
#ifdef USE_HID_DEVICE
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
#endif


  if (setup->bmRequestType == (HOST_TO_DEVICE |
                               CLASS_REQUEST |
                               INTERFACE_RECIPIENT) &&
      setup->bRequest == SET_CONTROL_LINE_STATE &&
      setup->wIndex == 0 &&
      setup->wLength == 0) {
    uint8_t new_rs232state;

    /* Host do device: DTR or RTS notification */
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
        USBDwrite(ENDP2, (uint8_t const *)&state, sizeof(state));
      if (ep2queue < 2)
        ++ep2queue;
    }
  
    /* Set new state. */
    if (rs232state != new_rs232state) {
      rs232state = new_rs232state;
      refresh = 1;
    }

    return REQUEST_SUCCESS;
  }

  /*HID st*/
  return REQUEST_ERROR;
}



/*Funkcja ClassInDataSetup obsługuje niestandardowe żądania 
(specyficzne dla konkretnej klasy lub konkretnego dostawcy) 
i wymagające przesłania danych z urządzenia do kontrolera.*/
usb_result_t ClassInDataSetup(usb_setup_packet_t const *setup,
                              uint8_t const **data,
                              uint16_t *length) {
                              
  if (setup->bmRequestType == (DEVICE_TO_HOST |
                               CLASS_REQUEST |
                               INTERFACE_RECIPIENT) &&
      setup->bRequest == GET_LINE_CODING &&
      setup->wValue == 0 &&
      setup->wIndex == 0) {
    *data = (const uint8_t *)&rs232coding;
    *length = sizeof(rs232coding);
    return REQUEST_SUCCESS;
  }

  /*HID*/
#ifdef USE_HID_DEVICE
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
#endif

  return REQUEST_ERROR;
}


/*Funkcja ClassOutDataSetup obsługuje niestandardowe żądania (specyficzne dla
konkretnej klasy lub konkretnego dostawcy) 
i wymagające przesłania danych z kontrolera do urządzenia*/
usb_result_t ClassOutDataSetup(usb_setup_packet_t const *setup,
                               uint8_t **data) {
  if (setup->bmRequestType == (HOST_TO_DEVICE |
                               CLASS_REQUEST |
                               INTERFACE_RECIPIENT) &&
      setup->bRequest == SET_LINE_CODING &&
      setup->wValue == 0 &&
      setup->wIndex == 0 &&
      setup->wLength == sizeof(rs232coding)) {
    *data = (uint8_t *)&rs232coding;
    return REQUEST_SUCCESS;
  }

  return REQUEST_ERROR;
}




/*zakonczono faze obslugi zadania stasu i znajduja sie w buforze, powiazane z funkcja: ClassOutDataSetup*/
void ClassStatusIn(usb_setup_packet_t const *setup) {
  if (setup->bmRequestType == (HOST_TO_DEVICE |
                               CLASS_REQUEST |
                               INTERFACE_RECIPIENT) &&
      setup->bRequest == SET_LINE_CODING &&
      setup->wValue == 0 &&
      setup->wIndex == 0 &&
      setup->wLength == sizeof(rs232coding)) {
  }
}

/*Wiadomo*/
void EP2IN() {
  if (ep2queue > 0)
    --ep2queue;
  if (ep2queue > 0)
    USBDwrite(ENDP2, (uint8_t const *)&state, sizeof(state));
}
static uint8_t const help[] =

  " Hello Crazy World! \n\r"
  "";


/*Wiadomo wolane jak EP1OUT skonczyl odbierac dane*/
void EP1OUT() {
  uint8_t buffer[BLK_BUFF_SIZE];
  uint16_t i, len;

  len = USBDread(ENDP1, buffer, BLK_BUFF_SIZE);
  for (i = 0; i < len; ++i) {
    switch (buffer[i]) {
      case 'G':

        break;
      case 'g':

        break;
      case ' ':
      case '\n':
      case '\r':
      case '\t':
        break;
      default:
        USBDwriteEx(ENDP1, help, sizeof(help) - 1);
        return;
    }
  }
}

/*HID Section*/
static void EP3_HID_IN(void)
{
  busy = 0;
}

static void SoF(uint16_t frameNumber) {

  if (configuration > 0 && busy == 0) {
    /* We assume copy semantics. */
    USBDwrite(ENDP3, (uint8_t const *)&report, sizeof(report));
    /*TODO: Uncomment this line to see effect!! MOUSE!!!*/
    // report.x = report.y = 1;
    busy = 1;
  }
}