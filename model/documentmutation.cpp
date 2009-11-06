#include "documentmutation.h"
#include <QtDebug>

DocumentMutation::DocumentMutation()
{
}

DocumentMutation::DocumentMutation(const DocumentMutation& mutation)
        : m_items( mutation.m_items )
{
//    // Make deep copies of the items
//    foreach( Item item, mutation.m_items )
//    {
//        if ( item.endKeys )
//            item.endKeys = new QList<QString>(*(item.endKeys));
//        if ( item.map )
//            item.map = new QHash<QString,QString>(*(item.map));
//        m_items.append(item);
//    }
}

DocumentMutation::~DocumentMutation()
{
//    freeItems();
}

//void DocumentMutation::freeItems()
//{
//    foreach( Item item, m_items )
//    {
//        if ( item.endKeys )
//            delete item.endKeys;
//        if ( item.map )
//            delete item.map;
//    }
//}

void DocumentMutation::insertStart(const QString& tag, const StructuredDocument::AttributeList& attributes)
{
    Item item;
    item.attributes = attributes;
    item.type = ElementStart;
    item.text = tag;
    item.count = 1;
    m_items.append(item);
}

void DocumentMutation::insertStart(const QString& tag)
{
    Item item;
    item.type = ElementStart;
    item.text = tag;
    item.count = 1;
    m_items.append(item);
}

void DocumentMutation::insertEnd()
{
    Item item;
    item.type = ElementEnd;
    item.count = 1;
    m_items.append(item);
}

void DocumentMutation::retain(int count)
{
    if ( m_items.count() > 0 && m_items.last().type == Retain )
    {
        m_items.last().count += count;
        return;
    }
    Item item;
    item.type = Retain;
    item.count = count;
    m_items.append(item);
}

void DocumentMutation::insertChars(const QString& chars)
{
    if ( m_items.count() > 0 && m_items.last().type == InsertChars )
    {
        m_items.last().text += chars;
        m_items.last().count += chars.length();
        return;
    }
    Item item;
    item.type = InsertChars;
    item.text = chars;
    item.count = chars.length();
    m_items.append(item);
}

void DocumentMutation::deleteStart(const QString& tag)
{
    Item item;
    item.type = DeleteStart;
    item.text = tag;
    item.count = 1;
    m_items.append(item);
}

void DocumentMutation::deleteEnd()
{
    Item item;
    item.type = DeleteEnd;
    item.count = 1;
    m_items.append(item);
}

void DocumentMutation::deleteChars(const QString& chars)
{
    if ( m_items.count() > 0 && m_items.last().type == DeleteChars )
    {
        m_items.last().text += chars;
        m_items.last().count += chars.length();
        return;
    }
    Item item;
    item.type = DeleteChars;
    item.text = chars;
    item.count = chars.length();
    m_items.append(item);
}

void DocumentMutation::annotationBoundary(const QList<QString>& endKeys, const StructuredDocument::AnnotationChange& changes)
{
    Item item;
    item.endKeys = endKeys;
    item.annotations = changes;
    item.type = AnnotationBoundary;
    item.count = 1;
    m_items.append(item);
}

void DocumentMutation::clear()
{
    m_items.clear();
}

int DocumentMutation::count() const
{
    return m_items.count();
}

void DocumentMutation::shorten( Item& item, int len ) const
{
    if ( len == 0 )
        return;
    switch( item.type )
    {
        case NoItem:
        case AnnotationBoundary:
            break;
        case ElementStart:
        case ElementEnd:
        case DeleteStart:
        case DeleteEnd:
            item.count = 0;
            break;
        case InsertChars:
        case DeleteChars:
            item.count -= len;
            item.text = item.text.right( item.text.length() - len );
            break;
        case Retain:
           item.count -= len;
           break;
    }
}

DocumentMutation DocumentMutation::translate( const DocumentMutation& mutation ) const
{
    DocumentMutation result;

    if ( mutation.count() == 0 )
        return *this;
    if ( count() == 0 )
        return mutation;

    QList<Item>::const_iterator it1 = begin();
    QList<Item>::const_iterator it2 = mutation.begin();
    Item item1 = *it1;
    Item item2 = *it2;
    bool inDeleteChars = false;
    int inDeleteTag = 0;
    while( it1 != end() || it2 != mutation.end() )
    {
        bool next1 = false;
        bool next2 = false;
        switch( item1.type )
        {
            case ElementStart:
            case ElementEnd:
                result.retain(1);
                next1 = true;
                break;
            case InsertChars:
                result.retain(item1.count);
                next1 = true;
                break;
            case AnnotationBoundary:
                next1 = true;
                break;
            case NoItem:
                result.m_items.append(item2);
                next2 = true;
                break;
            case DeleteStart:
                if ( item2.type == DeleteStart )
                {
                    next1 = true;
                    next2 = true;
                }
                if ( item2.type == Retain )
                {
                    next1 = true;
                    shorten(item2, 1);
                    if ( item2.count == 0 )
                        next2 = true;
                }
                else if ( inDeleteTag > 0 && ( item2.type == ElementStart || item2.type == ElementEnd || item2.type == InsertChars ) )
                {
                    next2 = true;
                }
                else if ( item2.type == ElementStart || item2.type == ElementEnd || item2.type == InsertChars || item2.type == AnnotationBoundary )
                {
                    result.m_items.append(item2);
                    next2 = true;
                    break;
                }
                else if ( item2.type == NoItem )
                    next1 = true;
                else
                    qDebug("Ooooops");
                inDeleteTag++;
                break;
            case DeleteEnd:
                if ( item2.type == DeleteEnd )
                {
                    next1 = true;
                    next2 = true;
                }
                else if ( item2.type == Retain )
                {
                    next1 = true;
                    shorten(item2, 1);
                    if ( item2.count == 0 )
                        next2 = true;
                }
                else if ( item2.type == AnnotationBoundary )
                {
                    result.m_items.append(item2);
                    next2 = true;
                    break;
                }
                else
                    next2 = true;
                inDeleteTag--;
                break;
            case DeleteChars:
                if ( item2.type == Retain || item2.type == DeleteChars )
                {
                    int len = qMin(item1.count, item2.count);
                    shorten(item2, len);
                    shorten(item1, len);
                    if ( item1.count == 0 )
                    {
                        next1 = true;
                        inDeleteChars = false;
                    }
                    else
                        inDeleteChars = true;
                    if ( item2.count == 0 )
                        next2 = true;
                }
                else if ( inDeleteChars && ( item2.type == ElementStart || item2.type == ElementEnd || item2.type == InsertChars ) )
                {
                    next2 = true;
                }
                else if ( item2.type == ElementStart || item2.type == ElementEnd || item2.type == InsertChars || item2.type == AnnotationBoundary )
                {
                    result.m_items.append(item2);
                    next2 = true;
                }
                else if ( item2.type == NoItem )
                    next1 = true;
                else
                    qDebug("Ooooops");
                break;
            case Retain:
                if ( item2.type == Retain )
                {
                    int len = qMin(item1.count, item2.count);
                    result.retain(len);
                    shorten(item2, len);
                    shorten(item1, len);
                    if ( item1.count == 0 )
                        next1 = true;
                    if ( item2.count == 0 )
                        next2 = true;
                }
                else if ( item2.type == DeleteChars )
                {
                    int len = qMin(item1.count, item2.count);
                    result.deleteChars(item2.text.left(len));
                    shorten(item2, len);
                    shorten(item1, len);
                    if ( item1.count == 0 )
                        next1 = true;
                    if ( item2.count == 0 )
                        next2 = true;
                }
                else if ( item2.type == DeleteEnd || item2.type == DeleteStart )
                {
                    result.m_items.append(item2);
                    next2 = true;
                    shorten(item1, 1);
                    if ( item1.count == 0 )
                        next1 = true;
                }
                else
                {
                    result.m_items.append(item2);
                    next2 = true;
                }
                break;
        }

        if ( next1 )
        {
            it1++;
            if ( it1 == end() )
               item1.type = NoItem;
            else
               item1 = *it1;
        }
        if ( next2 )
        {
            it2++;
            if ( it2 == mutation.end() )
               item2.type = NoItem;
            else
               item2 = *it2;
        }
    }
    return result;
}

DocumentMutation DocumentMutation::concat( const DocumentMutation& mutation ) const
{
    DocumentMutation result;

    if ( mutation.count() == 0 )
        return *this;
    if ( count() == 0 )
        return mutation;

    QList<Item>::const_iterator it1 = begin();
    QList<Item>::const_iterator it2 = mutation.begin();
    Item item1 = *it1;
    Item item2 = *it2;
    while( it1 != end() || it2 != mutation.end() )
    {
        if ( item1.type == AnnotationBoundary || item2.type == NoItem )
        {
            result.m_items.append(item1);
            it1++;
            if ( it1 == end() )
               item1.type = NoItem;
            else
               item1 = *it1;
            continue;
        }
        if ( item2.type == AnnotationBoundary || item2.type == ElementStart || item2.type == ElementEnd || item2.type == InsertChars || item1.type == NoItem )
        {
            result.m_items.append(item2);
            it2++;
            if ( it2 == mutation.end() )
               item2.type = NoItem;
            else
               item1 = *it2;
            continue;
        }

        bool next1 = false;
        bool next2 = false;
        switch( item1.type )
        {
            case AnnotationBoundary:
                qDebug("Oooops");
                break;
            case ElementStart:
                if ( item2.type == DeleteStart )
                {
                    next1 = true;
                    next2 = true;
                }
                else if ( item2.type == Retain )
                {
                    result.m_items.append(item1);
                    shorten(item2,1);
                    if ( item2.count == 0 )
                        next2 = true;
                    next1 = true;
                }
                else
                    qDebug("Oooops");
                break;
            case ElementEnd:
                if ( item2.type == DeleteEnd )
                {
                    next1 = true;
                    next2 = true;
                }
                else if ( item2.type == Retain )
                {
                    result.m_items.append(item1);
                    shorten(item2,1);
                    if ( item2.count == 0 )
                        next2 = true;
                    next1 = true;
                }
                else
                    qDebug("Oooops");
                break;
            case InsertChars:
                if ( item2.type == DeleteChars )
                {
                    int len = qMin(item1.count, item2.count);
                    shorten(item1, len);
                    shorten(item2, len);
                    if ( item1.count == 0 )
                        next1 = true;
                    if ( item2.count == 0 )
                        next2 = true;
                }
                else if ( item2.type == Retain )
                {
                    int len = qMin(item1.count, item2.count);
                    result.insertChars(item1.text.left(len));
                    shorten(item1, len);
                    shorten(item2, len);
                    if ( item1.count == 0 )
                        next1 = true;
                    if ( item2.count == 0 )
                        next2 = true;
                }
                else
                    qDebug("Oooops");
                break;
            case NoItem:
                result.m_items.append(item2);
                next2 = true;
                break;
            case DeleteStart:
            case DeleteEnd:
            case DeleteChars:
                result.m_items.append(item1);
                next1 = true;
                break;
            case Retain:
                if ( item2.type == DeleteStart || item2.type == DeleteEnd )
                {
                    result.m_items.append(item2);
                    shorten(item1,1);
                }
                if ( item2.type == DeleteChars )
                {
                    int len = qMin(item1.count, item2.count);
                    result.deleteChars(item2.text.left(len));
                    shorten(item1, len);
                    shorten(item2, len);
                    if ( item1.count == 0 )
                        next1 = true;
                    if ( item2.count == 0 )
                        next2 = true;
                }
                else if ( item2.type == Retain )
                {
                    int len = qMin(item1.count, item2.count);
                    result.retain(len);
                    shorten(item1, len);
                    shorten(item2, len);
                    if ( item1.count == 0 )
                        next1 = true;
                    if ( item2.count == 0 )
                        next2 = true;
                }
                else
                    qDebug("Oooops");
                break;
        }

        if ( next1 )
        {
            it1++;
            if ( it1 == end() )
               item1.type = NoItem;
            else
               item1 = *it1;
        }
        if ( next2 )
        {
            it2++;
            if ( it2 == mutation.end() )
               item2.type = NoItem;
            else
               item2 = *it2;
        }
    }
    return result;
}

DocumentMutation DocumentMutation::compose( const DocumentMutation& mutation ) const
{
    DocumentMutation m = translate(mutation);
    return concat(m);
}

void DocumentMutation::print_()
{
    QString result = "";
    for( QList<Item>::const_iterator it = begin(); it != end(); ++it )
    {
        switch( (*it).type )
        {
            case ElementStart:
                result += "ElementStart " + (*it).text + " " + mapToString( (*it).attributes ) + "\n";
                break;
            case ElementEnd:
                result += "ElementStart\n";
                break;
            case Retain:
                result += "Retain " + QString::number((*it).count) + "\n";
                break;
            case InsertChars:
                result += "Insert \"" + (*it).text + "\"\n";
                break;
            case DeleteStart:
                result += "DeleteElementStart " + (*it).text + " " + mapToString( (*it).attributes ) + "\n";
                break;
            case DeleteEnd:
                result += "DeleteElementEnd\n";
                break;
            case DeleteChars:
                result += "Delete \"" + (*it).text + "\"\n";
                break;
            case AnnotationBoundary:
                {
                    QString s = "";
                    foreach( QString s2, (*it).endKeys)
                    {
                        s += s2 + ",";
                    }
                    result += "Annotation  endKeys={" + s + "} map={" + mapToString( (*it).annotations ) + "}\n";
                }
                break;
            case NoItem:
                break;
        }
    }

    qDebug() << result.toLatin1().constData();
}

QString DocumentMutation::mapToString(const StructuredDocument::AttributeList& map)
{
    QString result = "";
    foreach( QString key, map.keys() )
    {
        result += key + "=\"" + map[key] + "\" ";
    }
    return result;
}

QString DocumentMutation::mapToString(const StructuredDocument::AnnotationChange& map)
{
    QString result = "";
    foreach( QString key, map.keys() )
    {
        result += key + "=\"" + map[key].second + "\" ";
    }
    return result;
}

/*********************************************************************
  *
  * MutationDocument::Item
  *
  ********************************************************************/

//DocumentMutation::Item DocumentMutation::Item::deepCopy()
//{
//    Item result = *this;
//    if ( result.endKeys )
//        result.endKeys = new QList<QString>(*(endKeys));
//    if ( result.map )
//        result.map = new QHash<QString,QString>(*(map));
//    return result;
//}
