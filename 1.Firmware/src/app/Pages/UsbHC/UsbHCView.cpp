#include "UsbHCView.h"

using namespace Page;

void UsbHCView::Create(lv_obj_t* root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // Meter (圆盘)
    ui.meter = lv_meter_create(root);
    lv_obj_set_pos(ui.meter, 1, 0);
    lv_obj_set_size(ui.meter, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(ui.meter, LV_SCROLLBAR_MODE_OFF);

    // 圆点刻度
    ui.scale_pot = lv_meter_add_scale(ui.meter);
    lv_meter_set_scale_ticks(ui.meter, ui.scale_pot, 41, 2, 0, lv_color_make(0xff, 0x00, 0x00));
    lv_meter_set_scale_range(ui.meter, ui.scale_pot, 0, 360, 360, 270);

    // 蓝色圆点指示器
    ui.nd_img_circle = lv_meter_add_needle_img(ui.meter, ui.scale_pot, Resource.GetImage("dot_blue"), -100, 8);
    lv_meter_set_indicator_value(ui.meter, ui.nd_img_circle, 0);

    // Meter 样式
    lv_style_init(&style.meter);
    lv_style_set_radius(&style.meter, 240);
    lv_style_set_bg_color(&style.meter, lv_color_black());
    lv_style_set_bg_opa(&style.meter, 255);
    lv_style_set_border_width(&style.meter, 0);
    lv_obj_add_style(ui.meter, &style.meter, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_style_init(&style.ticks);
    lv_style_set_text_color(&style.ticks, lv_color_white());
    lv_style_set_text_font(&style.ticks, &lv_font_montserrat_14);
    lv_obj_add_style(ui.meter, &style.ticks, LV_PART_TICKS | LV_STATE_DEFAULT);

    // 标题标签 "USB H/C"
    ui.label_title = lv_label_create(root);
    lv_obj_set_pos(ui.label_title, 70, 40);
    lv_obj_set_size(ui.label_title, 100, 20);
    lv_label_set_text(ui.label_title, "USB H/C");

    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_bg_opa(&title_style, 0);
    lv_style_set_text_color(&title_style, lv_color_make(0x99, 0x99, 0x99));
    lv_style_set_text_font(&title_style, &lv_font_montserrat_14);
    lv_style_set_text_align(&title_style, LV_TEXT_ALIGN_CENTER);
    lv_obj_add_style(ui.label_title, &title_style, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 数值标签（显示力矩）
    ui.label_value = lv_label_create(root);
    lv_obj_set_pos(ui.label_value, 55, 67);
    lv_obj_set_size(ui.label_value, 130, 32);
    lv_obj_set_scrollbar_mode(ui.label_value, LV_SCROLLBAR_MODE_OFF);
    lv_label_set_text(ui.label_value, "T: 0.00");
    lv_label_set_long_mode(ui.label_value, LV_LABEL_LONG_WRAP);

    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_bg_opa(&label_style, 0);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_style_set_text_font(&label_style, &lv_font_montserrat_26);
    lv_style_set_text_align(&label_style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_pad_top(&label_style, 8);
    lv_obj_add_style(ui.label_value, &label_style, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 输入组
    ui.group = lv_group_create();
    lv_indev_set_group(lv_get_indev(LV_INDEV_TYPE_ENCODER), ui.group);
    lv_group_add_obj(ui.group, ui.meter);
    lv_group_focus_obj(ui.meter);
}

void UsbHCView::UpdateView(UsbHCInfo* info)
{
    // 更新蓝色圆点位置（角度取模 360°）
    float angle_deg = info->angle * 180.0f / 3.14159265f;
    int32_t display_angle = ((int32_t)angle_deg) % 360;
    if (display_angle < 0) display_angle += 360;
    lv_meter_set_indicator_value(ui.meter, ui.nd_img_circle, display_angle);

    // 更新力矩标签
    lv_label_set_text_fmt(ui.label_value, "T: %.2f", info->torque);
}

void UsbHCView::Delete()
{
    lv_group_del(ui.group);
    lv_style_reset(&style.meter);
    lv_style_reset(&style.ticks);
}
