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
    qsrand(2);
}

void RandomOT::randomOT()
{
    for( int i = 0; i < 200000; ++i )
    {
        StructuredDocument* doc = createDocument();
        // doc->print_();
        StructuredDocument doc2( *doc );

        DocumentMutation* m1 = createMutation(doc);
        // m1->print_();
        DocumentMutation* m2 = createMutation(doc);
        // m2->print_();

        QString out = "\ndoc1: " + doc->toString() + "\ndoc2: " + doc2.toString() + "\nm1: " + m1->toString() + "\nm2: " + m2->toString() + "\n";

        bool ok = true;
        QPair<DocumentMutation,DocumentMutation> pair = DocumentMutation::xform( *m1, *m2, &ok );
        QVERIFY2( ok == true, out.toAscii().constData() );

        out += "m1b: " + pair.first.toString() + "\nm2b: " + pair.second.toString() + "\n";

        ok = doc->apply( *m1, "a", true );
        QVERIFY2( ok == true, out.toAscii().constData() );
        ok = doc->apply( pair.second, "b", true );
        QVERIFY2( ok == true, out.toAscii().constData() );
        ok = doc2.apply(*m2, "b", true);
        QVERIFY2( ok == true, out.toAscii().constData() );
        ok = doc2.apply(pair.first, "a", true);
        QVERIFY2( ok == true, out.toAscii().constData() );

        out += "doc1b: " + doc->toString() + "\ndoc2b: " + doc2.toString() + "\n";

        // doc->print_();
        // doc2.print_();

        QVERIFY2( doc->toString() == doc2.toString(), out.toAscii().constData() );

        delete m1;
        delete m2;
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
        int stackCount = 0;
        int ins = 0;
        while ( (ins = qrand()) % 5 == 0 )
        {
            if ( ins % 3 == 0 )
            {
                QString tag = QString( QChar( 'a' + (qrand()%26) ) );
                m->insertStart(tag);
                stackCount++;
            }
            else if ( ins % 3 == 1 && stackCount > 0 )
            {
                stackCount--;
                m->insertEnd();
            }
            else
            {
                m->insertChars( QString( QChar( 'a' + (qrand()%26) ) ) );
            }
        }
        while( stackCount > 0 )
        {
            stackCount--;
            m->insertEnd();
        }

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
                    StructuredDocument::AttributeList attribs = doc->attributesAt(i);
                    attribs.remove("**t");
                    m->deleteStart( doc->tagAt(i), attribs );
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
    while( ( r = qrand() ) >= RAND_MAX / 20 || m.count() < 5 )
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
