Arc Operating System Readme Document

2009/09


System Requirements
-------------------
cmake >= 2.6.2
gcc >= 4.3
make >= 3.81
ruby >= 1.8.7
python >= 2.6.2


Build
-----
Like the CMake's regular build process, make a build directory at the top source directory, move into it, run CMake.

> mkdir build
> cd build
> cmake ..

This will set up the build directory with default configurations.  If you want to change some configurations, for example, to specify an install directory, run CCMake instead.  You'll see the CMake configuration screen on your console.

> mkdir build
> cd build
> ccmake ..

For the detail about the configuration screen, please refer the CMake manual.

After CMake, run Make

> make

This will download the L4 microkernel from the Internet, build it, and then build the Arc servers.

> make install

'make install' installs the executable files to the directory that you specified during the CCMake configuration.


Run
---



Arc Micro Shell
---------------
Arc Micro Shell (AMS) is a program that helps you to interact with the Arc servers through a keyboard and a console.  AMS provides you some useful commands.

ps                  Shows the list of running processes
kill <Thread ID>    Terminates the running process
ls <fs>:<dir>       Shows the list of files
free                Shows the number of free memory pages

To run a program, type its path and press return key.


Source Code Structure
---------------------


