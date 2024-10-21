#include <png++/png.hpp>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#undef NDEBUG
#include "orcacam.h"

void orca_frame_callback(ORCA_FRAME *frame, void *user_data,
                         size_t sz_user_data);

int main()
{
    int32 count;
    DCAMERR err = orca_list_devices(&count, 0, NULL);
    if (dcamerr_failed(err))
    {
        return 0;
    }
    printf("Found %d devices\n", count);
    if (count == 0)
    {
        return 0;
    }
    ORCACAM cam;
    err = orca_open_camera(0, &cam, DEFAULT_FRAME_COUNT);
    if (dcamerr_failed(err))
    {
        return 0;
    }
    printf("Opened camera\n");
    ORCA_CAM_INFO info;
    err = orca_device_info(cam, &info);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera model: %s %s\n", info.vendor, info.model);
    printf("Camera version: %s\n", info.cam_ver);
    printf("Camera bus: %s\n", info.bus);
    printf("Camera sensor size: %d x %d\n", info.width, info.height);
    printf("Camera pixel size: %.1f um x %.1f um\n", info.pixel_width,
           info.pixel_height);

    // Get sensor temperature
    double temp;
    err = orca_get_temperature(cam, &temp);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera temperature: %.1f C\n", temp);

    // Get sensor mode
    DCAMPROPMODEVALUE mode;
    err = orca_get_mode(cam, &mode);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera mode: 0x%x\n", mode);

    // TODO: Switch to photon counting mode
    // ERROR: Function requires stable or unstable state
    // mode = DCAMPROP_SENSORMODE__PHOTONNUMBERRESOLVING;
    // err = orca_switch_mode(cam, mode);
    // if (dcamerr_failed(err))
    // {
    //     goto close_cam;
    // }

    // TODO: Get the temperature set point
    // ERROR: Invalid property ID
    // err = orca_get_tempsetpoint(cam, &temp);
    // if (dcamerr_failed(err))
    // {
    //     goto close_cam;
    // }
    // printf("Camera temperature setpoint: %.1f C\n", temp);

    // Get exposure time
    double exposure;
    err = orca_get_exposure(cam, &exposure);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera exposure: %.6f s\n", exposure);
    // Set exposure time
    err = orca_set_exposure(cam, 0.0001);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
    err = orca_get_exposure(cam, &exposure);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera exposure: %.6f s\n", exposure);

    // Get pixel type
    DCAM_PIXELTYPE fmt;
    err = orca_get_pixel_fmt(cam, &fmt);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera pixel format: %d\n", fmt);

    // Set ROI
    // err = orca_set_roi(cam, 0, 0, 128, 128);
    // if (dcamerr_failed(err))
    // {
    //     goto close_cam;
    // }

    // Get acquisition frame rate
    double fps;
    err = orca_get_acq_framerate(cam, &fps);
    if (dcamerr_failed(err))
    {
        printf("Failed to get camera frame rate\n");
    }
    else
    {
        printf("Camera frame rate: %.1f fps\n", fps);
    }

    err = orca_set_acq_framerate(cam, 10.0);
    if (dcamerr_failed(err))
    {
        printf("Failed to set camera frame rate\n");
    }
    else
    {
        orca_get_acq_framerate(cam, &fps);
        printf("Set camera frame rate: %.1f fps\n", fps);
    }

    // Start capturing frames
    err = orca_start_capture(cam, orca_frame_callback, NULL, 0);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
    printf("Started capturing frames\n");
    sleep(11);
    // Stop capturing frames
    err = orca_stop_capture(cam);
    if (dcamerr_failed(err))
    {
        goto close_cam;
    }
close_cam:
    orca_close_camera(&cam);
    return 0;
}

void orca_frame_callback(ORCA_FRAME *frame, void *user_data,
                         size_t sz_user_data)
{
    static struct timespec last_time;
    static bool first  = true;
    static int32 count = 0;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (first)
    {
        printf("Capturing frames...\n");
        png::image<png::gray_pixel_16> img(frame->width, frame->height);
        for (int32 y = 0; y < frame->height; y++)
        {
            for (int32 x = 0; x < frame->width; x++)
            {
                if (frame->fmt == DCAM_PIXELTYPE_MONO8)
                    img[y][x] =
                        ((unsigned char *)frame->data)[y * frame->width + x]
                        << 8;
                else if (frame->fmt == DCAM_PIXELTYPE_MONO16)
                    img[y][x] =
                        ((unsigned short *)frame->data)[y * frame->width + x];
                else
                    img[y][x] = 0;
            }
        }
        img.write("frame.png");
        first = false;
    }
    if (++count % 100 == 0)
    {
        double dt = (now.tv_sec - last_time.tv_sec) +
                    (now.tv_nsec - last_time.tv_nsec) * 1e-9;
        printf("%8d: Frame rate: %.1f fps\n", count, 1.0 / dt);
    }
    last_time = now;
}