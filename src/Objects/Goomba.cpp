//
// Created by engin on 11.11.2017.
//


#include <SDL_render.h>
#include <SDL_mixer.h>

#include "../AABB.h"
#include "../Utils.h"
#include "InteractiveObject.h"
#include "Mario.h"
#include "CoinBox.h"
#include "Goomba.h"

Goomba::Goomba(SDL_Renderer *ren, int x, int y) {//FIXME this should not need  renderer and map
    collisionBox = new AABB(
            x * TILE_SIZE,
            x * TILE_SIZE + TILE_SIZE -1,
            y * TILE_SIZE,
            y * TILE_SIZE + TILE_SIZE -1); //-1 because 32 is not part of box. pixels 0 - TILE_SIZE, TILE_SIZE excluded

    collisionBox->setPhysicsState(AABB::PhysicsState::DYNAMIC);
    for (int i = 0; i < 2; i++) {
        std::string goombaImage = Utils::getResourcePath("goomba") + "goomba_" + std::to_string(i) + ".bmp";
        texture.push_back(Utils::loadTexture(ren, goombaImage));
    }

    std::string goombaImage = Utils::getResourcePath("goomba") + "goomba_dead.bmp";
    texture.push_back(Utils::loadTexture(ren, goombaImage));

    deadSound.push_back(Mix_LoadWAV("./res/sounds/stomp.wav"));
}

Goomba::~Goomba() {
    for (size_t i = 0; i < texture.size(); i++) {
        SDL_DestroyTexture(texture[i]);
    }
    delete(collisionBox);
}

SDL_Texture* Goomba::getTexture(long time) const {
    if (this->isSquashed) {
        return texture[2];
    }

    return texture[(time / 250) % 2];
}

AABB* Goomba::getPosition() const {
    return collisionBox;
}

TileTypes Goomba::getTileType() const {
    return TileTypes::GOOMBA;
}

void Goomba::render(SDL_Renderer* renderer, int x, int y, long time) {
    SDL_Rect screenPos;
    screenPos.x = collisionBox->getLeftBorder() - x;
    screenPos.y = collisionBox->getUpBorder() - y;
    screenPos.w = TILE_SIZE;
    screenPos.h = TILE_SIZE;

    SDL_RenderCopyEx(renderer, getTexture(time), 0, &screenPos, 0, 0, SDL_FLIP_NONE);

}

void Goomba::collideWithSide(std::shared_ptr<Context> context __attribute((unused)), TileTypes tile,
                             CollisionSide interactionSide, long time __attribute((unused))) {
    if (interactionSide == CollisionSide::DOWN) {

    }
    else if (interactionSide == CollisionSide::LEFT || interactionSide == CollisionSide::RIGHT || interactionSide == CollisionSide::INVALID) {
        if(tile != TileTypes::GROUND) {
            directionRight = !directionRight;
        }
    }

}

TileTypes Goomba::interactWithSide(std::shared_ptr<Context> context __attribute((unused)), std::shared_ptr<InteractiveObject> otherObject,
                                   CollisionSide interactionSide, long time) {
    if(hitTime != 0) {
        return TileTypes::GOOMBA;//if already interacted, don't allow again
    }

    // if mario is coming from top, kill
    if(interactionSide == CollisionSide::UP && otherObject->getTileType() == TileTypes::PLAYER) {
        if(!otherObject->isDead()) {
            isSquashed = true;
            collisionBox->setDownBorder(collisionBox->getDownBorder() - TILE_SIZE / 2);
            collisionBox->setPhysicsState(AABB::PhysicsState::STATIC);
            otherObject->getPosition()->setUpwardSpeed(Mario::JUMP_SPEED / 2);

            for (uint32_t i = 0; i < deadSound.size(); ++i) {
                Mix_PlayChannel(-1, deadSound[i], 0);
            }

            hitTime = time;
            die(getTileType());
            return TileTypes::EMPTY;
        }
        // swap direction
    } else if (interactionSide == CollisionSide::LEFT || interactionSide == CollisionSide::RIGHT) {
        if (otherObject->getTileType() == TileTypes::PLAYER) {
            otherObject->die(getTileType());
        } else {
            if(!directionChangeRequested) {
                directionRight = !directionRight;
                if (otherObject->getTileType() == TileTypes::GOOMBA) {
                    Goomba *otherGoomba = static_cast<Goomba *>(otherObject.get());
                    otherGoomba->directionChangeRequested = true;
                }
            }

        }
    }


    return TileTypes::GOOMBA;//no interaction yet
}

bool Goomba::waitingForDestroy() {
    return isRemoveWaiting; //there is no case we are expecting removal
}

void Goomba::step(long time) {
    if(directionChangeRequested) {
        directionRight = !directionRight;
        directionChangeRequested = false;
    }
    if(!isSquashed) {
        if (directionRight) {
            this->getPosition()->moveRight(1);
        } else {
            this->getPosition()->moveLeft(1);
        }
    }
    if(hitTime != 0 && time - hitTime >= 250) {
        isRemoveWaiting = true;
    }
}

void Goomba::die(TileTypes type) {
    if (type == TileTypes::OUT_OF_MAP) {
        this->isRemoveWaiting = true;
    }
}
