#ifndef CALCULATOR
#define CALCULATOR

#include <QString>
#include <functional>

class Calculator{
public:
    enum{
        UNKNOWN = 0,
        PARSE_INT,
        PARSE_REAL,
        PARSE_LABEL,
        DIVBYZERO,
        JUMPBBEGIN,
        INVALIDVALUE
        //missing STACKOVERFLOW
    };
    inline static quint16 P(int count) {return (quint16) (count << 8) | 0x1;}
    inline static quint16 R(int count) {return (quint16) (count << 8) | 0x2;}
    static Calculator forDump(std::function<qreal (const quint16*, int&)> realCoder);
    static Calculator forParse(std::function<void (qreal, QVector<quint16> &)> realEncoder,
                               std::function<void (int, QString::iterator&)> parseError); //either & or mutable, I choose & as it is less error prone even not so Qt'ish
    static Calculator forRun(std::function<void (QString pring)> printer,
                             std::function<void (qreal)> store,
                             std::function<qreal (const quint16*, int&)> realCoder,
                             std::function<int (int, int)> onError);
    Calculator(std::function<void (QString string)> printer,
               std::function<void (qreal)> store,
               std::function<qreal (const quint16*, int&)> realCoder,
               std::function<void (qreal, QVector<quint16> &)> realEncoder,
               std::function<int (int, int)> onError,
               std::function<void (int, QString::iterator&)> parseError);
    int run(const quint16* const data) const;
    QString dump(const quint16* const data, int len) const;
    QVector<quint16> parse(QString string) const;
private:
  QList<int> parseInt(QString::iterator& it) const;
  QList<qreal> parseReal(QString::iterator& it) const;
  QString parseLabel(QString::iterator& it) const;
  std::function<void (QString pring)> printer;
  std::function<void (qreal)> store;
  std::function<qreal (const quint16*s, int&)> realCoder;
  std::function<void (qreal, QVector<quint16>&)> realEncoder;
  std::function<int (int, int)> onError;
  std::function<void (int, QString::iterator&)> parseError;
};

#endif // CALCULATOR

