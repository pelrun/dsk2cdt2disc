copy dsk2cdt.exe, 2cdt.exe and a CDT into a empty working directory.

./dsk2cdt <file.cdt>

on windows (if you aren't using bash):

ren buildcdt.sh buildcdt.bat
buildcdt

on anything else:

sh ./buildcdt.sh

you will get side1.cdt, side2.cdt etc depending on the number of sides in the dsk.

