#ifndef ZENGINE_ZPHYSICSPIPELINE
#define ZENGINE_ZPHYSICSPIPELINE

#include <ZEngine/RTTI.hpp>
#include <ZEngine/ZCore.hpp>
#include <ZEngine/ZRef.hpp>
#include <ZEngine/ZComponent.hpp>

struct ZPhysicsPipeline : virtual RTTI::Enable {
	RTTI_DECLARE_TYPEINFO(ZPhysicsPipeline);

public:
	virtual void step(float timeStep) {}
};

#endif // !ZENGINE_ZPHYSICSPIPELINE
