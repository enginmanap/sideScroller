//
// Created by engin on 11.11.2017.
//

#ifndef MARIO_BRICK_H
#define MARIO_BRICK_H


#include <SDL_render.h>
#include <SDL_mixer.h>

#include "../AABB.h"
#include "../Utils.h"
#include "InteractiveObject.h"

class Brick : public InteractiveObject {
    SDL_Texture *texture;
    AABB* collisionBox;
    long breakTime = 0;
    Mix_Chunk *breakSound = NULL;
    Mix_Chunk *hitSound = NULL;
    bool isDestroyed = false;

public:
    Brick(SDL_Renderer *ren, int x, int y) {//FIXME this should not need  renderer and map
        collisionBox = new AABB(
                x * TILE_SIZE,
                x * TILE_SIZE + TILE_SIZE -1,
                y * TILE_SIZE,
                y * TILE_SIZE + TILE_SIZE -1); //-1 because 32 is not part of box. pixels 0 - TILE_SIZE, TILE_SIZE excluded


        std::string marioImage = Utils::getResourcePath("brick") + "brickred.bmp";
        texture = Utils::loadTexture(ren, marioImage);
        breakSound = Mix_LoadWAV("./res/sounds/blockbreak.wav");
        hitSound = Mix_LoadWAV("./res/sounds/blockhit.wav");
    }

    ~Brick() {
        SDL_DestroyTexture(texture);
        delete(collisionBox);

    }

    SDL_Texture* getTexture(long time  __attribute((unused))) const {
        return texture;
    };

    AABB* getPosition() const {
        return collisionBox;
    };

    TileTypes getTileType() const {
        return TileTypes::BRICK;
    }

    void render(SDL_Renderer* renderer, int x, int y, long time) {
        SDL_Rect screenPos;
        screenPos.x = collisionBox->getLeftBorder() - x;
        screenPos.y = collisionBox->getUpBorder() - y;
        screenPos.w = TILE_SIZE;
        screenPos.h = TILE_SIZE;
        if(breakTime != 0 && breakTime != time) {

            SDL_Rect texturePos;
            screenPos.w = texturePos.w = TILE_SIZE/2;
            screenPos.h = texturePos.h = TILE_SIZE/2;
            texturePos.x = 0;
            texturePos.y = 0;
            long animTime = time - breakTime;
            if(animTime > 500) {
                isDestroyed = true;
                animTime=500;//stop animation after 500
            }
            float upSpeed = sin(M_PI * (animTime)/500.0f);
            screenPos.y = screenPos.y - upSpeed*32;

            screenPos.x =screenPos.x - animTime / 15;
            //up left
            SDL_RenderCopyEx(renderer, getTexture(time), &texturePos, &screenPos, -animTime, 0, SDL_FLIP_NONE);
            screenPos.x += TILE_SIZE/2 + 2 *(animTime / 16);
            texturePos.x = TILE_SIZE/2;
            //up right
            SDL_RenderCopyEx(renderer, getTexture(time), &texturePos, &screenPos, animTime, 0, SDL_FLIP_NONE);
            texturePos.y = TILE_SIZE /2;
            screenPos.y += TILE_SIZE/2 + animTime /32;
            //down right
            SDL_RenderCopyEx(renderer, getTexture(time), &texturePos, &screenPos, animTime, 0, SDL_FLIP_NONE);
            texturePos.x = 0;
            screenPos.x -= TILE_SIZE/2 + 2 *(animTime / 16);
            //down left
            SDL_RenderCopyEx(renderer, getTexture(time), &texturePos, &screenPos, -animTime, 0, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopyEx(renderer, getTexture(time), 0, &screenPos, 0, 0, SDL_FLIP_NONE);
        }
    }

    TileTypes interactWithSide(std::shared_ptr<Context> context __attribute((unused)), std::shared_ptr<InteractiveObject> otherObject __attribute((unused)),
                                    CollisionSide interactionSide, long time) {
        if(breakTime != 0) {
            return this->getTileType();//if already interacted, don't allow again
        }
        if(interactionSide == CollisionSide::DOWN) {
            //isDestroyed = true;
            Mix_PlayChannel(-1, breakSound, 0);
            breakTime = time;
        }
        return this->getTileType();//no interaction yet
    };

    bool waitingForDestroy() {
        return isDestroyed; //there is no case we are expecting removal
    };

    void step(long time __attribute((unused))) {};

};


#endif //MARIO_BRICK_H
