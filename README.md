gidoodle (formerly gifbut) v508.009.010
==========
You run it and then pick a place and then it makes a gif! Easy!

Linux only for now. You'll also need ffmpeg with libx264 support.
This is an alpha, so stuff is a bit wonky, but it does the absolute minimum necessary to make gifs. If you run into problems, open an issue on this repo.

Prereqs
-------
* \*nix box
* ffmpeg with libx264 support (try `ffmpeg -h 2>&1 | grep libx264` and see if `--enable-libx264` shows up there)
* libx11 headers (in Ubuntu 16.10, this is found in libx11-dev)

Installation
------
Clone this repo, then `make install`. That will compile the application and copy it to `usr/local/bin/gidoo` (this may need `sudo`?). Optionally you can just run `make` and launch the binary from this project's `build` directory.

Usage
------
    # create mygif.gif in the current directory
    gidoo mygif.gif

    # wait 3s then begin recording a gif
    gidoo -d 3 mygif.gif

    # record a gif at half resolution
    gidoo -s 0.5 mygif.gif

    # help!
    gidoo --help

TODO
------
* "select window" mode
* confirm/clear selections before beginning
* clean up messages
    * show final gif details (size, location, dimensions)
    * add progress spinner
* visual indication of delay countdown
* interactive mode with a bunch of options after the video has been recorded
    * outfile location
    * slice beginning, slice end
* config file, presets, last options
* screenshot send to clipboard
