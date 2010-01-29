#include "xmppstanza.h"

void XmppTag::add( const QString& text )
{
    if ( m_children.count() == 0 || !m_children.last()->isText() )
        add( new XmppTag( text, this ) );
    else
        m_children.last()->appendText( text );
}

XmppTag* XmppTag::child( const QString& qualifiedName ) const
{
    foreach( QSharedPointer<XmppTag> tag, m_children )
    {
        if ( tag->isElement() && tag->qualifiedName() == qualifiedName )
            return tag.data();
    }
    return 0;
}

QList<XmppTag*> XmppTag::children( const QString& qualifiedName ) const
{
    QList<XmppTag*> result;
    foreach( QSharedPointer<XmppTag> tag, m_children )
    {
        if ( tag->isElement() && tag->qualifiedName() == qualifiedName )
            result.append( tag.data() );
    }
    return result;
}

QString XmppTag::operator[] ( const QString& qualifiedName ) const
{
    return m_attributes.value(qualifiedName).toString();
}

void XmppTag::setAttribute( const QString& qualifiedName, const QString& value )
{
    return m_attributes.append( qualifiedName, value );
}

void XmppTag::write( QXmlStreamWriter& writer ) const
{
    switch( m_type )
    {
        case Element:
            writer.writeStartElement( qualifiedName() );
            writer.writeAttributes( m_attributes );
            foreach( QSharedPointer<XmppTag> tag, m_children )
            {
                tag->write( writer );
            }
            writer.writeEndElement();
            break;
        case Text:
            writer.writeCharacters( text() );
            break;
        case CData:
            writer.writeCDATA( text() );
            break;
    }
}
