#include "structureddocument.h"
#include "documentmutation.h"
#include <QStack>
#include <QtDebug>

StructuredDocument::StructuredDocument(QObject* parent)
        : QObject(parent)
{
}

StructuredDocument::StructuredDocument(const StructuredDocument& doc)
        : QObject(), m_items( doc.m_items ), m_annotations( doc.m_annotations ), m_attributes( doc.m_attributes )
{
}

StructuredDocument::~StructuredDocument()
{
}

void StructuredDocument::insertStart( int index, const QString& tag, const AttributeList& attributes, const Annotation& anno)
{
    m_items.insert(index, QChar(0) );
    m_annotations.insert( index, anno );
    AttributeList m( attributes );
    m["**t"] = tag;
    m_attributes.insert( index, m );
}

bool StructuredDocument::apply(const DocumentMutation& mutation, const QString& author, bool check)
{
    if ( !m_authors.contains(author) )
        m_authors.append(author);

    onMutationStart(author);

    int stackCount = 0;
    int pos = 0;

    AnnotationChange annoUpdates;
    Annotation oldAnno;
    Annotation currentAnno;

    for( QList<DocumentMutation::Item>::const_iterator it = mutation.begin(); it != mutation.end(); ++it )
    {
        switch( (*it).type )
        {
            case DocumentMutation::ElementStart:
                insertStart(pos++, (*it).text, (*it).attributes, currentAnno);
                onInsertElementStart(pos - 1 );
                stackCount++;
                break;
            case DocumentMutation::ElementEnd:
                if ( stackCount == 0 )
                {
                    qDebug("Oooooops StructuredDocument 1");
                    return false;
                }
                m_items.insert(pos, QChar(1));
                m_annotations.insert(pos, currentAnno);
                m_attributes.insert(pos, AttributeList() );
                onInsertElementEnd(pos);
                pos++;
                break;
            case DocumentMutation::InsertChars:
                {
                    int startpos = pos;
                    for( int i = 0; i < (*it).text.length(); ++i )
                    {
                        m_items.insert(pos, (*it).text[i]);
                        m_annotations.insert(pos, currentAnno);
                        m_attributes.insert(pos, AttributeList() );
                        pos++;
                    }
                    onInsertChars( startpos, (*it).text );
                }
                break;
            case DocumentMutation::Retain:
                {
                    for( int i = 0; i < (*it).count; ++i, ++pos )
                    {
                        if ( pos >= m_items.count() )
                        {
                            qDebug("Oooooops StructuredDocument 2");
                            return false;
                        }
                        Annotation a = m_annotations[pos];
                        if ( a != oldAnno )
                        {
                            oldAnno = a;
                            currentAnno = oldAnno.merge( annoUpdates );
                        }
                        m_annotations[pos] = currentAnno;
                        QChar ch = m_items[pos];
                        // Start tag?
                        if ( ch.unicode() == 0 )
                        {
                            onRetainElementStart(pos);
                        }
                        // End tag?
                        else if ( ch.unicode() == 1 )
                        {
                            onRetainElementEnd(pos);
                        }
                        else
                            onRetainChar(pos);
                    }
                }
                break;
            case DocumentMutation::DeleteStart:
                {
                if ( pos >= m_items.count() )
                {
                    qDebug("Oooooops StructuredDocument 4");
                    return false;
                }
                Annotation a = m_annotations[pos];
                if ( a != oldAnno )
                {
                    oldAnno = a;
                    currentAnno = oldAnno.merge( annoUpdates );
                }
                stackCount++;
                onDeleteElementStart(pos);
                m_items.removeAt(pos);
                m_annotations.removeAt(pos);
                m_attributes.removeAt(pos);
            }
                break;
            case DocumentMutation::DeleteEnd:
                {
                if ( pos >= m_items.count() )
                {
                    qDebug("Oooooops StructuredDocument 5");
                    return false;
                }
                Annotation a = m_annotations[pos];
                if ( a != oldAnno )
                {
                    oldAnno = a;
                    currentAnno = oldAnno.merge( annoUpdates );
                }
                if ( stackCount == 0 )
                {
                    qDebug("Oooooops StructuredDocument 6");
                    return false;
                }
                stackCount--;
                onDeleteElementEnd(pos);
                m_items.removeAt(pos);
                m_annotations.removeAt(pos);
                m_attributes.removeAt(pos);
            }
                break;
            case DocumentMutation::DeleteChars:
                {
                    onDeleteChars(pos, (*it).text);
                    for( int i = 0; i < (*it).text.length(); ++i )
                    {
                        if ( pos >= m_items.count() )
                        {
                            qDebug("Oooooops StructuredDocument 7");
                            return false;
                        }
                        Annotation a = m_annotations[pos];
                        if ( a != oldAnno )
                        {
                            oldAnno = a;
                            currentAnno = oldAnno.merge( annoUpdates );
                        }
                        m_items.removeAt(pos);
                        m_annotations.removeAt(pos);
                        m_attributes.removeAt(pos);
                    }
                }
                break;
            case DocumentMutation::UpdateAttributes:
                {
                    onUpdateAttributes(pos, (*it).attributes);
                    AttributeList old = m_attributes[pos];
                    if ( check )
                    {
                        if ( typeAt(pos) != Start )
                        {
                            qDebug("UpdateAttributes works on start tags only");
                            return false;
                        }
                    }
                    foreach( QString key, (*it).attributes.keys() )
                    {
                        if ( key[0] == '-' )
                            continue;
                        QString value = (*it).attributes[key];
                        if ( check )
                        {
                            QString oldValue = (*it).attributes["-" + key];
                            if ( !oldValue.isNull() )
                            {
                                if ( !old.contains( key ) )
                                {
                                    qDebug("Ooooops, UpdateAttributes: old tag did not contain the attribute.");
                                    return false;
                                }
                                if ( old[key] != (*it).attributes["-" + key] )
                                {
                                    qDebug("Ooooops, UpdateAttributes: old tag did contain different attribute value.");
                                    return false;
                                }
                            }
                        }
                        if ( value.isNull() )
                            m_attributes[pos].remove(key);
                        else
                            m_attributes[pos].insert( key, value );
                    }
                    onUpdatedAttributes(pos);
                    pos++;
                }
                break;
            case DocumentMutation::ReplaceAttributes:
                {                    
                    onReplaceAttributes(pos, (*it).attributes);
                    int oldCount = 0;
                    AttributeList old = m_attributes[pos];
                    QString tag = old["**t"];
                    if ( check )
                    {
                        if ( typeAt(pos) != Start )
                        {
                            qDebug("Ooooooops, ReplaceAttributes works on start tags only");
                            return false;
                        }
                        // -1 because the **t attribute is contained here, too.
                        oldCount = old.count() - 1;
                    }
                    m_attributes[pos].clear();
                    m_attributes[pos].insert( "**t", tag );
                    foreach( QString key, (*it).attributes.keys() )
                    {
                        if ( key[0] == '-' )
                        {
                            if ( !check )
                                continue;
                            QString value = (*it).attributes[key];
                            oldCount--;
                            key = key.mid(1);
                            if ( !old.contains( key ) )
                            {
                                qDebug("Ooooops, old tag did not contain the attribute.");
                                return false;
                            }
                            if ( old[key] != value )
                            {
                                qDebug("Ooooops, old tag did contain a different attribute value.");
                                return false;
                            }
                            continue;
                        }

                        QString value = (*it).attributes[key];
                        if ( value.isNull() )
                            continue;
                        m_attributes[pos].insert( key, value );
                    }                    
                    if ( oldCount != 0 )
                    {
                        qDebug("Oooops, the old tag had more attributes");
                        return false;
                    }
                    onReplacedAttributes(pos);
                    pos++;
                }
                break;
            case DocumentMutation::AnnotationBoundary:                
                foreach( QString key, (*it).endKeys )
                {
                    annoUpdates.remove(key);
                }
                foreach( QString key, (*it).annotations.keys() )
                {
//                    if ( currentAnno.value(key) != (*it).annotations[key].second )
                        annoUpdates[key] = (*it).annotations[key];
                }
                onAnnotationUpdate(pos, annoUpdates);
                currentAnno = oldAnno.merge(annoUpdates);
                break;
        }
    }

    onMutationEnd();
    return true;
}

StructuredDocument::ItemType StructuredDocument::typeAt( int index ) const
{
    ushort ch = m_items[index].unicode();
    if ( ch == 0 )
        return Start;
    if ( ch == 1 )
        return End;
    return Char;
}

QString StructuredDocument::tagAt( int index ) const
{
    const AttributeList& a = m_attributes[index];
    return a["**t"];
}

QString StructuredDocument::toPlainText() const
{
    QString result = "";
    foreach( QChar item, m_items )
    {
        if ( item.unicode() >=2 )
            result += item;
    }
    return result;
}    

void StructuredDocument::print_() const
{
    qDebug() << toString();
}

QString StructuredDocument::toString() const
{
    QString result = "";

    Annotation anno;
    QStack<QString> stack;
    for( int i = 0; i < m_items.count(); ++i )
    {
        if ( m_annotations[i] != anno )
        {
            anno = m_annotations[i];
            result += "[";
            foreach( QString key, anno.keys() )
            {
                result += key + "=\"" +  anno.value(key) + "\" ";
            }
            result += "]";
        }

        QChar ch = m_items[i];
        switch( ch.unicode() )
        {
            case Start:
            {
                QString tag = tagAt(i);
                stack.push(tag);
                result += "<" + tag;
                const AttributeList& attribs = attributesAt(i);
                foreach( QString key, attribs.keys() )
                {
                    if ( key[0] == '*' )
                        continue;
                    result += " " + key + "=\"" + attribs[key] + "\"";
                }
                result += ">";
                break;
            }
            case End:
            {
                if ( stack.count() == 0 )
                {
                    result += "**** Too many closing tags";
                    qDebug() << result;
                    return result;
                }
                QString tag = stack.pop();
                result += "</" + tag + ">";
                break;
            }
            default:
                result += ch;
                break;
        }
    }

    return result;
}

void StructuredDocument::onMutationStart(const QString& author)
{
    Q_UNUSED(author);
}

void StructuredDocument::onRetainChar(int index)
{
    Q_UNUSED(index);
}

void StructuredDocument::onRetainElementStart(int index)
{
    Q_UNUSED(index);
}

void StructuredDocument::onRetainElementEnd(int index)
{
    Q_UNUSED(index);
}

void StructuredDocument::onDeleteChars(int index, const QString& chars)
{
    Q_UNUSED(index);
    Q_UNUSED(chars);
}

void StructuredDocument::onDeleteElementStart(int index)
{
    Q_UNUSED(index);
}

void StructuredDocument::onDeleteElementEnd(int index)
{
    Q_UNUSED(index);
}

void StructuredDocument::onInsertChars(int index, const QString& chars)
{
    Q_UNUSED(index);
    Q_UNUSED(chars);
}

void StructuredDocument::onInsertElementStart(int index)
{
    Q_UNUSED(index);
}

void StructuredDocument::onInsertElementEnd(int index)
{
    Q_UNUSED(index);
}

void StructuredDocument::onAnnotationUpdate(int index, const AnnotationChange& updates)
{
    Q_UNUSED(index);
    Q_UNUSED(updates);
}

void StructuredDocument::onUpdateAttributes(int index, const AttributeList& updates)
{
    Q_UNUSED(index);
    Q_UNUSED(updates);
}

void StructuredDocument::onUpdatedAttributes(int index)
{
    Q_UNUSED(index);
}

void StructuredDocument::onReplaceAttributes(int index, const AttributeList& updates)
{
    Q_UNUSED(index);
    Q_UNUSED(updates);
}

void StructuredDocument::onReplacedAttributes(int index)
{
    Q_UNUSED(index);
}

void StructuredDocument::onMutationEnd()
{
}

/*******************************************************************************
  *
  * StructuredDocument::Annotation
  *
  ******************************************************************************/


StructuredDocument::Annotation StructuredDocument::Annotation::merge(const AnnotationChange& update) const
{
    if ( isNull() )
    {
        AnnotationList result;
        foreach(QString key, update.keys())
        {
            QString val = update[key].second;
            result[key] = val;
        }
        return Annotation( result );
    }

    AnnotationList result(d->map);
    foreach(QString key, update.keys())
    {
        QString val = update[key].second;
        if ( val.isEmpty() )
            result.remove(key);
        else
            result[key] = val;
    }
    return Annotation( result );
}
