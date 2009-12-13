#ifndef DOCUMENTMUTATION_H
#define DOCUMENTMUTATION_H

#include <QHash>
#include <QString>
#include <QList>
#include <QPair>
#include "structureddocument.h"

/**
  * Mutates a StructuredDocument.
  */
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
        ReplaceAttributes,
        UpdateAttributes
    };

    /**
      * Represents a mutation operation.
      */
    struct Item
    {
        ItemType type;
        QList<QString> endKeys;
        StructuredDocument::AnnotationChange annotations;
        StructuredDocument::AttributeList attributes;
        QString text;
        int count;
    };

    void insertStart(const QString& tag, const StructuredDocument::AttributeList& attributes);
    void insertStart(const QString& tag);
    void insertEnd();
    void retain(int count);
    void insertChars(const QString& chars);
    void deleteStart(const QString& tag, const StructuredDocument::AttributeList& attributes);
    void deleteEnd();
    void deleteChars(const QString& chars);
    void annotationBoundary(const QList<QString>& endKeys, const StructuredDocument::AnnotationChange& changes);
    void updateAttributes( const QHash<QString,StructuredDocument::StringPair>& changes );
    void replaceAttributes( const StructuredDocument::AttributeList& oldAttributes, const StructuredDocument::AttributeList& newAttributes );

    void clear();
    bool isEmpty() const { return m_items.count() == 0; }
    QList<Item>::const_iterator begin() const { return m_items.constBegin(); }
    QList<Item>::const_iterator end() const { return m_items.constEnd(); }
    int count() const;

    static QPair<DocumentMutation,DocumentMutation> xform( const DocumentMutation& m1, const DocumentMutation& m2, bool* ok );

    QString toString() const;
    void print_();

private:
    void updateAttributes( const StructuredDocument::AttributeList& changes );
    void replaceAttributes( const StructuredDocument::AttributeList& changes );

    static void xformInsertElementStart( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok );
    static void xformInsertElementEnd( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok );
    static void xformInsertChars( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok );
    static void xformDeleteElementStart( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok );
    static void xformDeleteElementEnd( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok );
    static void xformDeleteChars( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok );
    static void xformRetain( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok );
    static void xformAnnotationBoundary( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool twisted, bool* ok );
    static void xformUpdateAttributes( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok );
    static void xformReplaceAttributes( DocumentMutation& r1, DocumentMutation& r2, Item& item1, Item& item2, bool& next1, bool& next2, StructuredDocument::AnnotationChange& anno1, StructuredDocument::AnnotationChange& anno2, bool* ok );

    static bool shorten( Item& item, int len );
    static QString mapToString(const StructuredDocument::AttributeList& map);
    static QString mapToString(const StructuredDocument::AnnotationChange& map);

    QList<Item> m_items;
};

#endif // DOCUMENTMUTATION_H
