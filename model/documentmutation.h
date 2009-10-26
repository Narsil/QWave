#ifndef DOCUMENTMUTATION_H
#define DOCUMENTMUTATION_H

#include <QHash>
#include <QString>
#include <QList>

class StructuredDocument;

class DocumentMutation
{
public:
    DocumentMutation();
    DocumentMutation(const DocumentMutation& mutation);
    ~DocumentMutation();

    enum ItemType
    {
        ElementStart,
        ElementEnd,
        Retain,
        InsertChars,
        DeleteStart,
        DeleteEnd,
        DeleteChars,
        AnnotationBoundary,
        /**
         * Used internally only.
         */
        NoItem
    };

    struct Item
    {
        ItemType type;
        QList<QString>* endKeys;
        QHash<QString,QString>* map;
        QString text;
        int count;

        Item deepCopy();
    };

    void insertStart(const QString& tag, const QHash<QString,QString>& map);
    void insertStart(const QString& tag);
    void insertEnd();
    void retain(int count);
    void insertChars(const QString& chars);
    void deleteStart(const QString& tag);
    void deleteEnd();
    void deleteChars(const QString& chars);
    void annotationBoundary(const QList<QString>& endKeys, const QHash<QString,QString>& changes);

    void clear();
    bool isEmpty() const { return m_items.count() == 0; }
    QList<Item>::const_iterator begin() const { return m_items.constBegin(); }
    QList<Item>::const_iterator end() const { return m_items.constEnd(); }
    int count() const;

    /**
      * @return the position of the item behind the last modified character in the document.
      * This position can be used as a cursor marker to show where the author if this mutation is typing currently.
      * If there are no characters in the current document or if nothing has been modified, 0 is returned.
      */
    int apply(StructuredDocument* doc) const;

    DocumentMutation compose( const DocumentMutation& mutation ) const;
    /**
      * Transforms this mutation such that it can be applied after otherMutation.
      * The idea is the following:
      *
      * apply(otherMutation,d) = d2
      * apply(this,d) = d3
      * apply( apply(otherMutation,d), this.translate(otherMutation) ) = d4
      *
      * Thus, this mutation is modified such that its purpose stays the same but now it can be applied
      * to apply(otherMutation,d) instead of just d.
      */
    DocumentMutation translate( const DocumentMutation& otherMutation ) const;
    DocumentMutation concat( const DocumentMutation& otherMutation ) const;

    void print_();

private:
    void freeItems();
    void shorten( Item& item, int len ) const;
    QString mapToString(const QHash<QString,QString>* map);

    QList<Item> m_items;
};

#endif // DOCUMENTMUTATION_H
