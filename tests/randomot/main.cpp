#include <QtTest/QtTest>
#include <QtGlobal>
#include <QHash>
#include <QStack>

#include "app/environment.h"
#include "app/settings.h"
#include "network/networkadapter.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "model/blipdocument.h"
#include "model/otprocessor.h"
#include "model/structureddocument.h"
#include "model/documentmutation.h"
#include "model/wavelist.h"
#include "model/participant.h"

#include <stdlib.h>

 class RandomOT: public QObject
 {
     Q_OBJECT
 private slots:
    void initTestCase();
    void randomOT();
    void cleanupTestCase();

private:
    StructuredDocument* createDocument();
 };

void RandomOT::initTestCase()
{
    // Seed
    qsrand(1);
}

void RandomOT::randomOT()
{
    for( int i = 0; i < 10; ++i )
    {
        StructuredDocument* doc = createDocument();
        doc->print_();

        delete doc;
    }
}

void RandomOT::cleanupTestCase()
{
}

StructuredDocument* RandomOT::createDocument()
{
    StructuredDocument* doc = new StructuredDocument();

    QStack<QString> tags;
    DocumentMutation m;
    int r = 0;
    while( ( r = qrand() ) >= RAND_MAX / 10 )
    {
        if ( r % 10 > 1 )
        {
            m.insertChars( QString( QChar( 'a' + (r%26) ) ) );
        }
        else if (r % 10 == 1 && tags.count() > 0 )
        {
            tags.pop();
            m.insertEnd();
        }
        else
        {
            StructuredDocument::AttributeList attribs;
            int count = qrand() % 10;
            for( int i = 0; i < count - 6; ++i )
                attribs[ QString( QChar( 'a' + (qrand()%26) ) ) ] = QString( QChar( 'a' + (qrand()%26) ) );
            QString tag = QString( QChar( 'a' + (qrand()%26) ) );
            m.insertStart( tag, attribs );
            tags.push(tag);
        }
    }

    for( int i = 0; i < tags.count(); ++i )
    {
        m.insertEnd();
    }

    // m.print_();

    doc->apply(m, "torben");
    return doc;
}

QTEST_MAIN(RandomOT)
#include "main.moc"
