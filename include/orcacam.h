/**
 * @file orcacam.h
 * @author Sunip K. Mukherjee (sunipkmukherjee@gmail.com)
 * @brief Wrapper for Hamamatsu DCAM API
 * @version 0.0.1
 * @date 2024-10-15
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef _ORCACAM_H_
#define _ORCACAM_H_

#include <string.h> // memset

#include "dcamapi/dcamapi4.h"
#include "dcamapi/dcamprop.h"

#include "orcacam_strerr.h"

#ifdef __cplusplus
// TODO: extern C
#endif // __cplusplus

#ifndef _Nullable
#define _Nullable
#endif // _Nullable

#ifndef _Nonnull
#define _Nonnull
#endif // _Nonnull

/**
 * @brief ORCA Camera Information
 *
 */
typedef struct _ORCA_CAM_INFO
{
    char vendor[64];   /*<! Camera vendor */
    char model[64];    /*<! Camera model */
    char id[64];       /*<! Camera ID */
    char bus[64];      /*<! Camera bus */
    char cam_ver[64];  /*<! Camera version */
    char drv_ver[64];  /*<! Driver version */
    char mod_ver[64];  /*<! Module version */
    char dcam_ver[64]; /*<! DCAM API version */
} ORCA_CAM_INFO;

#define ORCA_PTR_INIT(type, name)                                              \
    type name;                                                                 \
    {                                                                          \
        memset(&name, 0, sizeof(type));                                        \
        name.size = sizeof(type);                                              \
    }

#define ORCA_PTR_SUBINIT(type, name, sub)                                      \
    type name;                                                                 \
    {                                                                          \
        memset(&name, 0, sizeof(type));                                        \
        name.size     = sizeof(type);                                          \
        name.sub.size = sizeof(type);                                          \
    }

/**
 * @brief List all available devices
 *
 * @param count Output number of devices
 * @param sz_initopt Size of initopt
 * @param initopt Init options (DCAMAPI_INITOPTION)
 * @return DCAMERR
 */
DCAMERR orca_list_devices(int32 *_Nonnull count, int32 sz_initopt,
                          const int32 *_Nullable initopt);

/**
 * @brief Open a device
 *
 * @param index Device index
 * @param hdcam Output HDCAM handle
 * @return DCAMERR
 */
DCAMERR orca_open_device(int32 index, HDCAM *_Nonnull hdcam);

/**
 * @brief Obtain device information
 *
 * @param cam HDCAM handle
 * @param info Output ORCA_CAM_INFO
 * @return DCAMERR
 */
DCAMERR orca_device_info(HDCAM cam, ORCA_CAM_INFO *_Nonnull info);

DCAMERR orca_realloc_framebuffer(HDCAM cam, int32 num_frames);

DCAMERR orca_get_frame_size();

DCAMERR orca_start_capture(handle, cb, user_data, sz_user_data);

DCAMERR orca_stop_capture(handle);

DCAMERR orca_close_camera(handle);

DCAMERR orca_get_temperature(handle, double *temp);

DCAMERR orca_set_temperature(handle, double temp);

DCAMERR orca_get_sensor_size(handle, int32 *width, int32 *height);

DCAMERR orca_get_exposure(handle, double *exposure);

DCAMERR orca_set_exposure(handle, double exposure);

DCAMERR orca_get_pixel_fmt(void handle, DCAM_PIXELTYPE *_Nonnull fmt);

DCAMERR orca_set_pixel_fmt(void handle, DCAM_PIXELTYPE fmt);

DCAMERR orca_get_roi(void handle, int32 *x, int32 *y, int32 *w, int32 *h);

DCAMERR orca_set_roi(void handle, int32 x, int32 y, int32 w, int32 h);

DCAMERR orca_get_acq_framerate(void handle, double *_Nonnull fps);

DCAMERR orca_set_acq_framerate(void handle, double fps);

DCAMERR orca_get_attr(void handle, DCAMIDPROP prop, DCAMPROP_ATTR *_Nonnull attr);

DCAMERR orca_get_value(void handle, DCAMIDPROP prop, double *_Nonnull value);

DCAMERR orca_set_value(void handle, DCAMIDPROP prop, double value);

DCAMERR orca_setget_value(void handle, DCAMIDPROP prop, double *_Nonnull value, int32 option DCAM_DEFAULT_ARG);

DCAMERR orca_query_value(void handle, DCAMIDPROP prop, double *_Nonnull value, int32 option DCAM_DEFAULT_ARG);

DCAMERR orca_get_next_id(void handle, int32 *_Nonnull prop, int32 option DCAM_DEFAULT_ARG);

DCAMERR orca_get_name(void handle, int32 prop, char *_Nonnull text, int32 textbytes);

DCAMERR orca_get_value_text(void handle, DCAMIDPROP prop, double value, char *_Nonnull text, int32 textbytes);


#ifdef __cplusplus
// TODO: extern C
#endif // __cplusplus
#endif // _ORCACAM_H_
