#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>  
#include "libusbd.h"

#include "switch_hid.h"
#include "bmp.h"

// sleep for a specified amount of milliseconds
int msleep(long msec)
{
    struct timespec ts;
    int res;
    if (msec < 0) {
        errno = EINVAL;
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
    return res;
}

volatile sig_atomic_t stop;

#define PRINT_WIDTH 320
#define PRINT_HEIGHT 120

// time in ms to wait between changing inputs
// value too high = repeated inputs
// value too low = skipped inputs
// 24 seems fine although a few inputs might get skipped...
#define WAIT_MS 24
// timeout for the usb endpoint
#define EP_TIMEOUT 33

uint8_t pixel_grid[PRINT_HEIGHT][PRINT_WIDTH];

void inthand(int signum) {
    stop = 1;
}

bool load_bmp(const char * filename) {
    BMPHeader bmp;
    BITMAPINFOHEADER info;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("failed to read bmp\n");
        return false;
    }
    // read the bmp header
    fread(&bmp, sizeof(BMPHeader), 1, fp);
    // we only support windows BITMAPINFOHEADER format files
    if (bmp.dib_header_size != sizeof(BITMAPINFOHEADER) + sizeof(int)) {
        printf("invalid BMP format (header size: %i, expected BITMAPINFOHEADER / %lu)\n", bmp.dib_header_size, sizeof(BITMAPINFOHEADER) + sizeof(int));
        return false;
    }
    // read the BITMAPINFOHEADER
    fread(&info, sizeof(BITMAPINFOHEADER), 1, fp);
    // make sure the resolution and bit depth are correct
    if (info.width != PRINT_WIDTH || info.height != PRINT_HEIGHT) {
        printf("invalid BMP resolution (%ix%i, expected %ix%i)\n", info.width, info.height, PRINT_WIDTH, PRINT_HEIGHT);
        return false;
    }
    if (info.compression_method != 0) {
        printf("compressed BMP files are not supported\n");
        return false;
    }
    // TODO: impl reading other bitmap colour depths (and maybe PNG?)
    if (info.bit_per_pixel != 1) {
        printf("invalid BMP color depth (%i, expected 1)\n", info.bit_per_pixel);
        return false;
    }
    if (info.color_count != 2) {
        printf("invalid BMP color count (%i, expected 2)\n", info.color_count);
        return false;
    }
    // below is code for 1-bit / 2colour bitmap
    // determine which color is brighter, in a super duper hacky manner
    int colour_intensity[2] = { 0 };
    for (int i = 0; i < info.color_count; i++) {
        ColourEntry colour;
        fread(&colour, sizeof(ColourEntry), 1, fp);
        // red is the most intense, then blue, then green
        // the science behind it: i made it up
        colour_intensity[i] += (colour.red) * 10;
        colour_intensity[i] += (colour.blue) * 5;
        colour_intensity[i] += (colour.green) * 1;
    }
    // if colour 2 is more "intense" than colour 1, use that as our black pixel
    uint8_t colour1 = 0xFF;
    uint8_t colour2 = 0x00;
    if (colour_intensity[0] < colour_intensity[1]) {
        colour1 = 0x00;
        colour2 = 0xFF;
    }
    // read the pixel data
    int bytes_per_row = info.width / 8 + ((info.width % 8) ? 1 : 0);
    for (int row = 0; row < info.height; row++) {
        int column = 0;
        // for some reasons, rows are stored reversed
        fseek(fp, bmp.pixel_offset + ((info.height - row - 1) * bytes_per_row), SEEK_SET);
        for (int i = 0; i < bytes_per_row; i++) {
            uint8_t read_char = fgetc(fp);
            for (int j = 1; j <= 8; j++) {
                pixel_grid[row][column] = (read_char & (1 << (8-j))) ? colour1 : colour2;
                column++;
            }
        }
    }
    return true;
}

int main(int argc, char**argv)
{
    // prepare an example pixel grid for testing
    // * * * * * 
    //  * * * * *
    // * * * * *
    // repeating, the absolute worst case for optimisations
    memset(pixel_grid, 0, sizeof(pixel_grid));
    for (int i = 0; i < PRINT_HEIGHT; i++) {
        for (int j = i % 2; j < PRINT_WIDTH; j += 2) {
            pixel_grid[i][j] = 0xFF;
        }
    }

    if (argc != 2) {
        printf("usage: %s /path/to/2-color.bmp\n", argv[0]);
        return 0;
    }
    // passing % will use the test pattern prepared earlier
    // otherwise try to load a bmp file
    if (argv[1][0] != '%' && !load_bmp(argv[1])) {
        printf("error: not continuing\n");
        return 1;
    }

    // set up program
    libusbd_ctx_t* pCtx;
    uint8_t iface_num = 0;
    uint64_t ep_out;
    signal(SIGINT, inthand);
    libusbd_init(&pCtx);
    // set up device - act as a hori pokken controller
    libusbd_set_vid(pCtx, 0x0f0d);
    libusbd_set_pid(pCtx, 0x0092);
    libusbd_set_version(pCtx, 0x0100);
    libusbd_set_class(pCtx, 0);
    libusbd_set_subclass(pCtx, 0);
    libusbd_set_protocol(pCtx, 0);
    libusbd_set_manufacturer_str(pCtx, "HORI CO.,LTD.");
    libusbd_set_product_str(pCtx, "POKKEN CONTROLLER");
    libusbd_set_serial_str(pCtx, "");
    // set up HID interface
    libusbd_iface_alloc(pCtx, &iface_num);
    libusbd_config_finalize(pCtx);
    libusbd_iface_set_class(pCtx, iface_num, 3);
    libusbd_iface_set_subclass(pCtx, iface_num, 0);
    libusbd_iface_set_protocol(pCtx, iface_num, 0);
    uint8_t hid_desc[] = {0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, 0x5A, 0x00};
    libusbd_iface_standard_desc(pCtx, iface_num, 0x21, 0xF, hid_desc, sizeof(hid_desc));
    libusbd_iface_nonstandard_desc(pCtx, iface_num, 0x22, 0xF, JoystickReport, sizeof(JoystickReport));
    libusbd_iface_add_endpoint(pCtx, iface_num, USB_EPATTR_TTYPE_INTR, USB_EP_DIR_IN, 64, 5, 0, &ep_out);
    libusbd_iface_finalize(pCtx, iface_num);

    USB_JoystickReport_Input_t out;
    int ret = 0;
    int idx = 0;
    bool has_init = false;
    int print_x = 0;
    int print_y = 0;
    int last_x = 0;
    int last_y = 0;
    bool reversing = false;

    // main run loop
    while (!stop)
    {
        memset(&out, 0, sizeof(out));
        out.HAT = HAT_CENTER;
        out.LX = STICK_CENTER;
        out.LY = STICK_CENTER;
        out.RX = STICK_CENTER;
        out.RY = STICK_CENTER;
        // init to go to 0,0 on an empty grid
        if (!has_init) {
            // tell the terminal we're cleaning up
            if (idx == 1)
                printf("cleaning drawing state\n");
            // spam A presses to get past the controller connection screen
            if (idx < 30 && (idx & 1))
                out.Button |= SWITCH_A;
            // press left stick to clear the page
            else if (idx == 50)
                out.Button |= SWITCH_LCLICK;
            // press L a few times (alternating) to get to the smallest pixel type
            else if ((idx & 1) && idx < 50)
                out.Button |= SWITCH_L;
            // set sticks to navigate to top left of drawing
            out.LX = STICK_MIN;
            out.LY = STICK_MIN;
            // idx is roughly 450 after everything's all done and clear in the worst case
            if (idx >= 450) has_init = true;
        // actually do inputs every other loop run
        } else if (idx & 1) {
            // if we're at the end of a line, move onto the next one
            if (print_x >= PRINT_WIDTH) {
                print_x = PRINT_WIDTH - 1;
                print_y++;
                reversing = true;
            } else if (print_x < 0) {
                print_x = 0;
                print_y++;
                reversing = false;
            }
            // check if this is the end of the line (no more white after this)
            // and check for the start of the next line after that
            bool end_of_line = false;
            bool has_tripped = false;
            if (!reversing) {
                for (int i = print_x; i < PRINT_WIDTH; i++) {
                    if (pixel_grid[print_y][i] >= 0x80) has_tripped = true;
                }
                for (int i = print_x; i < PRINT_WIDTH; i++) {
                    if (pixel_grid[print_y + 1][i] >= 0x80) has_tripped = true;
                }
                end_of_line = !has_tripped;
            } else {
                for (int i = print_x; i >= 0; i--) {
                    if (pixel_grid[print_y][i] >= 0x80) has_tripped = true;
                }
                for (int i = print_x; i >= 0; i--) {
                    if (pixel_grid[print_y + 1][i] >= 0x80) has_tripped = true;
                }
                end_of_line = !has_tripped;
            }
            if (end_of_line) {
                print_y++;
                reversing = !reversing;
            }
            // if we're past the height then we're done
            if (print_y >= PRINT_HEIGHT) {
                printf("printing completed\n");
                break;
            }
            // set the dpad position based on our last position
            if (print_x > last_x && print_y > last_y)
                out.HAT = HAT_BOTTOM_RIGHT;
            else if (print_x < last_x && print_y > last_y)
                out.HAT = HAT_BOTTOM_LEFT;
            else if (print_x > last_x && print_y < last_y)
                out.HAT = HAT_TOP_RIGHT;
            else if (print_x < last_x && print_y < last_y)
                out.HAT = HAT_TOP_LEFT;
            else if (print_x > last_x)
                out.HAT = HAT_RIGHT;
            else if (print_x < last_x)
                out.HAT = HAT_LEFT;
            else if (print_y > last_y)
                out.HAT = HAT_BOTTOM;
            else if (print_y < last_y)
                out.HAT = HAT_TOP;
            printf("x: %i, y: %i\n", print_x, print_y);
            // if there's a pixel here, draw it
            if (pixel_grid[print_y][print_x] >= 0x80)
                out.Button |= SWITCH_A;
            // keep note of our last position
            last_x = print_x;
            last_y = print_y;
            // move forward in the image
            if (reversing)
                print_x--;
            else
                print_x++;
        }

        ret = libusbd_ep_write(pCtx, iface_num, ep_out, &out, sizeof(out), EP_TIMEOUT);
        if (ret == LIBUSBD_NOT_ENUMERATED) {
            printf("waiting for connection\n");
            sleep(1);
            idx = 0;
            has_init = false;
            continue;
        } else if (ret < 0) {
            printf("error (%08x)\n", ret);
            sleep(1);
        }
        msleep(WAIT_MS);
        idx++;
    }
    // send an empty hid report to prevent stuck inputs after exiting the loop
    memset(&out, 0, sizeof(out));
    libusbd_ep_write(pCtx, iface_num, ep_out, &out, sizeof(out), EP_TIMEOUT);
    // free the usb context
    libusbd_free(pCtx);
    return 0;
}