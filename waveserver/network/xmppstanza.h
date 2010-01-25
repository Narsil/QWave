#ifndef XMPPSTANZA_H
#define XMPPSTANZA_H

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QList>
#include <QSharedPointer>

class XmppTag;
typedef QSharedPointer<XmppTag> XmppTagPtr;

class XmppTag
{
public:
    enum TagType
    {
        Element = 0,
        Text = 1,
        CData = 2
    };

    XmppTag(TagType type, XmppTag* parent = 0 ) : m_parent(parent), m_type(type) { }
    XmppTag(const QString& qualifiedName, const QXmlStreamAttributes& attribs, XmppTag* parent = 0 ) : m_parent(parent), m_type(Element), m_qualifiedName( qualifiedName ), m_attributes( attribs ) { }
    XmppTag(const QString& text, XmppTag* parent = 0) : m_parent(parent), m_type(Text), m_qualifiedName(text) { }
    XmppTag(const QString& text, TagType type, XmppTag* parent = 0) : m_parent(parent), m_type(type), m_qualifiedName(text) { }

    XmppTag* parent() const { return m_parent; }

    QString text() const { return m_qualifiedName; }
    void setText( const QString& name ) { m_qualifiedName = name; }
    void appendText( const QString& name ) { m_qualifiedName += name; }

    QString qualifiedName() const { return m_qualifiedName; }
    void setQualifiedName( const QString& name ) { m_qualifiedName = name; }

    QXmlStreamAttributes& attributes() { return m_attributes; }
    const QXmlStreamAttributes& attributes() const { return m_attributes; }
    void setAttributes( const QXmlStreamAttributes& attributes ) { m_attributes = attributes; }
    QString operator[] ( const QString& qualifiedName ) const;
    bool hasAttribute(const QString& qualifiedName ) const { return m_attributes.hasAttribute( qualifiedName ); }

    QList<XmppTagPtr>* children() { return &m_children; }
    const QList<XmppTagPtr>* children() const { return &m_children; }
    XmppTag* child( const QString& qualifiedName ) const;
    QList<XmppTag*> children( const QString& qualifiedName ) const;
    void add( XmppTag* tag ) { m_children.append( XmppTagPtr( tag ) ); }
    void add( const QString& text );

    bool isText() const { return m_type == Text; }
    bool isElement() const { return m_type == Element; }
//    bool isCDatat() const { return m_type == CData; }
    TagType tagType() const { return m_type; }

    void write( QXmlStreamWriter& writer ) const;

private:
    XmppTag* m_parent;
    TagType m_type;
    QString m_qualifiedName;
    QXmlStreamAttributes m_attributes;
    QList<XmppTagPtr> m_children;
};

class XmppStanza : public XmppTag
{
public:
    XmppStanza( const QString& qualifiedName ) : XmppTag( qualifiedName, Element ) { }
    XmppStanza( const QString& qualifiedName, const QXmlStreamAttributes& attribs ) : XmppTag( qualifiedName, attribs ) { }

    QString from() const { return attributes().value("from").toString(); }
    QString to() const { return attributes().value("to").toString(); }
    QString id() const { return attributes().value("id").toString(); }
};


#endif // XMPPSTANZA_H
