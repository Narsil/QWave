#ifndef DOCUMENTMUTATION_H
#define DOCUMENTMUTATION_H

#include <QHash>
#include <QString>
#include <QList>
#include <QPair>
#include "structureddocument.h"

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
        QList<QString> endKeys;
        StructuredDocument::AnnotationChange annotations;
        StructuredDocument::AttributeList attributes;
        QString text;
        int count;

//        Item deepCopy();
    };

    void insertStart(const QString& tag, const StructuredDocument::AttributeList& attributes);
    void insertStart(const QString& tag);
    void insertEnd();
    void retain(int count);
    void insertChars(const QString& chars);
    void deleteStart(const QString& tag);
    void deleteEnd();
    void deleteChars(const QString& chars);
    void annotationBoundary(const QList<QString>& endKeys, const StructuredDocument::AnnotationChange& changes);

    void clear();
    bool isEmpty() const { return m_items.count() == 0; }
    QList<Item>::const_iterator begin() const { return m_items.constBegin(); }
    QList<Item>::const_iterator end() const { return m_items.constEnd(); }
    int count() const;

    DocumentMutation compose( const DocumentMutation& mutation ) const;
    /**
      * Transforms the other mutation such that it can be applied after this mutation.
      * The idea is the following:
      *
      * apply(otherMutation,d) = d2
      * apply(this,d) = d3
      * apply( apply(this,d), this.translate(otherMutation) ) = d4
      *
      * Thus, the other mutation is modified such that its purpose stays the same but now it can be applied
      * to apply(this,d) instead of just d.
      */
    DocumentMutation translate( const DocumentMutation& otherMutation ) const;
    DocumentMutation concat( const DocumentMutation& otherMutation ) const;

    static QPair<DocumentMutation,DocumentMutation> xform( const DocumentMutation& m1, const DocumentMutation& m2, bool* ok );

    void print_();

private:
    static bool shorten( Item& item, int len );
    QString mapToString(const StructuredDocument::AttributeList& map);
    QString mapToString(const StructuredDocument::AnnotationChange& map);

    QList<Item> m_items;
};

#endif // DOCUMENTMUTATION_H
