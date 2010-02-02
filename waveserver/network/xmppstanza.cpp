#include "xmppstanza.h"
#include <QXmlStreamAttribute>

XmppTag::XmppTag(const QString& qualifiedName, const QXmlStreamAttributes& attribs, XmppTag* parent )
    : m_parent(parent), m_type(Element), m_qualifiedName( qualifiedName )
{
    foreach( QXmlStreamAttribute a, attribs )
    {
        m_attributes[ a.name().toString() ] = a.value().toString();
    }
}

void XmppTag::add( const QString& text )
{
    if ( m_children.count() == 0 || !m_children.last()->isText() )
        add( new XmppTag( text, this ) );
    else
        m_children.last()->appendText( text );
}

void XmppTag::addCData( const QString& text )
{
    add( new XmppTag( text, XmppTag::CData, this ) );
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
    return m_attributes.value(qualifiedName);
}

void XmppTag::setAttribute( const QString& qualifiedName, const QString& value )
{
    m_attributes[ qualifiedName ] = value;
}

void XmppTag::write( QXmlStreamWriter& writer ) const
{
    switch( m_type )
    {
        case Element:
            writer.writeStartElement( qualifiedName() );
            foreach( QString key, m_attributes.keys() )
            {
                writer.writeAttribute( key, m_attributes[key] );
            }
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
