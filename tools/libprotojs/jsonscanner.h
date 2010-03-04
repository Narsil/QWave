#ifndef JSONSCANNER_H
#define JSONSCANNER_H

#include <string>
#include <QtGlobal>

using namespace std;

class JSONScanner
{
public:
    enum Token
    {
        BeginObject = 1,
        EndObject = 2,
        BeginArray = 3,
        EndArray = 4,
        Comma = 5,
        TrueValue = 6,
        FalseValue = 7,
        StringValue = 8,
        NumberValue = 9,
        Colon = 10,
        End = 0,
        Error = 1000
    };

    JSONScanner(const char* ptr, int len) { m_ptr = ptr, m_len = len; }

    Token next();

    string stringValue(bool *ok);
    double doubleValue(bool *ok);
    float floatValue(bool *ok);
    qint32 int32Value(bool *ok);
    quint32 uint32Value(bool *ok);
    qint64 int64Value(bool *ok);
    quint64 uint64Value(bool *ok);
    int enumValue(bool *ok);
    int tagValue(bool *ok);

private:    
    bool ishexnstring(const QString& string);
    QString unescape( const QByteArray& ba, bool* ok );

    const char* m_ptr;
    int m_len;
    const char* m_value;
    int m_valueLen;
};

#endif // JSONSCANNER_H
