/**
 * @file orcacam_strerr.h
 * @author Sunip K. Mukherjee (sunipkmukherjee@gmail.com)
 * @brief Error strings for Hamamatsu DCAM API
 * @version 0.0.1
 * @date 2024-10-15
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _ORCACAM_STRERR_H_
#define _ORCACAM_STRERR_H_

#include "dcamapi/dcamapi4.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/**
 * @brief Check if a DCAMERR is a failure
 *
 * @param err DCAMERR error code
 * @return int 1 if failed, 0 otherwise
 */
static inline int orcaerr_failed(DCAMERR err) { return (int)err < 0; }

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wswitch-enum"
/**
 * @brief Get the error string for a DCAM error code
 *
 * @param err DCAM error code
 * @return const char* Error string
 */
static inline const char *orcacam_sterr(DCAMERR err)
{
#define STR(ccase, str)                                                        \
    case ccase:                                                                \
        return str;
#define DEF(ccase) STR(ccase, #ccase)
    switch (err)
    {
        STR(DCAMERR_NOTREADY, "Not Ready")
        STR(DCAMERR_NOTSTABLE, "Function requires stable or unstable state")
        STR(DCAMERR_UNSTABLE, "Function does not support unstable state")
        STR(DCAMERR_NOTBUSY, "Function requires busy state")
        STR(DCAMERR_BUSY, "Function cannot process in busy state")
        STR(DCAMERR_EXCLUDED, "Resource is exclusive and already used")
        STR(DCAMERR_COOLINGTROUBLE, "Cooling trouble")
        STR(DCAMERR_NOTRIGGER, "Necessary trigger is not detected")
        STR(DCAMERR_TEMPERATURE_TROUBLE, "Temperature warning")
        STR(DCAMERR_TOOFREQUENTTRIGGER, "Input too frequent trigger")
        STR(DCAMERR_ABORT, "Aborted")
        STR(DCAMERR_TIMEOUT, "Timed out")
        STR(DCAMERR_LOSTFRAME, "Frame data is lost")
        STR(DCAMERR_MISSINGFRAME_TROUBLE,
            "Frame is lost due to low level driver bug")
        STR(DCAMERR_INVALIDIMAGE, "Invalid image data")
        STR(DCAMERR_NORESOURCE, "Not enough resources")
        STR(DCAMERR_NOMEMORY, "Not enough memory")
        STR(DCAMERR_NOMODULE, "Module not found")
        STR(DCAMERR_NODRIVER, "Driver not found")
        STR(DCAMERR_NOCAMERA, "Camera not found")
        STR(DCAMERR_NOGRABBER, "Frame grabber not found")
        STR(DCAMERR_NOCOMBINATION, "No combination found in registry")
        STR(DCAMERR_FAILOPEN, "Failed to open")
        STR(DCAMERR_FRAMEGRABBER_NEEDS_FIRMWAREUPDATE,
            "Frame grabber needs firmware update")
        STR(DCAMERR_INVALIDMODULE, "dcam_init() found invalid module")
        STR(DCAMERR_INVALIDCOMMPORT, "Invalid serial port")
        STR(DCAMERR_FAILOPENBUS, "Bus or driver failed to open")
        STR(DCAMERR_FAILOPENCAMERA, "Camera failed to open")
        STR(DCAMERR_DEVICEPROBLEM, "Device initialization failed (for maico)")
        STR(DCAMERR_INVALIDCAMERA, "Invalid camera")
        STR(DCAMERR_INVALIDHANDLE, "Invalid camera handle")
        STR(DCAMERR_INVALIDPARAM, "Invalid parameter")
        STR(DCAMERR_INVALIDVALUE, "Invalid property value")
        STR(DCAMERR_OUTOFRANGE, "Value is out of range")
        STR(DCAMERR_NOTWRITABLE, "Property is not writable")
        STR(DCAMERR_NOTREADABLE, "Property is not readable")
        STR(DCAMERR_INVALIDPROPERTYID, "Invalid property ID")
        STR(DCAMERR_NEWAPIREQUIRED, "New API required")
        STR(DCAMERR_WRONGHANDSHAKE, "Unexpected error code from camera")
        STR(DCAMERR_NOPROPERTY,
            "There is no alternative or influence ID, or no more property ID")
        STR(DCAMERR_INVALIDCHANNEL,
            "The property ID specifies an invalid channel")
        STR(DCAMERR_INVALIDVIEW, "The property ID specifies an invalid view")
        STR(DCAMERR_INVALIDSUBARRAY,
            "The property ID specifies an invalid subarray")
        STR(DCAMERR_ACCESSDENY,
            "Property can not be accessed during this DCAM status")
        STR(DCAMERR_NOVALUETEXT, "Property does not have value text")
        STR(DCAMERR_WRONGPROPERTYVALUE, "At least one property value is wrong")
        STR(DCAMERR_DISHARMONY,
            "The paired camera does not have the same parameter")
        STR(DCAMERR_FRAMEBUNDLESHOULDBEOFF, "Frame bundle should be off")
        DEF(DCAMERR_INVALIDFRAMEINDEX)
        DEF(DCAMERR_INVALIDSESSIONINDEX)
        STR(DCAMERR_NOCORRECTIONDATA,
            "Dark and shading correction data not found")
        STR(DCAMERR_CHANNELDEPENDENTVALUE,
            "Each channel has its own property value, cannot aggregate")
        STR(DCAMERR_VIEWDEPENDENTVALUE,
            "Each view has its own property value, cannot aggregate")
        STR(DCAMERR_NODEVICEBUFFER,
            "The frame count is larger than the device memory size")
        STR(DCAMERR_REQUIREDSNAP,
            "The capture mode is sequenced on using device memory");
        STR(DCAMERR_LESSSYSTEMMEMORY, "Not enough system memory")
        STR(DCAMERR_INVALID_SELECTEDLINES,
            "The combination of the selected lines are invalid, e.g. "
            "DCAM_ID_PROP_SELECTEDLINES_VPOS + "
            "DCAM_ID_PROP_SELECTEDLINES_VSIZE is greater than the number of "
            "vertical lines of the sensor")
        STR(DCAMERR_NOTSUPPORT, "Camera does not support the function or "
                                "property with current settings")
        STR(DCAMERR_FAILREADCAMERA, "Failed to read data from camera")
        STR(DCAMERR_FAILWRITECAMERA, "Failed to write data to the camera")
        STR(DCAMERR_CONFLICTCOMMPORT, "Conflict with the COM port name")
        STR(DCAMERR_OPTICS_UNPLUGGED, "Optics part is unplugged")
        STR(DCAMERR_FAILCALIBRATION, "Failed calibration")
        STR(DCAMERR_MISMATCH_CONFIGURATION,
            "Mismatch between camera output and frame grabber specs")
        STR(DCAMERR_INVALIDMEMBER_3, "3rd member variable is invalid")
        STR(DCAMERR_INVALIDMEMBER_5, "5th member variable is invalid")
        STR(DCAMERR_INVALIDMEMBER_7, "7th member variable is invalid")
        STR(DCAMERR_INVALIDMEMBER_8, "8th member variable is invalid")
        DEF(DCAMERR_INVALIDMEMBER_9)
        STR(DCAMERR_FAILEDOPENRECFILE, "Failed to open the file")
        STR(DCAMERR_INVALIDRECHANDLE, "Invalid DCAMREC handle")
        STR(DCAMERR_FAILEDWRITEDATA, "DCAMREC failed to write the data")
        STR(DCAMERR_FAILEDREADDATA, "DCAMREC failed to read the data")
        STR(DCAMERR_NOWRECORDING, "DCAMREC is recording data now")
        STR(DCAMERR_WRITEFULL, "DCAMREC writes full frame of the session")
        STR(DCAMERR_ALREADYOCCUPIED,
            "DCAMREC handle is already occupied by other HDCAM")
        STR(DCAMERR_TOOLARGEUSERDATASIZE,
            "DCAMREC set a large value to user data size")
        STR(DCAMERR_INVALIDWAITHANDLE, "DCAMWAIT is an invalid handle")
        STR(DCAMERR_NEWRUNTIMEREQUIRED, "DCAM Module Version is older than the "
                                        "version that the camera requests")
        STR(DCAMERR_VERSIONMISMATCH,
            "Camera returns the error on setting parameter to limit version")
        DEF(DCAMERR_RUNAS_FACTORYMODE)
        STR(DCAMERR_IMAGE_UNKNOWNSIGNATURE,
            "Image header signature is unknown or corrupted")
        STR(DCAMERR_IMAGE_NEWRUNTIMEREQUIRED,
            "Version of image header is newer than version that used DCAM "
            "supports")
        STR(DCAMERR_IMAGE_ERRORSTATUSEXIST, "Image header stands error status")
        STR(DCAMERR_IMAGE_HEADERCORRUPTED, "Image header value is strange")
        STR(DCAMERR_IMAGE_BROKENCONTENT, "Image content is corrupted")
        STR(DCAMERR_UNKNOWNMSGID, "Unknown message ID")
        STR(DCAMERR_UNKNOWNSTRID, "Unknown string ID")
        STR(DCAMERR_UNKNOWNPARAMID, "Unknown parameter ID")
        STR(DCAMERR_UNKNOWNBITSTYPE, "Unknown bitmap bits type")
        STR(DCAMERR_UNKNOWNDATATYPE, "Unknown frame data type")
        STR(DCAMERR_NONE, "No error")
        STR(DCAMERR_INSTALLATIONINPROGRESS, "Installation in progress")
        STR(DCAMERR_UNREACH, "Internal error")
        STR(DCAMERR_UNLOADED, "Calling after process terminated")
        STR(DCAMERR_THRUADAPTER, "Adapter error")
        STR(DCAMERR_NOCONNECTION, "Lost connection to camera")
        STR(DCAMERR_NOTIMPLEMENT, "Not implemented")
        STR(DCAMERR_DELAYEDFRAME, "Frame waiting re-load from hardware buffer "
                                  "with SNAPSHOT(EX) of DEVICEBUFFER MODE")
        STR(DCAMERR_FAILRELOADFRAME,
            "Failed to re-load frame from hardware buffer with SNAPSHOT(EX) of "
            "DEVICEBUFFER MODE")
        STR(DCAMERR_CANCELRELOADFRAME,
            "Cancel re-load frame from hardware buffer with SNAPSHOT(EX) of "
            "DEVICEBUFFER MODE")
        STR(DCAMERR_DEVICEINITIALIZING, "Device initializing")
        STR(DCAMERR_APIINIT_INITOPTIONBYTES,
            "DCAMAPI_INIT::initoptionbytes is invalid")
        STR(DCAMERR_APIINIT_INITOPTION, "DCAMAPI_INIT::initoption is invalid")
        STR(DCAMERR_INITOPTION_COLLISION_BASE,
            "Collision with initoption in DCAMAPI_INIT")
        STR(DCAMERR_INITOPTION_COLLISION_MAX,
            "Collision with initoption in DCAMAPI_INIT")
        STR(DCAMERR_MISSPROP_TRIGGERSOURCE,
            "The trigger mode is internal or syncreadout on using device "
            "memory")
        STR(DCAMERR_SUCCESS, "Success")
    default:
    {
        if (err >= DCAMERR_INITOPTION_COLLISION_BASE &&
            err <= DCAMERR_INITOPTION_COLLISION_MAX)
        {
            return "Collision with initoption in DCAMAPI_INIT";
        }
        else
        {
            return "Unknown error";
        }
    }
    }
#undef STR
#undef DEF
}
#pragma GCC diagnostic pop

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
