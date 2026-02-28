#ifndef __USB_HC_H__
#define __USB_HC_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 初始化 USB HID+CDC 复合设备（仅首次调用生效）
void usb_hc_init(void);

// 启用/禁用 USB H/C 模式（控制 HID 上报和 CDC 力矩接收）
void usb_hc_set_active(bool active);

// 获取 PC 通过 CDC 发送的目标力矩值（由 Core 1 FOC task 调用）
float usb_hc_get_target_torque(void);

// 由 Core 1 FOC task 调用，写入最新的电机状态供 Core 0 USB task 上报
void usb_hc_update_state(float angle, float velocity);

// 获取当前共享状态（供 UI 显示用）
float usb_hc_get_angle(void);
float usb_hc_get_velocity(void);

#ifdef __cplusplus
}
#endif

#endif /* __USB_HC_H__ */
