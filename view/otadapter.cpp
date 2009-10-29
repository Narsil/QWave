#include "otadapter.h"
#include "model/blip.h"
#include "model/wavelet.h"
#include "model/wave.h"
#include "blipgraphicsitem.h"
#include "graphicstextitem.h"
#include "model/otprocessor.h"
#include "model/structureddocument.h"
#include "model/documentmutation.h"
#include "app/environment.h"
#include "network/networkadapter.h"
#include "model/waveletdelta.h"

#include <QStack>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlock>

OTAdapter::OTAdapter(BlipGraphicsItem* parent )
        : QObject( parent ), m_suspendContentsChange(false)
{
    connect(blip(), SIGNAL(update(const DocumentMutation&)), SLOT(update(const DocumentMutation&)));
}

Blip* OTAdapter::blip() const
{
    return blipItem()->blip();
}

GraphicsTextItem* OTAdapter::textItem() const
{
    return blipItem()->textItem();
}

BlipGraphicsItem* OTAdapter::blipItem() const
{
    return (BlipGraphicsItem*)parent();
}

void OTAdapter::onContentsChange( int position, int charsRemoved, int charsAdded )
{
    if ( m_suspendContentsChange )
        return;

    QTextDocument* doc = textItem()->document();
    StructuredDocument* bdoc = blip()->document();

    // Did the user modify the first block in the first blib? -> change the title
    if ( blip()->isRootBlip() && doc->findBlock(position).blockNumber() == doc->begin().blockNumber() )
    {
        QString title = doc->begin().text().mid( textItem()->forbiddenTextRange() );
        emit titleChanged(title);
    }

    // Construct a document mutation which reflects the change made to the QTextDocument.
    DocumentMutation m;
    int index = this->mapToBlip(position);
    m.retain(index);

    if ( charsRemoved > 0 )
    {
        QString text = "";
        int pos = index;
        for( int i = index; i < index + charsRemoved; ++i, pos++ )
        {
            const StructuredDocument::Item& item = (*bdoc)[pos];
            switch ( item.type )
            {
                case StructuredDocument::Char:
                    text += item.ch;
                    break;
                case StructuredDocument::Start:
                    {
                        if ( text != "" )
                        {
                            m.deleteChars(text);
                            text = "";
                        }
                        m.deleteStart(item.tagType());
                        int stack = 1;
                        while( stack > 0 )
                        {
                            pos++;
                            const StructuredDocument::Item& item2 = (*bdoc)[pos];
                            switch ( item2.type )
                            {
                                case StructuredDocument::Char:
                                    m.deleteChars( QString(item2.ch) );
                                    break;
                                case StructuredDocument::Start:
                                    stack++;
                                    m.deleteStart(item2.tagType());
                                    break;
                                case StructuredDocument::End:
                                    stack--;
                                    m.deleteEnd();
                                    break;
                            }
                        }
                    }
                    break;
                case StructuredDocument::End:
                        qDebug("OOOoooops, should not have encountered the end tag.");
                        break;
            }
        }
        if ( text != "" )
        {
            m.deleteChars(text);
            text = "";
        }
    }

    if ( charsAdded > 0 )
    {
        QString text = "";
        for( int i = 0; i < charsAdded; ++i )
        {
            QChar ch = doc->characterAt(position + i);
            if ( ch == QChar::ParagraphSeparator )
            {
                if ( text != "" )
                {
                    m.insertChars(text);
                    text = "";
                }
                if ( position + i < doc->characterCount() )
                {
                   m.insertStart("line", QHash<QString,QString>() );
                   m.insertEnd();
                }
            }
            else
                text += ch;
        }
        m.insertChars(text);
    }
    m.retain( bdoc->count() - index );

    // Send the mutation to the OTProcessor
    WaveletDelta delta;
    WaveletDeltaOperation op;
    op.setMutation(m);
    op.setDocumentId(blip()->id());
    delta.addOperation(op);
    blip()->wavelet()->processor()->handleSend(delta);
    bdoc->print_();

    // Send the mutation
    // environment()->networkAdapter()->send(m, blip()->wavelet()->id(), blip()->id());

//    for( int i = 0; i < doc->characterCount(); ++i )
//    {
//        qDebug("%i", doc->characterAt(i).unicode());
//    }
}

void OTAdapter::setGraphicsText()
{
    setGraphicsText(m_authorNames);
}

void OTAdapter::setGraphicsText(const QString& names)
{
    m_suspendContentsChange = true;
    m_authorNames = names;

    textItem()->document()->clear();
    QTextCursor cursor( textItem()->document() );
    QTextCharFormat format;

    textItem()->textCursor().insertText(names);
    textItem()->setForbiddenTextRange(names.length());

    QStack<int> stack;
    stack.push(0);
    bool isFirstLine = true;

    StructuredDocument* doc = blip()->document();
    StructuredDocument::Annotation annotation;
    annotation.endPos = 0;
    QString text = "";
    int pos = 0;
    for( QList<StructuredDocument::Item>::const_iterator it = doc->begin(); it != doc->end(); ++it, pos++ )
    {
        if ( pos == annotation.endPos )
        {
            if ( text != "" )
            {
                cursor.insertText(text, format);
                text = "";
            }
            annotation = doc->annotation(pos);
            if ( annotation.map["bold"] == "true" )
                format.setFontWeight(QFont::Bold);
            else
                format.setFontWeight(QFont::Normal);
            if ( annotation.map["italic"] == "true" )
                format.setFontItalic(true);
            else
                format.setFontItalic(false);
        }
        switch( (*it).type )
        {
            case StructuredDocument::Char:
                if ( stack.top() == 1 )
                    text += (*it).ch;
                break;
            case StructuredDocument::Start:
                if ( (*it).data.map )
                {
                    QString key = (*it).data.map->value("type");
                    if ( key == "body" )
                        stack.push(1);
                    else if ( key == "contributor" )
                        stack.push(2);
                    else if ( key == "line" )
                    {
                        if ( text != "" )
                        {
                            cursor.insertText(text, format);
                            text = "";
                        }
                        if ( !isFirstLine )
                            cursor.insertBlock();
                        stack.push(3);
                        isFirstLine = false;
                    }
                    else if ( key == "image" )
                        stack.push(4);
                    else
                        stack.push(5);
                }
                break;
            case StructuredDocument::End:
                int t = stack.pop();
                // Safety check
                if ( t == 0 )
                {
                    qDebug("Ooooops");
                    continue;
                }
                if ( t == 1 && text != "" )
                {
                    cursor.insertText(text, format);
                    text = "";
                }
                break;
        }
    }

    m_suspendContentsChange = false;
}

int OTAdapter::mapToBlip(int position)
{
    int charIndex = 0;
    int blockCount = 0;
    QTextDocument* qdoc = textItem()->document();

    // Skip the non-editable characters which are in the QTextDocument but not in the blip.
    // Now count the number of characters and line breaks up to 'position'
    for( int i = textItem()->forbiddenTextRange(); i < position; ++i )
    {
        QChar ch = qdoc->characterAt(i);
        if ( ch == QChar::ParagraphSeparator )
            blockCount++;
        else
            charIndex++;
    }

    int charsSeen = 0;
    int linesSeen = 0;
    int pos = 0;

    QStack<int> stack;
    stack.push(0);

    // Skip the required number of characters and newlines.
    StructuredDocument* doc = blip()->document();
    for( QList<StructuredDocument::Item>::const_iterator it = doc->begin(); it != doc->end(); ++it, pos++ )
    {
        switch( (*it).type )
        {
            case StructuredDocument::Char:
                // Text in the body element?
                if ( stack.top() == 1 )
                {
                    if ( charsSeen == charIndex && linesSeen - 1 == blockCount )
                        return pos;
                    charsSeen++;
                }
                break;
            case StructuredDocument::Start:
                if ( (*it).data.map )
                {
                    QString key = (*it).data.map->value("type");
                    if ( key == "body" )
                        stack.push(1);
                    else if ( key == "contributor" )
                        stack.push(2);
                    else if ( key == "line" )
                    {
                        if ( linesSeen - 1 == blockCount && charsSeen == charIndex )
                            return pos;
                        stack.push(3);
                    }
                    else if ( key == "image" )
                        stack.push(4);
                    else
                        stack.push(5);
                }
                break;
            case StructuredDocument::End:
                int t = stack.pop();
                // Safety check
                if ( t == 0 )
                {
                    qDebug("Ooooops, malformed doc");
                    continue;
                }
                // End of the last line? -> return this position since there are no additional characters
                if ( t == 1 )
                    return pos;
                else if ( t == 3 )
                {
                    linesSeen++;
                    if ( charsSeen == charIndex && linesSeen - 1 == blockCount )
                        return pos + 1;
                }
                break;
        }
    }
    qDebug("Oooops. Index not found");
    return -1;
}

Environment* OTAdapter::environment() const
{
    return blip()->wavelet()->wave()->environment();
}

void OTAdapter::update( const DocumentMutation& mutation )
{
    setGraphicsText();
}
