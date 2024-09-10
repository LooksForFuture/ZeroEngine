#ifndef ZENGINE_ZCOMPONENT
#define ZENGINE_ZCOMPONENT

#include <ZEngine/Rtti.hpp>
#include <ZEngine/ZRef.hpp>

struct ZEntity;

struct ZComponent : virtual RTTI::Enable {
	friend class ZEngine;
	friend struct ZEntity;

	template <typename T>
	friend ZRef<T> createEntity();
	friend void destroyEntity(ZRef<ZEntity> entity);

	RTTI_DECLARE_TYPEINFO(ZComponent);

private:
	bool m_enabled = true;
	bool markedForDelete = false;
	ZRef<ZEntity> owner; //the entity which the component has been attached to

protected:
	ZRef<ZComponent> self;

	virtual void onAttach() {} //is called when created and attached to an entity

	virtual void awake() {}
	virtual void start() {} //is called after awake

	virtual void fixedUpdate() {} //is called at a fixed timestep
	virtual void update() {} //is called every frame
	virtual void lateUpdate() {} //is called everyframe after update

	virtual void onDestroy() {} //is called after owner's onDestroy is called

	virtual void receiveEvent(ZEvent* event) {}; //is called whenevr an event is triggered for the owners

public:
	// update loop related
	bool pauseEnabled = false;
	bool updateEnabled = true;

	bool isMarkedForDelete() {
		return markedForDelete;
	}

	bool isEnabled() {
		return m_enabled;
	}

	inline ZRef<ZEntity> getOwner() {
		return owner;
	}

	ZRef<ZEntity> getSelf() {
		return self;
	}
};

#define GENERATET_COMPONENT_BODY(name, parent) friend class ZEngine; \
friend struct ZEntity; \
template <typename T> \
friend ZRef<T> createEntity(); \
friend void destroyEntity(ZRef<ZEntity> entity); \
RTTI_DECLARE_TYPEINFO(##name, ##parent);

#endif // !ZENGINE_ZCOMPONENT
