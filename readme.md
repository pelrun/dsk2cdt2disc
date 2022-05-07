# DSK2CDT2DISC
## A silly disc imaging tool
James Churchill/pelrun@gmail.com 1996,2014,2020

![Windows build](https://github.com/pelrun/dsk2cdt2disc/workflows/Windows%20build/badge.svg)

Uses exomizer, 2cdt, and CALLBCA1.
Many thanks to their authors - Magnus Lind et al, arnoldemu and cngsoft.

This tool lets you get a physical copy of a .dsk image, using your CPC and a tape cable.

Usage:
```
dsk2cdt <input.dsk>
```
will output side1.cdt, side2.cdt etc depending on the number of sides in the image.

Drag-and-drop onto the app icon will also work, if the commandline isn't to your taste.

Put a blank disc (or one you don't care about) into your CPC, then enter
```
|tape
run"
```
Play the CDT with your favourite CDT/TZX tool and send the audio to the CPC using the tape cable.

Some TZX apps may not correctly handle the generated CDT's; if you get "read errors" try another tool.
I have a webapp at https://pelrun.github.io/PlayUEF/PlayUEF.html that is known to work well.

The loader will *not* wait or warn you about losing your disc contents!
