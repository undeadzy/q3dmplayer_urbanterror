Q3DMPlayer 0.0.1 for UrT
========================

Modified with the undeadzy_iourt changes so it can play Urban Terror 4.x
demos without any modifications or extra commendline options.

This is a demo player to help with generating stats from quake3 demos.  It
will run through your demo executing whatever SO/DLL/QVM code that you need.
It's almost identical to a normal ioquake3 client except it runs faster:

* No sound
* No rendering
* No SDL  (on Linux, only links to linux-gate, libdl, libm, libc, ld-linux)

There's a similarly named project (Q3DMParse) that's for only parsing the
demo file.  It's much faster but it doesn't update all of the client side
metadata and it has a lot of #ifdefs.  This is a much more comprehensive
approach since it runs the QVM code but it is slower.

This now uses a SQLite3 backend to store messages since Com_Printf is
unreliable (skips output at higher timescale).  This sql log is very minimal
in what it captures.  If you want to capture more, see t/sqlite3 which has
support for all QVM messages.

You'll get a bunch of warnings at the start that it cannot load models
since there's no rendering.

The reason for this project is to create log files for parsing.  In order
to get stats, you either have to parse through the *.dm_68 file and
reconstruct the way the messages are displayed (Q3DMParse way) or
you need to run the QVM code.

On my system (core 2 duo with 4GB RAM), I had these numbers with a timedemo
of a normal 25 minute Urban Terror 4.1.1 match:

    normally:  33585 frames 156.2s 215.0 fps  1.0/4.7/85.0/1.8ms
    this prog: 33585 frames  34.8s 964.8 fps  0.0/1.0/3.0/0.1ms

When I take out the NET_Sleep call and allow msec to be 0, I get this:

    !clamp:    33585 frames  28.7s 1171.1 fps 0.0/0.9/4.0/0.5ms

When I set march=native and remove the NET_Sleep calls, I get this

    !clamp&march: 33585 frames  27.7s 1214.4 fps 0.0/0.8/6.0/0.5ms


TODO
----

* Is it ok to remove the NET_Sleep call?  Why is it clamped to 1ms now?
* Find a way to force it to run as fast as possible while still writing
  out all console messages for parsing
* Make it work recursively on a directory and exit when finished.
