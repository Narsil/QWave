#ifndef STRUCTUREDDOCUMENT_H
#define STRUCTUREDDOCUMENT_H

#include <QObject>
#include <QChar>
#include <QHash>
#include <QList>
#include <QString>
#include <QSharedData>

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

    class AnnotationData : public QSharedData
    {
        public:
            AnnotationData() { }
            AnnotationData(const AnnotationData& annotation) : QSharedData(annotation), map( annotation.map ) { }
            AnnotationData(const QHash<QString,QString>& m) : map( m ) { }

            QHash<QString,QString> map;
    };

    class Annotation
    {
    public:
        Annotation() { }
        Annotation(const QHash<QString,QString>& map) { d = new AnnotationData(map); }
        Annotation( const Annotation& annotation ) { d = annotation.d; }

        bool isNull() const { return d.data() == 0; }
        QString value(const QString& key) const { if ( isNull() ) return QString::null; return d->map[key]; }
        QList<QString> keys() const { if ( isNull() ) return QList<QString>(); return d->map.keys(); }

        Annotation merge(const QHash<QString,QString>& map) const;

        bool operator==( const Annotation& a ) const { return a.d.data() == d.data() || (a.d.data() && d.data() && a.d->map == d->map); }
        bool operator!=( const Annotation& a ) const { return a.d.data() != d.data() || (d.data() && a.d->map != d->map); }

    private:
        QSharedDataPointer<AnnotationData> d;
    };

    typedef QHash<QString,QString> AttributeList;

    StructuredDocument(QObject* parent = 0);
    StructuredDocument(const StructuredDocument& doc);
    ~StructuredDocument();

    /**
      * @return false is an error occured. In this case the document is malformed and cannot be used any further.
      */
    virtual bool apply(const DocumentMutation& mutation);

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

    void setCursor( const QString& name, int position );
    void removeCursor( const QString& name );

    QString toPlainText() const;

    void print_();

protected:
    virtual void onMutationStart();
    virtual void onRetainChar(int index);
    virtual void onRetainElementStart(int index);
    virtual void onRetainElementEnd(int index);
    virtual void onDeleteChars(int index, const QString& chars);
    virtual void onDeleteElementStart(int index);
    virtual void onDeleteElementEnd(int index);
    virtual void onInsertChars(int index, const QString& chars);
    virtual void onInsertElementStart(int index);
    virtual void onInsertElementEnd(int index);
    virtual void onAnnotationUpdate(int index, const QHash<QString,QString>& updates);
    virtual void onMutationEnd();

private:
    void insertStart( int index, const QString& tag, const QHash<QString,QString>& map, const Annotation& anno);

    QList<QChar> m_items;
    QList<Annotation> m_annotations;
    QList<AttributeList> m_attributes;
    QHash<QString,int> m_cursors;



//    void writeAnnotation( int pos, const QHash<QString,QString>& map );
};

#endif // STRUTCUREDOCUMENT_H
