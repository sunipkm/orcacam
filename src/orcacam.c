#include "orcacam.h"
#include <stdlib.h>

static int orca_init = 0;

DCAMERR orca_list_devices(int32 *count, int32 sz_initopt, const int32 *initopt)
{
    if (orca_init == 0)
    {
        atexit(dcamapi_uninit);
        orca_init = 1;
    }
    assert(count);
    *count = 0;
    ORCA_PTR_INIT(DCAMAPI_INIT, init);
    init.initoptionbytes = initopt ? sz_initopt : 0;
    init.initoption      = initopt;
    DCAMERR err          = dcamapi_init(&init);
    if (dcamerr_failed(err))
    {
        *count = 0;
        return err;
    }
}

DCAMERR orca_open_device(int32 idx, HDCAM *hdcam)
{
    assert(hdcam);
    ORCA_PTR_INIT(DCAMDEV_OPEN, open);
    open.index  = idx;
    DCAMERR err = dcamdev_open(&open);
    if (dcamerr_failed(err))
    {
        *hdcam = NULL;
        return err;
    }
    *hdcam = open.hdcam;
    return err;
}

DCAMERR orca_device_info(HDCAM cam, ORCA_CAM_INFO *info)
{
    assert(info);
    DCAMERR err;
    ORCA_PTR_INIT(DCAMDEV_STRING, param);
    param.text      = info->vendor;
    param.textbytes = sizeof(info->vendor);
    param.iString   = DCAM_IDSTR_VENDOR;
    err             = dcamdev_getstring(cam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->model;
    param.textbytes = sizeof(info->model);
    param.iString   = DCAM_IDSTR_MODEL;
    err             = dcamdev_getstring(cam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->id;
    param.textbytes = sizeof(info->id);
    param.iString   = DCAM_IDSTR_CAMERAID;
    err             = dcamdev_getstring(cam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->bus;
    param.textbytes = sizeof(info->bus);
    param.iString   = DCAM_IDSTR_BUS;
    err             = dcamdev_getstring(cam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->cam_ver;
    param.textbytes = sizeof(info->cam_ver);
    param.iString   = DCAM_IDSTR_CAMERAVERSION;
    err             = dcamdev_getstring(cam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->drv_ver;
    param.textbytes = sizeof(info->drv_ver);
    param.iString   = DCAM_IDSTR_DRIVERVERSION;
    err             = dcamdev_getstring(cam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->mod_ver;
    param.textbytes = sizeof(info->mod_ver);
    param.iString   = DCAM_IDSTR_MODULEVERSION;
    err             = dcamdev_getstring(cam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->dcam_ver;
    param.textbytes = sizeof(info->dcam_ver);
    param.iString   = DCAM_IDSTR_DCAMAPIVERSION;
    err             = dcamdev_getstring(cam, &param);
    return err;
}
