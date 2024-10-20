#include <stdio.h>

#undef NDEBUG
#include "orcacam.h"

int main()
{
    int32 count;
    DCAMERR err = orca_list_devices(&count, 0, NULL);
    if (dcamerr_failed(err))
    {
        return 0;
    }
    printf("Found %d devices\n", count);
    ORCACAM cam;
    err = orca_open_camera(0, &cam, DEFAULT_FRAME_COUNT);
    if (dcamerr_failed(err))
    {
        return 0;
    }
    ORCA_CAM_INFO info;
    err = orca_device_info(cam, &info);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera model: %s\n", info.model);
    printf("Camera version: %s\n", info.cam_ver);
    printf("Camera bus: %s\n", info.bus);
close_cam:
    orca_close_camera(&cam);
    return 0;
}