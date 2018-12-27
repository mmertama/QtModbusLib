* This library has been delayed a bit, therefore libmodbus it is originally wrapped is a bit dated but nothing prevents to try the most recent version.
* It has been tested on OSX, Android, Windows and Linux (Ubuntu), yet this released pro file may support only OSX, but I assume other platforms are very easy to build.  
* libmodbus is found https://libmodbus.org/
* Download and extract libmodbus (3.1.2) into source folder. (expected to be extracted with name libmodbus-3.1.2)
* For android a special build step "make INSTALL_ROOT="../android_lib install" has to be created
* For desktop add make install build step.
* (this is not valid anymore as generated file is released with sources) Python has to be installed. (some header files are generated during the build process)
* In Windows use MinGW builds, otherwise "unistd.h" is not found (There are ways to solve that, but just using MinGW is easier)


