#include <fstream>
#include "game.h"

//Global variable definitions
class RTKTest : public Game{
    FILE *m_debug;
    RTK::RTKRuntime m_runtime;

    RTK::WindowManager *m_winMan;

public:
    RTKTest(int screenWidth, int screenHeight) : Game(screenWidth, screenHeight){
        m_debug = fopen("debug.txt","w");
        m_winMan = new RTK::WindowManager({0,0,1600,900},0.1f,10);

        std::string name = "Name";
        auto window = new RTK::DynamicWindow({10,10,1000,800},name);
        window->EnableDrawWindow();

        name = "Second";
        auto window2 = new RTK::DynamicWindow({0,0,400,200},name);
        window2->EnableDrawWindow();
        window2->SetHeaderSize(0.25);

        std::string sampleText = "Here is an example of a sentence";
        auto text = new RTK::TextBox({0,window->GetHeaderOffset(),500,300},sampleText);
        text->SetTheme(RTK::LoadDefaultTheme());
        text->SetMinimumFontSize(12);
        text->SetFontSize(64);
        text->SetFontMargin({0.05,0.05});
       // text->SetCharacterLimit(50);
        text->EnableDrawCharacterCount();

        sampleText = "0";
        auto numeric = new RTK::TextBox({0,window2->GetHeaderOffset(),100,50},sampleText);
        numeric->SetTheme(RTK::LoadDefaultTheme());
        numeric->DisableWrapAtMinimumFontSize();
        numeric->EnableDrawCharacterCount();
        numeric->SetCharacterLimit(8);
        numeric->SetFilterFunction(&RTK::IsNumeric);
        numeric->SetFontMargin({0.05,0.05});
        numeric->SetHorizontalAlign(RTK::TextAlign::Start);
        numeric->FixedTextSize();

        auto check = new RTK::CheckBox({550,window->GetHeaderOffset(),200,200});

        auto but = new RTK::Button({window2->GetSize().x - 75,window2->GetHeaderOffset(),75,75},"Stuff", [](){printf("Big chungus\n");});
        but->SetFontMargin({0.1,0.1});
        but->FindMaxFontSize();
        but->SetAlign(RTK::TextAlign::Center);

        auto dropdown = new RTK::Dropdown({300,500,150,75},name);
        dropdown->AddOption(name);
        sampleText = "asdf";
        dropdown->AddOption(sampleText);
        sampleText = "another one";
        dropdown->AddOption(sampleText);
        sampleText = "qwerty";
        dropdown->AddOption(sampleText);

        window->AddElement(dropdown);
        window->AddElement(text);
        window->AddElement(check);

        window2->AddElement(but);
        window2->AddElement(numeric);

        m_winMan->AddWindow(window);
        m_winMan->AddWindow(window2);

        m_runtime.AddElement(m_winMan);
        m_runtime.RegisterFile("json.txt","debug");
    }

    void UpdateFrame() override{
        //m_winMan->Update();
        m_runtime.Update();
        if(IsKeyPressed(KEY_TAB)){
            fprintf(m_debug,"----------------\n");
        }
    }

    void DrawFrame() override{
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        //m_winMan->Draw();
        m_runtime.Draw();
        EndDrawing();
    }

    ~RTKTest(){
        m_winMan->WriteDebugInfo(m_debug);
        nlohmann::json j;
        //m_winMan->ToJson(j);
        m_runtime.ToJson();
        m_runtime.SaveJson("debug");
        //std::ofstream file("json.txt");
        //file << j.dump(4);
        //file.close();
        std::cout << j.dump(4) << std::endl;



        delete m_winMan;
        fclose(m_debug);
    }

};

//Main
int main(int argc, char **argv) {

    RTKTest game(screenWidth, screenHeight);
    game.Run();
    return 0;

}
