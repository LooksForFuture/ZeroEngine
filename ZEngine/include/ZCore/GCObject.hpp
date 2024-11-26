#ifndef GCOBJECT_H
#define GCOBJECT_H

#include <ZCore/Rtti.hpp>

struct GCObject : virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(GCObject);
};

#endif
