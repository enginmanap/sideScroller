//
// Created by engin on 11.11.2017.
//

#include <SDL_render.h>
#include <SDL_mixer.h>

#include "../AABB.h"
#include "../Utils.h"
#include "InteractiveObject.h"
#include "Mario.h"
#include "HiddenCoin.h"
#include "Mushroom.h"


Mushroom::Mushroom(SDL_Renderer *ren, int x, int y) {//FIXME this should not need  renderer and map
    collisionBox = new AABB(
            x * TILE_SIZE,
            x * TILE_SIZE + TILE_SIZE -1,
            y * TILE_SIZE,
            y * TILE_SIZE + TILE_SIZE -1); //-1 because 32 is not part of box. pixels 0 - TILE_SIZE, TILE_SIZE excluded

    collisionBox->setPhysicsState(AABB::PhysicsState::DYNAMIC);

    std::string goombaImage = Utils::getResourcePath("powerUps") + "mushroom.bmp";
    texture.push_back(Utils::loadTexture(ren, goombaImage));

    //deadSound.push_back(Mix_LoadWAV("./res/sounds/stomp.wav"));
}

Mushroom::~Mushroom() {
    for (size_t i = 0; i < texture.size(); i++) {
        SDL_DestroyTexture(texture[i]);
    }
    delete(collisionBox);
}

SDL_Texture* Mushroom::getTexture(long time __attribute((unused))) const {
    return texture[0];
};

AABB* Mushroom::getPosition() const {
    return collisionBox;
};

Map::TileTypes Mushroom::getTileType() const {
    return Map::MUSHROOM;
}

void Mushroom::render(SDL_Renderer* renderer, int x, int y, long time) {
    SDL_Rect screenPos;
    screenPos.x = collisionBox->getLeftBorder() - x;
    screenPos.y = collisionBox->getUpBorder() - y;
    screenPos.w = TILE_SIZE;
    screenPos.h = TILE_SIZE;

    SDL_RenderCopyEx(renderer, getTexture(time), 0, &screenPos, 0, 0, SDL_FLIP_NONE);

}

void Mushroom::collideWithSide(std::shared_ptr<Context> context __attribute((unused)), Map::TileTypes tile,
                                int interactionSide, long time __attribute((unused))) {
    if (interactionSide == 1) {

    }
    else if (interactionSide == 3 || interactionSide == 4 || interactionSide == -1) {
        if(tile != Map::GROUND) {
            directionRight = !directionRight;
        }
    }

}

Map::TileTypes Mushroom::interactWithSide(std::shared_ptr<Context> context __attribute((unused)), std::shared_ptr<InteractiveObject> otherObject,
                                int interactionSide, long time __attribute((unused))) {
    if(hitTime != 0) {
        return Map::MUSHROOM;//if already interacted, don't allow again
    }

    // if mario is coming from top, kill
    if(otherObject->getTileType() == Map::TileTypes::PLAYER) {
        if(!otherObject->isDead()) {
            isSquashed = true;

            die(getTileType());
            Mario *player = static_cast<Mario *>(otherObject.get());
            player->grow();
            return Map::EMPTY;
        }
        // swap direction
    } else if (interactionSide == 3 || interactionSide == 4) {
        if(!directionChangeRequested) {
            directionRight = !directionRight;
        }
    }


    return Map::MUSHROOM;//no interaction yet
}

bool Mushroom::waitingForDestroy() {
    return isDead(); //there is no case we are expecting removal
};

void Mushroom::step(long time __attribute((unused))) {
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

};
