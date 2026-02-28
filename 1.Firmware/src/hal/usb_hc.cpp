#include "usb_hc.h"
#include <Arduino.h>
#include "config.h"

#if !ARDUINO_USB_MODE
#include "USB.h"
#include "USBHID.h"
#include "USBCDC.h"

// ============================================================
// HID Report: 8 bytes = float angle + float velocity
// ============================================================

#pragma pack(push, 1)
typedef struct {
    float angle;       // motor.shaft_angle (rad, 累积绝对值)
    float velocity;    // motor.shaft_velocity (rad/s)
} UsbHCReport_t;
#pragma pack(pop)

// Vendor-Defined HID Report Descriptor: 8 bytes input report
static const uint8_t hid_report_desc[] = {
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,        // Usage (Vendor Usage 1)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Vendor Usage 1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0xC0               // End Collection
};

// ============================================================
// Custom HID Device class
// ============================================================

class UsbHCHIDDevice : public USBHIDDevice {
public:
    UsbHCHIDDevice() {}

    void begin(USBHID* hid) {
        _hid = hid;
        hid->addDevice(this, sizeof(hid_report_desc));
    }

    uint16_t _onGetDescriptor(uint8_t* dst) override {
        memcpy(dst, hid_report_desc, sizeof(hid_report_desc));
        return sizeof(hid_report_desc);
    }

    bool sendReport(const UsbHCReport_t* report) {
        if (_hid == nullptr) return false;
        return _hid->SendReport(0, (const uint8_t*)report, sizeof(UsbHCReport_t));
    }

private:
    USBHID* _hid = nullptr;
};

// ============================================================
// Shared state (Core 1 writes, Core 0 reads)
// ============================================================

static volatile float usb_hc_shared_angle = 0.0f;
static volatile float usb_hc_shared_velocity = 0.0f;

// CDC → FOC: PC 发送的目标力矩
static volatile float usb_target_torque = 0.0f;

// 是否激活 USB H/C 模式
static volatile bool usb_hc_active_flag = false;

// USB 是否已初始化
static bool usb_hc_initialized = false;

// ============================================================
// Watchdog: CDC 数据超时自动归零力矩
// ============================================================
#define USB_HC_WATCHDOG_TIMEOUT_MS  100
static volatile uint32_t usb_last_rx_millis = 0;

// ============================================================
// USB objects
// ============================================================

static USBHID HID;
static USBCDC USBSerialHC;
static UsbHCHIDDevice hid_device;
static TaskHandle_t handleTaskUsbHC = nullptr;

// ============================================================
// CDC RX: 接收 4 字节裸 float 力矩
// ============================================================

static uint8_t cdc_rx_buf[4];
static volatile uint8_t cdc_rx_pos = 0;

static void onCdcRxData(void) {
    while (USBSerialHC.available()) {
        uint8_t b = USBSerialHC.read();
        cdc_rx_buf[cdc_rx_pos++] = b;
        if (cdc_rx_pos >= 4) {
            float torque;
            memcpy(&torque, cdc_rx_buf, 4);
            usb_target_torque = torque;
            usb_last_rx_millis = millis();  // 喂狗：更新最后收数据时间
            cdc_rx_pos = 0;
        }
    }
}

static void onCdcEvent(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == ARDUINO_USB_CDC_EVENTS) {
        switch (event_id) {
            case ARDUINO_USB_CDC_RX_EVENT:
                if (usb_hc_active_flag) {
                    onCdcRxData();
                }
                break;
            case ARDUINO_USB_CDC_DISCONNECTED_EVENT:
                // CDC 断连：立即归零力矩（不等 watchdog 超时）
                usb_target_torque = 0.0f;
                cdc_rx_pos = 0;
                break;
            default:
                break;
        }
    }
}

// ============================================================
// USB HID 上报任务 (Core 0)
// ============================================================

static void TaskUsbHCReport(void* pvParameters) {
    while (1) {
        if (usb_hc_active_flag) {
            UsbHCReport_t report;
            report.angle = usb_hc_shared_angle;
            report.velocity = usb_hc_shared_velocity;
            hid_device.sendReport(&report);
        }
        vTaskDelay(1);  // 1ms
    }
}

// ============================================================
// Public API
// ============================================================

void usb_hc_init(void) {
    if (usb_hc_initialized) return;
    usb_hc_initialized = true;

    // 设置 HID 轮询间隔 bInterval=1 (1ms)
    HID.begin();
    hid_device.begin(&HID);

    USBSerialHC.onEvent(onCdcEvent);
    USBSerialHC.begin();

    USB.begin();

    // 在 Core 0 上创建 USB 上报任务
    xTaskCreatePinnedToCore(
        TaskUsbHCReport,
        "UsbHCReport",
        2048,
        nullptr,
        1,
        &handleTaskUsbHC,
        LVGL_RUNNING_CORE  // Core 0
    );

    Serial.println("USB H/C initialized (HID + CDC composite)");
}

void usb_hc_set_active(bool active) {
    usb_hc_active_flag = active;
    if (active) {
        usb_last_rx_millis = millis();  // 重置 watchdog，避免进入页面立刻误判超时
    } else {
        usb_target_torque = 0.0f;  // 退出时清零力矩
    }
}

float usb_hc_get_target_torque(void) {
    // Watchdog: 超过 100ms 未收到 CDC 数据 → 强制归零力矩
    if (usb_hc_active_flag &&
        (millis() - usb_last_rx_millis > USB_HC_WATCHDOG_TIMEOUT_MS)) {
        usb_target_torque = 0.0f;
    }
    return usb_target_torque;
}

void usb_hc_update_state(float angle, float velocity) {
    usb_hc_shared_angle = angle;
    usb_hc_shared_velocity = velocity;
}

float usb_hc_get_angle(void) {
    return usb_hc_shared_angle;
}

float usb_hc_get_velocity(void) {
    return usb_hc_shared_velocity;
}

#else
// ARDUINO_USB_MODE != 0: USB HID not available on this config
void usb_hc_init(void) {}
void usb_hc_set_active(bool active) { (void)active; }
float usb_hc_get_target_torque(void) { return 0.0f; }
void usb_hc_update_state(float angle, float velocity) { (void)angle; (void)velocity; }
float usb_hc_get_angle(void) { return 0.0f; }
float usb_hc_get_velocity(void) { return 0.0f; }
#endif /* !ARDUINO_USB_MODE */
