#include <iostream>
#include <ZCore/ZEngine.hpp>

int main(int argc, char** argv) {
    ZEngine app("Scorpio");
    Entity ent1 = Entity::spawn();
    Entity ent2 = Entity::spawn();
    return 0;
}
