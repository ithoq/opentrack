#include "vjoystick.h"
#include "api/plugin-api.hpp"
#include "compat/util.hpp"

#include <cstring>
#include <QDebug>

#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

// required for api headers
#include <windows.h>

#undef PPJOY_MODE
#include <public.h>
#include <vjoyinterface.h>

#define OPENTRACK_VJOYSTICK_ID 1

const unsigned char handle::axis_ids[6] =
{
    HID_USAGE_X,
    HID_USAGE_Y,
    HID_USAGE_Z,
    HID_USAGE_RX,
    HID_USAGE_RY,
    HID_USAGE_RZ,
//    HID_USAGE_SL0,
//    HID_USAGE_SL1,
//    HID_USAGE_WHL,
};

constexpr double handle::val_minmax[6];

void handle::init()
{
#if 0
    bool ret = true;

    for (unsigned i = 0; i < axis_count; i++)
    {
        ret &= GetVJDAxisExist(OPENTRACK_VJOYSTICK_ID, axis_ids[i]);
        if (!ret) { qDebug() << "axis" << i << "doesn't exist"; break; }
        ret &= GetVJDAxisMin(OPENTRACK_VJOYSTICK_ID, axis_ids[i], &axis_min[i]);
        if (!ret) { qDebug() << "axis" << i << "can't get min value"; break; };
        ret &= GetVJDAxisMax(OPENTRACK_VJOYSTICK_ID, axis_ids[i], &axis_max[i]);
        if (!ret) { qDebug() << "axis" << i << "can't get max value"; break; };
    }

    if (!ret)
    {
        (void) RelinquishVJD(OPENTRACK_VJOYSTICK_ID);
        joy_state = state_fail;
    }
    else
        (void) ResetVJD(OPENTRACK_VJOYSTICK_ID);
#else
    (void) ResetVJD(OPENTRACK_VJOYSTICK_ID);
#endif
}

handle::handle()
{
    const bool ret = AcquireVJD(OPENTRACK_VJOYSTICK_ID);
    if (!ret)
    {
        if (!isVJDExists(OPENTRACK_VJOYSTICK_ID))
            joy_state = state_notent;
        else
            joy_state = state_fail;
    }
    else
    {
        joy_state = state_success;
        init();
    }
}

handle::~handle()
{
    if (joy_state == state_success)
    {
        (void) RelinquishVJD(OPENTRACK_VJOYSTICK_ID);
        joy_state = state_fail;
    }
}

LONG handle::to_axis_value(unsigned axis_id, double val)
{
    const double minmax = val_minmax[axis_id];
    const double min = axis_min[axis_id];
    const double max = axis_max[axis_id];

    return LONG(clamp((val+minmax) * max / (2*minmax) - min, min, max));
}

vjoystick_proto::vjoystick_proto()
{
    if (h.get_state() != state_success)
    {
        QMessageBox msgbox;
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText("vjoystick driver missing");
        msgbox.setInformativeText("vjoystick won't work without the driver installed.");

        QPushButton* driver_button = msgbox.addButton("Download the driver", QMessageBox::ActionRole);
        QPushButton* project_site_button = msgbox.addButton("Visit project site", QMessageBox::ActionRole);
        msgbox.addButton(QMessageBox::Close);

        (void) msgbox.exec();

        if (msgbox.clickedButton() == driver_button)
        {
            static const char* download_driver_url = "https://sourceforge.net/projects/vjoystick/files/latest/download";
            QDesktopServices::openUrl(QUrl(download_driver_url, QUrl::StrictMode));
        }
        else if (msgbox.clickedButton() == project_site_button)
        {
            static const char* project_site_url = "http://vjoystick.sourceforge.net/site/";
            QDesktopServices::openUrl(QUrl(project_site_url, QUrl::StrictMode));
        }
    }
}

vjoystick_proto::~vjoystick_proto()
{
}

void vjoystick_proto::pose(const double *pose)
{
    if (h.get_state() != state_success)
        return;

    for (unsigned i = 0; i < handle::axis_count; i++)
    {
        SetAxis(h.to_axis_value(i, pose[i]), OPENTRACK_VJOYSTICK_ID, handle::axis_ids[i]);
    }
}

OPENTRACK_DECLARE_PROTOCOL(vjoystick_proto, vjoystick_dialog, vjoystick_metadata)
