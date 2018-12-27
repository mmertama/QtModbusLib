#include "miniconnection.h"
#include "calculator.h"
#include <QTimer>

using namespace ModBUS;
static const int Smiley = 8;
static const int Command = 20;
static const int CommandAddr = 20;


enum COMMANDS{
    ECHO = 12,
    SAYHI,
    CALC
};

void MiniConnection::addLine(QString line) const{
    qDebug() << "addLine:" << line;
    m_printer.print(line);
}

void MiniConnection::massert(QString pre, bool ok) const {
    if(ok)
        addLine(pre + " ok");
    else{
        addLine("ERROR:" + pre + ":" + ModBusLibrary::strError());
    }
}


MiniClient::MiniClient(const QHostAddress& addr, int port, const Printer& printer, QObject* parent) : MiniConnection(printer, parent){
    Modbus* modbus = ModBusLibrary::newTcp(addr, port, this);
    m_modbus = modbus;
    modbus->setDebug(true);
    modbus->setResponseTimeout(3, 0);
    connect(modbus, &ModBUS::Modbus::connected, this, &MiniClient::modbusConnected);
    modbus->connectAsync();

}

void MiniClient::modbusConnected(Modbus* modbus, bool success){
    massert("Connect", success);
    if(success){
        connect(modbus, &ModBUS::Modbus::wrote, this, &MiniClient::wrote);
        connect(modbus, &ModBUS::Modbus::bitsRead, this, &MiniClient::bytesRead);
        connect(modbus, &ModBUS::Modbus::wordsRead, this, &MiniClient::wordsRead);
        emit connected(this);
    }
    else{
        emit error("Connect");
    }
}

void MiniClient::testRead3(){
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        Vec16 reg(3, 0);
        int status  = m_modbus->readRegisters(0, reg);
        if(reg.length()){
            QString s(QString("read %1 %2 %3").arg(reg[0]).arg(reg[1]).arg(reg[2]));
            addLine(s);
            }
        massert("Receive", status > 0);
        });
    timer->start(1000);
}

void MiniClient::test123(){
   massert("Test 123", m_modbus->writeRegister(20, 123));
}

void MiniClient::testCalculator(){
    TestSequence(m_modbus);
}


void MiniClient::TestSequence(Modbus* modbus){
    modbus->writeBit(Command, true);
    modbus->writeRegister(CommandAddr, SAYHI);
    calc0(modbus);
    fibonacci(modbus);
}
void MiniClient::wrote(Modbus*, int id, int status){
    if(status < 0)
        addLine(QString("wrote id:%1 length:%2").arg(id).arg(status));
    if(status <= 0)
        addLine(ModBusLibrary::strError());
}

void MiniClient::bytesRead(Modbus*, int id, Vec8 bytes, int status){
    if(status < 0)
        addLine(QString("read bytes id:%1 length:%2 status:%3").arg(id).arg(bytes.length()).arg(status));
    if(bytes.length() <= 0)
        addLine(ModBusLibrary::strError());
}

void MiniClient::wordsRead(Modbus*, int id, Vec16 bytes, int status){
    addLine(QString("read words id:%1 length:%2 status:%3").arg(id).arg(bytes.length()).arg(status));
    if(bytes.length() <= 0)
        addLine(ModBusLibrary::strError());
}


void MiniClient::testAllRegisters(){
    QString registers;
    for(int i = 0; i < MODBUS_MAX_READ_REGISTERS; i++){
        Vec16 v = m_modbus->readRegisters(i, 1);
        QString item = QString("%1:%2 ").arg(i).arg(v[0]);
        qDebug() << item;
        registers.append(item);
    }
}

void MiniClient::calc0(Modbus* modbus){
    modbus->writeBit(Smiley, true);
    QString cal("P2;3+=P5;3*P6s-=P10;3/dw=!");
    Vec16 calc = Calculator::forParse(
                [](qreal r, Vec16& v){ModBusLibrary::setReal(r, v);},
                [this, cal](int err, QString::iterator& it) mutable{
                    const int at = std::distance(cal.begin(), it);
                    addLine(QString("Parse error %1 at %2: %3").arg(err).arg(cal[at]) .arg(at));
                }).parse(cal);
    calc.prepend(CALC);
    modbus->writeBit(Command, true);
    modbus->writeRegisters(CommandAddr, calc);
    Vec16 vec = modbus->readInputRegisters(CommandAddr, sizeof(float));
    int at = 0;
    qreal result = ModBusLibrary::getReal(vec, at);
    addLine(QString::number(result));
}

#define LABEL(x) const int x = calc.length() - 1

void MiniClient::fibonacci(Modbus* modbus){
    modbus->writeBit(718, Smiley, true);
    QString fib("R10P0d=P1R14j!dr+d=rR1s-dR13zrrR14j");
    Vec16 calc = Calculator::forParse(
                [](qreal r, Vec16& v){
                    ModBusLibrary::setReal(r, v);
                },
                [this, fib](int err, QString::iterator& it) mutable{
                    const int at = std::distance(fib.begin(), it);
                    addLine(QString("Parse error %1 at %2: %3").arg(err).arg(fib[at]) .arg(at));
                })
                .parse(fib);
    calc.prepend(CALC);
    modbus->writeBit(Command, true);
    modbus->writeRegisters(453, CommandAddr, calc);
    modbus->readInputRegisters(44, CommandAddr + 2, 1);
}

MiniServer::MiniServer(QHostAddress* addr, int port, const Printer& printer, QObject* parent) : MiniConnection(printer, parent){
    ModbusTcp* modbus =  addr != nullptr ?
                ModBusLibrary::newTcp(*addr, port, this) :
                ModBusLibrary::newTcp(port, this);
    m_modbus = modbus;
    SocDescriptor des = modbus->listen(10);
    modbus->setSlave(true);
    modbus->newMapping();
    massert("Listen",  des.isValid());
    if(des.isValid()){
        connect(modbus, &ModbusTcp::accepted, this, &MiniServer::accepted);
        modbus->accept(des);
    }
}

void MiniServer::accepted(ModbusTcp* modbus, SocDescriptor socket){
    massert("Accepted", socket.isValid());
    if(socket.isValid()){
        connect(modbus, &Modbus::received, this, &MiniServer::received);
        massert("Receiving", modbus->receive());
    }
}

void MiniServer::aboutToQuit(){
    if(m_modbus)
        m_modbus->close();
}

void MiniServer::received(Modbus* modbus, Vec8 data){
    ModbusMapping* map = modbus->mapping();
    modbus->reply(data, *map);
    if(map->bits()[Smiley]){
        addLine(":-)");
        map->bits()[Smiley] = 0;
    }
    if(map->bits()[Command]){
        const int command = map->registers()[CommandAddr];
        switch(command){
        case SAYHI:
            addLine("Hi!");
            break;
        case CALC:
            const quint16* const p = &(map->registers()[CommandAddr + 1]);
            int count = 0;
            int result = Calculator::forRun([this](QString line){
                                addLine(line);
                            },
                            [map, count](qreal value) mutable {
                            ModBusLibrary::setReal(value, &(map->inputRegisters()[CommandAddr]), count);
                            },
                            [](const quint16* const p, int& pos){
                                return ModBusLibrary::getReal(p, pos);
                            },
                            [this, p](int err, int at) -> int {
                                addLine(QString("Error: %1, cmd %2 at %3").arg(err).arg(p[at]).arg(at));
                                return err;
                            }
            ).run(p);
            if(result != 0)
                 map->inputRegisters()[CommandAddr + 2] = result;
            break;
        }
        if(command != 0){
            map->bits()[Command] = 0;
            map->registers()[CommandAddr] = 0;
        }
    }
    if(m_receiving)
        massert("Receiving", modbus->receive());
}


