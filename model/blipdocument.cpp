#include "blipdocument.h"
#include "documentmutation.h"
#include <QStack>

BlipDocument::BlipDocument(QObject* parent)
        : StructuredDocument( parent )

{
}

BlipDocument::BlipDocument( const StructuredDocument& doc)
        : StructuredDocument( doc )
{
}

void BlipDocument::onMutationStart(const QString& author)
{
    m_currentAuthor = author;
    m_inBody = false;
    m_afterLine = false;
    m_pos = -1;
    m_stack.clear();
    m_cursorpos = -1;

    emit mutationStart();
}

void BlipDocument::onRetainChar(int index)
{
    Q_UNUSED(index);
    if ( m_inBody && m_afterLine )
        m_pos++;
}

void BlipDocument::onRetainElementStart(int index)
{
    QString tag = tagAt(index);
    m_stack.push(tag);
    if ( tag == "body" )
        m_inBody = true;
}

void BlipDocument::onRetainElementEnd(int index)
{
    QString tag = m_stack.pop();
    if ( m_inBody )
    {
        if ( tag == "line" )
        {
            m_afterLine = true;
            m_pos++;
        }
        else if ( tag == "body" )
            m_inBody = false;
    }
}

void BlipDocument::onDeleteChars(int index, const QString& chars)
{
    Q_UNUSED(index);
    if ( m_inBody && m_afterLine )
        emit deletedText( m_pos, chars );
    m_cursorpos = m_pos;
}

void BlipDocument::onDeleteElementStart(int index)
{
    if ( m_inBody && typeAt(index) == Start && tagAt(index) == "line" )
    {
        emit deletedLineBreak(m_pos);
        m_cursorpos = m_pos;
    }
}

void BlipDocument::onDeleteElementEnd(int index)
{
    Q_UNUSED(index);
}

void BlipDocument::onInsertChars(int index, const QString& chars)
{
    Q_UNUSED(index);
    if ( m_inBody && m_afterLine )
    {
        emit insertedText( m_pos, chars );
        m_pos += chars.length();
        m_cursorpos = m_pos;
    }
}

void BlipDocument::onInsertElementStart(int index)
{
    Q_UNUSED(index);
    QString tag = tagAt(index);
    m_stack.push(tag);
    if ( tag == "line" )
    {
        m_afterLine = false;
    }
    else if ( tag == "body" )
    {
        m_inBody = true;
    }
}

void BlipDocument::onInsertElementEnd(int index)
{
    QString tag = m_stack.pop();
    if ( m_inBody && tag == "line" )
    {
        // Insert right after the body tag?
        if ( m_pos == - 1 )
        {
            // Do nothing. TODO: This is not always correct. But it catches the insertion of the very first line.... emit insertedLineBreak( 0, 0 );
        }
        else
            emit insertedLineBreak( m_pos );
        m_afterLine = true;
        m_pos++;
        m_cursorpos = m_pos;
    }
}

void BlipDocument::onAnnotationUpdate(int index, const QHash<QString,QString>& updates)
{
    // TODO
}

void BlipDocument::onMutationEnd()
{
    if ( m_cursorpos != -1 )
        emit setCursor( m_cursorpos, m_currentAuthor );

    emit mutationEnd();
}

