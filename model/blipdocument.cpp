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

void BlipDocument::onMutationStart()
{
    m_inBody = false;
    m_afterLine = false;
    m_inlinePos = 0;
    m_lineCount = -1;
    m_stack.clear();

    emit mutationStart();
}

void BlipDocument::onRetainChar(int index)
{
    Q_UNUSED(index);
    if ( m_inBody && m_afterLine )
        m_inlinePos++;
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
            m_lineCount++;
            m_inlinePos = 0;
        }
        else if ( tag == "body" )
            m_inBody = false;
    }
}

void BlipDocument::onDeleteChars(int index, const QString& chars)
{
    Q_UNUSED(index);
    if ( m_inBody && m_afterLine )
        emit deletedText( m_lineCount, m_inlinePos, chars );
}

void BlipDocument::onDeleteElementStart(int index)
{
    if ( m_inBody && typeAt(index) == Start && tagAt(index) == "line" )
        emit deletedLineBreak(m_lineCount, m_inlinePos);
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
        emit insertedText( m_lineCount, m_inlinePos, chars );
        m_inlinePos += chars.length();
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
        if ( m_lineCount == - 1 )
        {
            // Do nothing. TODO: This is not always correct. But it catches the insertion of the very first line.... emit insertedLineBreak( 0, 0 );
        }
        else
            emit insertedLineBreak( m_lineCount, m_inlinePos );
        m_afterLine = true;
        m_lineCount++;
        m_inlinePos = 0;
    }
}

void BlipDocument::onAnnotationUpdate(int index, const QHash<QString,QString>& updates)
{
    // TODO
}

void BlipDocument::onMutationEnd()
{
    emit mutationEnd();
}

