//
// Created by engin on 11.11.2017.
//

#ifndef MARIO_WORLD_H
#define MARIO_WORLD_H


#include "Objects/InteractiveObject.h"
#include "Objects/Mario.h"
#include "Constants.h"
#include <memory>
#include <sstream>
#include <iomanip>
#include <SDL_ttf.h>

class World {
    std::vector<std::shared_ptr<InteractiveObject>> objects;
    SDL_Texture *coinsTextTexture = nullptr;
    TTF_Font *font = nullptr;
    SDL_Color textColor;
    SDL_Renderer *ren = nullptr;
    std::shared_ptr<Mario> mario = nullptr;
    SDL_Rect coinsRect;
    SDL_Rect coinImgPos;
    SDL_Rect marioTextRect;
    SDL_Texture *marioTextTexture = nullptr;
    SDL_Texture *scoreTexture = nullptr;
    SDL_Rect scoreRect;
    SDL_Texture *coinTexture = nullptr;
    TileTypes tiles[224][15];

public:

    SDL_Renderer *getRen() const {
        return ren;
    }
    void load(std::string logicFile, int &error);
    TileTypes getTileObject(int x, int y);
    SDL_Rect getAndRemoveObject(TileTypes types);
    SDL_Rect getObject(TileTypes type);
    void addObject(std::shared_ptr<InteractiveObject> object) {
        objects.push_back(object);
    }

    void updateScore() {
        std::ostringstream score;
        score << std::setw(6) << std::setfill('0') << mario->getScore();
        SDL_DestroyTexture(scoreTexture);
        SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, score.str().c_str(),
                                                             textColor);
        scoreTexture = SDL_CreateTextureFromSurface(ren, scoreSurface);
        SDL_FreeSurface(scoreSurface);
    }

    void updateCoins() {
        SDL_DestroyTexture(coinsTextTexture);
        SDL_Surface *coinsTextSurface = TTF_RenderText_Solid(font, ("*" + std::to_string(mario->getCoins())).c_str(),
                                                             textColor);
        coinsTextTexture = SDL_CreateTextureFromSurface(ren, coinsTextSurface);
        SDL_FreeSurface(coinsTextSurface);
    }

    void renderCoins() {
        SDL_RenderCopy(ren, coinTexture, nullptr, &coinImgPos);
        SDL_RenderCopy(ren, coinsTextTexture, NULL, &coinsRect);
    }
    
    void renderHUD() {
        updateCoins();
        updateScore();
        SDL_RenderCopy(ren, marioTextTexture, nullptr, &marioTextRect);
        SDL_RenderCopy(ren, scoreTexture, nullptr, &scoreRect);
        renderCoins();
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
            objects[i]->render(ren, x, y ,time);
        }
        mario->render(ren, x, y, time);
        renderHUD();
    }

    /**
     *
     * Checks if the box is at collition with given tile coordinates.
     * The coordinates are not graphical, but logical.
     *
     * @param rightSpeed 1 for right, -1 for left, 0 for stopped
     * @param downSpeed 1 for up, -1 for down, 0 for stopped
     * @return true if collides
     */
    TileTypes collide(int rightSpeed, int downSpeed, long time, std::shared_ptr<Context> context,
                           std::shared_ptr<InteractiveObject> interactiveObject);

    void stepSimulation(long time, std::shared_ptr<Context> context);

    void stepSingleObject(long time, const std::shared_ptr<Context> &context,
                          const std::shared_ptr<InteractiveObject> interactiveObject) {
        AABB *aabb = interactiveObject->getPosition();

        if(aabb->getPhysicsState() == AABB::PhysicsState::STATIC) {
            return;
        }

        if (aabb->getPhysicsState() == AABB::PhysicsState::DYNAMIC && mario.get()->isKilled()) {
            return;
        }

        int horizontalSpeed = aabb->getHorizontalSpeed();

        TileTypes tile = collide(horizontalSpeed, 0, time, context, interactiveObject);

        if (tile == TileTypes::EMPTY || aabb->getPhysicsState() != AABB::DYNAMIC) {
            aabb->setLeftBorder(aabb->getLeftBorder() + horizontalSpeed);
            aabb->setRightBorder(aabb->getRightBorder() + horizontalSpeed);
        }

        aabb->setHorizontalSpeed(0);

        if (aabb->isHasJumpTriggered()) {
            aabb->setHasJumpTriggered(false);
            tile = collide(0, 1, time, context, interactiveObject);

            if (tile != TileTypes::EMPTY && aabb->getPhysicsState() == AABB::DYNAMIC) {
                aabb->setUpwardSpeed(aabb->getUpwardRequest());
            }
            aabb->setUpwardRequest(0);
        }
        int upwardSpeed = aabb->getUpwardSpeed();
        tile = collide(0, -1 * upwardSpeed, time, context, interactiveObject);
        //check if moving with upward speed is possible
        if (tile == TileTypes::OUT_OF_MAP || aabb->getPhysicsState() != AABB::DYNAMIC) {
            if (aabb->getUpwardSpeed() < 0) {
                // mario dies
                interactiveObject->die(tile);
            }
        }
        if ((tile != TileTypes::EMPTY && tile != TileTypes::OUT_OF_MAP) && aabb->getPhysicsState() == AABB::DYNAMIC) {//if not possible, match the tile, and then stop
            int curSize = (aabb->getDownBorder() - aabb->getUpBorder());
            aabb->setUpBorder(aabb->getUpBorder() - upwardSpeed);
            if (aabb->getUpwardSpeed() > 0) {
                aabb->setUpBorder(((aabb->getUpBorder() + TILE_SIZE) / TILE_SIZE) * TILE_SIZE);
            } else {
                aabb->setUpBorder((aabb->getUpBorder() / TILE_SIZE) * TILE_SIZE);
            }
            aabb->setDownBorder(aabb->getUpBorder() + curSize);
            aabb->setUpwardSpeed(0);
            aabb->setHasJumped(false);
        } else { //if possible update
            aabb->setUpBorder(aabb->getUpBorder() - aabb->getUpwardSpeed());
            aabb->setDownBorder(aabb->getDownBorder() - aabb->getUpwardSpeed());
            aabb->setUpwardSpeed(aabb->getUpwardSpeed() - 1);
        }
    }

    World(SDL_Renderer *ren) : ren(ren) {
        font = TTF_OpenFont("res/fonts/emulogic.ttf", 8);
        textColor.r = 255;
        textColor.g = 255;
        textColor.b = 255;
        textColor.a = 1;

        coinsRect.x = 300;
        coinsRect.y = 10;
        coinsRect.w = 35;
        coinsRect.h = 15;

        coinImgPos.x = 290;
        coinImgPos.y = 10;
        coinImgPos.w = 10;
        coinImgPos.h = 16;
        
        marioTextRect.x = 25;
        marioTextRect.y = 10;
        marioTextRect.w = 80;
        marioTextRect.h = 16;

        scoreRect.x = 25;
        scoreRect.y = 25;
        scoreRect.w = 100;
        scoreRect.h = 16;

        SDL_Surface *marioTextSurface = TTF_RenderText_Solid(font, "MARIO",
                                                             textColor);
        marioTextTexture = SDL_CreateTextureFromSurface(ren, marioTextSurface);
        SDL_FreeSurface(marioTextSurface);
        coinTexture = Utils::loadTexture(ren, Utils::getResourcePath() + "coin_text_icon.bmp");
    }

    void setMario(std::shared_ptr<Mario> mario) {
        this->mario = mario;
    }

};


#endif //MARIO_WORLD_H


