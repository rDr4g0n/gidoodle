gidoodle (formerly gifbut) v508.009.010
==========
You run it and then pick a place and then it makes a gif! Easy!

You'll need ffmpeg with libx264 support.

To install this guy all up in your stuff, `make install`. After that `gidoo` from the terminal will kick off the highly polished and sickeasy process.

TODO
------
* test portability
* probably want to rewrite in python to clean this all up

* "select window" mode
* confirm/clear selections before beginning
* push config options up to user
    * scale
    * skip palette generation
    * choose out filename/path
    * start delay
    * preserve logs
* clean up messages
    * show final gif details (size, location, dimensions)
    * add progress spinner
* ensure ffmpeg is all up on
    * requires --enable-libx264
* slice last n seconds from stop?
* visual indication of delay countdown
