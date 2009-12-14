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

void DocumentMutation::deleteStart(const QString& tag, const StructuredDocument::AttributeList& attributes)
{
    Item item;
    item.type = DeleteStart;
    item.text = tag;
    item.attributes = attributes;
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
    if ( m_items.count() > 0 && m_items.last().type == AnnotationBoundary )
    {
        StructuredDocument::AnnotationChange a = m_items.last().annotations;
        QList<QString> e = m_items.last().endKeys;
        foreach( QString key, endKeys )
        {
            if ( a.contains( key ) )
                a.remove(key);
            if ( !e.contains( key ) )
                e.append(key);
        }
        foreach( QString key, changes.keys() )
        {
            StructuredDocument::StringPair pair = changes[key];
            a[key] = pair;
        }
        m_items.last().annotations = a;
        m_items.last().endKeys = e;
        return;
    }

    Item item;
    item.endKeys = endKeys;
    item.annotations = changes;
    item.type = AnnotationBoundary;
    item.count = 1;
    m_items.append(item);
}

void DocumentMutation::updateAttributes( const QHash<QString,StructuredDocument::StringPair>& changes )
{
    Item item;
    item.type = UpdateAttributes;
    item.count = 1;

    foreach( QString key, changes.keys() )
    {
        StructuredDocument::StringPair p = changes[key];
        item.attributes[ "-" + key ] = p.first;
        item.attributes[key] = p.second;
    }

    m_items.append(item);
}

void DocumentMutation::updateAttributes( const StructuredDocument::AttributeList& changes )
{
    Item item;
    item.type = UpdateAttributes;
    item.count = 1;
    item.attributes = changes;
    m_items.append(item);
}

void DocumentMutation::replaceAttributes( const StructuredDocument::AttributeList& oldAttributes, const StructuredDocument::AttributeList& newAttributes )
{
    Item item;
    item.type = ReplaceAttributes;
    item.count = 1;

    foreach( QString key, oldAttributes.keys() )
    {
        item.attributes[ "-" + key ] = oldAttributes[key];
    }

    foreach( QString key, newAttributes.keys() )
    {
        item.attributes[ key ] = newAttributes[key];
    }

    m_items.append(item);
}

void DocumentMutation::replaceAttributes( const StructuredDocument::AttributeList& changes )
{
    Item item;
    item.type = ReplaceAttributes;
    item.count = 1;
    item.attributes = changes;
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

QString DocumentMutation::toString() const
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
            case ReplaceAttributes:
                result += "ReplaceAttributes ";
                foreach( QString k, (*it).attributes.keys() )
                {
                    result += k + "=" + (*it).attributes[k] + " ";
                }
                result += "\n";
                break;
            case UpdateAttributes:
                result += "UpdateAttributes ";
                foreach( QString k, (*it).attributes.keys() )
                {
                    if ( k[0] == '-' )
                        continue;
                    result += k + "=" + (*it).attributes["-" + k] + "->" + (*it).attributes[k] + " ";
                }
                result += "\n";
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

    return result;
}

void DocumentMutation::print_()
{
    qDebug() << toString().toLatin1().constData();
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
        result += key + "=\"" + map[key].first + "\"->\"" + map[key].second + "\" ";
    }
    return result;
}

void DocumentMutation::xformInsertElementStart( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok )
{
    Q_UNUSED(item2);
    Q_UNUSED(ok);
    Q_UNUSED(anno1);
    Q_UNUSED(anno2);
    r1.m_items.append(item1);
    r2.retain( 1 );
    next1 = true;
    next2 = false;
}

void DocumentMutation::xformInsertElementEnd( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok )
{
    Q_UNUSED(item2);
    Q_UNUSED(ok);
    Q_UNUSED(anno1);
    Q_UNUSED(anno2);
    r1.m_items.append(item1);
    r2.retain( 1 );
    next1 = true;
    next2 = false;
}

void DocumentMutation::xformInsertChars( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok )
{
    Q_UNUSED(item2);
    Q_UNUSED(ok);
    Q_UNUSED(anno1);
    Q_UNUSED(anno2);

    r1.insertChars( item1.text );
    r2.retain( item1.text.length() );
    next1 = true;
    next2 = false;
}

void DocumentMutation::xformAnnotationBoundary( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool twisted, bool* ok )
{
    Q_UNUSED(item2);
    Q_UNUSED(ok);

    StructuredDocument::AnnotationChange a = item1.annotations;
    QList<QString> e = item1.endKeys;
    StructuredDocument::AnnotationChange a2;
    QList<QString> e2;

    if ( !twisted )
    {
        foreach( QString key, e )
        {
            if ( anno2.contains(key) )
            {
                e.removeOne(key);
                a2[key] = anno2[key];
            }
            anno1.remove(key);
        }
        foreach( QString key, a.keys() )
        {
            anno1[key] = a[key];
            if ( anno2.contains(key) )
            {
                if ( anno2[key].second == a[key].second )
                    e2.append(key);
                else
                {
                    a2[key] = anno2[key];
                    a2[key].first = a[key].second;
                }
                a.remove(key);
            }
        }
    }
    else
    {
        foreach( QString key, e )
        {
            if ( anno2.contains(key) )
            {
                a2[key] = anno2[key];
            }
            anno1.remove(key);
        }
        foreach( QString key, a.keys() )
        {
            anno1[key] = a[key];
            if ( anno2.contains(key) )
            {
                if ( anno2[key].second == a[key].second )
                    a.remove(key);
                else
                {
                    a[key].first = anno2[key].second;
                    e2.append(key);
                }
            }
        }
    }

    if ( e2.count() != 0 || a2.count() != 0 )
    {
        r2.annotationBoundary( e2, a2 );
    }

    if ( e.count() == 0 && a.count() == 0 )
    {
        next1 = true;
        return;
    }

    r1.annotationBoundary( e, a );
    next1 = true;
    next2 = false;
}

void DocumentMutation::xformRetain( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok )
{
    switch( item2.type )
    {
    case ElementStart:
        xformInsertElementStart( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case ElementEnd:
        xformInsertElementEnd( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case InsertChars:
        xformInsertChars( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case AnnotationBoundary:
        xformAnnotationBoundary( r2, r1, item2, item1, next2, next1, anno2, anno1, true, ok );
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
        if ( !shorten( item1, 1 ) )
            next1 = true;
        r2.deleteStart( item2.text, item2.attributes );
        next2 = true;
        break;
    case DeleteEnd:
        if ( !shorten( item1, 1 ) )
            next1 = true;
        r2.deleteEnd();
        next2 = true;
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
        break;
    case ReplaceAttributes:
        r1.retain(1);
        if ( !shorten( item1, 1 ) )
            next1 = true;
        r2.replaceAttributes( item2.attributes );
        next2 = true;
        break;
    case UpdateAttributes:
        r1.retain(1);
        if ( !shorten( item1, 1 ) )
            next1 = true;
        r2.updateAttributes( item2.attributes );
        next2 = true;
        break;
    }
}

void DocumentMutation::xformDeleteElementStart( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok )
{
    switch( item2.type )
    {
    case ElementStart:
        xformInsertElementStart( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case ElementEnd:
        xformInsertElementEnd( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case InsertChars:
        xformInsertChars( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case AnnotationBoundary:
        xformAnnotationBoundary( r2, r1, item2, item1, next2, next1, anno2, anno1, true, ok );
        break;
    case Retain:
        // xformRetain( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        r1.deleteStart( item1.text, item1.attributes );
        next1 = true;
        if ( !shorten( item2, 1 ) )
            next2 = true;
        break;
    case DeleteStart:
        next1 = true;
        next2 = true;
        break;
    case UpdateAttributes:
        r1.deleteStart( item1.text, item1.attributes );
        next1 = true;
        next2 = true;
        break;
    case ReplaceAttributes:
        r1.deleteStart( item1.text, item1.attributes );
        next1 = true;
        next2 = true;
        break;
    case DeleteEnd:
    case DeleteChars:
        qDebug("The two mutations are not compatible");
        *ok = false;
        break;
    }
}

void DocumentMutation::xformDeleteElementEnd( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok )
{
    switch( item2.type )
    {
    case ElementStart:
        xformInsertElementStart( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case ElementEnd:
        xformInsertElementEnd( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case InsertChars:
        xformInsertChars( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case AnnotationBoundary:
        xformAnnotationBoundary( r2, r1, item2, item1, next2, next1, anno2, anno1, true, ok );
        break;
    case Retain:
        // xformRetain( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        r1.deleteEnd();
        next1 = true;
        if ( !shorten( item2, 1 ) )
            next2 = true;
        break;
    case DeleteEnd:
        next1 = true;
        next2 = true;
        break;
    case DeleteStart:
    case DeleteChars:
    case UpdateAttributes:
    case ReplaceAttributes:
        qDebug("The two mutations are not compatible");
        *ok = false;
        break;
    }
}

void DocumentMutation::xformDeleteChars( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok )
{
    switch( item2.type )
    {
    case ElementStart:
        xformInsertElementStart( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case ElementEnd:
        xformInsertElementEnd( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case InsertChars:
        xformInsertChars( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case AnnotationBoundary:
        xformAnnotationBoundary( r2, r1, item2, item1, next2, next1, anno2, anno1, true, ok );
        break;
    case Retain:
        // xformRetain( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        {
            int len = qMin( item1.text.length(), item2.count );
            r1.deleteChars( item1.text.left(len) );
            if ( !shorten( item1, len ) )
                next1 = true;
            if ( !shorten( item2, len ) )
                next2 = true;
        }
        break;
    case DeleteChars:
        {
            int len = qMin( item1.text.length(), item2.text.length() );
            if ( !shorten( item1, len ) )
                next1 = true;
            if ( !shorten( item2, len ) )
                next2 = true;
        }
        break;
    case DeleteStart:
    case DeleteEnd:
    case UpdateAttributes:
    case ReplaceAttributes:
        qDebug("The two mutations are not compatible");
        *ok = false;
        break;
    }
}

void DocumentMutation::xformUpdateAttributes( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok )
{
    switch( item2.type )
    {
    case ElementStart:
        xformInsertElementStart( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case ElementEnd:
        xformInsertElementEnd( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case InsertChars:
        xformInsertChars( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case AnnotationBoundary:
        xformAnnotationBoundary( r2, r1, item2, item1, next2, next1, anno2, anno1, true, ok );
        break;
    case Retain:        
        r1.updateAttributes( item1.attributes );
        next1 = true;
        r2.retain(1);
        if ( !shorten( item2, 1 ) )
            next2 = true;
        break;
    case DeleteStart:
        next1 = true;
        r2.deleteStart( item2.text, item2.attributes );
        next2 = true;
        break;
    case UpdateAttributes:
        {
            StructuredDocument::AttributeList attribs1 = item1.attributes;
            StructuredDocument::AttributeList attribs2 = item2.attributes;
            foreach( QString key, item2.attributes.keys() )
            {
                if ( key[0] != '-' )
                    continue;
                key = key.mid(1);
                if ( item1.attributes.contains(key) )
                {
                    attribs2["-" + key] = item1.attributes[key];
                    attribs1.remove(key);
                    attribs1.remove("-" + key);
                }
            }
            r1.updateAttributes(attribs1);
            r2.updateAttributes(attribs2);
            next1 = true;
            next2 = true;
        }
        break;
    case ReplaceAttributes:
        {
            r1.retain(1);
            next1 = true;
            StructuredDocument::AttributeList attribs2 = item2.attributes;
            foreach( QString key, item1.attributes.keys() )
            {
                if ( key[0] == '-' )
                    continue;
                if ( item2.attributes.contains(key) )
                {
                    if ( item1.attributes[key].isNull() )
                        attribs2.remove("-" + key);
                    else
                        attribs2[ "-" + key ] = item1.attributes[key];
                }
                else if ( !item1.attributes[key].isNull() )
                    attribs2[ "-" + key ] = item1.attributes[key];
            }
            r2.replaceAttributes(attribs2);
            next2 = true;
        }
        break;
    case DeleteEnd:
    case DeleteChars:
        qDebug("The two mutations are not compatible");
        *ok = false;
        break;
    }
}

void DocumentMutation::xformReplaceAttributes( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok )
{
    switch( item2.type )
    {
    case ElementStart:
        xformInsertElementStart( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case ElementEnd:
        xformInsertElementEnd( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case InsertChars:
        xformInsertChars( r2, r1, item2, item1, next2, next1, anno2, anno1, ok );
        break;
    case AnnotationBoundary:
        xformAnnotationBoundary( r2, r1, item2, item1, next2, next1, anno2, anno1, true, ok );
        break;
    case Retain:
        r1.replaceAttributes( item1.attributes );
        next1 = true;
        r2.retain(1);
        if ( !shorten( item2, 1 ) )
            next2 = true;
        break;
    case DeleteStart:
        next1 = true;
        r2.deleteStart( item2.text, item2.attributes );
        next2 = true;
        break;
    case UpdateAttributes:
        {
            StructuredDocument::AttributeList attribs1 = item1.attributes;
            StructuredDocument::AttributeList attribs2 = item2.attributes;
            foreach( QString key, item1.attributes.keys() )
            {
                if ( key[0] == '-' )
                    continue;
                if ( attribs2.contains(key) )
                    attribs2[ "-" + key ] = item1.attributes[key];
            }
            foreach( QString key, item2.attributes.keys() )
            {
                if ( key[0] == '-' )
                    continue;
                if ( !attribs1.contains(key) )
                    attribs2["-" + key] = QString::null;
                attribs1[key] = item2.attributes[key];
                attribs1["-" + key] = item2.attributes[key];
            }
            r1.replaceAttributes( attribs1 );
            r2.updateAttributes( attribs2 );
            next1 = true;
            next2 = true;
        }
        break;
    case ReplaceAttributes:
        {
            r1.retain(1);
            next1 = true;
            StructuredDocument::AttributeList attribs2 = item2.attributes;
            foreach( QString key, item2.attributes.keys() )
            {
                if ( key[0] != '-' )
                    continue;
                if ( !item1.attributes.contains(key.mid(1)) )
                    attribs2.remove( key );
            }
            foreach( QString key, item1.attributes.keys() )
            {
                if ( key[0] == '-' )
                    continue;
                attribs2["-" + key] = item1.attributes[key];
            }
            r2.replaceAttributes( attribs2 );
            next2 = true;
        }
        break;
    case DeleteEnd:
    case DeleteChars:
        qDebug("The two mutations are not compatible");
        *ok = false;
        break;
    }
}

QPair<DocumentMutation,DocumentMutation> DocumentMutation::xform( const DocumentMutation& m1, const DocumentMutation& m2, bool* ok )
{
    *ok = true;
    if ( m1.isEmpty() || m2.isEmpty() )
        return QPair<DocumentMutation,DocumentMutation>(m1,m2);

    DocumentMutation r1;
    DocumentMutation r2;
    StructuredDocument::AnnotationChange anno1;
    StructuredDocument::AnnotationChange anno2;

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
            if ( it2 != m2.end() )
                item2 = *it2;
            continue;
        }
        if ( it2 == m2.end() )
        {
            r1.m_items.append( item2 );
            it1++;
            if ( it1 != m1.end() )
                item1 = *it1;
            continue;
        }

        bool next1 = false;
        bool next2 = false;
        switch( item1.type )
        {
            case ElementStart:
                xformInsertElementStart( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
                break;
            case ElementEnd:
                xformInsertElementEnd( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
                break;
            case InsertChars:
                xformInsertChars( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
                break;
            case Retain:
                xformRetain( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
                break;
            case DeleteStart:
                xformDeleteElementStart( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
                break;
            case DeleteEnd:
                xformDeleteElementEnd( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
                break;
            case DeleteChars:
                xformDeleteChars( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
                break;
            case UpdateAttributes:
                xformUpdateAttributes( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
                break;
            case ReplaceAttributes:
                xformReplaceAttributes( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
                break;
            case AnnotationBoundary:
                xformAnnotationBoundary( r1, r2, item1, item2, next1, next2, anno1, anno2, false, ok );
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
        case UpdateAttributes:
        case ReplaceAttributes:
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
