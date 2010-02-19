#ifndef XMPPSTANZA_H
#define XMPPSTANZA_H

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QList>
#include <QHash>

#include "actor/imessage.h"

class XmppTag;

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
    XmppTag(const QString& qualifiedName, const QXmlStreamAttributes& attribs, XmppTag* parent = 0 );
    XmppTag(const QString& text, XmppTag* parent = 0) : m_parent(parent), m_type(Text), m_qualifiedName(text) { }
    XmppTag(const QString& text, TagType type, XmppTag* parent = 0) : m_parent(parent), m_type(type), m_qualifiedName(text) { }
    XmppTag(const XmppTag& tag );
    virtual ~XmppTag();

    XmppTag* parent() const { return m_parent; }

    QString text() const { return m_qualifiedName; }
    void setText( const QString& name ) { m_qualifiedName = name; }
    void appendText( const QString& name ) { m_qualifiedName += name; }

    QString qualifiedName() const { return m_qualifiedName; }
    void setQualifiedName( const QString& name ) { m_qualifiedName = name; }

    QHash<QString,QString>& attributes() { return m_attributes; }
    const QHash<QString,QString>& attributes() const { return m_attributes; }
    QString attribute( const QString& qualifiedName ) const { return m_attributes[qualifiedName]; }
    void setAttribute( const QString& qualifiedName, const QString& value );
    QString operator[] ( const QString& qualifiedName ) const;
    bool hasAttribute(const QString& qualifiedName ) const { return m_attributes.contains( qualifiedName ); }

    QList<XmppTag*>& children() { return m_children; }
    const QList<XmppTag*>& children() const { return m_children; }
    QList<XmppTag*> children( const QString& qualifiedName ) const;
    XmppTag* child( const QString& qualifiedName ) const;    
    XmppTag* childAt( int index ) const { return m_children[index]; }
    /**
      * The tag takes ownership of the child tag.
      */
    void add( XmppTag* tag ) { m_children.append( tag ); tag->m_parent = this; }
    void add( const QString& text );
    void addCData( const QString& text );

    bool isText() const { return m_type == Text; }
    bool isElement() const { return m_type == Element; }
    bool isCData() const { return m_type == CData; }
    TagType tagType() const { return m_type; }

    void write( QXmlStreamWriter& writer ) const;

private:
    XmppTag* m_parent;
    TagType m_type;
    QString m_qualifiedName;
    QHash<QString,QString> m_attributes;
    QList<XmppTag*> m_children;
};

class XmppStanza : public IMessage, public XmppTag
{
public:
    XmppStanza( const QString& qualifiedName ) : XmppTag( qualifiedName, Element ), m_kind(Unknown) { }
    XmppStanza( const QString& qualifiedName, const QXmlStreamAttributes& attribs ) : XmppTag( qualifiedName, attribs ), m_kind(Unknown) { }
    XmppStanza( const XmppStanza& stanza ) : IMessage( stanza ), XmppTag( stanza ), m_kind( stanza.m_kind ) { }

    QString from() const { return attributes()["from"]; }
    QString to() const { return attributes()["to"]; }
    QString stanzaId() const { return attributes()["id"]; }
    QString type() const { return attributes()["type"]; }

    enum Kind
    {
        Unknown = 0,
        WaveletUpdate = 1,
        MessageReceipt = 2,
        HistoryRequest = 3,
        HistoryResponse = 4,
        SignerRequest = 5,
        SignerResponse = 6,
        PostSigner = 7,
        PostSignerResponse = 8,
        SubmitRequest = 9,
        SubmitResponse = 10,
        DiscoInfo = 11,
        DiscoInfoResponse = 12,
        DiscoItems = 13,
        DiscoItemsResponse = 14,
        Error = 1000
    };

    Kind kind() const { return m_kind; }
    void setKind( Kind kind ) { m_kind = kind; }

private:
    Kind m_kind;
};


#endif // XMPPSTANZA_H
