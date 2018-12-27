#ifndef QTMODBUS_PRIVATE_H
#define QTMODBUS_PRIVATE_H

#include <functional>
#include <QFuture>
#include "qtmodbus.h"

typedef struct _modbus modbus_t;

namespace ModBUS {

class ModbusTcpPrivate : public ModbusTcp{
public:
    ModbusTcpPrivate(QObject* parent, ModbusPrivate*);
    virtual ~ModbusTcpPrivate();
    bool startAccept(std::function<int(modbus_t*, int*)>, SocDescriptor soc);
private:
    int m_socket = -1;
    QFuture<void> m_future;
};

class ModbusTcpv4 : public ModbusTcpPrivate{
    Q_OBJECT
public:
    ModbusTcpv4(QObject* parent, ModbusPrivate*);
    virtual ~ModbusTcpv4();
    SocDescriptor listen(int nbConnection);
    bool accept(SocDescriptor s);
};

class ModbusTcpPi : public ModbusTcpPrivate{
    Q_OBJECT
public:
    ModbusTcpPi(QObject* parent, ModbusPrivate*);
    virtual ~ModbusTcpPi();
    SocDescriptor listen(int nbConnection);
    bool accept(SocDescriptor s);
};

}

#endif // QTMODBUS_PRIVATE_H

