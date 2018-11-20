/*
 * Copyright (c) 2018 Amine Ben Hassouna <amine.benhassouna@gmail.com>
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without
 * limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
 * ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "game.h"

#define NUMBER_OF_LIFES    5

bool Game_start(SDL_Renderer *renderer, int w, int h)
{
    // Init grid
    Grid grid = {0};

    // Set grid dimensions
    int padding = 50;
    grid.rect.w = MIN(w - padding*2, h - padding*2);
    grid.rect.h = grid.rect.w;

    // Set number of cells
    grid.xCells = 10;
    grid.yCells = 10;

    // Set cells interspace
    grid.xInterspace = 6;
    grid.yInterspace = 6;

    // Set backgroud and border color
    grid.backgroundColor = COLOR_DARK_GRAY;
    grid.borderColor = COLOR_GRAY;

    // Ajust size and center
    Grid_ajustSize(&grid);
    Grid_alignCenter(&grid, w, h);

    if(!Grid_init(&grid))
    {
        fprintf(stderr, "Grid fail to initialize !\n");
        return false;
    }

    // Initialize framerate manager : 30 FPS
    FPSmanager fpsmanager;
    SDL_initFramerate(&fpsmanager);
    SDL_setFramerate(&fpsmanager, 30);

    // Initialize start time (in ms)
    long long last = Utils_time();

    // Falling brick coordinates
    int fallingBrickX, fallingBrickY;
    Game_resetFallingBrick(&grid, &fallingBrickX, &fallingBrickY);
    int fallingBrickSpeed = 2;

    // Floating brick coodrdinates
    int floatingBrickX = grid.xCells / 2;
    int floatingBrickY = grid.yCells - 2;

    // Color initial position
    grid.cells[floatingBrickX][floatingBrickY].rectColor = COLOR_BLUE;

    // Score and lifes
    int score = 0;
    int lifes = NUMBER_OF_LIFES;


    // Event loop exit flag
    bool quit = false;

    // Event loop
    while(!quit)
    {
        SDL_Event e;

        // Get available event
        while(SDL_PollEvent(&e))
        {
            // User requests quit
            if(e.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
            else if(e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    quit = true;
                    break;

                case SDLK_RIGHT:
                    if(floatingBrickX < grid.xCells - 1
                            // No brick at the right of the floating brick
                            && (floatingBrickY != fallingBrickY || floatingBrickX != fallingBrickX - 1) )
                    {
                        // Un-color the floating brick last position
                        grid.cells[floatingBrickX][floatingBrickY].rectColor = grid.backgroundColor;

                        // Color the floating brick new position
                        floatingBrickX++;
                        grid.cells[floatingBrickX][floatingBrickY].rectColor = COLOR_BLUE;
                    }
                    break;

                case SDLK_LEFT:
                    if(floatingBrickX > 0
                            // No brick at the left of the floating brick
                            && (floatingBrickY != fallingBrickY || floatingBrickX != fallingBrickX + 1) )
                    {
                        // Un-color the floating brick last position
                        grid.cells[floatingBrickX][floatingBrickY].rectColor = grid.backgroundColor;

                        // Color the floating brick new position
                        floatingBrickX--;
                        grid.cells[floatingBrickX][floatingBrickY].rectColor = COLOR_BLUE;
                    }
                    break;
                }
            }
        }

        // Move the falling brick
        if(Utils_time() - last >= 1000 / fallingBrickSpeed)
        {
            if(fallingBrickY >= 0)
            {
                // Un-color the falling brick last position
                grid.cells[fallingBrickX][fallingBrickY].rectColor = grid.backgroundColor;
            }

            if(fallingBrickY < grid.yCells - 1)
            {
                // Go to next position
                fallingBrickY++;

                // Check collision between the falling brick and the floating brick
                if(fallingBrickX == floatingBrickX && fallingBrickY == floatingBrickY)
                {
                    score++;
                    fallingBrickSpeed++;

                    // Reset position
                    Game_resetFallingBrick(&grid, &fallingBrickX, &fallingBrickY);
                }
                else
                {
                    // Color the falling brick new position
                    grid.cells[fallingBrickX][fallingBrickY].rectColor = COLOR_RED;
                }
            }
            else
            {
                // Remove a life
                lifes--;
                if(lifes <= 0)
                {
                    // Reset lifes
                    lifes = NUMBER_OF_LIFES;

                    // Remove from score
                    score--;
                }

                // Reset position
                Game_resetFallingBrick(&grid, &fallingBrickX, &fallingBrickY);
            }

            last = Utils_time();
        }

        // Set background color
        Utils_setBackgroundColor(renderer, COLOR_DARK_GRAY);

        // Render grid
        Grid_render(&grid, renderer);

        // Show score, number of remaining lifes and speed
        Game_showBoard(renderer, grid.rect.x + grid.xInterspace, grid.rect.y - 20, score, lifes, fallingBrickSpeed);

        // Update screen
        SDL_RenderPresent(renderer);

        // Delay
        SDL_framerateDelay(&fpsmanager);
    }

    return true;
}

void Game_showBoard(SDL_Renderer *renderer, int x, int y, int score, int lifes, int speed)
{
    char board[30];
    sprintf(board, "Score: %d  Lifes: %d  Speed: %d", score, lifes, speed);
    stringRGBA(renderer, x, y, board,
               COLOR_LIGHT_GRAY.r, COLOR_LIGHT_GRAY.g, COLOR_LIGHT_GRAY.b, COLOR_LIGHT_GRAY.a);
}

void Game_resetFallingBrick(Grid *grid, int *x, int *y)
{
    *x = Utils_rand(0, grid->xCells - 1); // Generate random number
    *y = -1;
}
