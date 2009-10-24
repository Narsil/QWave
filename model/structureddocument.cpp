#include "structureddocument.h"
#include <QStack>
#include <QtDebug>

StructuredDocument::StructuredDocument(QObject* parent)
        : QObject(parent), m_newItems(0), m_newAnnotations(0)
{
    m_items = new QList<Item>();
    m_annotations = new QList<Annotation>();
    Annotation annotation;
    annotation.startPos = 0;
    annotation.endPos = 0;
    m_annotations->append(annotation);
}

StructuredDocument::~StructuredDocument()
{
    freeItems( m_items );
    delete m_annotations;
    if ( m_newItems )
        freeItems( m_newItems );
    if ( m_newAnnotations )
        delete m_newAnnotations;
}

void StructuredDocument::freeItems(QList<Item>* items)
{
    foreach( Item item, *items )
    {
        if ( item.type == Start && item.data.map )
            delete item.data.map;
    }
    delete items;
}

const StructuredDocument::Annotation& StructuredDocument::annotation(int pos) const
{
    for( int i = 0; i < m_annotations->count(); ++i )
    {
        if ( m_annotations->at(i).startPos <= pos && m_annotations->at(i).endPos > pos )
            return m_annotations->at(i);
    }
    return m_annotations->last();
}

int StructuredDocument::annotationIndex(int pos) const
{
    for( int i = 0; i < m_annotations->count(); ++i )
    {
        if ( m_annotations->at(i).startPos <= pos && m_annotations->at(i).endPos > pos )
            return i;
    }
    return m_annotations->count() - 1;
}

void StructuredDocument::beginDelta()
{
    m_newItems = new QList<Item>();
    m_newAnnotations = new QList<Annotation>();
    m_deltaPos = 0;
    Annotation annotation( m_annotations->at(0) );
    m_newAnnotations->append(annotation);
    m_deltaAnnotationIndex = 0;
    m_annotationChanges.clear();
}

void StructuredDocument::writeAnnotation( const QHash<QString,QString>& map )
{
    if ( m_newAnnotations->last().isEqual(map) )
        return;
    Annotation a( m_newItems->count(), m_newItems->count(), map);
    if ( m_newItems->count() == m_newAnnotations->last().startPos)
    {
        (*m_newAnnotations)[m_newAnnotations->count() - 1] = a;
        return;
    }
    else
    {
        m_newAnnotations->last().endPos = m_newItems->count();
        m_newAnnotations->append(a);
    }
}

void StructuredDocument::insertStart(const QString& tag)
{
    insertStart(tag, QHash<QString,QString>());
}

void StructuredDocument::insertStart(const QString& tag, const QHash<QString,QString>& map)
{
    Item item;
    item.type = Start;
    item.data.map = new QHash<QString,QString>(map);
    item.data.map->insert("type", tag);
    writeAnnotation( (*m_annotations)[m_deltaAnnotationIndex].merge(m_annotationChanges) );
    m_newItems->append(item);
}

void StructuredDocument::insertEnd()
{
    Item item;
    item.type = End;
    writeAnnotation( (*m_annotations)[m_deltaAnnotationIndex].merge(m_annotationChanges) );
    m_newItems->append(item);
}

void StructuredDocument::insertChars(const QString& chars)
{
    for( int i = 0; i < chars.length(); ++i )
    {
        Item item;
        item.type = Char;
        item.ch = chars[i];
        writeAnnotation( (*m_annotations)[m_deltaAnnotationIndex].merge(m_annotationChanges) );
        m_newItems->append(item);
    }
}

void StructuredDocument::incDeltaPos()
{
    foreach( QString cursorName, m_cursors.keys() )
    {
        if ( m_deltaPos == m_cursors[cursorName] )
            m_cursors[cursorName] = m_items->count();
    }

    int index = annotationIndex(m_deltaPos);
    if ( index != m_deltaAnnotationIndex )
    {
        m_deltaAnnotationIndex = index;
        writeAnnotation( (*m_annotations)[m_deltaAnnotationIndex].merge(m_annotationChanges) );
    }
    m_deltaPos++;
}

void StructuredDocument::retain(int count)
{
    for( int i = 0; i < count; ++i )
    {
        if ( m_items->count() <= m_deltaPos )
            break;
        incDeltaPos();
        m_newItems->append( m_items->at(m_deltaPos - 1) );
    }
}

void StructuredDocument::deleteStart(const QString& tag)
{
    if ( m_items->count() <= m_deltaPos )
    {
        qDebug("Oooops");
        // Oooops
        return;
    }
    Item item = m_items->at(m_deltaPos);
    if ( item.type != Start || item.data.map->value("type") != tag )
    {
        qDebug("Oooops");
        // Oooops
        return;
    }
    delete item.data.map;
    incDeltaPos();
}

void StructuredDocument::deleteEnd()
{
    if ( m_items->count() <= m_deltaPos )
    {
        qDebug("Oooops");
        // Oooops
        return;
    }
    Item item = m_items->at(m_deltaPos);
    if ( item.type != End )
    {
        qDebug("Oooops");
        // Oooops
        return;
    }
    incDeltaPos();
}

void StructuredDocument::deleteChars(const QString& chars)
{
    for( int i = 0; i < chars.length(); ++i )
    {
        if ( m_items->count() <= m_deltaPos )
        {
            qDebug("Oooops");
            // Oooops
            return;
        }
        Item item = m_items->at(m_deltaPos);
        if ( item.type != Char || item.ch != chars[i] )
        {
            qDebug("Oooops");
            // Oooops
            return;
        }
        incDeltaPos();
    }
}

void StructuredDocument::annotationBoundary(const QList<QString>& endKeys, const QHash<QString,QString>& changes)
{
    foreach(QString endkey, endKeys)
    {
        m_annotationChanges.remove(endkey);
    }
    foreach(QString key, changes.keys())
    {
        m_annotationChanges[key] = changes[key];
    }
    writeAnnotation( (*m_annotations)[m_deltaAnnotationIndex].merge(m_annotationChanges) );
//    writeAnnotation(m_newAnnotations->last().merge(m_annotationChanges));
}

void StructuredDocument::endDelta()
{
    m_newAnnotations->last().endPos = m_newItems->count();

    delete m_items;
    m_items = m_newItems;
    m_annotations = m_newAnnotations;
    m_newItems = 0;
    m_newAnnotations = 0;
}

void StructuredDocument::setCursor( const QString& name, int position )
{
    m_cursors[name] = position;
}

void StructuredDocument::removeCursor( const QString& name )
{
    m_cursors.remove(name);
}

int StructuredDocument::countDelta() const
{
    if ( m_newItems )
        return m_newItems->count();
    return 0;
}

const StructuredDocument::Item& StructuredDocument::operator[] ( int index ) const
{
    return (*m_items)[index];
}

QString StructuredDocument::toPlainText() const
{
    QString result = "";
    foreach( Item item, *m_items )
    {
        switch( item.type )
        {
            case Start:
                break;
            case End:
                break;
            case Char:
                result += item.ch;
                break;
        }
    }
    return result;
}

void StructuredDocument::print_()
{
    QString result = "";

    int index = -1;
    int annoIndex = -1;
    QStack<QString> stack;
    foreach( Item item, *m_items )
    {
        index++;
        int ai = annotationIndex(index);
        if ( ai != annoIndex )
        {
            annoIndex = ai;
            result += "[";
            foreach( QString key, m_annotations->at(annoIndex).map.keys() )
            {
                result += key + "=\"" +  m_annotations->at(annoIndex).map[key] + "\" ";
            }
            result += "]";
        }

        switch( item.type )
        {
            case Start:
            {
                QString tag = item.data.map->value("type");
                stack.push(tag);
                result += "<" + tag;
                foreach( QString key, item.data.map->keys() )
                {
                    if ( key == "type" )
                        continue;
                    result += key + "=\"" + item.data.map->value(key) + "\" ";
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
            case Char:
                result += item.ch;
                break;
        }
    }

    qDebug() << result;
}

/*******************************************************************************
  *
  * StructuredDocument::Item
  *
  ******************************************************************************/

QString StructuredDocument::Item::tagType() const
{
    if ( type == Start && data.map )
        return data.map->value("type");
    return QString::null;
}

/*******************************************************************************
  *
  * StructuredDocument::Annotation
  *
  ******************************************************************************/

StructuredDocument::Annotation::Annotation()
{
    // Do nothing by intention
}

StructuredDocument::Annotation::Annotation(const Annotation& annotation)
        : startPos(annotation.startPos), endPos(annotation.endPos), map(annotation.map)
{
    // Do nothing by intention
}

StructuredDocument::Annotation::Annotation(int start, int end, const QHash<QString,QString>& m)
        : startPos(start), endPos(end), map(m)
{
    // Do nothing by intention
}

QHash<QString,QString> StructuredDocument::Annotation::merge(QHash<QString,QString> update) const
{
    QHash<QString,QString> result(map);
    foreach(QString key, update.keys())
    {
        QString val = update[key];
        if ( val.isEmpty() )
            result.remove(key);
        else
            result[key] = val;
    }
    return result;
}

bool StructuredDocument::Annotation::isEqual(const QHash<QString,QString>& other)
{
    if ( map.count() != other.count() )
        return false;
    foreach(QString key, map.keys())
    {
        if ( map[key] != other[key] )
            return false;
    }
    return true;
}
