include("config.pri")

QT       += core network concurrent

QT       -= gui

CONFIG += c++11

TARGET = qtmodbus
TEMPLATE = lib
CONFIG += staticlib #comment this line to do dynamic linking (if used as shared lib). IMHO dlls shall be used only when there are centralized deployment. Otherwise that a) has no benefits (i.e. typically there is only one app using) b) versioning problems.

DEFINES += QTMODBUS_LIBRARY

DEFINES += STATIC_MODBUSLIB

LIBMODBUS_SRC = libmodbus-3.1.2/src

message("Using Modbus:")
message($$LIBMODBUS_SRC)

DEFINES += MODBUSPATH=$$LIBMODBUS_SRC

INCLUDEPATH += $$LIBMODBUS_SRC

SOURCES += qtmodbus.cpp \
    $${LIBMODBUS_SRC}/modbus-data.c \
    $${LIBMODBUS_SRC}/modbus-rtu.c \
    $${LIBMODBUS_SRC}/modbus-tcp.c \
    $${LIBMODBUS_SRC}/modbus.c

HEADERS += qtmodbus.h\
        qtmodbus_global.h \
    $${LIBMODBUS_SRC}/modbus-private.h \
    $${LIBMODBUS_SRC}/modbus-rtu-private.h \
    $${LIBMODBUS_SRC}/modbus-rtu.h \
    $${LIBMODBUS_SRC}/modbus-tcp.h \
    $${LIBMODBUS_SRC}/modbus-version.h \
    config.h \
    $${LIBMODBUS_SRC}/modbus.h \
    qtmodbus_private.h \
    section.h

MODBUS_CMD = $${_PRO_FILE_PWD_}/define2const.py $${_PRO_FILE_PWD_}/$${LIBMODBUS_SRC}/modbus.h $${_PRO_FILE_PWD_}/modbus_const.template

QMAKE_EXTRA_TARGETS = modbus_const
modbus_const.target = modbus_const.h
modbus_const.depends = $${LIBMODBUS_SRC}/modbus.h modbus_const.template
modbus_const.commands = python $$MODBUS_CMD

DESTDIR = $${PROJECT_LIBS}

DISTFILES +=    \
    readme.txt  \
    modbus_const.template

# There definitions may need tuning, TBC (osx seems to be ok :-)
## Define to 1 if you have the `accept4' function. */
DEFINES+=HAVE_ACCEPT4=1

## Define to 1 if you have the <arpa/inet.h> header file. */
DEFINES+=HAVE_ARPA_INET_H=1

## Define to 1 if you have the <byteswap.h> header file. */
DEFINES+=HAVE_BYTESWAP_H=1

## Define to 1 if you have the declaration of `TIOCM_RTS', and to 0 if you don't. */
DEFINES+=HAVE_DECL_TIOCM_RTS=1

## Define to 1 if you have the declaration of `TIOCSRS485', and to 0 if you don't. */
DEFINES+=HAVE_DECL_TIOCSRS485=1

## Define to 1 if you have the declaration of `__CYGWIN__', and to 0 if you don't. */
DEFINES+=HAVE_DECL___CYGWIN__=1

## Define to 1 if you have the <dlfcn.h> header file. */
DEFINES+=HAVE_DLFCN_H=1

## Define to 1 if you have the <errno.h> header file. */
DEFINES+=HAVE_ERRNO_H

## Define to 1 if you have the <fcntl.h> header file. */
DEFINES+=HAVE_FCNTL_H=1

## Define to 1 if you have the `fork' function. */
DEFINES+=HAVE_FORK=1

## Define to 1 if you have the `getaddrinfo' function. */
DEFINES+=HAVE_GETADDRINFO=1

## Define to 1 if you have the `gettimeofday' function. */
DEFINES+=HAVE_GETTIMEOFDAY=1

## Define to 1 if you have the `inet_ntoa' function. */
DEFINES+=HAVE_INET_NTOA=1

## Define to 1 if you have the <inttypes.h> header file. */
DEFINES+=HAVE_INTTYPES_H=1

## Define to 1 if you have the <limits.h> header file. */
DEFINES+=HAVE_LIMITS_H=1

## Define to 1 if you have the <linux/serial.h> header file. */
DEFINES+=HAVE_LINUX_SERIAL_H=1

## Define to 1 if you have the <memory.h> header file. */
DEFINES+=HAVE_MEMORY_H=1

## Define to 1 if you have the `memset' function. */
DEFINES+=HAVE_MEMSET=1

## Define to 1 if you have the <netdb.h> header file. */
DEFINES+=HAVE_NETDB_H=1

## Define to 1 if you have the <netinet/in.h> header file. */
DEFINES+=HAVE_NETINET_IN_H=1

## Define to 1 if you have the <netinet/tcp.h> header file. */
DEFINES+=HAVE_NETINET_TCP_H=1

## Define to 1 if you have the `select' function. */
DEFINES+=HAVE_SELECT=1

## Define to 1 if you have the `socket' function. */
DEFINES+=HAVE_SOCKET=1

## Define to 1 if you have the <stdint.h> header file. */
DEFINES+=HAVE_STDINT_H=1

## Define to 1 if you have the <stdlib.h> header file. */
DEFINES+=HAVE_STDLIB_H=1

## Define to 1 if you have the `strerror' function. */
DEFINES+=HAVE_STRERROR=1

## Define to 1 if you have the <strings.h> header file. */
DEFINES+=HAVE_STRINGS_H=1

## Define to 1 if you have the <string.h> header file. */
DEFINES+=HAVE_STRING_H=1

## Define to 1 if you have the `strlcpy' function. */
DEFINES+=HAVE_STRLCPY=1

## Define to 1 if you have the <sys/ioctl.h> header file. */
DEFINES+=HAVE_SYS_IOCTL_H=1

## Define to 1 if you have the <sys/socket.h> header file. */
DEFINES+=HAVE_SYS_SOCKET_H=1

## Define to 1 if you have the <sys/stat.h> header file. */
DEFINES+=HAVE_SYS_STAT_H=1

## Define to 1 if you have the <sys/time.h> header file. */
DEFINES+=HAVE_SYS_TIME_H=1

## Define to 1 if you have the <sys/types.h> header file. */
DEFINES+=HAVE_SYS_TYPES_H=1

## Define to 1 if you have the <termios.h> header file. */
DEFINES+=HAVE_TERMIOS_H=1

## Define to 1 if you have the <time.h> header file. */
DEFINES+=HAVE_TIME_H=1

## Define to 1 if you have the <unistd.h> header file. */
DEFINES+=HAVE_UNISTD_H=1

## Define to 1 if you have the `vfork' function. */
DEFINES+=HAVE_VFORK=1

## Define to 1 if you have the <vfork.h> header file. */
DEFINES+=HAVE_VFORK_H=1

## Define to 1 if you have the <winsock2.h> header file. */
DEFINES+=HAVE_WINSOCK2_H=1

## Define to 1 if `fork' works. */
DEFINES+=HAVE_WORKING_FORK=1

## Define to 1 if `vfork' works. */
DEFINES+=HAVE_WORKING_VFORK=1

