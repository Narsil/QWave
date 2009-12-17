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
#include "model/contacts.h"
#include "model/attachment.h"
#include "caret.h"
#include "gadgets/gadgetview.h"

#include <QStack>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTimer>

OTAdapter::OTAdapter(BlipGraphicsItem* parent )
        : QObject( parent ), m_suspendContentsChange(false), m_blockUpdate(false), m_timer(0)
{
    // Connect to changes made to the BlipDocument. In response the slots will update the GUI.
    connect( blip()->document(), SIGNAL(deletedLineBreak(int)), SLOT(deleteLineBreak(int)));
    connect( blip()->document(), SIGNAL(insertedLineBreak(int)), SLOT(insertLineBreak(int)));
    connect( blip()->document(), SIGNAL(deletedText(int,QString)), SLOT(deleteText(int,QString)));
    connect( blip()->document(), SIGNAL(insertedText(int,QString)), SLOT(insertText(int,QString)));
    connect( blip()->document(), SIGNAL(mutationStart()), SLOT(mutationStart()));
    connect( blip()->document(), SIGNAL(mutationEnd()), SLOT(mutationEnd()));
    connect( blip()->document(), SIGNAL(insertImage(int,QString,QImage,QString)), SLOT(insertImage(int,QString,QImage,QString)));
    connect( blip()->document(), SIGNAL(insertGadget(int,QString,QString)), SLOT(insertGadget(int,QString,QString)));
    connect( blip()->document(), SIGNAL(setCursor(int,QString)), SLOT(setCursor(int,QString)));
    connect( blip()->document(), SIGNAL(setStyle(QString,QString,int,int)), SLOT(setStyle(QString,QString,int,int)));
}

OTAdapter::~OTAdapter()
{
    if ( m_timer )
        delete m_timer;
    foreach( Cursor* c, m_cursors.values() )
        delete c;
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

void OTAdapter::onStyleChange( int position, int charsFormatted, const QString& style, const QString& value )
{
    if ( m_suspendContentsChange )
        return;

    StructuredDocument* bdoc = blip()->document();

    // Check if the user formatted some cursors (i.e carets)
    foreach( Cursor* c, m_cursors.values() )
    {
        QString a = CaretInterface::caretOwner(c->m_textCursor);
        if ( a != c->m_participant->address() )
            charsFormatted--;
    }

    // Nothing to format at all?
    if ( charsFormatted == 0 )
        return;

    // Construct a document mutation which reflects the change made to the QTextDocument.
    DocumentMutation m;
    // Map the position inside of QTextDocument to an index inside the StructuredDocument
    int index = this->mapToBlip(position);
    // Skip all characters up to this point of modification
    m.retain(index);

    // StructuredDocument::Annotation anno2 = bdoc->annotationAt(index);
    // QString oldvalue = anno2.value(style);

    // Choose some value which will trigger the if clause below when it is executed the first time
    QString oldvalue = "$$$$$$$";

    // Count number of characters since (a) the start of the style change or (b) the last annotation boundary
    int docFormatted = 0;
    int changeStart = 0;
    int pos = index;

    // Iterate over the document and find out where annotations need to be changed.
    for( int i = 0; i < charsFormatted; ++i, pos++ )
    {
        StructuredDocument::Annotation anno = bdoc->annotationAt(pos);
        QString current = anno.value(style);

        // There is a style change in the document?
        if ( current != oldvalue )
        {
            // The document style does not have the desired value?
            if ( current != value )
            {
                m.retain(docFormatted - changeStart);
                changeStart = docFormatted;
                oldvalue = current;
                // Begin an annotation update
                StructuredDocument::AnnotationChange change;
                change[style].first = oldvalue;
                change[style].second = value;
                m.annotationBoundary( QList<QString>(), change );
            }
            // The document has the desried style value and this is the beginning of the style change?
            else if ( oldvalue == "$$$$$$$" )
            {
                oldvalue = current;
            }
            // From here on the document style has the desired value (But it did not have it before)
            else
            {
                m.retain(docFormatted - changeStart);
                changeStart = docFormatted;
                oldvalue = current;
                // End the annotation update here
                QList<QString> endkeys;
                endkeys.append( style );
                m.annotationBoundary( endkeys, StructuredDocument::AnnotationChange() );
            }
        }

        switch ( bdoc->typeAt(pos) )
        {
            case StructuredDocument::Char:
            {
                docFormatted++;
                break;
            }
            case StructuredDocument::Start:
                {
                    docFormatted++;
                    // This must be an <image>, <gadget> or <line> tag. The contents of it will be ignored. So skip it.
                    int stack = 1;
                    while( stack > 0 )
                    {
                        pos++;
                        docFormatted++;
                        switch ( bdoc->typeAt(pos) )
                        {
                        case StructuredDocument::Char:
                            break;
                        case StructuredDocument::Start:
                            stack++;
                            break;
                        case StructuredDocument::End:
                            stack--;
                            break;
                        }
                    }
                }
                break;
            case StructuredDocument::End:
                qDebug("OTAdapter::onStyleChange OOOoooops, should not have encountered the end tag.");
                break;
        }
    }
    if ( oldvalue != value )
    {
        QList<QString> endKeys;
        endKeys.append(style);
        m.retain(docFormatted - changeStart);
        m.annotationBoundary( endKeys, StructuredDocument::AnnotationChange() );
        m.retain( bdoc->count() - index - docFormatted );
    }
    else
        m.retain( bdoc->count() - index - changeStart );

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

    // Check if the user deleted some cursors (i.e carets)
    if ( charsRemoved > 0 )
    {
        foreach( Cursor* c, m_cursors.values() )
        {
            QString a = CaretInterface::caretOwner(c->m_textCursor);
            if ( a != c->m_participant->address() )
            {
                charsRemoved--;
                m_cursors.remove(c->m_participant->address());
                delete c;
            }
        }
    }

    // Construct a document mutation which reflects the change made to the QTextDocument.
    DocumentMutation m;
    int index = this->mapToBlip(position);
    m.retain(index);

    int docRemoved = 0;
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
                    docRemoved++;
                    break;
                case StructuredDocument::Start:
                    {
                        if ( text != "" )
                        {
                            m.deleteChars(text);
                            text = "";
                        }
                        m.deleteStart(bdoc->tagAt(pos), bdoc->attributesAt(pos));
                        docRemoved++;
                        int stack = 1;
                        while( stack > 0 )
                        {
                            pos++;
                            docRemoved++;
                            switch ( bdoc->typeAt(pos) )
                            {
                                case StructuredDocument::Char:
                                    m.deleteChars( QString(bdoc->charAt(pos) ) );
                                    break;
                                case StructuredDocument::Start:
                                    stack++;
                                    m.deleteStart(bdoc->tagAt(pos), bdoc->attributesAt(pos));
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
                        qDebug("OTAdapter::onContentsChange  OOOoooops, should not have encountered the end tag.");
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
    if ( bdoc->count() - index - docRemoved > 0 )
        m.retain( bdoc->count() - index - docRemoved );

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
}

void OTAdapter::setGraphicsText()
{
    foreach( Cursor* c, m_cursors.values() )
    {
        delete c;
    }
    m_cursors.clear();

    // Get user names    
    m_authorNames = "";
    foreach( QString name, blip()->authors() )
    {
        if ( m_authorNames != "" )
            m_authorNames += ",";
        if ( name == blip()->wavelet()->wave()->environment()->localUser()->address() )
            m_authorNames += tr("me");
        else
            m_authorNames += name;
    }
    m_authorNames += ": ";
    if ( blip()->authors().count() > 0 )
        blipItem()->setAuthorPixmap(blip()->wavelet()->environment()->contacts()->addParticipant( blip()->authors().first() )->pixmap());
    else
    {
        if ( blip()->creator() == blip()->wavelet()->wave()->environment()->localUser() )
            m_authorNames = "me:";
        else
            m_authorNames = blip()->creator()->name() + m_authorNames;
        blipItem()->setAuthorPixmap(blip()->creator()->pixmap());
    }

    m_suspendContentsChange = true;

    textItem()->document()->clear();
    QTextCursor cursor( textItem()->document() );
    QTextCharFormat format;

    textItem()->textCursor().insertText(m_authorNames);
    textItem()->setForbiddenTextRange(m_authorNames.count());

    QStack<int> stack;
    stack.push(0);
    bool isFirstLine = true;
    // Used for the <image> tag
    QString attachmentId;
    // Used for the <gadget> tag
    QUrl gadgetUrl;

    StructuredDocument* doc = blip()->document();
    doc->print_();
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
            if ( anno.value("style/textDecoration") == "underline" )
            {
                format.setFontUnderline(true);
                format.setFontStrikeOut(false);
            }
            else if ( anno.value("style/textDecoration") == "line-through" )
            {
                format.setFontStrikeOut(true);
                format.setFontUnderline(false);
            }
            else
            {
                format.setFontStrikeOut(false);
                format.setFontUnderline(false);
            }
        }
        switch( doc->typeAt(i) )
        {
            case StructuredDocument::Char:
                if ( stack.top() == 1 || stack.top() == 5 )
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
                    {
                        if ( text != "" )
                        {
                            cursor.insertText(text, format);
                            text = "";
                        }
                        StructuredDocument::AttributeList attribs = doc->attributesAt(i);
                        attachmentId = attribs["attachment"];
                        stack.push(4);
                    }
                    else if ( key == "caption" )
                    {
                        text = "";
                        stack.push(5);
                    }
                    else if ( key == "gadget" )
                    {
                        if ( text != "" )
                        {
                            cursor.insertText(text, format);
                            text = "";
                        }
                        StructuredDocument::AttributeList attribs = doc->attributesAt(i);
                        gadgetUrl = QUrl( attribs["url"] );
                        stack.push(7);
                    }
                    else
                        stack.push(6);
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
                // </image>
                else if ( t == 4 )
                {
                    Attachment* attachment = blipItem()->blip()->wavelet()->attachment(attachmentId);
                    if ( attachment )
                        textItem()->insertImage( &cursor, attachmentId, attachment->thumbnail(), text );
                    else
                        textItem()->insertImage( &cursor, attachmentId, QImage(), text );
                    text = "";
                }
                // </gadget>
                else if ( t == 7 )
                {
                    textItem()->insertGadget( &cursor, gadgetUrl );
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
        else if ( ch == QChar::ObjectReplacementCharacter )
        {
            bool iscaret = false;
            foreach( Cursor* cursor, m_cursors.values() )
            {
                if ( cursor->m_textCursor.position() == i )
                {
                    // Do nothing by intention because this is a caret.
                    iscaret = true;
                    break;
                }
            }

            if ( !iscaret )
                charIndex++;
        }
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
                    {
                        if ( linesSeen - 1 == blockCount && charsSeen == charIndex )
                            return i;
                        stack.push(4);
                    }
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

void OTAdapter::mutationStart()
{
    if ( m_blockUpdate )
        return;
    m_suspendContentsChange = true;

    // Delete all cursors from the text as to not interfer with the character counting
    foreach( Cursor* c, m_cursors.values() )
    {
        qDebug("HIDE CURSOR %i", c->m_textCursor.position());
        Q_ASSERT( CaretInterface::caretOwner(c->m_textCursor) == c->m_participant->address() );
        // c->m_textCursor.setPosition( c->m_textCursor.position() - 1 );
        // c->m_textCursor.deleteChar();
        c->m_textCursor.deletePreviousChar();
    }
}

void OTAdapter::insertText(int inlinePos, const QString& text )
{
    if ( m_blockUpdate )
        return;
    QTextDocument* doc = textItem()->document();
    QTextCursor cursor(doc);
    cursor.setPosition(inlinePos + textItem()->forbiddenTextRange());
    cursor.insertText(text);
}

void OTAdapter::deleteText(int inlinePos, const QString& text )
{
    if ( m_blockUpdate )
        return;
    QTextDocument* doc = textItem()->document();
    QTextCursor cursor(doc);
    cursor.setPosition(inlinePos + textItem()->forbiddenTextRange());
    for( int i = 0; i < text.length(); i++ )
        cursor.deleteChar();
}

void OTAdapter::deleteLineBreak(int inlinePos)
{
    if ( m_blockUpdate )
        return;
    QTextDocument* doc = textItem()->document();
    QTextCursor cursor(doc);
    cursor.setPosition(inlinePos + textItem()->forbiddenTextRange());
    cursor.deleteChar();
}

void OTAdapter::insertLineBreak(int inlinePos)
{
    if ( m_blockUpdate )
        return;
    QTextDocument* doc = textItem()->document();
    QTextCursor cursor(doc);
    cursor.setPosition(inlinePos + textItem()->forbiddenTextRange());
    cursor.insertBlock();
}

void OTAdapter::setStyle( const QString& style, const QString& value, int startPos, int endPos )
{
    if ( m_blockUpdate )
        return;
    QTextDocument* doc = textItem()->document();
    QTextCursor cursor(doc);
    cursor.setPosition(startPos + textItem()->forbiddenTextRange());
    cursor.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor, endPos - startPos );
    QTextCharFormat format;
    if ( style == "style/fontWeight" )
    {
        if ( value == "bold" )
            format.setFontWeight( QFont::Bold );
        else
            format.setFontWeight( QFont::Normal );
    }
    else if ( style == "style/fontStyle" )
    {
        if ( value == "italic" )
            format.setFontItalic(true);
        else
            format.setFontItalic(false);
    }
    else if ( style == "style/textDecoration" )
    {
        if ( value == "underline" )
        {
            format.setFontUnderline(true);
            format.setFontStrikeOut(false);
        }
        else if ( value == "line-through" )
        {
            format.setFontStrikeOut(true);
            format.setFontUnderline(false);
        }
        else
        {
            format.setFontStrikeOut(false);
            format.setFontUnderline(false);
        }
    }
    else
        qDebug("Unsupported style");
    // TODO: Handle all styles here
    cursor.mergeCharFormat(format);
}

void OTAdapter::setCursor(int inlinePos, const QString& author)
{
    if ( m_blockUpdate )
        return;
    if ( author == environment()->localUser()->address() )
        return;

    qDebug("SET CURSOR %i", inlinePos);
    Cursor* c = m_cursors[author];
    if ( !c )
    {
        c = new Cursor( environment()->contacts()->addParticipant(author), QDateTime::currentDateTime() );
        Q_ASSERT( c->m_participant != 0 );
        m_cursors[author] = c;
    }
    else
        c->m_timestamp = QDateTime::currentDateTime();
    QTextDocument* doc = textItem()->document();
    c->m_textCursor = QTextCursor(doc);
    c->m_textCursor.setPosition(inlinePos + textItem()->forbiddenTextRange());
}

void OTAdapter::insertImage( int inlinePos, const QString& attachmentId, const QImage& image, const QString& caption )
{
    if ( m_blockUpdate )
        return;

    QTextDocument* doc = textItem()->document();
    QTextCursor cursor(doc);
    cursor.setPosition(inlinePos + textItem()->forbiddenTextRange());
    textItem()->insertImage( &cursor, attachmentId, image, caption);
}

void OTAdapter::insertGadget( int inlinePos, const QString& url, const QString& author )
{
    Q_UNUSED(author)

    if ( m_blockUpdate )
        return;

    QTextDocument* doc = textItem()->document();
    QTextCursor cursor(doc);
    cursor.setPosition(inlinePos + textItem()->forbiddenTextRange());
    textItem()->insertGadget( &cursor, QUrl( url ));
}

void OTAdapter::mutationEnd()
{
    if ( m_blockUpdate )
        return;

    // Show the cursors
    // Delete all cursors from the text as to not interfer with the character counting
    foreach( Cursor* c, m_cursors.values() )
    {
        qDebug("SHOW CURSOR %i", c->m_textCursor.position());
        textItem()->insertCaret( &c->m_textCursor, c->m_participant->name(), Qt::red, c->m_participant->address() );
    }
    if ( m_cursors.count() > 0 )
        startOldCursorsTimer();

    // Did this modify the first block in the first blib? -> change the title
    if ( blip()->isFirstRootBlip()  )
    {
        QString title = textItem()->document()->begin().text().mid( textItem()->forbiddenTextRange() );
        emit titleChanged(title);
    }

    m_suspendContentsChange = false;
}

void OTAdapter::startOldCursorsTimer()
{
    if ( m_timer )
        return;
    m_timer = new QTimer();
    connect( m_timer, SIGNAL(timeout()), SLOT(removeOldCursors()));
    m_timer->start(2000);
}

void OTAdapter::removeOldCursors()
{
    QDateTime now = QDateTime::currentDateTime();

    // Delete all cursors from the text as to not interfer with the character counting
    foreach( Cursor* c, m_cursors.values() )
    {
        if ( c->m_timestamp.secsTo( now ) >= 2 )
        {
            if ( !c->m_textCursor.isNull() )
            {
                qDebug("DEL cursor at %i", c->m_textCursor.position() );
                Q_ASSERT( CaretInterface::caretOwner(c->m_textCursor) == c->m_participant->address() );
                m_suspendContentsChange = true;
                c->m_textCursor.deletePreviousChar();
                m_suspendContentsChange = false;
            }
            m_cursors.remove(c->m_participant->address());
            delete c;
        }
    }

    if ( m_timer && m_cursors.count() == 0 )
    {
        delete m_timer;
        m_timer = 0;
    }
}

void OTAdapter::gadgetSubmit( GadgetView* view, const QString& key, const QString& value )
{
    m_blockUpdate = true;

    // TODO

    m_blockUpdate = false;
}

void OTAdapter::gadgetSubmit( GadgetView* view, const QHash<QString,QString>& delta )
{
    m_blockUpdate = true;

    // TODO

    m_blockUpdate = false;
}
