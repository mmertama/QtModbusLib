#ifndef QTMODBUS_H
#define QTMODBUS_H

#include <QHostAddress>
#include <QBuffer>
#include <QPointer>
#include "modbus_const.h"
#include "qtmodbus_global.h"
#include "section.h"

class ModbusPrivate;


namespace ModBUS {

typedef QVector<quint8> Vec8;
typedef QVector<quint16> Vec16;


class QTMODBUSSHARED_EXPORT SocDescriptor{
  public:
    SocDescriptor(int value) : m_value(value){}
    bool isValid() const {return m_value != -1;}
    operator int() const {return m_value;}
private:
     const int m_value;
};


class QTMODBUSSHARED_EXPORT ModbusMapping {
public:
    virtual ~ModbusMapping();
    virtual quint8*   bits()  = 0;
    virtual quint8*  inputBits()  = 0;
    virtual quint16*  inputRegisters()  = 0;
    virtual quint16*  registers()  = 0;
    virtual int  bitsLen() const = 0;
    virtual int  inputBitsLen() const = 0;
    virtual int  inputRegistersLen() const = 0;
    virtual int  registersLen() const = 0;
};


typedef Section<quint8> Sec8;
typedef Section<quint16> Sec16;

class QTMODBUSSHARED_EXPORT Modbus : public QObject{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Modbus)
public:
    Modbus(QObject* parent, ModbusPrivate*);
    virtual ~Modbus();
public:
    enum ErrorRecoveryMode{
            ERRORRECOVERY_NONE          = 0,
            ERRORRECOVERY_LINK          = (1<<1),
            ERRORRECOVERY_PROTOCOL      = (1<<2),
            ERRORRECOVERY_ALL           = (ERRORRECOVERY_LINK | ERRORRECOVERY_PROTOCOL)
    };
public:
    bool setSlave(bool slave);
    bool setErrorRecovery(ErrorRecoveryMode mode);
    bool setSocket(int s);
    SocDescriptor getSocket() const;

    bool getResponseTimeout(quint32 &toSec, quint32 &toUsec) const;
    bool setResponseTimeout(quint32 toSec, quint32 toUsec);

    bool getByteTimeout(quint32 &toSec, quint32 &toUsec) const;
    bool setByteTimeout(quint32 toSec, quint32 toUsec);

    int headerLength() const;

    bool connect();

    bool isConnected() const;

    /*
     * This command calls connect in its own thread, as libmodbus:: TCP connect eventually calls
     * os select, that may have issues in in windows systems. TBC, TBT
     */
    bool connectAsync();
    void close();

    int flush();
    void setDebug(bool debug);

    Vec8 readBits(int addr, int nb);
    Vec8 readInputBits(int addr, int nb);
    Vec16 readRegisters(int addr, int nb);
    Vec16 readInputRegisters(int addr, int nb);

    int readBits(int addr, Sec8 sec, int nb = -1);
    int readInputBits(int addr, Sec8 sec, int nb = -1);
    int readRegisters(int addr, Sec16 sec, int nb = -1);
    int readInputRegisters(int addr, Sec16 sec, int nb = -1);

    bool readBits(int id, int addr, int nb);
    bool readInputBits(int id, int addr, int nb);
    bool readRegisters(int id, int addr, int nb);
    bool readInputRegisters(int id, int addr, int nb);

    bool writeBit(int coil_addr, bool status);
    bool writeRegister(int reg_addr, quint16 value);

    int writeBits(int addr, Sec8 data);
    int writeRegisters(int addr, Sec16 data);

    bool maskWriteRegister(int addr, quint16 andMask, quint16 orMask);

    int writeAndReadRegisters(int writeAddr, Sec16 src, int readAddr, int nb, Sec16 sec);

    Vec8 reportSlaveId(int maxDest, int* written = nullptr);

    bool writeBit(int id, int coil_addr, bool status);
    bool writeRegister(int id, int reg_addr, quint16 value);

    bool writeBits(int id, int addr, Sec8 data);
    bool writeRegisters(int id, int addr, Sec16 data);

    bool maskWriteRegister(int id, int addr, quint16 andMask, quint16 orMask);

    bool writeAndReadRegisters(int id, int writeAddr, const Sec16 src, int readAddr, int nb);

    bool reportSlaveId(int id, int maxDest);

    ModbusMapping* mapping() const;

    ModbusMapping* newMapping(int nbBits = MODBUS_MAX_READ_BITS, int nbInputBits = MODBUS_MAX_WRITE_BITS, int nbRegisters = MODBUS_MAX_READ_REGISTERS, int nbInputRegisters = MODBUS_MAX_WRITE_REGISTERS, bool takeOwnership = true);

    int sendRawRequest(Vec8 req);

    int reply(Vec8 req, ModbusMapping& mapping);
    int replyException(Vec8 req, unsigned int exceptionCode);

    bool receive();
    bool receiveConfirmation();

    bool setRetryAttempts(int retries, int waitMs = 0);

signals:
    void received(Modbus*, ModBUS::Vec8 data);
    void confirmationReceived(Modbus*, ModBUS::Vec8 data);
    void wrote(Modbus*, int id, int status);
    void bitsRead(Modbus*, int id, ModBUS::Vec8 data, int status);
    void wordsRead(Modbus*, int id, ModBUS::Vec16 data, int status);
    void inputBitsRead(Modbus*, int id, ModBUS::Vec8 data, int status);
    void inputWordsRead(Modbus*, int id, ModBUS::Vec16 data, int status);
    void connected(Modbus*, bool success);


protected:
     QScopedPointer<ModbusPrivate> d_ptr;
};

class QTMODBUSSHARED_EXPORT ModbusRtu : public Modbus{
    Q_OBJECT

public:
    enum{MODBUSRTURTS_NONE = 0, MODBUSRTURTSUP = 1, MODBUSRTURTS_DOWN = 2};
public:
    ModbusRtu(QObject* parent, ModbusPrivate*);
    virtual ~ModbusRtu();
    int setSerialMode(int mode);
    int serialMode() const;
    int setRts(int mode);
    int getRts() const;
};


class QTMODBUSSHARED_EXPORT ModbusTcp : public Modbus{
    Q_OBJECT
public:
    static const int TCP_DEFAULT_PORT  =  502;
    static const int TCPSLAVE  =  0xFF;
    ModbusTcp(QObject* parent, ModbusPrivate*);
    virtual ~ModbusTcp();
    virtual SocDescriptor listen(int nbConnection) = 0;
    virtual bool accept(SocDescriptor s) = 0;
signals:
    void accepted(ModbusTcp*, int s);
};

class QTMODBUSSHARED_EXPORT ModBusLibrary{
public:
    static QString version();
    static bool versionCheck(int major, int minor, int micro);
    static ModbusRtu* newRtu(QString device, int baud, char parity, int dataBit, int stopBit, QObject* parent = NULL);
    static ModbusTcp* newTcp(QHostAddress ipaddress, int port = ModbusTcp::TCP_DEFAULT_PORT, QObject* parent = NULL);
    static ModbusTcp* newTcpPi(QHostAddress node, int port = ModbusTcp::TCP_DEFAULT_PORT, QObject* parent = NULL);
    static ModbusTcp* newTcpPi(QHostAddress node, QString service, QObject* parent = NULL);
    static ModbusTcp* newTcp(int port = ModbusTcp::TCP_DEFAULT_PORT, QObject* parent = NULL);
    static ModbusTcp* newTcpPi(int port = ModbusTcp::TCP_DEFAULT_PORT, QObject* parent = NULL);
    static ModbusTcp* newTcpPi(QString service, QObject* parent = NULL);

    static inline quint8 getHighByte(quint16 data) {return quint8 (data >> 8);}

    static inline quint8 getLowByte(quint16 data) {return quint8 (data);}

    template <typename T>
    static qint32 getInt32(QVector<T> tab, int index) {return tab[index] << (8 * sizeof(T)) + tab[index + 1];}
    template <typename T>
        static qint32 getInt32(const T* const tab, int index) {return tab[index] << (8 * sizeof(T)) + tab[index + 1];}


    static inline void set(Vec8 tab, int index, quint16 value) {tab[index] = (quint8) (value >> 8); tab[index + 1] = quint8 (value) & 0xFF;}
    static inline void set(quint8* tab, int index, quint16 value) {tab[index] = (quint8) (value >> 8); tab[index + 1] = quint8 (value) & 0xFF;}

    static void setBitsFromByte(Vec8& des, int idx, const quint8 value);
    static void setBitsFromBytes(Vec8& dest, int idx, int nbBits, Vec8 tabByte, int index);

    static void setBitsFromByte(quint8* dest, int idx, const quint8 value);
    static void setBitsFromBytes(quint8* dest, int idx, int nbBits, const quint8* const tabByte, int index);


    static quint8 getByteFromBits(Vec8 src, int idx, unsigned int nbBits);
    static qreal getReal(Vec16 src, int& index);
    static qreal getRealDcba(Vec16 src, int& index);

    static quint8 getByteFromBits(const quint8* const src, int idx, unsigned int nbBits);
    static qreal getReal(const quint16* const src, int& index);
    static qreal getRealDcba(const quint16* const src, int& index);

    static void setReal(qreal f, Vec16& dest);
    static void setRealDcba(qreal f, Vec16& dest);

    static void setReal(qreal f, Vec16& dest, int& index);
    static void setRealDcba(qreal f, Vec16& dest, int& index);

    static void setReal(qreal f, quint16* dest, int& index);
    static void setRealDcba(qreal f, quint16* dest, int& index);

    static QString strError(int errnum);
    static QString strError();
};

} //namespace Modbus

Q_DECLARE_METATYPE(ModBUS::Vec8)
Q_DECLARE_METATYPE(ModBUS::Vec16)

#endif // QTMODBUS_H
