//
// Created by engin on 11.11.2017.
//

#ifndef MARIO_WORLD_H
#define MARIO_WORLD_H


#include "Objects/InteractiveObject.h"
#include <memory>

class World {
    std::vector<InteractiveObject*> objects;
    Map* map;

public:
    void addObject(InteractiveObject* object) {
        objects.push_back(object);
    }

    /**
     *
     * @param ren  renderer
     * @param x -> current world x
     * @param y -> current world y
     * @param time -> get ticks
     */
    void render(SDL_Renderer *ren, int x, int y, long time) {
        for (unsigned int i = 0; i < objects.size(); ++i) {
            objects[i]->render(ren,x,y,time);
        }
    }

    /**
     *
     * Checks if the box is at collition with given tile coordinates.
     * The coordinates are not graphical, but logical.
     *
     * @param map
     * @param rightSpeed 1 for right, -1 for left, 0 for stopped
     * @param downSpeed 1 for up, -1 for down, 0 for stopped
     * @return true if collides
     */
    Map::TileTypes collide(int rightSpeed, int downSpeed, InteractiveObject* playerObject, long time) {
        Map::TileTypes tile = Map::EMPTY;
        //we need 4 checks, since at any given time, object can be at 4 different places.
        tile = std::max(tile, map->getTileObject((playerObject->getPosition()->getLeftBorder() + rightSpeed)/32, (playerObject->getPosition()->getUpBorder() + downSpeed)/32));
        tile = std::max(tile, map->getTileObject((playerObject->getPosition()->getLeftBorder() + rightSpeed)/32, (playerObject->getPosition()->getDownBorder() + downSpeed)/32));
        tile = std::max(tile, map->getTileObject((playerObject->getPosition()->getRightBorder() + rightSpeed)/32, (playerObject->getPosition()->getUpBorder() + downSpeed)/32));
        tile = std::max(tile, map->getTileObject((playerObject->getPosition()->getRightBorder() + rightSpeed)/32, (playerObject->getPosition()->getDownBorder() + downSpeed)/32));

        // mario fall down from ground
        if (tile == Map::TileTypes::OUT_OF_MAP) {

        }


        InteractiveObject* collidingObject = NULL;
        int collisionSide = 0;//1 down, 2 up, 3 left 4 right
        for (unsigned int i = 0; i < objects.size(); ++i) {
            // that 4 if finds if that objects collide
            if(playerObject->getPosition()->getUpBorder() + downSpeed> objects[i]->getPosition()->getDownBorder()) {
                continue;
            }

            if(playerObject->getPosition()->getDownBorder() + downSpeed< objects[i]->getPosition()->getUpBorder()) {
                continue;
            }

            if(playerObject->getPosition()->getRightBorder() + rightSpeed < objects[i]->getPosition()->getLeftBorder()) {
                continue;
            }

            if(playerObject->getPosition()->getLeftBorder() + rightSpeed > objects[i]->getPosition()->getRightBorder()) {
                continue;
            }
            if(tile < objects[i]->getTileType()) {

                collidingObject = objects[i];
                //now we know there is a collition, check what is the direction of collision
                if(playerObject->getPosition()->getUpBorder() > objects[i]->getPosition()->getDownBorder()) {
                    tile = objects[i]->getTileType();
                    collisionSide = 1;
                }
                if(playerObject->getPosition()->getDownBorder() < objects[i]->getPosition()->getUpBorder()) {
                    tile = objects[i]->getTileType();
                    collisionSide = 2;
                }

                if(playerObject->getPosition()->getRightBorder() < objects[i]->getPosition()->getLeftBorder()) {
                    tile = objects[i]->getTileType();
                    collisionSide = 3;
                }

                if(playerObject->getPosition()->getLeftBorder() > objects[i]->getPosition()->getRightBorder()) {
                    tile = objects[i]->getTileType();
                    collisionSide = 4;
                }
            }
        }
        if(collidingObject != NULL) {
            collidingObject->interactWithSide(collisionSide, time);
        }
        return tile;
    }

    void stepSimulation(InteractiveObject *interactiveObject, long time) {
        AABB* aabb = interactiveObject->getPosition();

        int horizontalSpeed = aabb->getHorizontalSpeed();

        Map::TileTypes tile = collide(horizontalSpeed, 0, interactiveObject, time);

        if (tile == Map::EMPTY) {
            aabb->setLeftBorder(aabb->getLeftBorder() + horizontalSpeed);
            aabb->setRightBorder(aabb->getRightBorder() + horizontalSpeed);
        }

        aabb->setHorizontalSpeed(0);

        if (aabb->isHasJumpTriggered()) {
            aabb->setHasJumpTriggered(false);
            tile = collide(0, 1, interactiveObject, time);

            if (tile != Map::EMPTY) {
                aabb->setUpwardSpeed(aabb->getUpwardRequest());
            }
            aabb->setUpwardRequest(0);
        }
        int upwardSpeed = aabb->getUpwardSpeed();
        tile = collide(0, -1 * upwardSpeed, interactiveObject, time);
        //check if moving with upward speed is possible
        if (tile == Map::OUT_OF_MAP) {
            if (aabb->getUpwardSpeed() < 0) {
                // mario dies
                aabb->die();
                std::cout << "Mario dies\n";
            }
        }
        if(tile != Map::EMPTY) {//if not possible, match the tile, and then stop
            aabb->setUpBorder(aabb->getUpBorder() - upwardSpeed);
            if(aabb->getUpwardSpeed() > 0) {
                aabb->setUpBorder(((aabb->getUpBorder() + TILE_SIZE) / TILE_SIZE) * TILE_SIZE);
            } else {
                aabb->setUpBorder((aabb->getUpBorder() / TILE_SIZE) * TILE_SIZE);
            }
            aabb->setDownBorder(aabb->getUpBorder() + TILE_SIZE -1);
            aabb->setUpwardSpeed(0);
            aabb->setHasJumped(false);
        } else { //if possible update
            aabb->setUpBorder(aabb->getUpBorder() - aabb->getUpwardSpeed());
            aabb->setDownBorder(aabb->getDownBorder() - aabb->getUpwardSpeed());
            aabb->setUpwardSpeed(aabb->getUpwardSpeed() - 1);
        }

        for(unsigned int i = 0; i < objects.size(); i++) {
            if(objects[i]->waitingForDestroy()) {
                delete objects[i];
                objects.erase(objects.begin() + i);
            }
        }
    }

    World(Map* map) {
        this->map = map;
    }

};


#endif //MARIO_WORLD_H


