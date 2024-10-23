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
extern "C"
{
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
    char vendor[64];     /*<! Camera vendor */
    char model[64];      /*<! Camera model */
    char id[64];         /*<! Camera ID */
    char bus[64];        /*<! Camera bus */
    char cam_ver[64];    /*<! Camera version */
    char drv_ver[64];    /*<! Driver version */
    char mod_ver[64];    /*<! Module version */
    char dcam_ver[64];   /*<! DCAM API version */
    int32 width;         /*<! Sensor width */
    int32 height;        /*<! Sensor height */
    double pixel_width;  /*<! Pixel width */
    double pixel_height; /*<! Pixel height */
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
    int32 rsvd;        //!< Reserved
} ORCA_FRAME;

/**
 * @brief Orca camera frame callback
 *
 * @param frame Frame data
 * @param user_data Pointer to user data
 * @param sz_user_data Size of user data
 */
typedef void (*OrcaFrameCallback)(ORCA_FRAME * _Nonnull, void * _Nullable,  size_t);

/**
 * @brief Default number of frames
 *
 */
#define DEFAULT_FRAME_COUNT 10

/**
 * @brief ORCA Camera handle
 *
 */
typedef struct _ORCACAM *ORCACAM;

/**
 * @brief Initialize a DCAM API data structure
 *
 * @param type Data structure type
 * @param name Variable name
 *
 */
#define ORCA_PTR_INIT(type, name)                                              \
    type name;                                                                 \
    {                                                                          \
        memset(&name, 0, sizeof(type));                                        \
        name.size = sizeof(type);                                              \
    }

/**
 * @brief Initialize a DCAM API data structure with different size type name
 *
 * @param type Data structure type
 * @param name Variable name
 * @param sub Struct size field name
 */
#define ORCA_PTR_SUBINIT(type, name, sub)                                      \
    type name;                                                                 \
    {                                                                          \
        memset(&name, 0, sizeof(type));                                        \
        name.sub = sizeof(type);                                               \
    }

/**
 * @brief List all available devices.
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
DCAMERR orca_device_info(ORCACAM cam, ORCA_CAM_INFO *_Nonnull info);

/**
 * @brief Re-allocate the framebuffer
 *
 * @param cam ORCACAM handle
 * @param num_frames Number of frames
 * @return DCAMERR
 */
DCAMERR orca_realloc_framebuffer(ORCACAM cam, size_t num_frames);

/**
 * @brief Get the frame width and height
 *
 * @param cam ORCACAM handle
 * @param width Output width
 * @param height Output height
 * @return DCAMERR
 */
DCAMERR orca_get_frame_size(ORCACAM cam, int32 * _Nonnull width, int32 *_Nonnull height);

/**
 * @brief Get camera sensor mode (DCAM_IDPROP_SENSORMODE)
 *
 * @param cam ORCACAM handle
 * @param mode Output mode (DCAMPROPMODEVALUE::DCAMPROP_SENSORMODE__*)
 * @return DCAMERR
 */
DCAMERR orca_get_mode(ORCACAM cam, DCAMPROPMODEVALUE *_Nonnull mode);

/**
 * @brief Set camera sensor mode (DCAM_IDPROP_SENSORMODE)
 *
 * @param cam ORCACAM handle
 * @param mode Mode (DCAMPROPMODEVALUE::DCAMPROP_SENSORMODE__*)
 * @return DCAMERR
 */
DCAMERR orca_switch_mode(ORCACAM cam, DCAMPROPMODEVALUE mode);

/**
 * @brief Start image acquisition (no callback API)
 *
 * @param cam ORCACAM handle
 * @param frame Frame handle. Must be pre-allocated. orca_start_acquisition fills in the image frame information (width, height, image format).
 * @return DCAMERR
 */
DCAMERR orca_start_acquisition(ORCACAM cam, ORCA_FRAME *_Nonnull frame);

/**
 * @brief Acquire an image frame
 *
 * @param cam ORCACAM handle
 * @param frame Frame handle, passed through orca_start_acquisition. orca_acquire_image fills in the frame data.
 * @param timeout Timeout in milliseconds
 * @return DCAMERR
 */
DCAMERR orca_acquire_image(ORCACAM cam, ORCA_FRAME *_Nonnull frame, int32 timeout);

/**
 * @brief Stop image acquisition (no callback API)
 *
 * @param cam ORCACAM handle
 * @return DCAMERR
 */
DCAMERR orca_stop_acquisition(ORCACAM cam);

/**
 * @brief Start image acquisition (callback API)
 *
 * @param cam ORCACAM handle
 * @param cb Frame callback function
 * @param user_data User data pointer
 * @param sz_user_data Size of user data
 * @return DCAMERR
 */
DCAMERR orca_start_capture(ORCACAM cam, OrcaFrameCallback _Nonnull cb, void *_Nullable user_data, size_t sz_user_data DCAM_DEFAULT_ARG);

/**
 * @brief Stop image acquisition
 *
 * @param cam ORCACAM handle
 * @return DCAMERR
 */
DCAMERR orca_stop_capture(ORCACAM cam);

/**
 * @brief Close the camera and release resources
 *
 * @param cam ORCACAM handle
 * @return DCAMERR
 */
DCAMERR orca_close_camera(ORCACAM *_Nonnull cam);

/**
 * @brief Read sensor temperature
 *
 * @param cam ORCACAM handle
 * @param temp Output temperature (default unit: Celsius)
 * @return DCAMERR
 */
DCAMERR orca_get_temperature(ORCACAM cam, double *_Nonnull temp);

/**
 * @brief Get sensor cooling setpoint
 *
 * @param cam ORCACAM handle
 * @param temp Temperature setpoint (default unit: Celsius)
 * @return DCAMERR
 */
DCAMERR orca_get_tempsetpoint(ORCACAM cam, double *_Nonnull temp);

/**
 * @brief Set sensor cooling setpoint
 *
 * @param cam ORCACAM handle
 * @param temp Temperature setpoint (default unit: Celsius)
 * @return DCAMERR
 */
DCAMERR orca_set_tempsetpoint(ORCACAM cam, double temp);

/**
 * @brief Get sensor size
 *
 * @param cam ORCACAM handle
 * @param width Sensor width
 * @param height Sensor height
 * @return DCAMERR
 */
DCAMERR orca_get_sensor_size(ORCACAM cam, int32 *_Nonnull width, int32 *_Nonnull height);

/**
 * @brief Get exposure time
 *
 * @param cam ORCACAM handle
 * @param exposure Exposure time (seconds)
 * @return DCAMERR
 */
DCAMERR orca_get_exposure(ORCACAM cam, double *_Nonnull exposure);

/**
 * @brief Set exposure time
 *
 * @param cam ORCACAM handle
 * @param exposure Exposure time (seconds)
 * @return DCAMERR
 */
DCAMERR orca_set_exposure(ORCACAM cam, double exposure);

/**
 * @brief Get pixel format
 *
 * @param cam ORCACAM handle
 * @param fmt Pixel format (DCAM_PIXELTYPE)
 * @return DCAMERR
 */
DCAMERR orca_get_pixel_fmt(ORCACAM cam, DCAM_PIXELTYPE *_Nonnull fmt);

/**
 * @brief Set pixel format
 *
 * @param cam ORCACAM handle
 * @param fmt Pixel format (DCAM_PIXELTYPE)
 * @return DCAMERR
 */
DCAMERR orca_set_pixel_fmt(ORCACAM cam, DCAM_PIXELTYPE fmt);

/**
 * @brief Get region of interest (ROI)
 *
 * @param cam ORCACAM handle
 * @param x X-coordinate
 * @param y Y-coordinate
 * @param w Width
 * @param h Height
 * @return DCAMERR
 */
DCAMERR orca_get_roi(ORCACAM cam, int32 *_Nonnull x, int32 *_Nonnull y, int32 *_Nonnull w, int32 *_Nonnull h);

/**
 * @brief Set region of interest (ROI)
 *
 * @param cam ORCACAM handle
 * @param x X-coordinate
 * @param y Y-coordinate
 * @param w Width
 * @param h Height
 * @return DCAMERR
 */
DCAMERR orca_set_roi(ORCACAM cam, int32 x, int32 y, int32 w, int32 h);

/**
 * @brief Get acquisition frame rate
 *
 * @param cam ORCACAM handle
 * @param fps Frame rate (frames per second)
 * @return DCAMERR
 */
DCAMERR orca_get_acq_framerate(ORCACAM cam, double *_Nonnull fps);

/**
 * @brief Set acquisition frame rate
 *
 * @param cam ORCACAM handle
 * @param fps Frame rate (frames per second)
 * @return DCAMERR
 */
DCAMERR orca_set_acq_framerate(ORCACAM cam, double fps);

/**
 * @brief Get property attributes. NOT FOR GENERAL USE.
 *
 * @param cam ORCACAM handle
 * @param prop Property ID
 * @param attr Output property attributes
 * @return DCAMERR
 */
DCAMERR orca_get_attr(ORCACAM cam, DCAMIDPROP prop, DCAMPROP_ATTR *_Nonnull attr);

/**
 * @brief Get property value. NOT FOR GENERAL USE.
 *
 * @param cam ORCACAM handle
 * @param prop Property ID
 * @param value Output property value
 * @return DCAMERR
 */
DCAMERR orca_get_value(ORCACAM cam, DCAMIDPROP prop, double *_Nonnull value);

/**
 * @brief Set property value. NOT FOR GENERAL USE.
 *
 * @param cam ORCACAM handle
 * @param prop Property ID
 * @param value Property value
 * @return DCAMERR
 */
DCAMERR orca_set_value(ORCACAM cam, DCAMIDPROP prop, double value);

/**
 * @brief Set and get property value. NOT FOR GENERAL USE.
 *
 * @param cam ORCACAM handle
 * @param prop Property ID
 * @param value Property value
 * @param option Property option
 * @return DCAMERR
 */
DCAMERR orca_setget_value(ORCACAM cam, DCAMIDPROP prop, double *_Nonnull value,
                          int32 option DCAM_DEFAULT_ARG);

/**
 * @brief Query property value. NOT FOR GENERAL USE.
 *
 * @param cam ORCACAM handle
 * @param prop Property ID
 * @param value Property value
 * @param option Property option
 * @return DCAMERR
 */
DCAMERR orca_query_value(ORCACAM cam, DCAMIDPROP prop, double *_Nonnull value,
                         int32 option DCAM_DEFAULT_ARG);

/**
 * @brief Get next property ID. NOT FOR GENERAL USE.
 *
 * @param cam ORCACAM handle
 * @param prop Property ID
 * @param option Property option
 * @return DCAMERR
 */
DCAMERR orca_get_next_id(ORCACAM cam, int32 *_Nonnull prop,
                         int32 option DCAM_DEFAULT_ARG);

/**
 * @brief Get property name. NOT FOR GENERAL USE.
 *
 * @param cam ORCACAM handle
 * @param prop Property ID
 * @param text Property name text output buffer
 * @param textbytes Text buffer size
 * @return DCAMERR
 */
DCAMERR orca_get_name(ORCACAM cam, int32 prop, char *_Nonnull text, int32 textbytes);

/**
 * @brief Get property value text. NOT FOR GENERAL USE.
 *
 * @param cam ORCACAM handle
 * @param prop Property ID
 * @param value Property value
 * @param text Property value text output buffer
 * @param textbytes Text buffer size
 * @return DCAMERR
 */
DCAMERR orca_get_value_text(ORCACAM cam, DCAMIDPROP prop, double value,
                            char *_Nonnull text, int32 textbytes);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // _ORCACAM_H_
