//
// Created by engin on 11.11.2017.
//


#include <SDL_render.h>
#include <SDL_mixer.h>

#include "../AABB.h"
#include "../Utils.h"
#include "InteractiveObject.h"
#include "Mario.h"
#include "Brick.h"
#include "CoinBox.h"
#include "Goomba.h"
#include "Koopa.h"
#include "../Context.h"

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
    if (isHit) {
        return texture[0];
    }

    if (this->isSquashed) {
        return texture[2];
    }

    return texture[(time / 250) % 2];
}

AABB* Goomba::getPosition() const {
    return collisionBox;
}

TileTypes Goomba::getTileType() const {
    if (isHit) {
        return TileTypes::EMPTY;
    } else {
        return TileTypes::GOOMBA;
    }
}

void Goomba::render(SDL_Renderer* renderer, int x, int y, long time) {
    SDL_Rect screenPos;
    screenPos.x = collisionBox->getLeftBorder() - x;
    screenPos.y = collisionBox->getUpBorder() - y;
    screenPos.w = TILE_SIZE;
    screenPos.h = TILE_SIZE;
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (isHit) {
        flip = SDL_FLIP_VERTICAL;
        if (collisionBox->getUpBorder() > SCREEN_HEIGHT) {
            isRemoveWaiting = true;
        }
    }



    SDL_RenderCopyEx(renderer, getTexture(time), 0, &screenPos, 0, 0, flip);

}

void Goomba::collideWithSide(std::shared_ptr<Context> context __attribute((unused)), TileTypes tile[[gnu::unused]],
                             CollisionSide interactionSide, long time __attribute((unused))) {
    if (interactionSide == CollisionSide::DOWN) {

    }
    else if (interactionSide == CollisionSide::LEFT || interactionSide == CollisionSide::RIGHT || interactionSide == CollisionSide::INVALID) {
        directionRight = !directionRight;
    }

}

TileTypes Goomba::interactWithSide(std::shared_ptr<Context> context, std::shared_ptr<InteractiveObject> otherObject,
                                   CollisionSide interactionSide, long time) {
    if(squashTime != 0) {
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

            squashTime = time;
            context->getHUD()->animateScore(score, collisionBox->getLeftBorder(), collisionBox->getUpBorder(), time);
            die(getTileType());
            return TileTypes::EMPTY;
        }
        // swap direction
    } else if ((interactionSide == CollisionSide::LEFT || interactionSide == CollisionSide::RIGHT)
                && otherObject->getTileType() == TileTypes::PLAYER) {
        otherObject->die(getTileType());
    }

    if (interactionSide == CollisionSide::LEFT || interactionSide == CollisionSide::RIGHT) {
        if(otherObject->getTileType() == TileTypes::FLOWER ||
                otherObject->getTileType() == TileTypes::MUSHROOM) {

        } else if(!directionChangeRequested) {
            directionRight = !directionRight;
            if (otherObject->getTileType() == TileTypes::GOOMBA) {
                Goomba *otherGoomba = static_cast<Goomba *>(otherObject.get());
                otherGoomba->directionChangeRequested = true;
            }
        }
    }

    if (interactionSide == CollisionSide::LEFT || interactionSide == CollisionSide::RIGHT) {
        if (otherObject->getTileType() == KOOPA) {
            Koopa *koopa = static_cast<Koopa *>(otherObject.get());
            if (koopa->getShellMoving()) {
                isHit = true;
                collisionBox->setPhysicsState(AABB::NON_INTERACTIVE);
                collisionBox->setUpwardSpeed(8);
                collisionBox->setUpBorder(collisionBox->getUpBorder() + TILE_SIZE/4);
                collisionBox->setDownBorder(collisionBox->getDownBorder() + TILE_SIZE/4);
                context->getHUD()->animateScore(score, collisionBox->getLeftBorder(), collisionBox->getUpBorder(), time);

            }
        }
    }

    if ((otherObject->getTileType() == TileTypes::BRICK ||
            otherObject->getTileType() == TileTypes::COIN_BOX ||
            otherObject->getTileType() == TileTypes::MUSHROOM_BOX ||
            otherObject->getTileType() == TileTypes::HIDDEN_COIN_BOX ||
            otherObject->getTileType() == TileTypes::HIDDEN_MUSHROOM)
            && interactionSide == CollisionSide::DOWN) {
        if ((static_cast<BoxBase *>(otherObject.get())->isWhileHit())) {
            isHit = true;
            collisionBox->setPhysicsState(AABB::NON_INTERACTIVE);
            collisionBox->setUpwardSpeed(8);
            collisionBox->setUpBorder(collisionBox->getUpBorder() + TILE_SIZE/4);
            collisionBox->setDownBorder(collisionBox->getDownBorder() + TILE_SIZE/4);
            context->getHUD()->animateScore(score, collisionBox->getLeftBorder(), collisionBox->getUpBorder(), time);

        }
    }

    if (otherObject->getTileType() == TileTypes::COIN_BOX && interactionSide == CollisionSide::DOWN) {
        if ((static_cast<CoinBox *>(otherObject.get())->isWhileHit())) {
            isHit = true;
            collisionBox->setPhysicsState(AABB::NON_INTERACTIVE);
            collisionBox->setUpwardSpeed(8);
            collisionBox->setUpBorder(collisionBox->getUpBorder() + TILE_SIZE/4);
            collisionBox->setDownBorder(collisionBox->getDownBorder() + TILE_SIZE/4);
            context->getHUD()->animateScore(score, collisionBox->getLeftBorder(), collisionBox->getUpBorder(), time);

        }
    }

    if (otherObject->getTileType() == TileTypes::PLAYER) {
        if ((static_cast<Mario *>(otherObject.get())->getStar())) {
            isHit = true;
            collisionBox->setPhysicsState(AABB::NON_INTERACTIVE);
            collisionBox->setUpwardSpeed(8);
            collisionBox->setUpBorder(collisionBox->getUpBorder() + TILE_SIZE/4);
            collisionBox->setDownBorder(collisionBox->getDownBorder() + TILE_SIZE/4);
            context->getHUD()->animateScore(score, collisionBox->getLeftBorder(), collisionBox->getUpBorder(), time);
        }
    }

    if (otherObject->getTileType() == TileTypes::FIREBALL) {
        isHit = true;
        collisionBox->setPhysicsState(AABB::NON_INTERACTIVE);
        collisionBox->setUpwardSpeed(8);
        collisionBox->setUpBorder(collisionBox->getUpBorder() + TILE_SIZE / 4);
        collisionBox->setDownBorder(collisionBox->getDownBorder() + TILE_SIZE / 4);
        context->getHUD()->animateScore(score, collisionBox->getLeftBorder(), collisionBox->getUpBorder(), time);
    }

    return TileTypes::GOOMBA;//no interaction yet
}

bool Goomba::waitingForDestroy() const {
    return isRemoveWaiting;
}

void Goomba::step(std::shared_ptr<Context> context __attribute((unused)), long time) {
    if (isHit) {
        return;
    }
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
    if(squashTime != 0 && time - squashTime >= 250) {
        isRemoveWaiting = true;
    }
}

void Goomba::die(TileTypes type) {
    if (type == TileTypes::OUT_OF_MAP) {
        this->isRemoveWaiting = true;
    }
}

