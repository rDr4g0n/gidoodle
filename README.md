gidoodle (formerly gifbut) v508.009.010
==========
You run it and then pick a place and then it makes a gif! Easy!

Linux only for now. You'll also need ffmpeg with libx264 support.
This is an alpha, so stuff is a bit wonky, but it does the absolute minimum necessary to make gifs. If you run into problems, open an issue on this repo.

Installation
------
To install this guy all up in your stuff, clone this repo, then `make`. That will compile the application and copy it to `usr/local/bin/gidoo` (this may need `sudo`?). After that `gidoo` from the terminal will kick off the highly polished and sickeasy process.

TODO
------
* "select window" mode
* confirm/clear selections before beginning
* escape to cancel (when in drawing mode)
* clean up messages
    * show final gif details (size, location, dimensions)
    * add progress spinner
    * "ctrl+c" to quit
* ensure ffmpeg is all up on
    * requires --enable-libx264
    * find location of ffmpeg instead of assuming its location
* visual indication of delay countdown
* interactive mode with a bunch of options after the video has been recorded
    * outfile location
    * slice beginning, slice end
