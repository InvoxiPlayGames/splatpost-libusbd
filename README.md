# splatpost-libusbd

A pixel printer for Splatoon 3's drawing/posting feature, using the [libusbd](https://github.com/shinyquagsire23/libusbd) library.

## Building

(As of right now, requires an Apple Silicon Mac with **SIP DISABLED (!! NOT A GOOD IDEA !!)** and at least 3 USB-C ports.)

Compile [libusbd](https://github.com/shinyquagsire23/libusbd), then copy `libusbd.dylib` to this folder and type `make` to build splatpost-libusbd. Check the libusbd README for information on compatibility and the setup process.

## Usage

1. Prepare a 1-bit / 2-colour bitmap of the image you want to use.
2. Use detached joycon or a Pro Controller (or other controller) to start Splatoon 3 and get to the post drawing screen.
3. Disconnect all your controllers (pressing the sync button / disconnecting the cable).
4. Plug your Mac into the Switch with a USB-C to USB-A cable. (currently, libusbd uses the top right port / third port along)
5. Run `./splatpost /path/to/your/bitmap.bmp` in a Terminal. Within ~30 minutes maximum, the image should be printed.
   * The image might not start drawing, your Switch still showing the controller connection screen. If this happens, CTRL+C and re-run the command.
   * Sometimes the drawing process can get slightly offset after missing a few inputs. It might help to close everything on your Mac.

*Typing `./splatpost %` will print an example alternating pixel grid.*

## TODO:

* Fix the "desync" issues where an input or two will be missed, offsetting the whole image.
* Splatoon 2 support
* More BMP colour modes + dithering
* Faster printing (TSP solving algorithm?)
* More input image files (dithering, PNG?)