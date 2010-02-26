#ifndef STOREFOLK_H
#define STOREFOLK_H

#include "actor/actorfolk.h"

class Store;

class StoreFolk : public ActorFolk
{
public:
    StoreFolk();

    static StoreFolk* store();

protected:
    /**
      * The id is not a hierarchical name.
      */
    virtual ActorGroup* group( const QString& id, bool createOnDemand );

private:
    // Currently there is only one store. TODO: Use multiple stores in multiple threads
    Store* m_store;

    static StoreFolk* s_store;
};

#endif // STOREFOLK_H
