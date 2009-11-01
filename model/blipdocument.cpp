#include "blipdocument.h"
#include "documentmutation.h"
#include <QStack>

BlipDocument::BlipDocument(QObject* parent)
        : StructuredDocument( parent )
{
}

BlipDocument::BlipDocument( const StructuredDocument& doc)
        : StructuredDocument( doc )
{
}

//bool BlipDocument::apply(const DocumentMutation& mutation)
//{
//    bool inBody = false;
//    bool afterLine = false;
//    int pos = 0;
//
//    QStack<QString> stack;
//    stack.push("");
//    int lineCount = -1;
//    int inlinePos;
//
//    for( QList<DocumentMutation::Item>::const_iterator it = mutation.begin(); it != mutation.end(); ++it )
//    {
//        switch( (*it).type )
//        {
//            case DocumentMutation::ElementStart:
//                if ( (*it).text == "line" && inBody )
//                    emit insertedLineBreak(lineCount,inlinePos);
//                lineCount++;
//                inlinePos = 0;
//                if ( (*it).map )
//                    m_items.insert(pos++, Item((*it).text, *((*it).map)));
//                else
//                    m_items.insert(pos++, Item((*it).text, QHash<QString,QString>()));
//                stack.push((*it).text);
//                break;
//            case DocumentMutation::ElementEnd:
//                {
//                    QString t = stack.pop();
//                    if ( t.isEmpty() )
//                        return false;
//                    m_items.insert(pos++, Item((*it).text) );
//                }
//                break;
//            case DocumentMutation::Retain:
//                {
//                    for( int i = 0; i < (*it).count; ++i, ++pos )
//                    {
//                        if ( pos >= m_items.count() )
//                            return false;
//                        const StructuredDocument::Item item = m_items[pos];
//                        if ( item.type == StructuredDocument::Start )
//                        {
//                            if (item.tagType() == "body" )
//                                inBody = true;
//                            else if (item.tagType() == "line" )
//                                afterLine = false;
//                            stack.push( item.tagType() );
//                        }
//                        else if ( item.type == StructuredDocument::End )
//                        {
//                            QString t = stack.pop();
//                            if ( t.isEmpty() )
//                                return false;
//                            if ( t == "body" )
//                                inBody = false;
//                            else if ( inBody && t == "line" )
//                            {
//                                afterLine = true;
//                                inlinePos = 0;
//                                lineCount++;
//                            }
//                        }
//                        else if ( item.type == StructuredDocument::Char && afterLine )
//                        {
//                            inlinePos++;
//                        }
//                    }
//                }
//                break;
//            case DocumentMutation::InsertChars:
//                {
//                    emit insertedText( lineCount, inlinePos, (*it).text );
//                    for( int i = 0; i < (*it).text.length(); ++i )
//                    {
//                        m_items.insert(pos++, Item((*it).text[i]) );
//                    }
//                }
//                break;
//            case DocumentMutation::DeleteStart:
//                {
//                    stack.push( (*it).text );
//                    m_items[pos].free();
//                    m_items.removeAt(pos);
//                }
//                break;
//            case DocumentMutation::DeleteEnd:
//                {
//                    QString t = stack.pop();
//                    if ( t.isEmpty() )
//                        return false;
//                    if ( t == "line" )
//                        emit deletedLineBreak(lineCount, inlinePos);
//                    m_items.removeAt(pos);
//                }
//                break;
//            case DocumentMutation::DeleteChars:
//                {
//                    emit deletedText( lineCount, inlinePos, (*it).text );
//                    for( int i = 0; i < (*it).text.length(); ++i )
//                    {
//                        if ( pos >= m_items.count() )
//                            return false;
//                        m_items.removeAt(pos);
//                    }
//                }
//                break;
//            case DocumentMutation::AnnotationBoundary:
//                // TODO
////                if ( (*it).map && (*it).endKeys )
////                    doc->annotationBoundary(*((*it).endKeys), *((*it).map));
////                else if ( (*it).endKeys )
////                    doc->annotationBoundary(*((*it).endKeys), QHash<QString,QString>());
////                else if ( (*it).map )
////                    doc->annotationBoundary(QList<QString>(), *((*it).map));
////                else
////                    doc->annotationBoundary(QList<QString>(), QHash<QString,QString>());
////                cursorPosition = doc->countDelta();
//                break;
//            case DocumentMutation::NoItem:
//                break;
//        }
//    }
//    return true;
//}
