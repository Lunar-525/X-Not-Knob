#ifndef __USBHC_H__
#define __USBHC_H__

#include "UsbHCModel.h"
#include "UsbHCView.h"

namespace Page
{

class UsbHC : public PageBase
{
public:
    UsbHC();
    virtual ~UsbHC();

    virtual void onCustomAttrConfig();
    virtual void onViewLoad();
    virtual void onViewDidLoad();
    virtual void onViewWillAppear();
    virtual void onViewDidAppear();
    virtual void onViewWillDisappear();
    virtual void onViewDidDisappear();
    virtual void onViewDidUnload();

    void UsbHCEventHandler(lv_event_t* event, lv_event_code_t code);

private:
    void Update();
    void AttachEvent(lv_obj_t* obj);
    static void onTimerUpdate(lv_timer_t* timer);
    static void onEvent(lv_event_t* event);

private:
    UsbHCView* View;
    UsbHCModel* Model;
    lv_timer_t* timer;
    uint32_t exitPressStart;  // 长按退出计时起点（直接 GPIO 轮询）
};

}

#endif /* __USBHC_H__ */
