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

#include <assert.h>
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

/**
 * @brief ORCA Image Frame
 *
 */
typedef struct _ORCA_FRAME
{
    char *data;         //!< Frame data
    int32 width;        //!< Frame width
    int32 height;       //!< Frame height
    DCAM_PIXELTYPE fmt; //!< Frame pixel format
    int32 row_stride;   //!< Frame row stride (bytes)
} ORCA_FRAME;

typedef void (*OrcaFrameCallback)(ORCA_FRAME *, void *, size_t);

/**
 * @brief Default number of frames
 *
 */
#define DEFAULT_FRAME_COUNT 10

typedef struct _ORCACAM *ORCACAM;

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
        name.sub = sizeof(type);                                               \
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
DCAMERR orca_open_camera(int32 index, ORCACAM *_Nonnull cam, size_t num_frames);

/**
 * @brief Obtain device information
 *
 * @param cam HDCAM handle
 * @param info Output ORCA_CAM_INFO
 * @return DCAMERR
 */
DCAMERR orca_device_info(ORCACAM cam, ORCA_CAM_INFO *info);

DCAMERR orca_realloc_framebuffer(ORCACAM cam, size_t num_frames);

DCAMERR orca_get_frame_size(ORCACAM cam, int32 *width, int32 *height);

DCAMERR orca_get_mode(ORCACAM cam, DCAMPROPMODEVALUE *mode);

DCAMERR orca_switch_mode(ORCACAM cam, DCAMPROPMODEVALUE mode);

DCAMERR orca_start_capture(ORCACAM cam, OrcaFrameCallback cb, void *user_data, size_t sz_user_data);

DCAMERR orca_stop_capture(ORCACAM cam);

DCAMERR orca_close_camera(ORCACAM *cam);

DCAMERR orca_get_temperature(ORCACAM cam, double *temp);

DCAMERR orca_set_temperature(ORCACAM cam, double temp);

DCAMERR orca_get_sensor_size(ORCACAM cam, int32 *width, int32 *height);

DCAMERR orca_get_exposure(ORCACAM cam, double *exposure);

DCAMERR orca_set_exposure(ORCACAM cam, double exposure);

DCAMERR orca_get_pixel_fmt(ORCACAM cam, DCAM_PIXELTYPE *fmt);

DCAMERR orca_set_pixel_fmt(ORCACAM cam, DCAM_PIXELTYPE fmt);

DCAMERR orca_get_roi(ORCACAM cam, int32 *x, int32 *y, int32 *w, int32 *h);

DCAMERR orca_set_roi(ORCACAM cam, int32 x, int32 y, int32 w, int32 h);

DCAMERR orca_get_acq_framerate(ORCACAM cam, double *fps);

DCAMERR orca_set_acq_framerate(ORCACAM cam, double fps);

DCAMERR orca_get_attr(ORCACAM cam, DCAMIDPROP prop, DCAMPROP_ATTR *attr);

DCAMERR orca_get_value(ORCACAM cam, DCAMIDPROP prop, double *value);

DCAMERR orca_set_value(ORCACAM cam, DCAMIDPROP prop, double value);

DCAMERR orca_setget_value(ORCACAM cam, DCAMIDPROP prop, double *value,
                          int32 option DCAM_DEFAULT_ARG);

DCAMERR orca_query_value(ORCACAM cam, DCAMIDPROP prop, double *value,
                         int32 option DCAM_DEFAULT_ARG);

DCAMERR orca_get_next_id(ORCACAM cam, int32 *prop,
                         int32 option DCAM_DEFAULT_ARG);

DCAMERR orca_get_name(ORCACAM cam, int32 prop, char *text, int32 textbytes);

DCAMERR orca_get_value_text(ORCACAM cam, DCAMIDPROP prop, double value,
                            char *text, int32 textbytes);

#ifdef __cplusplus
// TODO: extern C
#endif // __cplusplus
#endif // _ORCACAM_H_
