#include "UsbHCModel.h"
#include "app/Accounts/Account_Master.h"
#include "hal/usb_hc.h"

using namespace Page;

void UsbHCModel::Init()
{
    account = new Account("UsbHCModel", AccountSystem::Broker(), 0, this);
    account->Subscribe("Motor");
}

void UsbHCModel::Deinit()
{
    if (account) {
        delete account;
        account = nullptr;
    }
}

void UsbHCModel::GetStatus(UsbHCInfo* info)
{
    info->torque = usb_hc_get_target_torque();
}

void UsbHCModel::ChangeMotorMode(int mode)
{
    AccountSystem::Motor_Info_t motor_info;
    motor_info.cmd = AccountSystem::MOTOR_CMD_CHANGE_MODE;
    motor_info.motor_mode = mode;
    motor_info.init_position = 0;
    account->Notify("Motor", &motor_info, sizeof(motor_info));
}
