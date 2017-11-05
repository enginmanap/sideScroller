//
// Created by engin on 29.10.2017.
//

#ifndef MARIO_MARIO_H
#define MARIO_MARIO_H

#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

#include <SDL2/SDL.h>
#include <SDL_rect.h>

#include "Constants.h"
#include "Utils.h"
#include "Map.h"

class AABB {
    int x1,y1, x2,y2;
    int maxRight = 0;
public:
    int upwardSpeed = 0;

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
    Map::TileTypes collide(Map map, int rightSpeed, int downSpeed) {
        Map::TileTypes tile = Map::EMPTY;
        //we need 4 checks, since at any given time, object can be at 4 different places.

        tile = std::max(tile, map.getTileObject((x1 + rightSpeed)/32, (y1 + downSpeed)/32));
        tile = std::max(tile, map.getTileObject((x1 + rightSpeed)/32, (y2 + downSpeed)/32));
        tile = std::max(tile, map.getTileObject((x2 + rightSpeed)/32, (y1 + downSpeed)/32));
        tile = std::max(tile, map.getTileObject((x2 + rightSpeed)/32, (y2 + downSpeed)/32));
        return tile;
    }

    bool moveRight() {
        x1 += 2;
        x2 += 2;
        maxRight = std::max(x2, maxRight);
    }

    bool moveLeft() {
        x1 -= 2;
        x2 -= 2;
    }

    void step(Map map) {
        Map::TileTypes tile = collide(map, 0, -1 * upwardSpeed);
        //check if moving with upward speed is possible.
        if(tile != Map::EMPTY) {//if not possible, stop
            upwardSpeed = 0;
        } else { //if possible update
            y1 -= upwardSpeed;
            y2 -= upwardSpeed;
            upwardSpeed -= 1;
        }
    }

    void jump(Map &map) {
        std::cout << "pressed up" << std::endl;
        if(collide(map, 0, 1) != Map::EMPTY) {//jump only if you are stepping on something
            upwardSpeed = 18;
        }

    }

    int getLeftBorder() const {
        return x1;
    }

    int getRightBorder() const {
        return x2;
    }

    int getUpBorder() const {
        return y1;
    }

    int getDownBorder() const {
        return y2;
    }

    int getMaxRight() const {
        return maxRight;
    }

    void setAABB(int left, int right, int up, int down ) {
        x1 = left;
        x2 = right;
        y1 = up;
        y2 = down;
        maxRight = x2;
    }


};

class Mario {
public:
    enum TextureNames {
        STAND, MOVE
    };

private:
    std::map<TextureNames, std::vector<SDL_Texture *>> textures;
    int screenWidth;
    Map currentMap;
    AABB collitionBox;

public:

    Mario(SDL_Rect mapPosition, SDL_Renderer *ren, int screenWidth, Map currentMap, int &error);

    ~Mario() {
        SDL_DestroyTexture(textures[STAND][0]);
        SDL_DestroyTexture(textures[MOVE][0]);
        SDL_DestroyTexture(textures[MOVE][1]);
        SDL_DestroyTexture(textures[MOVE][2]);
    }

    SDL_Texture *getTexture(TextureNames requiredTexture, long time) {
        switch (requiredTexture) {
            case STAND:
                return textures[STAND][0];
            case MOVE:
                return textures[MOVE][(time / 75) % 3];
        }
    }

    AABB getPositionRect() const {
        return collitionBox;
    }

    SDL_Rect move(bool left, bool right, bool jump, bool crouch) {
        if(jump) {
            collitionBox.jump(currentMap);
        }
        if (left) {
            if (collitionBox.getLeftBorder() + (320) > collitionBox.getMaxRight()) {
                Map::TileTypes tile = collitionBox.collide(currentMap, -2, 0);
                if (tile == Map::PLAYER || tile == Map::EMPTY) {
                    collitionBox.moveLeft();
                }
            } else {
                //std::cout <<"max right was " << collitionBox.maxRight << ", " << " while x1 was" << collitionBox.getLeftBorder() << std::endl;
            }
        }
        if (right) {
            Map::TileTypes tile = collitionBox.collide(currentMap, 2, 0);
            if(tile == Map::PLAYER || tile == Map::EMPTY) {
                collitionBox.moveRight();
            } else {
                std::cout << "right movement not possible because the tile is " << tile << std::endl;
            }
        }
        collitionBox.step(currentMap);
    }

};


#endif //MARIO_MARIO_H
