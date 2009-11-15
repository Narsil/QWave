#ifndef STRUCTUREDDOCUMENT_H
#define STRUCTUREDDOCUMENT_H

#include <QObject>
#include <QChar>
#include <QHash>
#include <QList>
#include <QString>
#include <QSharedData>
#include <QPair>

class DocumentMutation;

class StructuredDocument : public QObject
{
    Q_OBJECT
public:

    enum ItemType
    {
        Start = 0,
        End = 1,
        Char = 2
    };

    typedef QHash<QString,QString> AttributeList;
    typedef QHash<QString,QString> AnnotationList;
    typedef QPair<QString,QString> StringPair;
    typedef QHash<QString,StringPair> AnnotationChange;

    class AnnotationData : public QSharedData
    {
        public:
            AnnotationData() { }
            AnnotationData(const AnnotationData& annotation) : QSharedData(annotation), map( annotation.map ) { }
            AnnotationData(const AnnotationList& m) : map( m ) { }

            AnnotationList map;
    };

    class Annotation
    {
    public:
        Annotation() { }
        Annotation(const AnnotationList& map) { d = new AnnotationData(map); }
        Annotation( const Annotation& annotation ) { d = annotation.d; }

        bool isNull() const { return d.data() == 0; }
        QString value(const QString& key) const { if ( isNull() ) return QString::null; return d->map[key]; }
        QList<QString> keys() const { if ( isNull() ) return QList<QString>(); return d->map.keys(); }

        Annotation merge(const AnnotationChange& map) const;

        bool operator==( const Annotation& a ) const { return a.d.data() == d.data() || (a.d.data() && d.data() && a.d->map == d->map); }
        bool operator!=( const Annotation& a ) const { return a.d.data() != d.data() || (d.data() && a.d->map != d->map); }

    private:
        QSharedDataPointer<AnnotationData> d;
    };

    StructuredDocument(QObject* parent = 0);
    StructuredDocument(const StructuredDocument& doc);
    ~StructuredDocument();

    /**
      * @return false is an error occured. In this case the document is malformed and cannot be used any further.
      */
    virtual bool apply(const DocumentMutation& mutation, const QString& author);

    const QList<QString>& authors() { return m_authors; }

    QList<QChar>::const_iterator begin() const { return m_items.constBegin(); }
    QList<QChar>::const_iterator end() const { return m_items.constEnd(); }
    int count() const { return m_items.count(); }
    QChar charAt( int index ) const { return m_items[index]; }
    const Annotation& annotationAt( int index ) const { return m_annotations[index]; }
    ItemType typeAt( int index ) const;
    const AttributeList& attributesAt( int index ) const { return m_attributes[index]; }
    /**
      * Only meaningful if there is a start tag at this position.
      */
    QString tagAt( int index ) const;

    QString toPlainText() const;

    void print_() const;
    QString toString() const;

protected:
    /**
      * While the document is mutated, this set of functions is called. This allows, for example, keeping
      * a graphical representation of the text in sync with the document obtained from wave servers.
      */
    virtual void onMutationStart(const QString& author);
    virtual void onRetainChar(int index);
    virtual void onRetainElementStart(int index);
    virtual void onRetainElementEnd(int index);
    virtual void onDeleteChars(int index, const QString& chars);
    virtual void onDeleteElementStart(int index);
    virtual void onDeleteElementEnd(int index);
    virtual void onInsertChars(int index, const QString& chars);
    virtual void onInsertElementStart(int index);
    virtual void onInsertElementEnd(int index);
    virtual void onAnnotationUpdate(int index, const AnnotationChange& updates);
    virtual void onMutationEnd();

private:
    void insertStart( int index, const QString& tag, const AttributeList& attributes, const Annotation& anno);

    QList<QChar> m_items;
    QList<Annotation> m_annotations;
    QList<AttributeList> m_attributes;
    QList<QString> m_authors;
};

#endif // STRUTCUREDOCUMENT_H
