#ifndef STRUCTUREDDOCUMENT_H
#define STRUCTUREDDOCUMENT_H

#include <QObject>
#include <QChar>
#include <QHash>
#include <QList>
#include <QString>

class StructuredDocument : public QObject
{
    Q_OBJECT
public:

    enum ItemType
    {
        Char = 0,
        Start = 1,
        End = 2
    };

    struct Item
    {
        ItemType type;
        QChar ch;
        union Data
        {
            QHash<QString,QString>* map;
        } data;

        QString tagType() const;
    };

    struct Annotation
    {
        Annotation();
        Annotation(const Annotation& annotation);
        Annotation(int startPos, int endPos, const QHash<QString,QString>& map);

        QHash<QString,QString> merge(QHash<QString,QString>) const;
        bool isEqual(const QHash<QString,QString>& map);

        int startPos;
        int endPos;
        QHash<QString,QString> map;
    };

    StructuredDocument(QObject* parent = 0);
    ~StructuredDocument();

    void beginDelta();
    void insertStart(const QString& tag, const QHash<QString,QString>& map);
    void insertStart(const QString& tag);
    void insertEnd();
    void retain(int count);
    void insertChars(const QString& chars);
    void deleteStart(const QString& tag);
    void deleteEnd();
    void deleteChars(const QString& chars);
    void annotationBoundary(const QList<QString>& endKeys, const QHash<QString,QString>& changes);
    /**
      * This function is only meaningful while a delta is being applied.
      */
    int countDelta() const;
    void endDelta();

    QList<Item>::const_iterator begin() const { return m_items->constBegin(); }
    QList<Item>::const_iterator end() const { return m_items->constEnd(); }
    int count() const { return m_items->count(); }
    const Item& operator[] ( int index ) const;

    void setCursor( const QString& name, int position );
    void removeCursor( const QString& name );

    const Annotation& annotation(int pos) const;
    int annotationIndex(int pos) const;

    QString toPlainText() const;

    void print_();

private:
    void freeItems(QList<Item>* items);
    void writeAnnotation( const QHash<QString,QString>& map );
    void incDeltaPos();

    QList<Item>* m_items;
    QList<Annotation>* m_annotations;
    int m_deltaPos;
    int m_deltaAnnotationIndex;
    QList<Item>* m_newItems;
    QList<Annotation>* m_newAnnotations;
    QHash<QString,QString> m_annotationChanges;
    QHash<QString,int> m_cursors;
};

#endif // STRUTCUREDOCUMENT_H
