#include <png++/png.hpp>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#undef NDEBUG
#include "orcacam.h"

const char *sensormode_str(DCAMPROPMODEVALUE val)
{
    switch (val)
    {
    case DCAMPROP_SENSORMODE__AREA:
        return "AREA";
    case DCAMPROP_SENSORMODE__SLIT:
        return "SLIT";
    case DCAMPROP_SENSORMODE__LINE:
        return "LINE";
    case DCAMPROP_SENSORMODE__TDI:
        return "TDI";
    case DCAMPROP_SENSORMODE__FRAMING:
        return "FRAMING";
    case DCAMPROP_SENSORMODE__PARTIALAREA:
        return "PARTIAL AREA";
    case DCAMPROP_SENSORMODE__SLITLINE:
        return "SLIT LINE";
    case DCAMPROP_SENSORMODE__TDI_EXTENDED:
        return "TDI EXTENDED";
    case DCAMPROP_SENSORMODE__PANORAMIC:
        return "PANORAMIC";
    case DCAMPROP_SENSORMODE__PROGRESSIVE:
        return "PROGRESSIVE";
    case DCAMPROP_SENSORMODE__SPLITVIEW:
        return "SPLIT VIEW";
    case DCAMPROP_SENSORMODE__DUALLIGHTSHEET:
        return "DUAL LIGHTSHEET";
    case DCAMPROP_SENSORMODE__PHOTONNUMBERRESOLVING:
        return "PHOTON NUMBER RESOLVING";
    case DCAMPROP_SENSORMODE__WHOLELINES:
        return "WHOLE LINES";
    default:
        return "UNKNOWN";
    }
}

int main(int argc, char *argv[])
{
    DCAMPROPMODEVALUE mode = DCAMPROP_SENSORMODE__AREA; // default
    if (argc > 1)
    {
        mode = DCAMPROP_SENSORMODE__PHOTONNUMBERRESOLVING;
    }
    int32 count;
    ORCA_FRAME frame;
    DCAMERR err    = orca_list_devices(&count, 0, NULL);
    if (orcaerr_failed(err))
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
    if (orcaerr_failed(err))
    {
        return 0;
    }
    printf("Opened camera\n");
    ORCA_CAM_INFO info;
    err = orca_device_info(cam, &info);
    if (orcaerr_failed(err))
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
    if (orcaerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera temperature: %.1f C\n", temp);

    // Set sensor mode
    err = orca_switch_mode(cam, mode);
    if (orcaerr_failed(err))
    {
        goto close_cam;
    }
    // Get sensor mode
    err = orca_get_mode(cam, &mode);
    if (orcaerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera mode: %s\n", sensormode_str(mode));

    // TODO: Get the temperature set point
    // ERROR: Invalid property ID
    // err = orca_get_tempsetpoint(cam, &temp);
    // if (orcaerr_failed(err))
    // {
    //     goto close_cam;
    // }
    // printf("Camera temperature setpoint: %.1f C\n", temp);

    // Get exposure time
    double exposure;
    err = orca_get_exposure(cam, &exposure);
    if (orcaerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera exposure: %.6f s\n", exposure);
    // Set exposure time
    exposure = 0.001;
    DCAMPROP_ATTR attr;
    err = orca_get_attr(cam, DCAM_IDPROP_EXPOSURETIME, &attr);
    if (orcaerr_failed(err))
    {
        printf("Failed to get camera exposure attribute\n");
    }
    else
    {
        printf("Min exposure: %.6f s\n", attr.valuemin);
        printf("Max exposure: %.6f s\n", attr.valuemax);
        exposure = attr.valuemin;
    }
    err = orca_set_exposure(cam, exposure);
    if (orcaerr_failed(err))
    {
        goto close_cam;
    }
    err = orca_get_exposure(cam, &exposure);
    if (orcaerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera exposure: %.6f s\n", exposure);

    // Get pixel type
    DCAM_PIXELTYPE fmt;
    err = orca_get_pixel_fmt(cam, &fmt);
    if (orcaerr_failed(err))
    {
        goto close_cam;
    }
    printf("Camera pixel format: %d\n", fmt);

    // Set ROI
    // err = orca_set_roi(cam, 0, 0, 128, 128);
    // if (orcaerr_failed(err))
    // {
    //     goto close_cam;
    // }

    // Get acquisition frame rate
    double fps;
    err = orca_get_acq_framerate(cam, &fps);
    if (orcaerr_failed(err))
    {
        printf("Failed to get camera frame rate\n");
    }
    else
    {
        printf("Camera frame rate: %.1f fps\n", fps);
    }

    err = orca_set_acq_framerate(cam, 10.0);
    if (orcaerr_failed(err))
    {
        printf("Failed to set camera frame rate\n");
    }
    else
    {
        orca_get_acq_framerate(cam, &fps);
        printf("Set camera frame rate: %.1f fps\n", fps);
    }
    // Start capturing frames
    err = orca_start_acquisition(cam, &frame);
    if (!orcaerr_failed(err))
    {
        struct timespec last;
        int captured = 0;
        double framerate_measured = 0.0;
        while (captured < 10)
        {
            err = orca_acquire_image(cam, &frame, int(1 / fps * 1000) + 100);
            if (err == DCAMERR_TIMEOUT)
            {
                continue;
            }
            else if (orcaerr_failed(err))
            {
                goto close_cam;
            }
            if (captured == 0)
            {
                clock_gettime(CLOCK_MONOTONIC, &last);
            }
            else
            {
                struct timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);
                double dt = (now.tv_sec - last.tv_sec) +
                            (now.tv_nsec - last.tv_nsec) * 1e-9;
                framerate_measured *= captured - 1;
                framerate_measured += 1.0 / dt;
                framerate_measured /= captured;
                last               = now;
            }
            captured++;
        }
        png::image<png::gray_pixel_16> img(frame.width, frame.height);
        for (int32 y = 0; y < frame.height; y++)
        {
            for (int32 x = 0; x < frame.width; x++)
            {
                if (frame.fmt == DCAM_PIXELTYPE_MONO8)
                    img[y][x] =
                        ((unsigned char *)frame.data)[y * frame.width + x]
                        << 8;
                else if (frame.fmt == DCAM_PIXELTYPE_MONO16)
                    img[y][x] =
                        ((unsigned short *)frame.data)[y * frame.width + x];
                else
                    img[y][x] = 0;
            }
        }
        img.write("frame.png");
        err = orca_stop_acquisition(cam);
        if (orcaerr_failed(err))
        {
            printf("Failed to stop camera acquisition\n");
        }
        else
        {
            printf("Stopped capturing frames\n");
        }
        printf("Measured frame rate: %.1f fps\n", framerate_measured);
    }
    else
    {
        printf("Failed to start camera acquisition\n");
    }
close_cam:
    orca_close_camera(&cam);
    return 0;
}
