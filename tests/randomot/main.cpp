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
    DocumentMutation* createMutation(StructuredDocument* doc);
 };

void RandomOT::initTestCase()
{
    // Seed
    qsrand(1);
}

void RandomOT::randomOT()
{
    for( int i = 0; i < 20; ++i )
    {
        StructuredDocument* doc = createDocument();
        doc->print_();

        DocumentMutation* m = createMutation(doc);
        m->print_();

        delete doc;
    }
}

void RandomOT::cleanupTestCase()
{
}

DocumentMutation* RandomOT::createMutation(StructuredDocument* doc)
{
    DocumentMutation* m = new DocumentMutation();

    bool deleting = false;
    bool stopDeleting = false;
    int deleteDepth = 0;
    for( int i = 0; i < doc->count(); ++i )
    {
        if ( deleting && qrand() % 3 == 0 )
        {
            if ( deleteDepth == 0 )
            {
                stopDeleting = false;
                deleting = false;
            }
            else
               stopDeleting = true;
        }
        switch( doc->typeAt(i) )
        {
            case StructuredDocument::Start:
            {
                int r = qrand();
                if ( r % 10 <= 1 || deleting )
                {
                    deleting = true;
                    m->deleteStart( doc->tagAt(i), doc->attributesAt(i) );
                    deleteDepth++;
                }
                else if ( r % 10 <= 3 )
                {
                    m->retain(1);
                }
                else if ( r % 10 <= 6 )
                {
                    // Update
                    StructuredDocument::AttributeList attribs = doc->attributesAt(i);
                    QHash<QString,StructuredDocument::StringPair> update;
                    int count = qrand() % 4 + 1;
                    for( int a = 0; a < count; ++a )
                    {
                        QString at( QChar( 'a' + (qrand()%10) ) );
                        update[at] = StructuredDocument::StringPair(attribs[at], QString( QChar( 'a' + (qrand()%26) ) ) );
                    }
                    m->updateAttributes(update);
                }
                else
                {
                    // Replace
                    StructuredDocument::AttributeList attribs = doc->attributesAt(i);
                    StructuredDocument::AttributeList newAttribs;
                    int count = qrand() % 3 + 1;
                    for( int i = 0; i < count; ++i )
                        newAttribs[ QString( QChar( 'a' + (qrand()%10) ) ) ] = QString( QChar( 'a' + (qrand()%26) ) );
                    attribs.remove("**t");
                    m->replaceAttributes( attribs, newAttribs );
                }
            }
            break;
            case StructuredDocument::End:
                if ( deleting )
                {
                    if ( deleteDepth == 0 )
                    {
                        m->retain(1);
                        stopDeleting = false;
                        deleting = false;
                    }
                    else
                    {
                        m->deleteEnd();
                        deleteDepth--;
                        if ( stopDeleting && deleteDepth == 0 )
                        {
                            stopDeleting = false;
                            deleting = false;
                        }
                    }
                }
                else
                    m->retain(1);
                break;
            case StructuredDocument::Char:
            {
                QString text( doc->charAt(i) );
                int r = qrand();
                if ( r % 10 <= 1 || deleting )
                {
                    deleting = true;
                    m->deleteChars(text);
                }
                else
                    m->retain(1);
            }
            break;
        }
    }

    return m;
}

StructuredDocument* RandomOT::createDocument()
{
    StructuredDocument* doc = new StructuredDocument();

    QStack<QString> tags;
    DocumentMutation m;
    int r = 0;
    while( ( r = qrand() ) >= RAND_MAX / 20 )
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
            int count = qrand() % 9;
            for( int i = 0; i < count - 5; ++i )
                attribs[ QString( QChar( 'a' + (qrand()%10) ) ) ] = QString( QChar( 'a' + (qrand()%26) ) );
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
