CSE 522:Real-Time Embedded Systems 
============================================
Assignment 1-Real-time Tasks Models in Linux
============================================
Team 12
-----------------------------
Team Members
-----------------------------
Chinmay Bolumbu
ASU ID:1213329869

Rama Kumar Kana Sundara
ASU ID:1213347614
=============================
Files Present
-----------------------------
ReadMe.txt
Report.pdf
task.c
Makefile
=============================
Steps to Execute
-----------------------------

1. Run 'make' command to create executable file.
2. Run the command "sudo ./a < inputfile.txt", where 'inputfile.txt' file can be replaces with different text files.

=============================
NOTE:
-----------------------------
-In Makefile replace SDK_HOME and PATHs with suitable paths. 
-In task.c,the path can be changed in the line #define PATH_TO_MICE_FILE "/dev/input/event4" to correct value
as per the settings of the tester machine.
-To enable/disable Priority Inversion mutexes change PI value to 1/0 in line "#define PI 1"


