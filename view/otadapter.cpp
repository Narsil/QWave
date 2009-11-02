#include "otadapter.h"
#include "model/blip.h"
#include "model/wavelet.h"
#include "model/wave.h"
#include "blipgraphicsitem.h"
#include "graphicstextitem.h"
#include "model/otprocessor.h"
#include "model/blipdocument.h"
#include "model/documentmutation.h"
#include "app/environment.h"
#include "network/networkadapter.h"
#include "model/waveletdelta.h"
#include "model/participant.h"

#include <QStack>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlock>

OTAdapter::OTAdapter(BlipGraphicsItem* parent )
        : QObject( parent ), m_suspendContentsChange(false), m_blockUpdate(false)
{
//    connect(blip(), SIGNAL(update(const DocumentMutation&)), SLOT(update(const DocumentMutation&)));
    connect( blip()->document(), SIGNAL(deletedLineBreak(int,int)), SLOT(deleteLineBreak(int,int)));
    connect( blip()->document(), SIGNAL(insertedLineBreak(int,int)), SLOT(insertLineBreak(int,int)));
    connect( blip()->document(), SIGNAL(deletedText(int,int,QString)), SLOT(deleteText(int,int,QString)));
    connect( blip()->document(), SIGNAL(insertedText(int,int,QString)), SLOT(insertText(int,int,QString)));
    connect( blip()->document(), SIGNAL(mutationStart()), SLOT(mutationStart()));
    connect( blip()->document(), SIGNAL(mutationEnd()), SLOT(mutationEnd()));
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
    if ( blip()->isFirstRootBlip() && doc->findBlock(position).blockNumber() == doc->begin().blockNumber() )
    {
        QString title = doc->begin().text().mid( textItem()->forbiddenTextRange() );
        emit titleChanged(title);
    }

    // Construct a document mutation which reflects the change made to the QTextDocument.
    DocumentMutation m;
    int index = this->mapToBlip(position);
    m.retain(index);

    qDebug("INDEX=%i", index);

    if ( charsRemoved > 0 )
    {
        QString text = "";
        int pos = index;
        for( int i = 0; i < charsRemoved; ++i, pos++ )
        {
            switch ( bdoc->typeAt(pos) )
            {
                case StructuredDocument::Char:
                    text += bdoc->charAt(pos);
                    break;
                case StructuredDocument::Start:
                    {
                        if ( text != "" )
                        {
                            m.deleteChars(text);
                            text = "";
                        }
                        m.deleteStart(bdoc->tagAt(pos));
                        int stack = 1;
                        while( stack > 0 )
                        {
                            pos++;
                            switch ( bdoc->typeAt(pos) )
                            {
                                case StructuredDocument::Char:
                                    m.deleteChars( QString(bdoc->charAt(pos) ) );
                                    break;
                                case StructuredDocument::Start:
                                    stack++;
                                    m.deleteStart(bdoc->tagAt(pos));
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
        if ( !text.isEmpty() )
            m.insertChars(text);
    }
    m.retain( bdoc->count() - index );

    // Send the mutation to the OTProcessor
    m_blockUpdate = true;
    WaveletDelta delta;
    WaveletDeltaOperation op;
    op.setMutation(m);
    op.setDocumentId(blip()->id());
    delta.addOperation(op);    
    blip()->wavelet()->processor()->handleSend(delta);
    m_blockUpdate = false;
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
    // Get user names    
    m_authorNames = "";
    foreach( Participant* p, blip()->authors() )
    {
        if ( m_authorNames != "" )
            m_authorNames += ",";
        if ( p == blip()->wavelet()->wave()->environment()->localUser() )
            m_authorNames += tr("me");
        else
            m_authorNames += p->name();
    }
    m_authorNames += ": ";
    if ( blip()->authors().length() > 0 )
        blipItem()->setAuthorPixmap(blip()->authors().first()->pixmap());
    else
    {
        m_authorNames = blip()->creator()->name() + m_authorNames;
        blipItem()->setAuthorPixmap(blip()->creator()->pixmap());
    }

    m_suspendContentsChange = true;

    textItem()->document()->clear();
    QTextCursor cursor( textItem()->document() );
    QTextCharFormat format;

    textItem()->textCursor().insertText(m_authorNames);
    textItem()->setForbiddenTextRange(m_authorNames.length());

    QStack<int> stack;
    stack.push(0);
    bool isFirstLine = true;

    StructuredDocument* doc = blip()->document();
    StructuredDocument::Annotation anno;
    QString text = "";
    for( int i = 0; i < doc->count(); ++i )
    {
        StructuredDocument::Annotation a = doc->annotationAt(i);
        if ( anno != a )
        {
            if ( text != "" )
            {
                cursor.insertText(text, format);
                text = "";
            }
            anno = a;
            if ( anno.value("style/fontWeight") == "bold" )
                format.setFontWeight(QFont::Bold);
            else
                format.setFontWeight(QFont::Normal);
            if ( anno.value("style/fontStyle") == "italic" )
                format.setFontItalic(true);
            else
                format.setFontItalic(false);
        }
        switch( doc->typeAt(i) )
        {
            case StructuredDocument::Char:
                if ( stack.top() == 1 )
                    text += doc->charAt(i);
                break;
            case StructuredDocument::Start:
                {
                    QString key = doc->tagAt(i);
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

    // Did this modify the first block in the first blib? -> change the title
    if ( blip()->isFirstRootBlip()  )
    {
        QString title = textItem()->document()->begin().text().mid( textItem()->forbiddenTextRange() );
        emit titleChanged(title);
    }
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

    QStack<int> stack;
    stack.push(0);

    // Skip the required number of characters and newlines.
    StructuredDocument* doc = blip()->document();
    for( int i = 0; i < doc->count(); ++i )
    {
        switch( doc->typeAt(i) )
        {
            case StructuredDocument::Char:
                // Text in the body element?
                if ( stack.top() == 1 )
                {
                    if ( charsSeen == charIndex && linesSeen - 1 == blockCount )
                        return i;
                    charsSeen++;
                }
                break;
            case StructuredDocument::Start:
                {
                    QString key = doc->tagAt(i);
                    if ( key == "body" )
                        stack.push(1);
                    else if ( key == "contributor" )
                        stack.push(2);
                    else if ( key == "line" )
                    {
                        if ( linesSeen - 1 == blockCount && charsSeen == charIndex )
                            return i;
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
                    return i;
                else if ( t == 3 )
                {
                    linesSeen++;
                    if ( charsSeen == charIndex && linesSeen - 1 == blockCount )
                        return i + 1;
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
//    if ( !m_blockUpdate )
//        setGraphicsText();
}

void OTAdapter::mutationStart()
{
    if ( m_blockUpdate )
        return;
    m_suspendContentsChange = true;
}

void OTAdapter::insertText( int lineCount, int inlinePos, const QString& text )
{
    if ( m_blockUpdate )
        return;
    QTextDocument* doc = textItem()->document();
    QTextBlock block = doc->findBlockByNumber(lineCount);
    QTextCursor cursor(block);
    if ( lineCount == 0 )
        cursor.setPosition(inlinePos + textItem()->forbiddenTextRange());
    else
        cursor.setPosition(inlinePos);
    cursor.insertText(text);
}

void OTAdapter::deleteText( int lineCount, int inlinePos, const QString& text )
{
    if ( m_blockUpdate )
        return;
}

void OTAdapter::deleteLineBreak(int lineCount, int inlinePos)
{
    if ( m_blockUpdate )
        return;
}

void OTAdapter::insertLineBreak(int lineCount, int inlinePos)
{
    if ( m_blockUpdate )
        return;
    QTextDocument* doc = textItem()->document();
    QTextBlock block = doc->findBlockByNumber(lineCount);
    QTextCursor cursor(block);
    cursor.setPosition(inlinePos);
    cursor.insertBlock();
}

void OTAdapter::mutationEnd()
{
    if ( m_blockUpdate )
        return;
    m_suspendContentsChange = false;

    // Did this modify the first block in the first blib? -> change the title
    if ( blip()->isFirstRootBlip()  )
    {
        QString title = textItem()->document()->begin().text().mid( textItem()->forbiddenTextRange() );
        emit titleChanged(title);
    }
}
