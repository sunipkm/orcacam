#include "orcacam.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef NDEBUG
#include <stdio.h>
#endif

#ifdef NDEBUG
#define ORCACALL(func, ...) func(__VA_ARGS__)
#else
#define ORCACALL(func, ...)                                                    \
    ({                                                                         \
        DCAMERR err = func(__VA_ARGS__);                                       \
        if (dcamerr_failed(err))                                               \
        {                                                                      \
            fprintf(stderr, "%s:%d:%s() -> %s\n", __FILE__, __LINE__,          \
                    __func__, orcacam_sterr(err));                             \
            fflush(stderr);                                                    \
        }                                                                      \
        err;                                                                   \
    })
#endif

static void *orcacam_capture_thread(void *inp);

void orcapi_uninit(void)
{
    DCAMERR err = dcamapi_uninit();
    if (dcamerr_failed(err))
    {
        fprintf(stderr, "ERROR: Could not de-init DCAMAPI -> %s (%d)\n",
                orcacam_sterr(err), err);
        fflush(stderr);
    }
}

struct _ORCA_THREAD_ARGS
{
    DCAMERR ret;
    HDCAM cam;
    HDCAMWAIT wait;
    void **frameptr;
    void *user_data;
    size_t sz_user_data;
    OrcaFrameCallback cb;
    int32 topoffset, rowbytes, width, height;
    DCAM_PIXELTYPE fmt;
};

struct _ORCACAM
{
    HDCAM hdcam;
    HDCAMWAIT hwait;
    atomic_bool capturing;
    char *framebuf; // DO NOT USE
    void **frameptr;
    size_t num_frames;
    size_t frame_size;
    pthread_t capture_thread;
};

DCAMERR orca_list_devices(int32 *count, int32 sz_initopt, const int32 *initopt)
{
    assert(count);
    *count = 0;
    ORCA_PTR_INIT(DCAMAPI_INIT, init);
    init.initoptionbytes = initopt ? sz_initopt : 0;
    init.initoption      = initopt;
    DCAMERR err          = ORCACALL(dcamapi_init, &init);
    if (dcamerr_failed(err))
    {
        *count = 0;
        return err;
    }
    *count = init.iDeviceCount; // number of devices
    return DCAMERR_SUCCESS;
}

DCAMERR orca_open_camera(int32 idx, ORCACAM *hdcam, size_t num_frames)
{
    DCAMERR err;
    int32 w, h;
    assert(hdcam);
    *hdcam = NULL;
    // Allocate memory for the camera object
    struct _ORCACAM *cam = (struct _ORCACAM *)malloc(sizeof(struct _ORCACAM));
    if (!cam)
    {
        err = DCAMERR_LESSSYSTEMMEMORY;
        goto ret;
    }
    memset(cam, 0, sizeof(struct _ORCACAM));
    // Initialize the camera object
    ORCA_PTR_INIT(DCAMDEV_OPEN, open);
    open.index = idx;
    err        = ORCACALL(dcamdev_open, &open);
    if (dcamerr_failed(err))
    {
        goto free_cam;
    }
    cam->hdcam = open.hdcam;
    // Initialize the wait object
    ORCA_PTR_INIT(DCAMWAIT_OPEN, wait);
    wait.hdcam = open.hdcam;
    err        = ORCACALL(dcamwait_open, &wait);
    if (dcamerr_failed(err))
    {
        goto close_camera;
    }
    cam->hwait     = wait.hwait;
    cam->capturing = ATOMIC_VAR_INIT(false);
    // Get the sensor size and set the ROI
    err = orca_get_sensor_size(cam, &w, &h);
    if (dcamerr_failed(err))
    {
        goto close_wait;
    }
    err = orca_set_roi(cam, 0, 0, w, h);
    if (dcamerr_failed(err))
    {
        goto close_wait;
    }
    assert(cam->framebuf);
    assert(cam->hwait);
    *hdcam = cam;
    goto ret; // success!
close_wait:
    ORCACALL(dcamwait_close, cam->hwait);
close_camera:
    ORCACALL(dcamdev_close, cam->hdcam);
free_cam:
    free(cam);
ret:
    return err;
}

DCAMERR orca_realloc_framebuffer(ORCACAM cam, size_t num_frames)
{
    assert(cam);
    DCAMERR err;
    bool need_realloc = false;
    if (atomic_load(&(cam->capturing)))
    {
        return DCAMERR_BUSY;
    }
    if (num_frames == 0)
    {
        num_frames = DEFAULT_FRAME_COUNT;
    }
    if (num_frames > 1000)
    {
        num_frames = 1000;
    }
    if (num_frames != cam->num_frames || !cam->frameptr || !cam->framebuf)
    {
        need_realloc = true;
    }
    double v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_BUFFER_FRAMEBYTES,
                   &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    int32 frame_size = (int32)v;
    if (frame_size != cam->frame_size)
    {
        cam->frame_size = 0;
        need_realloc    = true;
    }
    if (need_realloc)
    {
        if (cam->framebuf)
        {
            cam->framebuf =
                (char *)realloc(cam->framebuf, frame_size * num_frames);
        }
        else
        {
            cam->framebuf = (char *)malloc(frame_size * num_frames);
        }
        if (!cam->framebuf)
        {
            return DCAMERR_LESSSYSTEMMEMORY;
        }
        if (cam->frameptr)
        {
            cam->frameptr =
                (void **)realloc(cam->frameptr, sizeof(void *) * num_frames);
        }
        else
        {
            cam->frameptr = (void **)malloc(sizeof(void *) * num_frames);
        }
        if (!cam->frameptr)
        {
            free(cam->framebuf);
            cam->framebuf = NULL;
            return DCAMERR_LESSSYSTEMMEMORY;
        }
    }
    cam->frame_size = frame_size;
    cam->num_frames = num_frames;
    for (size_t i = 0; i < num_frames; i++)
    {
        cam->frameptr[i] = cam->framebuf + i * frame_size;
    }
    return err;
}

DCAMERR orca_device_info(ORCACAM cam, ORCA_CAM_INFO *info)
{
    assert(cam);
    assert(info);
    DCAMERR err;
    ORCA_PTR_INIT(DCAMDEV_STRING, param);
    param.text      = info->vendor;
    param.textbytes = sizeof(info->vendor);
    param.iString   = DCAM_IDSTR_VENDOR;
    err             = dcamdev_getstring(cam->hdcam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->model;
    param.textbytes = sizeof(info->model);
    param.iString   = DCAM_IDSTR_MODEL;
    err             = dcamdev_getstring(cam->hdcam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->id;
    param.textbytes = sizeof(info->id);
    param.iString   = DCAM_IDSTR_CAMERAID;
    err             = dcamdev_getstring(cam->hdcam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->bus;
    param.textbytes = sizeof(info->bus);
    param.iString   = DCAM_IDSTR_BUS;
    err             = dcamdev_getstring(cam->hdcam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->cam_ver;
    param.textbytes = sizeof(info->cam_ver);
    param.iString   = DCAM_IDSTR_CAMERAVERSION;
    err             = dcamdev_getstring(cam->hdcam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->drv_ver;
    param.textbytes = sizeof(info->drv_ver);
    param.iString   = DCAM_IDSTR_DRIVERVERSION;
    err             = dcamdev_getstring(cam->hdcam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->mod_ver;
    param.textbytes = sizeof(info->mod_ver);
    param.iString   = DCAM_IDSTR_MODULEVERSION;
    err             = dcamdev_getstring(cam->hdcam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    param.text      = info->dcam_ver;
    param.textbytes = sizeof(info->dcam_ver);
    param.iString   = DCAM_IDSTR_DCAMAPIVERSION;
    err             = dcamdev_getstring(cam->hdcam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    double w, h;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam,
                   DCAM_IDPROP_IMAGEDETECTOR_PIXELNUMHORZ, &w);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamprop_getvalue, cam->hdcam,
                   DCAM_IDPROP_IMAGEDETECTOR_PIXELNUMVERT, &h);
    if (dcamerr_failed(err))
    {
        return err;
    }
    info->width = (int32)w;
    info->height = (int32)h;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam,
                   DCAM_IDPROP_IMAGEDETECTOR_PIXELWIDTH, &w);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamprop_getvalue, cam->hdcam,
                   DCAM_IDPROP_IMAGEDETECTOR_PIXELHEIGHT, &h);
    if (dcamerr_failed(err))
    {
        return err;
    }
    info->pixel_width = w;
    info->pixel_height = h;
    return err;
}

DCAMERR orca_get_sensor_size(ORCACAM cam, int32 *wid, int32 *hei)
{
    assert(cam);
    assert(wid);
    assert(hei);
    DCAMERR err;
    double w, h;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam,
                   DCAM_IDPROP_IMAGEDETECTOR_PIXELNUMHORZ, &w);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamprop_getvalue, cam->hdcam,
                   DCAM_IDPROP_IMAGEDETECTOR_PIXELNUMVERT, &h);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *wid = (int32)w;
    *hei = (int32)h;
    return DCAMERR_SUCCESS;
}

DCAMERR orca_get_temperature(ORCACAM cam, double *temp)
{
    assert(cam);
    assert(temp);
    DCAMERR err;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_SENSORTEMPERATURE,
                   temp);
    return err;
}

DCAMERR orca_set_tempsetpoint(ORCACAM cam, double temp)
{
    assert(cam);
    DCAMERR err;
    err = ORCACALL(dcamprop_setvalue, cam->hdcam,
                   DCAM_IDPROP_SENSORTEMPERATURETARGET, temp);
    return err;
}

DCAMERR orca_get_tempsetpoint(ORCACAM cam, double *temp)
{
    assert(cam);
    DCAMERR err;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam,
                   DCAM_IDPROP_SENSORTEMPERATURETARGET, temp);
    return err;
}

DCAMERR orca_get_exposure(ORCACAM cam, double *exp)
{
    assert(cam);
    assert(exp);
    DCAMERR err;
    err =
        ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_EXPOSURETIME, exp);
    return err;
}

DCAMERR orca_set_exposure(ORCACAM cam, double exp)
{
    assert(cam);
    DCAMERR err;
    err =
        ORCACALL(dcamprop_setvalue, cam->hdcam, DCAM_IDPROP_EXPOSURETIME, exp);
    return err;
}

DCAMERR orca_get_pixel_fmt(ORCACAM cam, DCAM_PIXELTYPE *fmt)
{
    assert(cam);
    assert(fmt);
    DCAMERR err;
    double f;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_IMAGE_PIXELTYPE,
                   &f);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *fmt = (DCAM_PIXELTYPE)f;
    return err;
}

DCAMERR orca_set_pixel_fmt(ORCACAM cam, DCAM_PIXELTYPE fmt)
{
    assert(cam);
    if (atomic_load(&(cam->capturing)))
    {
        return DCAMERR_BUSY;
    }
    DCAMERR err;
    switch (fmt)
    {
    case DCAM_PIXELTYPE_MONO8:
    case DCAM_PIXELTYPE_MONO16:
        break;
    default:
        return DCAMERR_NOTSUPPORT;
    }
    err = ORCACALL(dcamprop_setvalue, cam->hdcam, DCAM_IDPROP_IMAGE_PIXELTYPE,
                   (double)fmt);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = orca_realloc_framebuffer(cam, cam->num_frames);
    return err;
}

DCAMERR orca_get_frame_size(ORCACAM cam, int32 *w, int32 *h)
{
    assert(cam);
    assert(w);
    assert(h);
    DCAMERR err;
    double v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_IMAGE_WIDTH, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *w  = (int32)v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_IMAGE_HEIGHT, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *h = (int32)v;
    return err;
}

DCAMERR orca_get_roi(ORCACAM cam, int32 *x, int32 *y, int32 *w, int32 *h)
{
    assert(cam);
    assert(x);
    assert(y);
    assert(w);
    assert(h);
    DCAMERR err;
    double v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYHPOS, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *x  = (int32)v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYVPOS, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *y = (int32)v;
    err =
        ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYHSIZE, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *w = (int32)v;
    err =
        ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYVSIZE, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *h = (int32)v;
    return err;
}

DCAMERR orca_set_roi(ORCACAM cam, int32 x, int32 y, int32 w, int32 h)
{
    assert(cam);
    if (atomic_load(&(cam->capturing)))
    {
        return DCAMERR_BUSY;
    }
    DCAMERR err;
    err = ORCACALL(dcambuf_release, cam->hdcam, 0);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamprop_setvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYMODE,
                   DCAMPROP_MODE__OFF);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamprop_setvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYHPOS,
                   (double)x);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamprop_setvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYVPOS,
                   (double)y);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamprop_setvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYHSIZE,
                   (double)w);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamprop_setvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYVSIZE,
                   (double)h);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamprop_setvalue, cam->hdcam, DCAM_IDPROP_SUBARRAYMODE,
                   DCAMPROP_MODE__ON);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = orca_realloc_framebuffer(cam, cam->num_frames);
    return err;
}

DCAMERR orca_get_acq_framerate(ORCACAM cam, double *fps)
{
    assert(cam);
    assert(fps);
    DCAMERR err;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_INTERNALFRAMERATE,
                   fps);
    return err;
}

DCAMERR orca_set_acq_framerate(ORCACAM cam, double fps)
{
    assert(cam);
    if (atomic_load(&(cam->capturing)))
    {
        return DCAMERR_BUSY;
    }
    DCAMERR err;
    err = ORCACALL(dcamprop_setvalue, cam->hdcam, DCAM_IDPROP_INTERNALFRAMERATE,
                   fps);
    return err;
}

DCAMERR orca_get_attr(ORCACAM cam, DCAMIDPROP prop, DCAMPROP_ATTR *attr)
{
    assert(cam);
    assert(attr);
    DCAMERR err;
    ORCA_PTR_SUBINIT(DCAMPROP_ATTR, param, cbSize);
    param.iProp = prop;
    err         = ORCACALL(dcamprop_getattr, cam->hdcam, &param);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *attr = param;
    return err;
}

DCAMERR orca_get_value(ORCACAM cam, DCAMIDPROP prop, double *value)
{
    assert(cam);
    assert(value);
    DCAMERR err;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, prop, value);
    return err;
}

DCAMERR orca_set_value(ORCACAM cam, DCAMIDPROP prop, double value)
{
    assert(cam);
    DCAMERR err;
    err = ORCACALL(dcamprop_setvalue, cam->hdcam, prop, value);
    return err;
}

DCAMERR orca_setget_value(ORCACAM cam, DCAMIDPROP prop, double *value,
                          int32 option)
{
    assert(cam);
    assert(value);
    DCAMERR err;
    err = ORCACALL(dcamprop_setgetvalue, cam->hdcam, prop, value, option);
    return err;
}

DCAMERR orca_query_value(ORCACAM cam, DCAMIDPROP prop, double *value,
                         int32 option DCAM_DEFAULT_ARG)
{
    assert(cam);
    assert(value);
    DCAMERR err;
    err = ORCACALL(dcamprop_queryvalue, cam->hdcam, prop, value, option);
    return err;
}

DCAMERR orca_get_next_id(ORCACAM cam, int32 *prop,
                         int32 option DCAM_DEFAULT_ARG)
{
    assert(cam);
    assert(prop);
    DCAMERR err;
    err = ORCACALL(dcamprop_getnextid, cam->hdcam, prop, option);
    return err;
}

DCAMERR orca_get_name(ORCACAM cam, int32 prop, char *text, int32 textbytes)
{
    assert(cam);
    assert(text);
    DCAMERR err;
    err = ORCACALL(dcamprop_getname, cam->hdcam, prop, text, textbytes);
    return err;
}

DCAMERR orca_get_value_text(ORCACAM cam, DCAMIDPROP prop, double value,
                            char *text, int32 textbytes)
{
    assert(cam);
    assert(text);
    DCAMERR err;
    ORCA_PTR_SUBINIT(DCAMPROP_VALUETEXT, param, cbSize);
    param.iProp     = prop;
    param.value     = value;
    param.text      = text;
    param.textbytes = textbytes;
    err             = ORCACALL(dcamprop_getvaluetext, cam->hdcam, &param);
    return err;
}

DCAMERR orca_start_capture(ORCACAM cam, OrcaFrameCallback cb, void *user_data,
                           size_t sz_user_data)
{
    assert(cam);
    assert(cb);
    if (atomic_load(&(cam->capturing)))
    {
        return DCAMERR_BUSY;
    }
    DCAMERR err;
    int32 topoffset, rowbytes, width, height;
    DCAM_PIXELTYPE pixeltype;
    double v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam,
                   DCAM_IDPROP_BUFFER_TOPOFFSETBYTES, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    topoffset = (int32)v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_BUFFER_ROWBYTES,
                   &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    rowbytes = (int32)v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_IMAGE_PIXELTYPE,
                   &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    pixeltype = (DCAM_PIXELTYPE)v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_IMAGE_WIDTH, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    width = (int32)v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_IMAGE_HEIGHT, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    height                        = (int32)v;

    ORCA_PTR_INIT(DCAMBUF_ATTACH, attach);
    attach.iKind       = DCAMBUF_ATTACHKIND_FRAME;
    attach.buffer      = cam->frameptr;
    attach.buffercount = cam->num_frames;
    err                = ORCACALL(dcambuf_attach, cam->hdcam, &attach);
    if (dcamerr_failed(err))
    {
        atomic_store(&(cam->capturing), false);
        return err;
    }

    struct _ORCA_THREAD_ARGS *args = (struct _ORCA_THREAD_ARGS *)malloc(
        sizeof(struct _ORCA_THREAD_ARGS));
    if (!args)
    {
        return DCAMERR_NORESOURCE;
    }
    args->cam          = cam->hdcam;
    args->wait         = cam->hwait;
    args->frameptr     = cam->frameptr;
    args->cb           = cb;
    args->user_data    = user_data;
    args->sz_user_data = sz_user_data;
    args->topoffset    = topoffset;
    args->rowbytes     = rowbytes;
    args->width        = width;
    args->height       = height;
    args->fmt          = pixeltype;
    atomic_store(&(cam->capturing), true);
    err = pthread_create(&(cam->capture_thread), NULL, orcacam_capture_thread,
                         (void *)args);
    if (err)
    {
        return DCAMERR_NORESOURCE;
    }
    err = dcamcap_start(cam->hdcam, DCAMCAP_START_SEQUENCE);
    if (dcamerr_failed(err))
    {
        atomic_store(&(cam->capturing), false);
        pthread_cancel(cam->capture_thread);
    }
    return DCAMERR_SUCCESS;
}

DCAMERR orca_stop_capture(ORCACAM cam)
{
    assert(cam);
    if (!atomic_load(&(cam->capturing)))
    {
        // printf("Not capturing\n");
        // fflush(stdout);
        return DCAMERR_NOTREADY;
    }
    DCAMERR err = ORCACALL(dcamcap_stop, cam->hdcam);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcambuf_release, cam->hdcam, 0);
    if (dcamerr_failed(err))
    {
        return err;
    }
    err = ORCACALL(dcamwait_abort, cam->hwait);
    atomic_store(&(cam->capturing), false);
    if (dcamerr_failed(err))
    {
        pthread_cancel(cam->capture_thread);
        return err;
    }
    void *ret;
    int rc = pthread_join(cam->capture_thread, &ret);
    if (rc)
    {
        return DCAMERR_NORESOURCE;
    }
    if (ret)
    {
        struct _ORCA_THREAD_ARGS *args = (struct _ORCA_THREAD_ARGS *)ret;
        err = args->ret;
        // printf("Capture thread returned %s\n", orcacam_sterr(err));
        // fflush(stdout);
        free(ret);
        // printf("Freed capture thread args\n");
        // fflush(stdout);
    }
    return (DCAMERR)err;
}

DCAMERR orca_close_camera(ORCACAM *cam_)
{
    DCAMERR err = DCAMERR_SUCCESS;
    ORCACAM cam = *cam_;
    if (!cam)
    {
        goto ret;
    }
    err = orca_stop_capture(cam);
    if (dcamerr_failed(err) && err != DCAMERR_NOTREADY)
    {
        fprintf(stderr, "Failed to stop capture: %s\n", orcacam_sterr(err));
    }
    err = ORCACALL(dcamwait_close, cam->hwait);
    if (dcamerr_failed(err))
    {
        fprintf(stderr, "Failed to close wait object: %s\n",
                orcacam_sterr(err));
    }
    // printf("Closed wait object\n");
    // fflush(stdout);
    sleep(1);
    err = ORCACALL(dcamdev_close, cam->hdcam);
    if (dcamerr_failed(err))
    {
        fprintf(stderr, "Failed to close camera: %s\n", orcacam_sterr(err));
    }
    // printf("Closed camera\n");
    // fflush(stdout);
    if (cam->framebuf)
    {
        free(cam->framebuf);
    }
    // printf("Freed frame buffer\n");
    // fflush(stdout);
    if (cam->frameptr)
    {
        free(cam->frameptr);
    }
    // printf("Freed frame pointer\n");
    // fflush(stdout);
    free(cam);
    // printf("Freed camera object\n");
    // fflush(stdout);
    *cam_ = NULL; // prevent use after free
    err = dcamapi_uninit();
ret:
    return err;
}

DCAMERR orca_get_mode(ORCACAM cam, DCAMPROPMODEVALUE *mode)
{
    assert(cam);
    assert(mode);
    DCAMERR err;
    double v;
    err = ORCACALL(dcamprop_getvalue, cam->hdcam, DCAM_IDPROP_SENSORMODE, &v);
    if (dcamerr_failed(err))
    {
        return err;
    }
    *mode = (DCAMPROPMODEVALUE)v;
    return err;
}

DCAMERR orca_switch_mode(ORCACAM cam, DCAMPROPMODEVALUE mode)
{
    assert(cam);
    if (atomic_load(&(cam->capturing)))
    {
        return DCAMERR_BUSY;
    }
    DCAMERR err;
    double v = (double)mode;
    err = ORCACALL(dcamprop_setgetvalue, cam->hdcam, DCAM_IDPROP_SENSORMODE, &v,
                   0);
    DCAMPROPMODEVALUE new_mode = (DCAMPROPMODEVALUE)v;
    if (dcamerr_failed(err))
    {
        return err;
    }
    if (new_mode != mode)
    {
        fprintf(stderr, "Failed to switch mode: Desired mode %d, got mode %d\n",
                mode, new_mode);
        return DCAMERR_NOTSUPPORT;
    }
    return err;
}

static void *orcacam_capture_thread(void *inp)
{
    if (!inp)
    {
        return NULL; // got NULL already
    }
    struct _ORCA_THREAD_ARGS *args = (struct _ORCA_THREAD_ARGS *)inp;
    if (!args->cam || !args->wait || !args->cb)
    {
        args->ret = DCAMERR_NORESOURCE;
        goto ret;
    }
    HDCAM cam = args->cam;
    HDCAMWAIT wait = args->wait;
    void **frameptr = args->frameptr;
    DCAMERR err          = DCAMERR_SUCCESS;
    ORCA_FRAME frame = {
        .data       = NULL,
        .width      = args->width,
        .height     = args->height,
        .fmt        = args->fmt,
        .row_stride = args->rowbytes,
    };
    OrcaFrameCallback cb = args->cb;
    void *user_data      = args->user_data;
    size_t sz_user_data  = args->sz_user_data;

    // get the timing info
    // double v;
    // double wait_time = 0;
    // err = ORCACALL(dcamprop_getvalue, cam->hdcam,
    // DCAM_IDPROP_INTERNAL_FRAMEINTERVAL, &v); if (dcamerr_failed(err))
    // {
    //     goto ret;
    // }
    // wait_time += v;

    // Start the wait handler
    ORCA_PTR_INIT(DCAMWAIT_START, start);
    start.eventmask = DCAMWAIT_CAPEVENT_FRAMEREADY;
    start.timeout   = 1000;

    // transfer info
    ORCA_PTR_INIT(DCAMCAP_TRANSFERINFO, xferinfo);

    while (true)
    {
        err = ORCACALL(dcamwait_start, wait, &start);
        if (dcamerr_failed(err))
        {
            if (err == DCAMERR_ABORT)
            {
                args->ret = DCAMERR_SUCCESS;
                break;
            }
            else if (err == DCAMERR_TIMEOUT)
            {
                continue;
            }
            else
            {
                args->ret = err;
                goto ret;
            }
        }
        // get capture info
        err = ORCACALL(dcamcap_transferinfo, cam, &xferinfo);
        if (dcamerr_failed(err))
        {
            continue;
        }
        // create the frame
        char *buf = (char *)frameptr[xferinfo.nNewestFrameIndex];
        buf += args->topoffset;
        frame.data = buf;
        // Execute the callback
        cb(&frame, user_data, sz_user_data);
    }
ret:
    return inp;
}
