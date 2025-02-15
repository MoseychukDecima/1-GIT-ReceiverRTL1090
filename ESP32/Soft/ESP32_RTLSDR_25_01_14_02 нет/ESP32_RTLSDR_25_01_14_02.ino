#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_intr_alloc.h"
#include "usb/usb_host.h"
#include <esp_task_wdt.h>
#include "esp32-hal-psram.h"
#include <elapsedMillis.h>

static const char* TAG_DAEMON = "DAEMON";
static const char* TAG_CLASS = "CLASS";
#define CLIENT_NUM_EVENT_MSG        5

const size_t USB_HID_DESC_SIZE = 9;

bool isRtlsdr = false;
bool isRtlsdrReady = false;
uint8_t RtlsdrInterval;
bool isRtlsdrPolling = false;
elapsedMillis RtlsdrTimer;

const size_t RTLSDR_IN_BUFFER_SIZE = 64;
usb_transfer_t* RtlsdrIn = NULL;




typedef union {
    struct {
        uint8_t bLength;                    /**< ������ ����������� � ������ */
        uint8_t bDescriptorType;            /**< ���������� ���, ����������� ��� ����������� HID. */
        uint16_t bcdHID;                    /**< ����� ������ ������������ USB HID � �������-���������� ���� (��������, 2.10 � ��� 210H) */
        uint8_t bCountryCode;               /**< �������� ���������, ���������������� ��� ������ ��������������� ������������. */
        uint8_t bNumDescriptor;             /**< �������� ���������, ����������� ���������� ������������ ������. */
        uint8_t bHIDDescriptorType;         /**< ��� ���������, ���������������� ��� ����������� ������. ��. ������ 7.1.2: Set_Descriptor ������ ������� �������� ����������� ������. */
        uint16_t wHIDDescriptorLength;      /**< �������� ���������, �������������� ����� ����� ������ ����������� ������. */
        uint8_t bHIDDescriptorTypeOpt;      /**< �������������� ��� ���������, ���������������� ��� ����������� ������. ��. ������ 7.1.2: ������ Set_Descriptor ��� ������� �������� ����������� ������. */
        uint16_t wHIDDescriptorLengthOpt;   /**< �������������� �������� ���������, �������������� ����� ����� ������ ����������� ������. */
    } USB_DESC_ATTR;
    uint8_t val[USB_HID_DESC_SIZE];
} usb_hid_desc_t;

const TickType_t HOST_EVENT_TIMEOUT = 1;
const TickType_t CLIENT_EVENT_TIMEOUT = 1;

usb_host_client_handle_t Client_Handle;
usb_device_handle_t      Device_Handle;
typedef void (*usb_host_enum_cb_t)(const usb_config_desc_t* config_desc);
static usb_host_enum_cb_t _USB_host_enumerate;

void show_dev_desc(const usb_device_desc_t* dev_desc);
void show_config_desc(const void* p);
uint8_t show_interface_desc(const void* p);
void show_endpoint_desc(const void* p);
void show_hid_desc(const void* p);
void show_interface_assoc(const void* p);
//====================================================================================
void _client_event_callback(const usb_host_client_event_msg_t* event_msg, void* arg);
void usbh_setup(usb_host_enum_cb_t enumeration_cb);
//void usbh_task(void);
//====================================================================================


void check_interface_desc_boot_rtlsdr(const void* p)
{
    const usb_intf_desc_t* intf = (const usb_intf_desc_t*)p;

    if ((intf->bInterfaceClass == USB_CLASS_HID) && (intf->bInterfaceSubClass == 1) && (intf->bInterfaceProtocol == 1))
    {
        isRtlsdr = true;
        ESP_LOGI(TAG_DAEMON, "Claiming a boot rtlsdr!");
        esp_err_t err = usb_host_interface_claim(Client_Handle, Device_Handle, intf->bInterfaceNumber, intf->bAlternateSetting);
        if (err != ESP_OK)
        {
            ESP_LOGI(TAG_DAEMON, "usb_host_interface_claim failed: %x", err);
        }
    }
}

//����������� �������� �����
void prepare_endpoint(const void* p)
{
    const usb_ep_desc_t* endpoint = (const usb_ep_desc_t*)p;
    esp_err_t err;

    //// ������ ���� ���������� ��� HID
    //if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_INT)
    //{
    //    ESP_LOGI("", "Not interrupt endpoint: 0x%02x", endpoint->bmAttributes);
    //    return;
    //}

    //if (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK)
    //{
    //    err = usb_host_transfer_alloc(KEYBOARD_IN_BUFFER_SIZE, 0, &KeyboardIn);
    //    if (err != ESP_OK)
    //    {
    //        KeyboardIn = NULL;
    //        ESP_LOGI("", "usb_host_transfer_alloc In fail: %x", err);
    //        return;
    //    }
    //    KeyboardIn->device_handle = Device_Handle;
    //    KeyboardIn->bEndpointAddress = endpoint->bEndpointAddress;
    //    KeyboardIn->callback = keyboard_transfer_cb;
    //    KeyboardIn->context = NULL;
    //    isKeyboardReady = true;
    //    KeyboardInterval = endpoint->bInterval;
    //    ESP_LOGI("", "USB boot keyboard ready");

    //    fprintf(stderr, "transfer->num_bytes %d\n", KEYBOARD_IN_BUFFER_SIZE);
    //    fprintf(stderr, "transfer->device_handle %d\n", KeyboardIn->device_handle);
    //    fprintf(stderr, "transfer->bEndpointAddress %x\n", KeyboardIn->bEndpointAddress);
    //    fprintf(stderr, "transfer->callback %d\n", KeyboardIn->callback);
    //    fprintf(stderr, "transfer->context %d\n", KeyboardIn->context);
    //    fprintf(stderr, "transfer->timeout_ms %d\n", KeyboardInterval);
    //}
    //else
    //{
    //    ESP_LOGI("", "Ignoring interrupt Out endpoint");
    //}
}









void show_config_desc_full(const usb_config_desc_t* config_desc)
{
    // Full decode of config desc.
    const uint8_t* p = &config_desc->val[0];
    static uint8_t USB_Class = 0;
    uint8_t bLength;
    for (int i = 0; i < config_desc->wTotalLength; i += bLength, p += bLength) {
        bLength = *p;
        if ((i + bLength) <= config_desc->wTotalLength) {
            const uint8_t bDescriptorType = *(p + 1);
            switch (bDescriptorType) {
            case USB_B_DESCRIPTOR_TYPE_DEVICE:
                ESP_LOGI("", "USB Device Descriptor should not appear in config");
                break;
            case USB_B_DESCRIPTOR_TYPE_CONFIGURATION:
                show_config_desc(p);
                break;
            case USB_B_DESCRIPTOR_TYPE_STRING:
                ESP_LOGI("", "USB string desc TBD");
                break;
            case USB_B_DESCRIPTOR_TYPE_INTERFACE:
                USB_Class = show_interface_desc(p);
                check_interface_desc_boot_rtlsdr(p);
                break;
            case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
                show_endpoint_desc(p);
               //!! if (isKeyboard && KeyboardIn == NULL) prepare_endpoint(p);
                break;
            case USB_B_DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
                // Should not be config config?
                ESP_LOGI("", "USB device qual desc TBD");
                break;
            case USB_B_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION:
                // Should not be config config?
                ESP_LOGI("", "USB Other Speed TBD");
                break;
            case USB_B_DESCRIPTOR_TYPE_INTERFACE_POWER:
                // Should not be config config?
                ESP_LOGI("", "USB Interface Power TBD");
                break;
            case 0x21:
                if (USB_Class == USB_CLASS_HID) {
                    show_hid_desc(p);
                }
                break;
            default:
                ESP_LOGI("", "Unknown USB Descriptor Type: 0x%x", bDescriptorType);
                break;
            }
        }
        else {
            ESP_LOGI("", "USB Descriptor invalid");
            return;
        }
    }
}

void logMemory() 
{
    log_d("Total heap: %d", ESP.getHeapSize());
    log_d("Free heap: %d", ESP.getFreeHeap());
    log_d("Total PSRAM: %d", ESP.getPsramSize());
    log_d("Free PSRAM: %d", ESP.getFreePsram());
    log_d("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
    log_d("spiram size %u", esp_spiram_get_size());
    log_d("FlashChipSize %u\n", ESP.getFlashChipSize());

}

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 1000);

    esp_log_level_set("*", ESP_LOG_VERBOSE);  // ������� ���������� ���������

    String ver_soft = __FILE__; 
    int val_srt = ver_soft.lastIndexOf('\\');
    ver_soft.remove(0, val_srt + 1);
    val_srt = ver_soft.lastIndexOf('.');
    ver_soft.remove(val_srt);
    Serial.println("\n*** Version " + ver_soft + "\n");
    if (psramInit()) 
    {
        Serial.println("PSRAM is correctly initialized\n");
    }
    else 
    {
        Serial.println("PSRAM not available");
    }

  //  logMemory();

    usbh_setup(show_config_desc_full);

}


void loop()
{

}



//=============================================================================

void show_dev_desc(const usb_device_desc_t* dev_desc)
{
    ESP_LOGI("", "bLength: %d", dev_desc->bLength);
    ESP_LOGI("", "bDescriptorType(device): %d", dev_desc->bDescriptorType);
    ESP_LOGI("", "bcdUSB: 0x%x", dev_desc->bcdUSB);
    ESP_LOGI("", "bDeviceClass: 0x%02x", dev_desc->bDeviceClass);
    ESP_LOGI("", "bDeviceSubClass: 0x%02x", dev_desc->bDeviceSubClass);
    ESP_LOGI("", "bDeviceProtocol: 0x%02x", dev_desc->bDeviceProtocol);
    ESP_LOGI("", "bMaxPacketSize0: %d", dev_desc->bMaxPacketSize0);
    ESP_LOGI("", "idVendor: 0x%x", dev_desc->idVendor);
    ESP_LOGI("", "idProduct: 0x%x", dev_desc->idProduct);
    ESP_LOGI("", "bcdDevice: 0x%x", dev_desc->bcdDevice);
    ESP_LOGI("", "iManufacturer: %d", dev_desc->iManufacturer);
    ESP_LOGI("", "iProduct: %d", dev_desc->iProduct);
    ESP_LOGI("", "iSerialNumber: %d", dev_desc->iSerialNumber);
    ESP_LOGI("", "bNumConfigurations: %d", dev_desc->bNumConfigurations);
}

void show_config_desc(const void* p)
{
    const usb_config_desc_t* config_desc = (const usb_config_desc_t*)p;

    ESP_LOGI("", "bLength: %d", config_desc->bLength);
    ESP_LOGI("", "bDescriptorType(config): %d", config_desc->bDescriptorType);
    ESP_LOGI("", "wTotalLength: %d", config_desc->wTotalLength);
    ESP_LOGI("", "bNumInterfaces: %d", config_desc->bNumInterfaces);
    ESP_LOGI("", "bConfigurationValue: %d", config_desc->bConfigurationValue);
    ESP_LOGI("", "iConfiguration: %d", config_desc->iConfiguration);
    ESP_LOGI("", "bmAttributes(%s%s%s): 0x%02x",
        (config_desc->bmAttributes & USB_BM_ATTRIBUTES_SELFPOWER) ? "Self Powered" : "",
        (config_desc->bmAttributes & USB_BM_ATTRIBUTES_WAKEUP) ? ", Remote Wakeup" : "",
        (config_desc->bmAttributes & USB_BM_ATTRIBUTES_BATTERY) ? ", Battery Powered" : "",
        config_desc->bmAttributes);
    ESP_LOGI("", "bMaxPower: %d = %d mA", config_desc->bMaxPower, config_desc->bMaxPower * 2);
}

uint8_t show_interface_desc(const void* p)
{
    const usb_intf_desc_t* intf = (const usb_intf_desc_t*)p;

    ESP_LOGI("", "bLength: %d", intf->bLength);
    ESP_LOGI("", "bDescriptorType (interface): %d", intf->bDescriptorType);
    ESP_LOGI("", "bInterfaceNumber: %d", intf->bInterfaceNumber);
    ESP_LOGI("", "bAlternateSetting: %d", intf->bAlternateSetting);
    ESP_LOGI("", "bNumEndpoints: %d", intf->bNumEndpoints);
    ESP_LOGI("", "bInterfaceClass: 0x%02x", intf->bInterfaceClass);
    ESP_LOGI("", "bInterfaceSubClass: 0x%02x", intf->bInterfaceSubClass);
    ESP_LOGI("", "bInterfaceProtocol: 0x%02x", intf->bInterfaceProtocol);
    ESP_LOGI("", "iInterface: %d", intf->iInterface);
    return intf->bInterfaceClass;
}

void show_endpoint_desc(const void* p)
{
    const usb_ep_desc_t* endpoint = (const usb_ep_desc_t*)p;
    const char* XFER_TYPE_NAMES[] = {
      "Control", "Isochronous", "Bulk", "Interrupt"
    };
    ESP_LOGI("", "bLength: %d", endpoint->bLength);
    ESP_LOGI("", "bDescriptorType (endpoint): %d", endpoint->bDescriptorType);
    ESP_LOGI("", "bEndpointAddress(%s): 0x%02x",
        (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) ? "In" : "Out",
        endpoint->bEndpointAddress);
    ESP_LOGI("", "bmAttributes(%s): 0x%02x",
        XFER_TYPE_NAMES[endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK],
        endpoint->bmAttributes);
    ESP_LOGI("", "wMaxPacketSize: %d", endpoint->wMaxPacketSize);
    ESP_LOGI("", "bInterval: %d", endpoint->bInterval);
}

void show_hid_desc(const void* p)
{
    usb_hid_desc_t* hid = (usb_hid_desc_t*)p;
    ESP_LOGI("", "bLength: %d", hid->bLength);
    ESP_LOGI("", "bDescriptorType (HID): %d", hid->bDescriptorType);
    ESP_LOGI("", "bcdHID: 0x%04x", hid->bcdHID);
    ESP_LOGI("", "bCountryCode: %d", hid->bCountryCode);
    ESP_LOGI("", "bNumDescriptor: %d", hid->bNumDescriptor);
    ESP_LOGI("", "bDescriptorType: %d", hid->bHIDDescriptorType);
    ESP_LOGI("", "wDescriptorLength: %d", hid->wHIDDescriptorLength);
    if (hid->bNumDescriptor > 1) {
        ESP_LOGI("", "bDescriptorTypeOpt: %d", hid->bHIDDescriptorTypeOpt);
        ESP_LOGI("", "wDescriptorLengthOpt: %d", hid->wHIDDescriptorLengthOpt);
    }
}

void show_interface_assoc(const void* p)
{
    usb_iad_desc_t* iad = (usb_iad_desc_t*)p;
    ESP_LOGI("", "bLength: %d", iad->bLength);
    ESP_LOGI("", "bDescriptorType: %d", iad->bDescriptorType);
    ESP_LOGI("", "bFirstInterface: %d", iad->bFirstInterface);
    ESP_LOGI("", "bInterfaceCount: %d", iad->bInterfaceCount);
    ESP_LOGI("", "bFunctionClass: 0x%02x", iad->bFunctionClass);
    ESP_LOGI("", "bFunctionSubClass: 0x%02x", iad->bFunctionSubClass);
    ESP_LOGI("", "bFunctionProtocol: 0x%02x", iad->bFunctionProtocol);
    ESP_LOGI("", "iFunction: %d", iad->iFunction);
}

//====================================================================================

void _client_event_callback(const usb_host_client_event_msg_t* event_msg, void* arg)
{
    esp_err_t err;
    switch (event_msg->event)
    {
        /**< A new device has been enumerated and added to the USB Host Library */
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
        ESP_LOGI("", "New device address: %d", event_msg->new_dev.address);
        err = usb_host_device_open(Client_Handle, event_msg->new_dev.address, &Device_Handle);
        if (err != ESP_OK) ESP_LOGI("", "usb_host_device_open: %x", err);

        usb_device_info_t dev_info;
        err = usb_host_device_info(Device_Handle, &dev_info);
        if (err != ESP_OK) ESP_LOGI("", "usb_host_device_info: %x", err);
        ESP_LOGI("", "speed: %d dev_addr %d vMaxPacketSize0 %d bConfigurationValue %d",
            dev_info.speed, dev_info.dev_addr, dev_info.bMaxPacketSize0,
            dev_info.bConfigurationValue);

        const usb_device_desc_t* dev_desc;
        err = usb_host_get_device_descriptor(Device_Handle, &dev_desc);
        if (err != ESP_OK) ESP_LOGI("", "usb_host_get_device_desc: %x", err);
        show_dev_desc(dev_desc);

        const usb_config_desc_t* config_desc;
        err = usb_host_get_active_config_descriptor(Device_Handle, &config_desc);
        if (err != ESP_OK) ESP_LOGI("", "usb_host_get_config_desc: %x", err);
        (*_USB_host_enumerate)(config_desc);
        break;
        /**< A device opened by the client is now gone */
    case USB_HOST_CLIENT_EVENT_DEV_GONE:
        ESP_LOGI("", "Device Gone handle: %x", event_msg->dev_gone.dev_hdl);
        break;
    default:
        ESP_LOGI("", "Unknown value %d", event_msg->event);
        break;
    }
}


void usbh_setup(usb_host_enum_cb_t enumeration_cb)
{
    ESP_LOGI(TAG_DAEMON, "Installing USB Host Library");
    const usb_host_config_t config = {
      .skip_phy_setup = false,
      .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };

    esp_err_t err = usb_host_install(&config);
    ESP_LOGI(TAG_DAEMON, "usb_host_install: %x", err);

    const usb_host_client_config_t client_config = {
      .is_synchronous = false,
      .max_num_event_msg = CLIENT_NUM_EVENT_MSG,
      .async = {
          .client_event_callback = _client_event_callback,
          .callback_arg = Client_Handle
      }
    };
    err = usb_host_client_register(&client_config, &Client_Handle);
    ESP_LOGI(TAG_DAEMON, "usb_host_client_register: %x", err);

    _USB_host_enumerate = enumeration_cb;
}
