#include "documentmutation.h"
#include <QtDebug>

DocumentMutation::DocumentMutation()
{
}

DocumentMutation::DocumentMutation(const DocumentMutation& mutation)
        : m_items( mutation.m_items )
{
}

DocumentMutation::~DocumentMutation()
{
}

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

//DocumentMutation DocumentMutation::concat( const DocumentMutation& mutation ) const
//{
//    DocumentMutation result;
//
//    if ( mutation.count() == 0 )
//        return *this;
//    if ( count() == 0 )
//        return mutation;
//
//    QList<Item>::const_iterator it1 = begin();
//    QList<Item>::const_iterator it2 = mutation.begin();
//    Item item1 = *it1;
//    Item item2 = *it2;
//    while( it1 != end() || it2 != mutation.end() )
//    {
//        if ( item1.type == AnnotationBoundary || item2.type == NoItem )
//        {
//            result.m_items.append(item1);
//            it1++;
//            if ( it1 == end() )
//               item1.type = NoItem;
//            else
//               item1 = *it1;
//            continue;
//        }
//        if ( item2.type == AnnotationBoundary || item2.type == ElementStart || item2.type == ElementEnd || item2.type == InsertChars || item1.type == NoItem )
//        {
//            result.m_items.append(item2);
//            it2++;
//            if ( it2 == mutation.end() )
//               item2.type = NoItem;
//            else
//               item1 = *it2;
//            continue;
//        }
//
//        bool next1 = false;
//        bool next2 = false;
//        switch( item1.type )
//        {
//            case AnnotationBoundary:
//                qDebug("Oooops");
//                break;
//            case ElementStart:
//                if ( item2.type == DeleteStart )
//                {
//                    next1 = true;
//                    next2 = true;
//                }
//                else if ( item2.type == Retain )
//                {
//                    result.m_items.append(item1);
//                    shorten(item2,1);
//                    if ( item2.count == 0 )
//                        next2 = true;
//                    next1 = true;
//                }
//                else
//                    qDebug("Oooops");
//                break;
//            case ElementEnd:
//                if ( item2.type == DeleteEnd )
//                {
//                    next1 = true;
//                    next2 = true;
//                }
//                else if ( item2.type == Retain )
//                {
//                    result.m_items.append(item1);
//                    shorten(item2,1);
//                    if ( item2.count == 0 )
//                        next2 = true;
//                    next1 = true;
//                }
//                else
//                    qDebug("Oooops");
//                break;
//            case InsertChars:
//                if ( item2.type == DeleteChars )
//                {
//                    int len = qMin(item1.count, item2.count);
//                    shorten(item1, len);
//                    shorten(item2, len);
//                    if ( item1.count == 0 )
//                        next1 = true;
//                    if ( item2.count == 0 )
//                        next2 = true;
//                }
//                else if ( item2.type == Retain )
//                {
//                    int len = qMin(item1.count, item2.count);
//                    result.insertChars(item1.text.left(len));
//                    shorten(item1, len);
//                    shorten(item2, len);
//                    if ( item1.count == 0 )
//                        next1 = true;
//                    if ( item2.count == 0 )
//                        next2 = true;
//                }
//                else
//                    qDebug("Oooops");
//                break;
//            case NoItem:
//                result.m_items.append(item2);
//                next2 = true;
//                break;
//            case DeleteStart:
//            case DeleteEnd:
//            case DeleteChars:
//                result.m_items.append(item1);
//                next1 = true;
//                break;
//            case Retain:
//                if ( item2.type == DeleteStart || item2.type == DeleteEnd )
//                {
//                    result.m_items.append(item2);
//                    shorten(item1,1);
//                }
//                if ( item2.type == DeleteChars )
//                {
//                    int len = qMin(item1.count, item2.count);
//                    result.deleteChars(item2.text.left(len));
//                    shorten(item1, len);
//                    shorten(item2, len);
//                    if ( item1.count == 0 )
//                        next1 = true;
//                    if ( item2.count == 0 )
//                        next2 = true;
//                }
//                else if ( item2.type == Retain )
//                {
//                    int len = qMin(item1.count, item2.count);
//                    result.retain(len);
//                    shorten(item1, len);
//                    shorten(item2, len);
//                    if ( item1.count == 0 )
//                        next1 = true;
//                    if ( item2.count == 0 )
//                        next2 = true;
//                }
//                else
//                    qDebug("Oooops");
//                break;
//        }
//
//        if ( next1 )
//        {
//            it1++;
//            if ( it1 == end() )
//               item1.type = NoItem;
//            else
//               item1 = *it1;
//        }
//        if ( next2 )
//        {
//            it2++;
//            if ( it2 == mutation.end() )
//               item2.type = NoItem;
//            else
//               item2 = *it2;
//        }
//    }
//    return result;
//}

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

QPair<DocumentMutation,DocumentMutation> DocumentMutation::xform( const DocumentMutation& m1, const DocumentMutation& m2, bool* ok )
{
    if ( m1.isEmpty() || m2.isEmpty() )
        return QPair<DocumentMutation,DocumentMutation>(m1,m2);

    DocumentMutation r1;
    DocumentMutation r2;

    QList<Item>::const_iterator it1 = m1.begin();
    QList<Item>::const_iterator it2 = m2.begin();
    Item item1 = *it1;
    Item item2 = *it2;
    while( it1 != m1.end() || it2 != m2.end() )
    {
        if ( it1 == m1.end() )
        {
            r2.m_items.append( item2 );
            it2++;
            item2 = *it2;
            continue;
        }
        if ( it2 == m2.end() )
        {
            r1.m_items.append( item2 );
            it1++;
            item1 = *it1;
            continue;
        }

        bool next1 = false;
        bool next2 = false;
        switch( item1.type )
        {
            case ElementStart:
                r1.m_items.append(item1);
                r2.retain( 1 );
                next1 = true;
                break;
            case ElementEnd:
                r1.m_items.append(item1);
                r2.retain( 1 );
                next1 = true;
                break;
            case InsertChars:
                r1.m_items.append(item1);
                r2.retain( item1.text.length() );
                next1 = true;
                break;
            case Retain:
                switch( item2.type )
                {
                    case ElementStart:
                    case ElementEnd:
                        r1.retain(1);
                        r2.m_items.append(item2);
                        next2 = true;
                        break;
                    case InsertChars:
                        r1.retain( item2.text.length() );
                        r2.m_items.append(item2);
                        next2 = true;
                        break;
                    case Retain:
                        {
                            int len = qMin(item1.count, item2.count );
                            r1.retain(len);
                            r2.retain(len);
                            if ( !shorten( item1, len ) )
                                next1 = true;
                            if ( !shorten( item2, len ) )
                                next2 = true;
                        }
                        break;
                    case DeleteStart:
                    case DeleteEnd:
                        if ( !shorten( item1, 1 ) )
                                next1++;
                        r2.m_items.append(item2);
                        break;
                    case DeleteChars:
                        {
                            int len = qMin(item1.count, item2.text.length() );
                            if ( !shorten( item1, len ) )
                                next1 = true;
                            r2.deleteChars( item2.text.left(len) );
                            if ( !shorten( item2, len ) )
                                next2 = true;
                        }
                    case AnnotationBoundary:
                        r2.m_items.append(item2);
                        next2 = true;
                }
                break;
            case DeleteStart:
            case DeleteEnd:
                switch( item2.type )
                {
                    case ElementStart:
                    case ElementEnd:
                        r1.retain(1);
                        r2.m_items.append(item2);
                        next2 = true;
                        break;
                    case InsertChars:
                        r1.retain(item2.text.length());
                        r2.m_items.append(item2);
                        next2 = true;
                        break;
                    case DeleteStart:
                    case DeleteEnd:
                    case DeleteChars:
                        if ( item1.type != item2.type )
                        {
                            qDebug("The two mutations are not compatible");
                            *ok = false;
                            return QPair<DocumentMutation,DocumentMutation>(m1,m2);
                        }
                        next1 = true;
                        next2 = true;
                        break;
                    case Retain:
                        r1.m_items.append(item1);
                        next1 = true;
                        if ( !shorten( item2, 1 ) )
                            next2 = true;
                        break;
                    case AnnotationBoundary:
                        r1.m_items.append(item1);
                        next1 = true;
                        break;
                }
                break;
            case DeleteChars:
                switch( item2.type )
                {
                    case ElementStart:
                    case ElementEnd:
                        r1.retain(1);
                        r2.m_items.append(item2);
                        next2 = true;
                        break;
                    case InsertChars:
                        r1.retain(item2.text.length());
                        r2.m_items.append(item2);
                        next2 = true;
                        break;
                    case DeleteStart:
                    case DeleteEnd:
                        qDebug("The two mutations are not compatible");
                        *ok = false;
                        return QPair<DocumentMutation,DocumentMutation>(m1,m2);
                    case DeleteChars:
                        {
                            int len = qMin( item1.text.length(), item2.text.length() );
                            if ( !shorten( item1, len ) )
                                next1 = true;
                            if ( !shorten( item2, len ) )
                                next2 = true;
                        }
                        break;
                    case Retain:
                        {
                            int len = qMin( item1.text.length(), item2.count );
                            r1.deleteChars( item1.text.left(len) );
                            if ( !shorten( item1, len ) )
                                next1 = true;
                            if ( !shorten( item2, len ) )
                                next2 = true;
                        }
                        break;
                    case AnnotationBoundary:
                        r1.m_items.append(item1);
                        next1 = true;
                        break;
                }
                break;
            case AnnotationBoundary:
                // TODO
                break;
        }

        if ( next1 )
        {
            it1++;
            if ( it1 != m1.end() )
                item1 = *it1;
        }
        if ( next2 )
        {
            it2++;
            if ( it2 != m2.end() )
                item2 = *it2;
        }
    }

    *ok = true;
    return QPair<DocumentMutation,DocumentMutation>(r1, r2);
}

bool DocumentMutation::shorten( Item& item, int len )
{
    if ( len == 0 )
        return false;
    switch( item.type )
    {
        case AnnotationBoundary:            
        case ElementStart:
        case ElementEnd:
        case DeleteStart:
        case DeleteEnd:
        case InsertChars:
            Q_ASSERT(false);
            return false;
        case DeleteChars:
            item.count -= len;
            item.text = item.text.right( item.text.length() - len );
            return item.count > 0;
        case Retain:
            item.count -= len;
            return item.count > 0;
    }

    Q_ASSERT(false);
    return true;
}
