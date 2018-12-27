#ifndef QTMODBUS_GLOBAL_H
#define QTMODBUS_GLOBAL_H

#include <QtCore/qglobal.h>


#ifdef STATIC_MODBUSLIB
#define QTMODBUSSHARED_EXPORT
#else
#pragma error("build static please")
#if defined(QTMODBUS_LIBRARY)
#  define QTMODBUSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTMODBUSSHARED_EXPORT Q_DECL_IMPORT
#endif
#endif

#endif // QTMODBUS_GLOBAL_H
