#ifndef __USBHC_MODEL_H
#define __USBHC_MODEL_H

#include "app/Utils/AccountSystem/Account.h"
#include "lvgl.h"
#include "hal/hal.h"
#include "hal/motor.h"

typedef struct {
    float angle;
    float velocity;
    float torque;
} UsbHCInfo;

namespace Page
{

class UsbHCModel
{
public:
    void Init();
    void Deinit();
    void GetStatus(UsbHCInfo* info);
    void ChangeMotorMode(int mode);
private:
    Account* account;
};

}

#endif
