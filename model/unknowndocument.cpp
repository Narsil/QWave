#include "unknowndocument.h"
#include "structureddocument.h"
#include "documentmutation.h"

UnknownDocument::UnknownDocument(const QString& id)
        : m_id(id)
{
    m_doc = new StructuredDocument();
}

UnknownDocument::~UnknownDocument()
{
    if ( m_doc )
        delete m_doc;
}

void UnknownDocument::receive( const DocumentMutation& mutation, const QString& author )
{
    if ( m_doc )
        m_doc->apply(mutation, author);
}
