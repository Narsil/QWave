#include "structureddocument.h"
#include "documentmutation.h"
#include <QStack>
#include <QtDebug>

StructuredDocument::StructuredDocument(QObject* parent)
        : QObject(parent)
{
}

StructuredDocument::StructuredDocument(const StructuredDocument& doc)
        : QObject(), m_items( doc.m_items ), m_annotations( doc.m_annotations ), m_cursors( doc.m_cursors )
{
}

StructuredDocument::~StructuredDocument()
{
}

void StructuredDocument::insertStart( int index, const QString& tag, const QHash<QString,QString>& map, const Annotation& anno)
{
    m_items.insert(index, QChar(0) );
    m_annotations.insert( index, anno );
    QHash<QString,QString> m( map );
    m["**t"] = tag;
    m_attributes.insert( index, m );
}

bool StructuredDocument::apply(const DocumentMutation& mutation)
{
    int stackCount = 0;
    int pos = 0;

    QHash<QString, QString> annoUpdates;
    Annotation oldAnno;
    Annotation currentAnno;

    for( QList<DocumentMutation::Item>::const_iterator it = mutation.begin(); it != mutation.end(); ++it )
    {
        switch( (*it).type )
        {
            case DocumentMutation::ElementStart:
                if ( (*it).map )
                    insertStart(pos++, (*it).text, *((*it).map), currentAnno);
                else
                    insertStart(pos++, (*it).text, QHash<QString,QString>(), currentAnno);
                stackCount++;
                break;
            case DocumentMutation::ElementEnd:
                if ( stackCount == 0 )
                    return false;
                m_items.insert(pos, QChar(1));
                m_annotations.insert(pos, currentAnno);
                m_attributes.insert(pos, AttributeList() );
                pos++;
                break;
            case DocumentMutation::InsertChars:
                {
                    for( int i = 0; i < (*it).text.length(); ++i )
                    {
                        m_items.insert(pos, (*it).text[i]);
                        m_annotations.insert(pos, currentAnno);
                        m_attributes.insert(pos, AttributeList() );
                        pos++;
                    }
                }
                break;
            case DocumentMutation::Retain:
                {
                    for( int i = 0; i < (*it).count; ++i, ++pos )
                    {
                        if ( pos >= m_items.count() )
                            return false;
                        Annotation a = m_annotations[pos];
                        if ( a != oldAnno )
                        {
                            oldAnno = a;
                            currentAnno = oldAnno.merge( annoUpdates );
                        }
                        m_annotations[pos] = currentAnno;
                        QChar ch = m_items[pos];
                        if ( ch.unicode() == 0 )
                            stackCount++;
                        else if ( ch.unicode() == 1 )
                        {
                            if ( stackCount == 0 )
                                return false;
                            stackCount--;
                        }
                    }
                }
                break;
            case DocumentMutation::DeleteStart:
                {
                if ( pos >= m_items.count() )
                    return false;
                Annotation a = m_annotations[pos];
                if ( a != oldAnno )
                {
                    oldAnno = a;
                    currentAnno = oldAnno.merge( annoUpdates );
                }
                stackCount++;
                m_items.removeAt(pos);
                m_annotations.removeAt(pos);
                m_attributes.removeAt(pos);
            }
                break;
            case DocumentMutation::DeleteEnd:
                {
                if ( pos >= m_items.count() )
                    return false;
                Annotation a = m_annotations[pos];
                if ( a != oldAnno )
                {
                    oldAnno = a;
                    currentAnno = oldAnno.merge( annoUpdates );
                }
                if ( stackCount == 0 )
                    return false;
                stackCount--;
                m_items.removeAt(pos);
                m_annotations.removeAt(pos);
                m_attributes.removeAt(pos);
            }
                break;
            case DocumentMutation::DeleteChars:
                {
                    for( int i = 0; i < (*it).text.length(); ++i )
                    {
                        if ( pos >= m_items.count() )
                            return false;
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
            case DocumentMutation::AnnotationBoundary:                
                if ( (*it).map && (*it).endKeys )
                {
                    foreach( QString key, *((*it).endKeys) )
                    {
                        annoUpdates.remove(key);
                    }
                }
                if ( (*it).map )
                {
                    foreach( QString key, (*it).map->keys() )
                    {
                        annoUpdates[key] = (*it).map->value(key);
                    }
                }
                currentAnno = oldAnno.merge(annoUpdates);
                break;
            case DocumentMutation::NoItem:
                break;
        }
    }
    return true;
}

void StructuredDocument::setCursor( const QString& name, int position )
{
    m_cursors[name] = position;
}

void StructuredDocument::removeCursor( const QString& name )
{
    m_cursors.remove(name);
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

void StructuredDocument::print_()
{
    QString result = "";

    Annotation anno;
    QStack<QString> stack;
    for( int i = 0; i < m_items.length(); ++i )
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
                    result += key + "=\"" + attribs[key] + "\" ";
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
                    return;
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

    qDebug() << result;
}

/*******************************************************************************
  *
  * StructuredDocument::Annotation
  *
  ******************************************************************************/

//QHash<QString,QString> StructuredDocument::Annotation::merge( const QHash<QString,QString>& changes, const QList<QString>& endKeys  )
//{
//    if ( isNull() )
//        return changes;
//    QHash<QString,QString> result(d->map);
//    foreach(QString key, changes.keys())
//    {
//        QString val = changes[key];
//        if ( val.isEmpty() )
//            result.remove(key);
//        else
//            result[key] = val;
//    }
//    foreach( QString e, endKeys )
//    {
//        result.remove(e);
//    }
//    return result;
//}

StructuredDocument::Annotation StructuredDocument::Annotation::merge(const QHash<QString,QString>& update) const
{
    if ( isNull() )
        return Annotation( update );
    QHash<QString,QString> result(d->map);
    foreach(QString key, update.keys())
    {
        QString val = update[key];
        if ( val.isEmpty() )
            result.remove(key);
        else
            result[key] = val;
    }
    return Annotation( result );
}
