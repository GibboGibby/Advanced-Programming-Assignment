# Advanced Programming Assignment

Repo for my advanced programming assignment

The main solution is split into 2 projects

- Server
- Client

And there is a core.h header file that includes the winsock (and opencv) header files, as well as information that would need to be shared between the two executable files or that doesn't fit in one specific file

This includes the structs that will be sent between the client and server
As well as the enums
And lambda functions
and others (maps and inline functions)

Compile Instructions:
A full build of opencv (including optional modules) needs to be installed. By default the visual studio project points to C:\OpenCV
Then open the solution and both projects should be able to be compiled
Once compiled they can be run from the command line
