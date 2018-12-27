#include "calculator.h"
#include <QStack>
#include <QHash>
#include <QStringList>

Calculator::Calculator(std::function<void (QString pring)> printer,
           std::function<void (qreal)> store,
           std::function<qreal (const quint16*, int&)> realCoder,
           std::function<void (qreal, QVector<quint16>&)> realEncoder,
           std::function<int (int, int)> onError,
            std::function<void (int, QString::iterator&)> parseError) :
    printer(printer),
    store(store),
    realCoder(realCoder),
    realEncoder(realEncoder),
    onError(onError),
    parseError(parseError){

}

QString Calculator::dump(const quint16* const data, int len) const {
    QString d;
    int pos = 0;
    while(pos < len){
        quint16 op = data[pos];
        pos++;
        if(op < 0xFF){
            d.append(char(op));
        }
        else{
            if(op & 0x1){
                int toPush = op >> 8;
                QStringList vals;
                while(toPush){
                    const qreal val = realCoder(data, pos);
                    vals.append(QString::number(val));
                    --toPush;
                }
                d.append(QString("P") + vals.join(";"));
            }
            if(op & 0x2){
                int toPush = op >> 8;
                QStringList vals;
                while(toPush){
                    const int val = data[pos];
                    vals.append(QString::number(val));
                    ++pos;
                    --toPush;
                }
                d.append(QString("R") + vals.join(";"));
            }
        }
    }
    return d;
}

QList<int> Calculator::parseInt(QString::iterator& it) const{
    QString intArray;
    while(it->isDigit() || (*it == '-' && (it + 1)->isDigit()) || *it == ';'){
        intArray.append(*it);
        it++;
    }
    --it;
    QStringList splitted = intArray.split(';', QString::SkipEmptyParts);
    QList<int> list;
    bool ok;
    for(auto s : splitted){
        list.append(s.toInt(&ok));
        if(ok == false)
            parseError(PARSE_INT, it);
    }
    return list;
}

QList<qreal> Calculator::parseReal(QString::iterator& it) const{
    QString intArray;
    while(it->isDigit() || (*it == '-' && (it + 1)->isDigit()) || *it == ';' || *it == '.'){
        intArray.append(*it);
        it++;
    }
    --it;
    QStringList splitted = intArray.split(';', QString::SkipEmptyParts);
    QList<qreal> list;
    bool ok;
    for(auto s : splitted){
        list.append(s.toDouble(&ok));
        if(ok == false)
            parseError(PARSE_REAL, it);
    }
    return list;
}

QString Calculator::parseLabel(QString::iterator& it) const{
    QString label;
    if(*it == '('){
        it++;
        while(*it != ')'){
            label.append(*it);
            if(*it < ' ' || *it > 0xF)
                parseError(PARSE_LABEL, it);
            it++;
        }
    }
    return label;
}

QVector<quint16> Calculator::parse(QString string) const {
    QHash<QString, int> symbols;
    QVector<quint16> data;
    for(auto it = string.begin(); it != string.end(); it++){
        const char c = it->toLatin1();
        switch(c){
        case 'R':{
                it++;
                QString label = parseLabel(it);
                if(label.isEmpty()){
                    QList<int> numbers =  parseInt(it);
                    data.append((numbers.length() << 8) | 0x2);
                    for(auto i : numbers)
                        data.append(i);
                }
                else{
                    data.append(symbols[label]);
                }
            } break;
        case 'P':{
            it++;
            QList<qreal> numbers =  parseReal(it);
            data.append((numbers.length() << 8) | 0x1);
            for(auto i : numbers)
                realEncoder(i, data);
        } break;
        case 'L':{
            it++;
            const int pos = std::distance(string.begin(), it);
            QString label = parseLabel(it);
            QStringList pair = label.split(',',QString::SkipEmptyParts);
            if(pair.length() == 2){
                const int offset = pair[1].toInt();
                symbols[pair[0]] = pos + offset;
            }
            else{
                if(pair.length() == 1)
                    parseError(PARSE_LABEL, it);
                symbols[label] = pos;
            }
        } break;
        case '#':
            while(*(it++) != '\n');
            break;
        default:
            if(c > ' ')
                data.append(c);
        }
    }
    return data;
}

int Calculator::run(const quint16* const data) const{
    int pos = 0;
    QStack<qreal> stack;
    for(;;){
        quint16 op = data[pos];
        ++pos;
        switch(op){
        case '!': return 0;
        case '=': printer(QString::number(stack.pop())); break;
        case 'd': stack.push(stack.top()); break;
        case 's': {
            const qreal v0 = stack.pop();
            const qreal v1 = stack.pop();
            stack.push(v0);
            stack.push(v1);
        } break;
        case 'w': store(stack.top()); break;
        case '+': stack.push(stack.pop() + stack.pop()); break;
        case '*': stack.push(stack.pop() * stack.pop()); break;
        case '-': stack.push(stack.pop() - stack.pop()); break;
        case '/':{
            const qreal v0 = stack.pop();
            const qreal v1 = stack.pop();
            if(v1 == 0.0) return onError(DIVBYZERO, pos - 1);
            stack.push(v0 / v1);
        }   break;
        case 'j':{
            const int p = (int) stack.pop();
            if(p < 0) return onError(JUMPBBEGIN, pos - 1);
            if(data[p] > 0xFF || data[p] == 0) return onError(-4, pos - 1);
            pos = p;
        }   break;
        case 'r': {
            const qreal v0 = stack.pop();
            const qreal v1 = stack.pop();
            const qreal v2 = stack.pop();
            stack.push(v1);
            stack.push(v0);
            stack.push(v2);
            } break;
        case 'z':{
            const int p = (int) stack.pop();
            if(p < 0) return onError(JUMPBBEGIN, pos - 1);
            if(data[p] > 0xFF || data[p] == 0) return onError(-4, pos - 1);
            const int z = (int) stack.pop();
            if(z == 0)
                pos = p;
        } break;
        default:{
            if(op < 0xFF){
                return onError(INVALIDVALUE, pos - 1);;
            }
            if(op & 0x1){
                int toPush = op >> 8;
                while(toPush){
                    stack.push(realCoder(data, pos));
                    --toPush;
                }
            }
            if(op & 0x2){
                int toPush = op >> 8;
                while(toPush){
                    stack.push(data[pos]);
                    ++pos;
                    --toPush;
                }
            }
        }
        }
    }
}

Calculator Calculator::forDump(std::function<qreal (const quint16*, int&)> realCoder){
    return Calculator(nullptr, nullptr, realCoder, nullptr, nullptr, nullptr);
}
Calculator Calculator::forParse(std::function<void (qreal, QVector<quint16>&)> realEncoder, std::function<void (int, QString::iterator&)> parseError){
    return Calculator(nullptr, nullptr, nullptr, realEncoder, nullptr, parseError);
}

Calculator Calculator::forRun(std::function<void (QString pring)> printer,
                         std::function<void (qreal)> store,
                         std::function<qreal (const quint16*, int&)> realCoder,
                         std::function<int (int, int)> onError){
    return Calculator(printer, store, realCoder, nullptr, onError, nullptr);
}


