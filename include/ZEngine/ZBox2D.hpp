#ifndef ZENGINE_BOX2D
#define ZENGINE_BOX2D

#include <ZEngine/Rtti.hpp>
#include <ZEngine/ZComponent.hpp>
#include <ZEngine/ZPhysicsPipeline.hpp>
#include <ZEngine/ZEntity.hpp>

#include <box2d/box2d.h>
#include <cstdint>
#include <vector>

class ZB2Collider : public ZComponent {
    friend class ZBox2DPhysics;
    GENERATET_COMPONENT_BODY(ZB2Collider, ZComponent);

private:
    int16_t colliderIndex = -1;

    ZRef<ZBox2DPhysics> physicsPipe;
    b2BodyType type;

    bool bullet = false;
    bool fixedRotation = false;
    bool sensor = false;

protected:
    b2Body* body;
    b2Fixture* fixture;

    void onAttach();
    void onDestroy();

public:
    b2BodyType getType() { return type; }
    void setType(b2BodyType bodyType) {
        type = bodyType;
        body->SetType(bodyType);
    }

    bool isBullet() { return bullet; }
    void setBullet(bool state) {
        bullet = state;
        body->SetBullet(state);
    }

    bool isFixedRotation() { return fixedRotation; }
    void setFixedRotation(bool state) {
        fixedRotation = state;
        body->SetFixedRotation(state);
    }

    bool isSensor() { return sensor; }
    void setSensor(bool state) {
        sensor = state;
        fixture->SetSensor(state);
    }

    Vec2 getPosition() {
        const b2Vec2& position = body->GetPosition();
        return Vec2(position.x, position.y);
    }

    float getRotation() {
        return body->GetAngle();
    }

    void setTransform(Vec3 position, float angle) {
        body->SetTransform(b2Vec2(position.x, position.y), angle);
    }

    Vec2 getLinearVelocity() {
        const b2Vec2& velocity = body->GetLinearVelocity();
        return Vec2(velocity.x, velocity.y);
    }

    void setLinenarVelocity(Vec2 velocity) {
        body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
    }

    float getAngularVelocity() {
        return body->GetAngularVelocity();
    }

    void setAngularVelocity(float omega) {
        body->SetAngularVelocity(omega);
    }
};

class B2ContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact);
    void EndContact(b2Contact* contact);
};

class ZBox2DPhysics : public ZPhysicsPipeline {
	RTTI_DECLARE_TYPEINFO(ZBox2DPhysics, ZPhysicsPipeline);

private:
	b2World world;
	Vec2 gravity;
    B2ContactListener listener;
	std::vector<ZRef<ZB2Collider>> colliders;

public:
    ZBox2DPhysics(Vec2 gravity = Vec2(0.0f, -9.8f)) : world(b2Vec2(gravity.x, gravity.y)), gravity(gravity) {
        world.SetContactListener(&listener);
    }

    Vec2 getGravity() { return gravity; }
    void setGravity(Vec2 gravity) {
        this->gravity = gravity;
        world.SetGravity(b2Vec2(gravity.x, gravity.y));
    }

    b2Body* createBody(const b2BodyDef* bodyDef) {
        return world.CreateBody(bodyDef);
    }

    void destoryBody(b2Body* body) {
        world.DestroyBody(body);
    }

    void addCollider(ZRef<ZB2Collider> collider) {
        if (collider->colliderIndex > 0) return;

        collider->colliderIndex = colliders.size();
        colliders.push_back(collider);
    }

    void removeCollider(ZRef<ZB2Collider> collider) {
        int16_t index = collider->colliderIndex;
        if (index < 0) return;

        if (colliders[index] == collider) {
            colliders.erase(colliders.begin() + index);
        }
    }

    void step(float timeStep);
};

void ZB2Collider::onAttach() {
    ZRef<ZBox2DPhysics> pipeline = getPhysicsPipeline();
    pipeline->addCollider(self);

    ZRef<ZEntity> owner = getOwner();
    Vec2 position = owner->getPosition();
    float rotation = owner->getRotation();

    b2BodyDef bodyDef;
    bodyDef.position.Set(position.x, position.y);
    bodyDef.angle = toRadians(rotation);
    bodyDef.allowSleep = true;
    bodyDef.type = b2BodyType::b2_staticBody;

    body = pipeline->createBody(&bodyDef);
    body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
}

void  ZB2Collider::onDestroy() {
    ZRef<ZBox2DPhysics> pipeline = getPhysicsPipeline();
    pipeline->removeCollider(self);
    pipeline->destoryBody(body);
}

class ZB2Contact {
    friend B2ContactListener;
    b2Contact* contact;

    ZRef<ZB2Collider> colliderA;
    ZRef<ZB2Collider> colliderB;

    ZRef<ZEntity> entity;

public:
    bool isTouching() { return contact->IsTouching(); }

    bool isEnabled() { return contact->IsEnabled(); }
    void setEnabled(bool state) { contact->SetEnabled(state); }

    ZRef<ZB2Collider> getColliderA() { return colliderA; }
    ZRef<ZB2Collider> getColliderB() { return colliderB; }

    ZRef<ZEntity> getEntity() { return entity; }
};

struct ZB2ContactBegin : ZEvent {
    RTTI_DECLARE_TYPEINFO(ZB2ContactBegin, ZEvent);
public:
    ZB2Contact contact;
};

struct ZB2ContactEnd : ZEvent {
    RTTI_DECLARE_TYPEINFO(ZB2ContactEnd, ZEvent);
public:
    ZB2Contact contact;
};

void B2ContactListener::BeginContact(b2Contact* contact) {
    ZB2Contact newContact;
    ZRef<ZEntity> entity1;
    ZRef<ZEntity> entity2;

    newContact.colliderA = reinterpret_cast<ZB2Collider*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer)->getSelf();
    newContact.colliderB = reinterpret_cast<ZB2Collider*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer)->getSelf();

    entity1 = newContact.colliderA->getOwner();
    entity2 = newContact.colliderB->getOwner();

    newContact.entity = entity2;

    ZB2ContactBegin event;
    event.contact = newContact;
    entity1->broadcast(&event);

    ZRef<ZB2Collider> swapCollider = newContact.colliderA;
    newContact.colliderA = newContact.colliderB;
    newContact.colliderB = swapCollider;
    newContact.entity = entity1;

    entity2->broadcast(&event);
}

void B2ContactListener::EndContact(b2Contact* contact) {
    ZB2Contact newContact;
    ZRef<ZEntity> entity1;
    ZRef<ZEntity> entity2;

    newContact.colliderA = reinterpret_cast<ZB2Collider*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer)->getSelf();
    newContact.colliderB = reinterpret_cast<ZB2Collider*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer)->getSelf();

    entity1 = newContact.colliderA->getOwner();
    entity2 = newContact.colliderB->getOwner();

    newContact.entity = entity2;

    ZB2ContactEnd event;
    event.contact = newContact;
    entity1->broadcast(&event);

    ZRef<ZB2Collider> swapCollider = newContact.colliderA;
    newContact.colliderA = newContact.colliderB;
    newContact.colliderB = swapCollider;
    newContact.entity = entity1;

    entity2->broadcast(&event);
}

class ZBoxCollider : public ZB2Collider {
public:
    void onAttach() {
        ZB2Collider::onAttach();
        b2PolygonShape shape;
        Vec2 ownerScale = getOwner()->getScale();
        shape.SetAsBox(ownerScale.x / 2 - shape.m_radius, ownerScale.y / 2 - shape.m_radius, b2Vec2(0.0f, 0.0f), 0);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;

        fixture = body->CreateFixture(&fixtureDef);
    }
};

class ZCircleCollider : public ZB2Collider {
public:
    void onAttach() {
        ZB2Collider::onAttach();
        b2CircleShape shape;
        shape.m_radius = getOwner()->getScale().x / 2;

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;

        fixture = body->CreateFixture(&fixtureDef);
    }
};

void ZBox2DPhysics::step(float timeStep) {
    int32 velocityIterations = 6;
    int32 positionIterations = 2;

    world.Step(timeStep, velocityIterations, positionIterations);

    for (ZRef<ZB2Collider> collider : colliders) {
        ZRef<ZEntity> owner = collider->getOwner();
        owner->setPosition(Vec2(collider->body->GetPosition().x, collider->body->GetPosition().y));
        owner->setRotation(toDegrees(collider->body->GetAngle()));
    }
}

#endif // !ZENGINE_BOX2D
