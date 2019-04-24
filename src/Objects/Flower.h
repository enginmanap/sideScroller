

#ifndef MARIO_FLOWER_H
#define MARIO_FLOWER_H


#include <SDL_render.h>
#include <SDL_mixer.h>

#include "../AABB.h"
#include "../Utils.h"
#include "InteractiveObject.h"
#include "../Constants.h"
#include "Mario.h"
#include "Coin.h"

class Flower : public InteractiveObject {
    std::vector<SDL_Texture *> texture;
    AABB* collisionBox;
    long hitTime = 0;
    std::vector<Mix_Chunk *>deadSound;
    bool isSquashed = false;
    bool directionRight = true;
    bool directionChangeRequested = false;
    long createTime = 0;

public:
    Flower(SDL_Renderer *ren, int x, int y);

    ~Flower();

    SDL_Texture* getTexture(long time __attribute((unused))) const override;

    AABB* getPosition() const override;

    TileTypes getTileType() const override;

    void render(SDL_Renderer* renderer, int x, int y, long time) override;

    void collideWithSide(std::shared_ptr<Context> context __attribute((unused)), TileTypes tile,
                         CollisionSide interactionSide, long time __attribute((unused))) override;

    TileTypes interactWithSide(std::shared_ptr<Context> context __attribute((unused)), std::shared_ptr<InteractiveObject> otherObject,
                               CollisionSide interactionSide, long time __attribute((unused))) override;

    bool waitingForDestroy() const override;

    void step(std::shared_ptr<Context> context __attribute((unused)), long time __attribute((unused))) override;

    void setCreateTime(long time);


};


#endif //MARIO_FLOWER_H
