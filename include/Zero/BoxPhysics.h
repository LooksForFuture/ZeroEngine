#include <box2d/box2d.h>

class Box2DCollider;

class B2Contact;

class B2ContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact);
    void EndContact(b2Contact* contact);
};

class Box2DPhysics : public PhysicsPipeline {
    b2World world;
    Vec2 gravity;
    std::vector<Box2DCollider*> colliders;

    B2ContactListener listener;

public:
    Box2DPhysics(Vec2 gravity = Vec2(0.0f, -9.8f)) : world(b2Vec2(gravity.x, gravity.y)), gravity(gravity) {
        world.SetContactListener(&listener);
    }

    Vec2 getGravity() { return gravity; }
    void setGravity(Vec2 gravity) {
        this->gravity = gravity;
        world.SetGravity(b2Vec2(gravity.x, gravity.y));
    }

    b2Body* createBody(b2BodyDef* bodyDef) {
        return world.CreateBody(bodyDef);
    }

    void destoryBody(b2Body* body) {
        world.DestroyBody(body);
    }

    void registerBody(Box2DCollider* collider) {
        colliders.push_back(collider);
    }

    void step(float timeStep);
};

class Box2DCollider : public Component {
protected:
    friend Box2DPhysics;
    Box2DPhysics* world;
    b2Body* body;
    b2BodyType type;
    b2Fixture* fixture;

    bool bullet = false;
    bool fixedRotation = false;
    bool sensor = false;
public:
    virtual void configure(b2BodyType bodyType, Vec2 position, float angle) {
        world = (Box2DPhysics*)engine->getPhysicsPipeline();
        world->registerBody(this);

        b2BodyDef bodyDef;
        Vec3 ownerPosition = transform->getPosition();
        bodyDef.position.Set(ownerPosition.x + position.x, ownerPosition.y + position.y);
        bodyDef.angle = -glm::radians(transform->getRotation().z + angle);
        bodyDef.allowSleep = true;

        type = bodyType;
        bodyDef.type = bodyType;

        body = world->createBody(&bodyDef);
        body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
    }

    virtual void onEnable() { body->SetEnabled(true); }

    virtual void onDisable() { body->SetEnabled(false); }

    b2BodyType getType() { return type; }
    b2BodyType setType(b2BodyType bodyType) {
        if (type != bodyType) {
            type = bodyType;
            body->SetType(bodyType);
        }
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

    virtual void onDestroy() {
        world->destoryBody(body);
    }
};

class B2Contact {
    friend B2ContactListener;
    b2Contact* contact;

    Box2DCollider* colliderA;
    Box2DCollider* colliderB;

    Entity* entity;

public:
    bool isTouching() { return contact->IsTouching(); }
    
    bool isEnabled() { return contact->IsEnabled(); }
    void setEnabled(bool state) { contact->SetEnabled(state); }

    Box2DCollider* getColliderA() { return colliderA; }
    Box2DCollider* getColliderB() { return colliderB; }

    Entity* getEntity() { return entity; }
};

struct B2ContactBegin : Event {
    RTTI_DECLARE_TYPEINFO(B2ContactBegin, Event);
public:
    B2Contact contact;
};

struct B2ContactEnd : Event {
    RTTI_DECLARE_TYPEINFO(B2ContactEnd, Event);
public:
    B2Contact contact;
};

void B2ContactListener::BeginContact(b2Contact* contact) {

    B2Contact newContact;
    Entity* entity1;
    Entity* entity2;

    newContact.colliderA = reinterpret_cast<Box2DCollider*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer);
    newContact.colliderB = reinterpret_cast<Box2DCollider*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer);

    entity1 = newContact.colliderA->getOwner();
    entity2 = newContact.colliderB->getOwner();

    newContact.entity = entity2;

    B2ContactBegin event;
    event.contact = newContact;
    entity1->broadcast(&event);

    Box2DCollider* swapCollider = newContact.colliderA;
    newContact.colliderA = newContact.colliderB;
    newContact.colliderB = swapCollider;
    newContact.entity = entity1;

    entity2->broadcast(&event);
}

void B2ContactListener::EndContact(b2Contact* contact) {

    B2Contact newContact;
    Entity* entity1;
    Entity* entity2;

    newContact.colliderA = reinterpret_cast<Box2DCollider*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer);
    newContact.colliderB = reinterpret_cast<Box2DCollider*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer);

    entity1 = newContact.colliderA->getOwner();
    entity2 = newContact.colliderB->getOwner();

    newContact.entity = entity2;

    B2ContactEnd event;
    event.contact = newContact;
    entity1->broadcast(&event);

    Box2DCollider* swapCollider = newContact.colliderA;
    newContact.colliderA = newContact.colliderB;
    newContact.colliderB = swapCollider;
    newContact.entity = entity1;

    entity2->broadcast(&event);
}

class BoxCollider : public Box2DCollider {
public:
    void configure(b2BodyType bodyType, Vec2 position = Vec2(0.0f), float angle = 0) {
        Box2DCollider::configure(bodyType, position, angle);
        b2PolygonShape shape;
        Vec3 ownerScale = transform->getScale();
        shape.SetAsBox(ownerScale.x / 2 - shape.m_radius, ownerScale.y / 2 - shape.m_radius, b2Vec2(0.0f, 0.0f), 0);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;

        fixture = body->CreateFixture(&fixtureDef);
    }
};

class CircleCollider : public Box2DCollider {
public:
    void configure(b2BodyType bodyType, Vec2 position = Vec2(0.0f), float angle = 0) {
        Box2DCollider::configure(bodyType, position, angle);
        b2CircleShape shape;
        shape.m_radius = transform->getScale().x / 2;

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;

        fixture = body->CreateFixture(&fixtureDef);
    }
};

void Box2DPhysics::step(float timeStep) {
    int32 velocityIterations = 6;
    int32 positionIterations = 2;

    world.Step(timeStep, velocityIterations, positionIterations);

    for (Box2DCollider* collider : colliders) {
        Transform* t = collider->getOwner()->transform;
        t->setPosition(Vec3(collider->body->GetPosition().x, collider->body->GetPosition().y, t->getPosition().z));
        t->setRotation(Vec3(t->getRotation().x, t->getRotation().y, -glm::degrees(collider->body->GetAngle())));
    }
}