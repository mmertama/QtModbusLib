#ifndef MINICONNECTION
#define MINICONNECTION

#include <QHostAddress>
#include "qtmodbus.h"

class Printer{
  public:
    virtual void print(QString line) const = 0;
};

class MiniConnection : public QObject{
  Q_OBJECT
public:
    MiniConnection(const Printer& printer, QObject* parent) : QObject(parent), m_printer(printer){}
    void massert(QString pre, bool ok) const;
    void addLine(QString line) const;
    void close(){delete m_modbus; m_modbus = nullptr;}
private:
    const Printer& m_printer;
protected:
    ModBUS::Modbus* m_modbus = nullptr;
};

class RakeServer;

class MiniClient : public MiniConnection{
    Q_OBJECT
public:
     MiniClient(const QHostAddress& addr, int port, const Printer& printer, QObject* parent);
     ~MiniClient(){}

private:
     void calc0(ModBUS::Modbus* modbus);
     void fibonacci(ModBUS::Modbus* modbus);
     void TestSequence(ModBUS::Modbus* modbus);
     void runRakeServer();
signals:
     void connected(MiniClient*);
     void error(QString reason);
public slots:
    void test123();
    void testRead3();
    void testCalculator();
    void modbusConnected(ModBUS::Modbus*, bool success);
    void testAllRegisters();
public slots:
    void wrote(ModBUS::Modbus*, int id, int status);
    void bytesRead(ModBUS::Modbus*, int id, ModBUS::Vec8 bytes, int status);
    void wordsRead(ModBUS::Modbus*, int id, ModBUS::Vec16 bytes, int status);
private:
};

class MiniServer : public MiniConnection{
    Q_OBJECT
public:
     MiniServer(QHostAddress* addr, int port, const Printer& printer, QObject* parent);
public slots:
    void accepted(ModBUS::ModbusTcp* modbus, ModBUS::SocDescriptor socket);
    void received(ModBUS::Modbus* modbus, ModBUS::Vec8 data);
    void aboutToQuit();
private:
    bool m_receiving = true;
};

#endif // MINICONNECTION

