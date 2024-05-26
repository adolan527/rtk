#include "game.h"

//Global variable definitions
using namespace RTK;
FILE *debug;
WindowManager *winMan;

//Function declarations
void InitGame(); //Contains all initializing

void DrawFrame(); //Contains all draw calls

void UpdateFrame(); //Contains all game logic

void CloseGame(); //Frees variables

//Main
int main(int argc, char **argv) {
    InitGame();

    while (!WindowShouldClose()) {
        DrawFrame();
        UpdateFrame();
    }

    CloseGame();


    return 0;
}

//Function Definitions
void InitGame() {

    InitWindow(screenWidth, screenHeight, "rtk");
    SetTargetFPS(60);

    debug = fopen("debug.txt","w");

    winMan = new WindowManager({0,0,1600,900},0.1f,10);

    std::string name = "Name";
    auto window = new DynamicWindow({10,10,1000,800},name);
    window->EnableDrawWindow();

    std::string sampleText = "Here is an example of a sentence";
    auto text = new TextBox({0,window->GetHeaderOffset(),500,300},sampleText);
    text->SetTheme(LoadDefaultTheme());
    text->SetMinimumFontSize(12);
    text->SetFontSize(64);
    text->SetFontMargin({0.05,0.05});

    auto check = new CheckBox({550,window->GetHeaderOffset(),200,200});

    auto but = new Button({0,500,100,100},"Stuff", [](){printf("Big chungus\n");});

    window->AddElement(text);
    window->AddElement(check);
    window->AddElement(but);

    winMan->AddWindow(window);
    winMan->AddWindow(window);
}

void UpdateFrame() {

    winMan->Update();

    if(IsKeyPressed(KEY_TAB)){
        fprintf(debug,"----------------\n");
    }
}

void DrawFrame() {
    BeginDrawing();
    ClearBackground(LIGHTGRAY);

    winMan->Draw();
    EndDrawing();

}


void CloseGame() {
    delete winMan;
    CloseWindow();
    fclose(debug);
}
