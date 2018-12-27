#include "qtmodbus.h"
#include "modbus.h"
#include "qtmodbus_private.h"
#include <unistd.h>
#include <QBuffer>
#include <QtConcurrent>
#include <errno.h>
#include <QMutex>

using namespace ModBUS;

#ifdef LOGALL
#define PRINT_R(x) (qDebug() << "return status:" << x)
#define PRINT_S(v, x) (qDebug() << "return status:" << x << " values:" << v.length())
#else
#define PRINT_R(x)
#define PRINT_S(v, x)
#endif

#define MPTR(xx) (xx.length() > 0 ? xx.data() : nullptr)

ModbusMapping::~ModbusMapping(){
}

class ModbusMappingPrivate : public ModbusMapping{
public:
    ModbusMappingPrivate(int nbits, int nInputBits, int nRegisters, int nInputRegisters);
    quint8*   bits()  {return MPTR(m_bits);}
    quint8*  inputBits()  {return MPTR(m_inputBits);}
    quint16*  inputRegisters()  {return MPTR(m_inputRegisters);}
    quint16*  registers()  {return MPTR(m_registers);}
    int  bitsLen() const {return m_bits.length();}
    int  inputBitsLen() const  {return m_inputBits.length();}
    int  inputRegistersLen() const  {return m_inputRegisters.length();}
    int  registersLen() const  {return m_registers.length();}
public:
    Vec8 m_bits;
    Vec8 m_inputBits;
    Vec16 m_registers;
    Vec16 m_inputRegisters;
};


ModbusMappingPrivate::ModbusMappingPrivate(int nbBits, int nbInputBits, int nbRegisters, int nbInputRegisters) :
    m_bits(nbBits, 0),
    m_inputBits(nbInputBits, 0),
    m_registers(nbRegisters, 0),
    m_inputRegisters(nbInputRegisters, 0){
}



class ModbusPrivate{
public:
    ModbusPrivate(modbus_t* mt) : m_mt(mt), m_mapping(nullptr), m_readMutex(QMutex::Recursive), m_writeMutex(QMutex::Recursive){}
    modbus_t* d() {return m_mt;}
    const modbus_t* d() const {return m_mt;}
    ~ModbusPrivate();
    void setMapping(ModbusMapping* m);
    ModbusMapping* mapping() const {return m_mapping;}
    bool retry(int value);
    void setRetryAttempts(int retries, int waitMs) {m_retries = retries; m_retryAttemps = retries; m_wait = waitMs;}
private:
    modbus_t* m_mt;
    ModbusMapping* m_mapping;
public:
    QMutex m_readMutex;
    QMutex m_writeMutex;
    int m_retries = 1;
    int m_retryAttemps = 1;
    int m_wait = 0;
};

#define LOCKR QMutexLocker _rlock(&(d_ptr->m_readMutex));
#define LOCKW QMutexLocker _wlock(&(d_ptr->m_writeMutex)); //Test: same lock, at least one fails! //Todo: check

void ModbusPrivate::setMapping(ModbusMapping* m){
    delete m_mapping;
    m_mapping = m;

}

bool ModbusPrivate::retry(int value){
    if(value < 0){
        if(m_retries >= 0 && m_retries < m_retryAttemps &&  m_wait > 0){
            qDebug() << "retry-wait" << m_retries << " " << value;
            QThread::msleep(m_wait);
        }
        --m_retries;

    }
    else{
        m_retries = m_retryAttemps;
    }
    return m_retries >= 0 && value < 0;
}

ModbusPrivate::~ModbusPrivate(){
    QMutexLocker r(&m_readMutex);
    QMutexLocker w(&m_writeMutex);
    delete m_mapping;
}

#define MODBUS (d_ptr->d())


#define RETRY_READ(_b) int _b = -1; while(d_ptr->retry(_b))
#define RETRY_WRITE(_b) int _b = -1; while(d_ptr->retry(_b))

Modbus::Modbus(QObject* parent, ModbusPrivate* modbus) : QObject(parent), d_ptr(modbus){
    qRegisterMetaType<ModBUS::Vec8>("ModbusVec8");
    qRegisterMetaType<ModBUS::Vec16>("ModbusVec16");
}

Modbus::~Modbus(){
    modbus_free(MODBUS);
}

bool Modbus::setSlave(bool slave){
    return modbus_set_slave(MODBUS, slave) == 0;
}

bool Modbus::setErrorRecovery(ErrorRecoveryMode mode){
    return modbus_set_error_recovery(MODBUS, (modbus_error_recovery_mode) mode) == 0;
}

bool Modbus::setSocket(int s){
    return modbus_set_socket(MODBUS, s) == 0;
}

SocDescriptor Modbus::getSocket() const{
    return modbus_get_socket(MODBUS);
}

bool Modbus::isConnected() const{
    return getSocket().isValid();
}

bool Modbus::getResponseTimeout(quint32 &toSec, quint32 &toUsec) const{
    return 0 == modbus_get_response_timeout(MODBUS, &toSec, &toUsec);
}

bool Modbus::setResponseTimeout(quint32 toSec, quint32 toUsec){
    return 0 == modbus_set_response_timeout(MODBUS, toSec, toUsec);
}

bool Modbus::getByteTimeout(quint32 &toSec, quint32 &toUsec) const{
    return 0 == modbus_get_byte_timeout(MODBUS, &toSec, &toUsec);
}

bool Modbus::setByteTimeout(quint32 toSec, quint32 toUsec){
    return 0 == modbus_set_byte_timeout(MODBUS, toSec, toUsec);
}

int Modbus::headerLength() const{
    return modbus_get_header_length(MODBUS);
}

bool Modbus::connect(){
    const int status =  modbus_connect(MODBUS);
    return status == 0;
}
void Modbus::close(){
    modbus_close(MODBUS);
}

int Modbus::flush(){
    return modbus_flush(MODBUS);
}

void Modbus::setDebug(bool flag){
    modbus_set_debug(MODBUS, flag);
}


int Modbus::readBits(int addr, Sec8 sec, int nb){
    if(nb < 0) nb = sec.length();
    Q_ASSERT((addr + nb) <= MODBUS_MAX_READ_BITS);
    LOCKR
    const int r = modbus_read_bits(MODBUS, addr, nb, sec.data());
    PRINT_S(sec, r);
    return r;
}


Vec8 Modbus::readBits(int addr, int nb){
    Vec8 vec(nb, 0);
    const int r = readInputBits(addr, vec);
    if(r >= 0)
        vec.resize(r);
    return vec;
}


int Modbus::readInputBits(int addr, Sec8 sec, int nb){
    if(nb < 0) nb = sec.length();
    Q_ASSERT((addr + nb) <= MODBUS_MAX_READ_BITS);
    LOCKR
    RETRY_READ(r)
        r = modbus_read_input_bits(MODBUS, addr, nb, sec.data());
    PRINT_S(sec, r);
    return r;
}

Vec8 Modbus::readInputBits(int addr, int nb){
    Vec8 vec(nb, 0);
    const int r = readInputBits(addr, vec);
    if(r >= 0)
        vec.resize(r);
    return vec;
}

int Modbus::readRegisters(int addr, Sec16 sec, int nb){
    if(nb < 0) nb = sec.length();
    Q_ASSERT((addr + nb) <= MODBUS_MAX_READ_REGISTERS);
    LOCKR
    RETRY_READ(r)
            r = modbus_read_registers(MODBUS, addr, nb, sec.data());
    PRINT_S(sec, r);
    return r;
}

Vec16 Modbus::readRegisters(int addr, int nb){
    Vec16 vec(nb, 0);
    const int r = readRegisters(addr, vec);
    if(r >= 0)
        vec.resize(r);
    return vec;
}


int Modbus::readInputRegisters(int addr, Sec16 sec, int nb){
    if(nb < 0) nb = sec.length();
    Q_ASSERT((addr + nb) <= MODBUS_MAX_READ_REGISTERS);
    LOCKR
    RETRY_READ(r)
            r = modbus_read_input_registers(MODBUS, addr, nb, sec.data());
    PRINT_S(sec, r);
    return r;
}

Vec16 Modbus::readInputRegisters(int addr, int nb){
    Vec16 vec(nb, 0);
    const int r = readInputRegisters(addr, vec);
    if(r >= 0)
        vec.resize(r);
    return vec;
}



bool Modbus::writeBit(int addr, bool status){
    Q_ASSERT(addr < MODBUS_MAX_WRITE_BITS -1);
    LOCKW
    RETRY_WRITE(r)
        r = modbus_write_bit(MODBUS, addr, status);
    PRINT_R(r);
    return r == 1;
}

bool Modbus::writeRegister(int addr, quint16 value){
    Q_ASSERT(addr < MODBUS_MAX_WRITE_REGISTERS -1);
    LOCKW
    RETRY_WRITE(r)
        r = modbus_write_register(MODBUS, addr, value);
    PRINT_R(r);
    return r == 1;
}

int Modbus::writeBits(int addr, Sec8 data){
    Q_ASSERT((addr + data.length()) < MODBUS_MAX_WRITE_BITS);
    LOCKW
    RETRY_WRITE(r)
        r = modbus_write_bits(MODBUS, addr, data.length(), data.data());
    PRINT_R(r);
    return r;
}

int Modbus::writeRegisters(int addr, Sec16 data){
    Q_ASSERT((addr + data.length()) < MODBUS_MAX_WRITE_REGISTERS);
    LOCKW
    const quint16* ptr = data.data();
    const int len = data.length();
    RETRY_WRITE(r)
        r = modbus_write_registers(MODBUS, addr, len, ptr);
    PRINT_R(r);
    return r;
}

bool Modbus::maskWriteRegister(int addr, quint16 and_mask, quint16 or_mask){
    Q_ASSERT(addr < MODBUS_MAX_WRITE_REGISTERS - 1);
    LOCKW
    RETRY_WRITE(r)
        r = modbus_mask_write_register(MODBUS, addr, and_mask, or_mask);
    PRINT_R(r);
    return r == 1;
}

int Modbus::writeAndReadRegisters(int writeAddr, Sec16 src, int readAddr, int nb, Sec16 sec){
    Q_ASSERT((writeAddr + src.length()) < MODBUS_MAX_WRITE_REGISTERS && (readAddr + nb) < MODBUS_MAX_WRITE_REGISTERS);
    LOCKR
    LOCKW
    RETRY_WRITE(r)
        r = modbus_write_and_read_registers(MODBUS, writeAddr, src.length(), src.data(),
                                           readAddr, nb, sec.data());
    PRINT_S(sec, r);
    return r;
}

Vec8 Modbus::reportSlaveId(int maxDest, int* written){
    Vec8 vec(maxDest, 0);
    LOCKR
    const int r = modbus_report_slave_id(MODBUS, maxDest, vec.data());
    if(written != NULL)
        *written = r;
    if(r >= 0)
        vec.resize(r);
    PRINT_S(vec, r);
    return vec;
}

ModbusMapping* Modbus::newMapping(int nbBits, int nbInputBits, int nbRegisters, int nbInputRegisters, bool takeOwnerShip){
    ModbusMapping* m = new ModbusMappingPrivate(nbBits, nbInputBits, nbRegisters, nbInputRegisters);
    if(takeOwnerShip)
        d_ptr->setMapping(m);
    return m;
}

ModbusMapping* Modbus::mapping() const {
    return d_ptr->mapping();
}



int Modbus::reply(Vec8 req, ModbusMapping& mapping){
    QScopedPointer<modbus_mapping_t> modmapping(new modbus_mapping_t);

    modmapping.data()->nb_bits = mapping.bitsLen();
    modmapping.data()->nb_input_bits = mapping.inputBitsLen();
    modmapping.data()->nb_registers = mapping.registersLen();
    modmapping.data()->nb_input_registers = mapping.inputRegistersLen();

    modmapping.data()->tab_bits = mapping.bits();
    modmapping.data()->tab_input_bits = mapping.inputBits();
    modmapping.data()->tab_registers = mapping.registers();
    modmapping.data()->tab_input_registers = mapping.inputRegisters();

    return modbus_reply(MODBUS, req.data(), req.length(), modmapping.data());
}

int Modbus::replyException(Vec8 req, unsigned int exceptionCode){
    return modbus_reply_exception(MODBUS, req.data(), exceptionCode);
}

int Modbus::sendRawRequest(Vec8 req){
    return modbus_send_raw_request(MODBUS, const_cast<quint8*>(req.data()), req.length());
}

bool Modbus::receive(){
      QFuture<void> future = QtConcurrent::run([this](){
        Vec8 query(MODBUS_RTU_MAX_ADU_LENGTH, 0);
        const int len = modbus_receive(MODBUS, query.data());
        if(len > 0){
            query.resize(len);
            emit Modbus::received(this, query);
        }
    });
      return future.isStarted();
}

bool Modbus::receiveConfirmation(){
    QFuture<void> future = QtConcurrent::run([this](){
        Vec8 query(MODBUS_RTU_MAX_ADU_LENGTH, 0);
        const int len = modbus_receive_confirmation(MODBUS, query.data());
        if(len > 0){
            query.resize(len);
            emit Modbus::confirmationReceived(this, query);
        }
    });
    return future.isStarted();
}


bool Modbus::readBits(int id, int addr, int nb){
    QFuture<void> future = QtConcurrent::run([this, id, addr, nb](){
        Vec8 vec(nb, 0);
        const int s = readBits(addr, vec);
        emit Modbus::bitsRead(this, id, vec, s);
    });
    return future.isStarted();
}


bool Modbus::readInputBits(int id, int addr, int nb){
    QFuture<void> future = QtConcurrent::run([this, id, addr, nb](){
        Vec8 vec(nb, 0);
        const int s = readInputBits(addr, vec);
        emit Modbus::inputBitsRead(this, id, vec, s);
    });
    return future.isStarted();
}

bool Modbus::readRegisters(int id, int addr, int nb){
    QFuture<void> future = QtConcurrent::run([this, id, addr, nb](){
        Vec16 vec(nb, 0);
        const int s = readRegisters(addr, vec);
        emit Modbus::wordsRead(this, id, vec, s);
    });
    return future.isStarted();
}
bool Modbus::readInputRegisters(int id, int addr, int nb){
    QFuture<void> future = QtConcurrent::run([this, id, addr, nb](){
        Vec16 vec(nb, 0);
        const int s = readInputRegisters(addr, vec);
        emit Modbus::inputWordsRead(this, id, vec, s);
    });
    return future.isStarted();
}


bool Modbus::writeBit(int id, int coil_addr, bool status){
    QFuture<void> future = QtConcurrent::run([this, id, coil_addr, status](){
        const int r = writeBit(coil_addr, status);
        emit Modbus::wrote(this, id, r);
    });
    return future.isStarted();
}

bool Modbus::writeRegister(int id, int reg_addr, quint16 value){
    QFuture<void> future = QtConcurrent::run([this, id, reg_addr, value](){
        const int r  = writeRegister(reg_addr, value);
        emit Modbus::wrote(this, id, r);
    });
    return future.isStarted();
}


bool Modbus::writeBits(int id, int addr, Sec8 data){
    QFuture<void> future = QtConcurrent::run([this, id, addr, data](){
        const int r = writeBits(addr, data);
        emit Modbus::wrote(this, id, r);
    });
    return future.isStarted();
}

bool Modbus::writeRegisters(int id, int addr, Sec16 data){
    QFuture<void> future = QtConcurrent::run([this, id, addr, data](){
            const int r = writeRegisters(addr, data);
            emit Modbus::wrote(this, id, r);
       });
    return future.isStarted();
}

bool Modbus::maskWriteRegister(int id, int addr, quint16 andMask, quint16 orMask){
    QFuture<void> future = QtConcurrent::run([this, id, addr, andMask, orMask](){
            const int r = maskWriteRegister(addr, andMask, orMask);
            emit Modbus::wrote(this, id, r);
       });
    return future.isStarted();
}

bool Modbus::writeAndReadRegisters(int id, int writeAddr, const Sec16 src, int readAddr, int nb){
    QFuture<void> future = QtConcurrent::run([this, id, writeAddr, src, readAddr, nb](){
            Vec16 vec(nb, 0);
            const int r = writeAndReadRegisters(writeAddr, src, readAddr, nb, vec);
            emit Modbus::wrote(this, id, r);
            emit Modbus::wordsRead(this, id, vec, r);
       });
    return future.isStarted();
}
bool Modbus::reportSlaveId(int id, int maxDest){
    QFuture<void> future = QtConcurrent::run([this, id, maxDest](){
        int s;
        Vec8 vec = reportSlaveId(maxDest, &s);
        emit Modbus::bitsRead(this, id, vec, s);
    });
    return future.isStarted();
}

bool Modbus::connectAsync(){
    QFuture<void> future = QtConcurrent::run([this](){
            bool success = connect();
            emit Modbus::connected(this, success);
       });
    return future.isStarted();
}


bool Modbus::setRetryAttempts(int retries, int waitMs){
    if(retries > 0){
        d_ptr->setRetryAttempts(retries, waitMs);
        return true;
        }
    return false;
}

/////////////////////////////////////////////

ModbusRtu::ModbusRtu(QObject* parent, ModbusPrivate* priv) : Modbus(parent, priv){
}

ModbusRtu::~ModbusRtu(){
}

int ModbusRtu::setSerialMode(int mode){
    return modbus_rtu_set_serial_mode(MODBUS, mode);
}

int ModbusRtu::serialMode() const{
    return modbus_rtu_get_serial_mode(MODBUS);
}

int ModbusRtu::setRts(int mode){
    return modbus_rtu_set_rts(MODBUS, mode);
}

int ModbusRtu::getRts() const{
    return modbus_rtu_get_rts(MODBUS);
}

///////////////////////////////////////////

ModbusTcp::ModbusTcp(QObject* parent, ModbusPrivate* priv) : Modbus(parent, priv){
}

ModbusTcp::~ModbusTcp(){
}

/////////////////////////////////////////////


ModbusTcpPrivate::ModbusTcpPrivate(QObject* parent, ModbusPrivate* priv) : ModbusTcp(parent, priv){
}

ModbusTcpPrivate::~ModbusTcpPrivate(){
    if(SocDescriptor(m_socket).isValid())
        ::close(m_socket);
    disconnect(); //does this prevent emits after now?
    m_future.waitForFinished();
}


bool ModbusTcpPrivate::startAccept(std::function<int(modbus_t*, int*)> f, SocDescriptor des){
    m_future = QtConcurrent::run([this, f, des](){
           m_socket = des;
           const int returnValue = f(MODBUS, &m_socket);
           emit ModbusTcp::accepted(this, SocDescriptor(returnValue));
    });
    return m_future.isStarted();
}

///////////////////////////////////////////


ModbusTcpv4::ModbusTcpv4(QObject* parent, ModbusPrivate* priv) : ModbusTcpPrivate(parent, priv){
}

ModbusTcpv4::~ModbusTcpv4(){
}

SocDescriptor ModbusTcpv4::listen(int nbConnection){
    return modbus_tcp_listen(MODBUS, nbConnection);
}

bool ModbusTcpv4::accept(SocDescriptor s){
    return startAccept(modbus_tcp_accept, s);
}

////////////////////////////////////////////////

ModbusTcpPi::ModbusTcpPi(QObject* parent, ModbusPrivate* priv) : ModbusTcpPrivate(parent, priv){
}

ModbusTcpPi::~ModbusTcpPi(){
}

SocDescriptor ModbusTcpPi::listen(int nbConnection){
    return modbus_tcp_pi_listen(MODBUS, nbConnection);
}

bool ModbusTcpPi::accept(SocDescriptor s){
    return startAccept(modbus_tcp_pi_accept, s);
}

//////////////////////////////////////////////

QString ModBusLibrary::version(){
    return QString(LIBMODBUS_VERSION_STRING);
}

bool ModBusLibrary::versionCheck(int major, int minor, int micro){
    return LIBMODBUS_VERSION_CHECK(major, minor, micro);
}


#define STR(s) (s.toLatin1().constData())
#define STRA(a) (a.isNull() ? NULL : STR(a.toString()))

ModbusRtu* ModBusLibrary::newRtu(QString device, int baud, char parity, int dataBit, int stopBit, QObject* parent){
    return new ModbusRtu(parent, new ModbusPrivate(modbus_new_rtu(STR(device), baud, parity, dataBit, stopBit)));
}

ModbusTcp* ModBusLibrary::newTcp(QHostAddress ipaddress, int port, QObject* parent){
    return new ModbusTcpv4(parent, new ModbusPrivate(modbus_new_tcp(STRA(ipaddress), port)));
}

ModbusTcp* ModBusLibrary::newTcpPi(QHostAddress node, int port, QObject* parent){
    return new ModbusTcpPi(parent, new ModbusPrivate(modbus_new_tcp_pi(STRA(node), STR(QString::number(port)))));
}

ModbusTcp* ModBusLibrary::newTcpPi(QHostAddress node, QString service, QObject* parent){
    return new ModbusTcpPi(parent, new ModbusPrivate(modbus_new_tcp_pi(STRA(node), STR(service))));
}

ModbusTcp* ModBusLibrary::newTcp(int port, QObject* parent){
    return new ModbusTcpv4(parent, new ModbusPrivate(modbus_new_tcp(NULL, port)));
}

ModbusTcp* ModBusLibrary::newTcpPi(int port, QObject* parent){
    return new ModbusTcpPi(parent, new ModbusPrivate(modbus_new_tcp_pi(NULL, STR(QString::number(port)))));
}

ModbusTcp* ModBusLibrary::newTcpPi(QString service, QObject* parent){
    return new ModbusTcpPi(parent, new ModbusPrivate(modbus_new_tcp_pi(NULL, STR(service))));
}

void ModBusLibrary::setBitsFromByte(Vec8& dest, int idx, const quint8 value){
    while(dest.length() <= idx)
        dest.append(0);
    modbus_set_bits_from_byte(dest.data(), idx, value);
}

void ModBusLibrary::setBitsFromByte(quint8* dest, int idx, const quint8 value){
    modbus_set_bits_from_byte(dest, idx, value);
}


void ModBusLibrary::setBitsFromBytes(Vec8& dest, int idx, int nbBits, Vec8 tabByte, int index){
    while(dest.length() <= idx + (nbBits - 1))
        dest.append(0);
    modbus_set_bits_from_bytes(dest.data(), idx, nbBits, tabByte.data() + index);
}

void ModBusLibrary::setBitsFromBytes(quint8* dest, int idx, int nbBits, const quint8* const tabByte, int index){
    modbus_set_bits_from_bytes(dest, idx, nbBits, tabByte + index);
}

quint8 ModBusLibrary::getByteFromBits(Vec8 src, int idx, unsigned int nbBits){
    return getByteFromBits(src.data(), idx, nbBits);
}

quint8 ModBusLibrary::getByteFromBits(const quint8* const src, int idx, unsigned int nbBits){
    return modbus_get_byte_from_bits(src, idx, nbBits);
}

qreal ModBusLibrary::getReal(Vec16 src, int& index){
    return getReal(src.data(), index);
}

qreal ModBusLibrary::getRealDcba(Vec16 src, int& index){
    return getRealDcba(src.data(), index);
}

void ModBusLibrary::setReal(qreal f, Vec16& dest){
    int index = dest.length();
    setReal(f, dest, index);
}

void ModBusLibrary::setRealDcba(qreal f, Vec16& dest){
    int index = dest.length();
    setRealDcba(f, dest, index);
}

void ModBusLibrary::setReal(qreal f, Vec16& dest, int& index){
    while(dest.length() <= index){
        dest.append(0);
        dest.append(0);
    }
    setReal(f, dest.data(), index);
}

void ModBusLibrary::setRealDcba(qreal f, Vec16& dest, int& index){
    while(dest.length() <= index){
        dest.append(0);
        dest.append(0);
    }
     setRealDcba(f, dest.data(), index);
}

void ModBusLibrary::setReal(qreal f, quint16 *dest, int &index){
    modbus_set_float(f, dest + index);
    index += sizeof(float) / sizeof(quint16);
}

void ModBusLibrary::setRealDcba(qreal f, quint16 *dest, int &index){
    modbus_set_float_dcba(f, dest + index);
    index += sizeof(float) / sizeof(quint16);
}

qreal ModBusLibrary::getReal(const quint16* const src, int& index){
    const qreal r = (qreal) modbus_get_float(src + index);
    index += sizeof(float) / sizeof(quint16);
    return r;
}

qreal ModBusLibrary::getRealDcba(const quint16* const src, int& index){
    const qreal r = (qreal) modbus_get_float_dcba(src + index);
    index += sizeof(float) / sizeof(quint16);
    return r;
}

QString ModBusLibrary::strError(){
    return strError(errno);
}

QString ModBusLibrary::strError(int errnum){
    return QString(modbus_strerror(errnum));
}
