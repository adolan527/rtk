//Includes
#include <iostream>
#include <string>
#include <vector>
#include "raylib.h"
#include "raymath.h"
#include "rtk.h"
#include "json.hpp"

class Game{
public:
    // Inherit this class and override functions to add your game's functionality.
    // This class is a wrapper for the raylib game loop.
    // It should also hold your "global" variables
    int m_screenWidth = 1600;
    int m_screenHeight = 900;

    Game(int screenWidth, int screenHeight){
        InitWindow(m_screenWidth, m_screenHeight, "rtk");
        SetTargetFPS(60);
    }

    ~Game(){
        CloseWindow();
    }

    void Run(){
        while (!WindowShouldClose()) {
            DrawFrame();
            UpdateFrame();
        }
    }

    virtual void DrawFrame(){
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }

    virtual void UpdateFrame(){

    }




};

//Global class declarations

//Global type definitions



//Global variable declarations
const int screenWidth = 1600;
const int screenHeight = 900;

//Global function declarations

