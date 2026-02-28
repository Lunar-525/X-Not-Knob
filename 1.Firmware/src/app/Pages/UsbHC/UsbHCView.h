#ifndef __USBHC_VIEW_H
#define __USBHC_VIEW_H

#include "app/app.h"
#include "../Page.h"
#include "UsbHCModel.h"

namespace Page
{

class UsbHCView
{
public:
    void Create(lv_obj_t* root);
    void Delete();
    void UpdateView(UsbHCInfo* info);

public:
    struct {
        lv_obj_t* meter;
        lv_meter_scale_t* scale_pot;
        lv_meter_indicator_t* nd_img_circle;
        lv_obj_t* label_value;
        lv_obj_t* label_title;
        lv_group_t* group;
    } ui;

    struct {
        lv_style_t meter;
        lv_style_t ticks;
    } style;
};

}

#endif
