* Download and extract libmodbus (3.1.2) into source folder. (expected to be extracted with name libmodbus-3.1.2)
* For android a special build step "make INSTALL_ROOT="../android_lib install" has to be created
* For desktop add make install build step.
* (this is not valid anymore as generated file is released with sources) Python has to be installed. (some header files are generated during the build process)
* In Windows use MinGW builds, otherwise "unistd.h" is not found (There are ways to solve that, but just using MinGW is easier)


