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

StructuredDocument* UnknownDocument::releaseDocument()
{
    StructuredDocument* d = m_doc;
    m_doc = 0;
    return d;
}

void UnknownDocument::receive( const DocumentMutation& mutation )
{
    if ( m_doc )
        mutation.apply( m_doc );
}
