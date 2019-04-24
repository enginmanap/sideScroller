#include <SDL_render.h>
#include <SDL_mixer.h>

#include "../AABB.h"
#include "../Utils.h"
#include "InteractiveObject.h"
#include "Mario.h"
#include "Star.h"
#include "Brick.h"
#include "CoinBox.h"


Star::Star(SDL_Renderer *ren, int x, int y) {//FIXME this should not need  renderer and map
    collisionBox = new AABB(
            x * TILE_SIZE,
            x * TILE_SIZE + TILE_SIZE -1,
            y * TILE_SIZE,
            y * TILE_SIZE + TILE_SIZE -1); //-1 because 32 is not part of box. pixels 0 - TILE_SIZE, TILE_SIZE excluded

    collisionBox->setPhysicsState(AABB::PhysicsState::DYNAMIC);

    for (int i = 0; i < 4; i++) {
        std::string starImage = Utils::getResourcePath("star") + "star_" + std::to_string(i) + ".bmp";
        texture.push_back(Utils::loadTexture(ren, starImage));
    }

    //deadSound.push_back(Mix_LoadWAV("./res/sounds/stomp.wav"));
}

Star::~Star() {
    for (size_t i = 0; i < texture.size(); i++) {
        SDL_DestroyTexture(texture[i]);
    }
    delete(collisionBox);
}

SDL_Texture* Star::getTexture(long time) const {
    return texture[(time / 250) % 4];
}

AABB* Star::getPosition() const {
    return collisionBox;
}

TileTypes Star::getTileType() const {
    return TileTypes::STAR;
}

void Star::render(SDL_Renderer* renderer, int x, int y, long time) {
    SDL_Rect screenPos;
    screenPos.x = collisionBox->getLeftBorder() - x;
    screenPos.y = collisionBox->getUpBorder() - y;
    screenPos.w = TILE_SIZE;
    screenPos.h = TILE_SIZE;

    SDL_RenderCopyEx(renderer, getTexture(time), 0, &screenPos, 0, 0, SDL_FLIP_NONE);

}

void Star::collideWithSide(std::shared_ptr<Context> context __attribute((unused)), TileTypes tile,
                               CollisionSide interactionSide, long time __attribute((unused))) {
    if (interactionSide == CollisionSide::DOWN && tile == TileTypes::GROUND) {
        this->getPosition()->jump(8);
    }
    else if (interactionSide == CollisionSide::LEFT || interactionSide == CollisionSide::RIGHT || interactionSide == CollisionSide::INVALID) {
        if(tile != TileTypes::GROUND) {
            directionRight = !directionRight;
        }
    }

}

TileTypes Star::interactWithSide(std::shared_ptr<Context> context __attribute((unused)), std::shared_ptr<InteractiveObject> otherObject,
                                     CollisionSide interactionSide, long time) {
    if(hitTime != 0) {
        return TileTypes::STAR;//if already interacted, don't allow again
    }

    // if mario is coming from top, kill
    if(otherObject->getTileType() == TileTypes::PLAYER) {
        if(!otherObject->isDead()) {

            die(getTileType());
            Mario *player = static_cast<Mario *>(otherObject.get());
            player->setStar(time);
            return TileTypes::EMPTY;
        }
        // swap direction
    } else if (interactionSide == CollisionSide::RIGHT || interactionSide == CollisionSide::LEFT) {
        if(!directionChangeRequested) {
            directionRight = !directionRight;
        }
    }

    if (otherObject->getTileType() == TileTypes::BRICK && interactionSide == CollisionSide::DOWN) {
        if ((static_cast<Brick *>(otherObject.get())->isWhileHit())) {
            this->getPosition()->jump(8);
        }
    }

    if (otherObject->getTileType() == TileTypes::COIN_BOX && interactionSide == CollisionSide::DOWN) {
        if ((static_cast<CoinBox *>(otherObject.get())->isWhileHit())) {
            this->getPosition()->jump(8);
        }
    }


    return TileTypes::MUSHROOM;//no interaction yet
}

bool Star::waitingForDestroy() const {
    return isDead(); //there is no case we are expecting removal
}

void Star::step(std::shared_ptr<Context> context __attribute((unused)), long time __attribute((unused))) {
    if(directionChangeRequested) {
        directionRight = !directionRight;
        directionChangeRequested = false;
    }

    if (directionRight) {
        this->getPosition()->moveRight(2);
    } else {
        this->getPosition()->moveLeft(2);
    }

}
