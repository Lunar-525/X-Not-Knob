#include "UsbHC.h"
#include <Arduino.h>
#include "hal/usb_hc.h"
#include "hal/motor.h"

using namespace Page;

UsbHC::UsbHC()
{
}

UsbHC::~UsbHC()
{
}

void UsbHC::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
}

void UsbHC::onViewLoad()
{
    Model = new UsbHCModel();
    View = new UsbHCView();

    Model->Init();
    View->Create(root);

    AttachEvent(root);
    AttachEvent(View->ui.meter);

    // 首次进入时初始化 USB HID+CDC（USB.begin 仅一次）
    usb_hc_init();
}

void UsbHC::onViewDidLoad()
{
}

void UsbHC::onViewWillAppear()
{
    // 切换电机到 USB 透传模式
    Model->ChangeMotorMode(MOTOR_USB_PASSTHROUGH);
    usb_hc_set_active(true);

    exitPressStart = 0;
    timer = lv_timer_create(onTimerUpdate, 50, this);  // 50ms 刷新 UI（UI 不需要太快）
}

void UsbHC::onViewDidAppear()
{
}

void UsbHC::onViewWillDisappear()
{
    // 退出时恢复电机默认模式，关闭 USB H/C
    usb_hc_set_active(false);
    Model->ChangeMotorMode(MOTOR_UNBOUND_COARSE_DETENTS);
}

void UsbHC::onViewDidDisappear()
{
    lv_timer_del(timer);
}

void UsbHC::onViewDidUnload()
{
    View->Delete();
    Model->Deinit();
    delete View;
    delete Model;
}

void UsbHC::AttachEvent(lv_obj_t* obj)
{
    lv_obj_set_user_data(obj, this);
    lv_obj_add_event_cb(obj, onEvent, LV_EVENT_ALL, this);
}

void UsbHC::UsbHCEventHandler(lv_event_t* event, lv_event_code_t code)
{
    // 注意：绝对不能用 LV_EVENT_PRESSED 退出！
    // 因为菜单页面按下按钮进入本页面时，同一个 press 事件会传递到本页面，
    // 导致立刻 Pop，形成 push/pop 死循环，两核同时操作 motor 导致失控。
    if (code == LV_EVENT_LONG_PRESSED) {
        // 长按 2 秒返回菜单
        printf("UsbHC: LV_EVENT_LONG_PRESSED - returning to menu\n");
        Manager->Pop();
    }
}

void UsbHC::onEvent(lv_event_t* event)
{
    lv_obj_t* obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);
    auto* instance = (UsbHC*)lv_obj_get_user_data(obj);
    if (instance == nullptr) return;
    instance->UsbHCEventHandler(event, code);
}

void UsbHC::onTimerUpdate(lv_timer_t* timer)
{
    UsbHC* instance = (UsbHC*)timer->user_data;
    instance->Update();
}

void UsbHC::Update()
{
    // 直接轮询按钮 GPIO 实现长按退出（不依赖 LVGL 事件系统）
    if (HAL::encoder_is_pushed()) {
        if (exitPressStart == 0) {
            exitPressStart = millis();
        } else if (millis() - exitPressStart > 2000) {
            printf("UsbHC: Button held >2s (GPIO poll) - returning to menu\n");
            exitPressStart = 0;
            Manager->Pop();
            return;
        }
    } else {
        exitPressStart = 0;
    }

    UsbHCInfo info;
    Model->GetStatus(&info);
    info.angle = usb_hc_get_angle();
    info.velocity = usb_hc_get_velocity();
    View->UpdateView(&info);
}
