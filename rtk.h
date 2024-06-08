//
// Created by Aweso on 5/19/2024.
//

#ifndef RTK_RTK_H
#define RTK_RTK_H

#include "raylib.h"
#include <string>
#include <list>
#include "json.hpp"
#include <fstream>
#include <unordered_set>

using json = nlohmann::json;

namespace RTK{
#define SPACING 16 //Based off of default parameters in DrawText()
#define GET_SPACING(_size) (_size < SPACING ? 1 : _size/SPACING)

#define MACRO_CTRL(_key) (IsKeyPressed(_key) && IsKeyDown(KEY_LEFT_CONTROL))
#define MACRO_SHIFT(_key) (IsKeyPressed(_key) && IsKeyDown(KEY_LEFT_SHIFT))
#define MACRO_ALT(_key) (IsKeyPressed(_key) && IsKeyDown(KEY_LEFT_ALT))


#define CONTINUOUS_TYPING_DELAY 35 // The amount of frames of holding down a key before it is spammed

bool IsAscii(int key){
    return ((key >= 32 && key <= 96) || (key >= 256 && key <= 265));
}
bool IsNumeric(int key){
    return (key <= 57 && key >= 45 && key != KEY_SLASH);

}

Rectangle RectangleFromJson(const json &j){
    auto r = j["rectangle"];
    return {r[0],r[1],r[2],r[3]};
}

void JsonFromRectangle(json &j, Rectangle r){
    j["rect"] = {r.x,r.y,r.width,r.height};
}

    typedef enum{
        Normal,
        Focused,
        Pressed,
        Disabled
    }GuiElementState;


    enum class TextAlign {
        Start,
        Center,
        End
    };
#define RTK_STATES_COUNT 4


    struct Theme{
        Color line[RTK_STATES_COUNT];
        Color text[RTK_STATES_COUNT];
        Color base[RTK_STATES_COUNT];
        Color background;
        float lineWidth;

        Font font;

        //operator overload for json

        friend void to_json(json& j, const Theme& theme){
            json temp;
            temp["line"] = {theme.line[0].r,theme.line[0].g,theme.line[0].b,theme.line[0].a,
                         theme.line[1].r,theme.line[1].g,theme.line[1].b,theme.line[1].a,
                         theme.line[2].r,theme.line[2].g,theme.line[2].b,theme.line[2].a,
                         theme.line[3].r,theme.line[3].g,theme.line[3].b,theme.line[3].a};
            temp["text"] = {theme.text[0].r,theme.text[0].g,theme.text[0].b,theme.text[0].a,
                         theme.text[1].r,theme.text[1].g,theme.text[1].b,theme.text[1].a,
                         theme.text[2].r,theme.text[2].g,theme.text[2].b,theme.text[2].a,
                         theme.text[3].r,theme.text[3].g,theme.text[3].b,theme.text[3].a};
            temp["base"] = {theme.base[0].r,theme.base[0].g,theme.base[0].b,theme.base[0].a,
                         theme.base[1].r,theme.base[1].g,theme.base[1].b,theme.base[1].a,
                         theme.base[2].r,theme.base[2].g,theme.base[2].b,theme.base[2].a,
                         theme.base[3].r,theme.base[3].g,theme.base[3].b,theme.base[3].a};
            temp["background"] = {theme.background.r,theme.background.g,theme.background.b,theme.background.a};
            temp["lineWidth"] = theme.lineWidth;
            temp["font"] = "times.ttf";
            j += temp;
        }

    };

    struct TextSettings{
        TextAlign horizontalAlign;
        TextAlign verticalAlign;
        float fontSize;
        Vector2 fontMargin; // percentage based. 200x200 rectangle with fontmargin 0.2,0.1, will have borders of
                            // 40px and 20px. Domain of (0,0.5).
        float spacing;

        //operator overload for json

        friend void to_json(json& j, const TextSettings& textSettings) {
            json temp;
            temp["horizontalAlign"] = (int) textSettings.horizontalAlign;
            temp["verticalAlign"] = (int) textSettings.verticalAlign;
            temp["fontSize"] = textSettings.fontSize;
            temp["fontMargin"] = {textSettings.fontMargin.x, textSettings.fontMargin.y};
            temp["spacing"] = textSettings.spacing;
            j += temp;
        }

    };

    Theme defaultTheme = {0};
Theme LoadDefaultTheme(){
    if(defaultTheme.font.glyphCount == 0){
        defaultTheme.font = LoadFontEx("times.ttf",64, nullptr,0);
        defaultTheme.line[Normal] = BLACK;
        defaultTheme.line[Focused] = ColorAlpha(BLACK, 0.9f);
        defaultTheme.line[Pressed] = ColorTint(BLACK, GREEN);
        defaultTheme.line[Disabled] = DARKGRAY;

        defaultTheme.text[Normal] = BLACK;
        defaultTheme.text[Focused] = ColorAlpha(BLACK, 0.9f);
        defaultTheme.text[Pressed] = ColorTint(BLACK, GREEN);
        defaultTheme.text[Disabled] = DARKGRAY;

        defaultTheme.base[Normal] = WHITE;
        defaultTheme.base[Focused] = LIGHTGRAY;
        defaultTheme.base[Pressed] = ColorTint(WHITE, GREEN);
        defaultTheme.base[Disabled] = GRAY;

        defaultTheme.background = {220, 220, 220, 255};

        defaultTheme.lineWidth = 5;
    }

    return defaultTheme;
}

TextSettings LoadDefaultTextSettings(){
    TextSettings settings = {
            .horizontalAlign = TextAlign::Start,
            .verticalAlign = TextAlign::Center,
            .fontSize = 16,
            .fontMargin = {0.25f,0.25f},
            .spacing = GET_SPACING(settings.fontSize)
    };
    return settings;

}
    void FindMaxFontSize(const char *text, TextSettings *textSettings, Rectangle rectangle, Theme theme, float minimumFontSize = 0) {
        textSettings->fontSize = rectangle.height;

        Vector2 measuredSize = MeasureTextEx(theme.font, text, textSettings->fontSize, textSettings->spacing);
        while (measuredSize.x > rectangle.width * (1 - 2 * textSettings->fontMargin.x) ||
               measuredSize.y > rectangle.height * (1 - 2 * textSettings->fontMargin.y)) {
            textSettings->fontSize -= 2;
            textSettings->spacing = GET_SPACING(textSettings->fontSize);
            measuredSize = MeasureTextEx(theme.font, text, textSettings->fontSize, textSettings->spacing);
        }
        if(textSettings->fontSize < minimumFontSize) {
            textSettings->fontSize = minimumFontSize;
            textSettings->spacing = GET_SPACING(textSettings->fontSize);
        }
    }

    void DrawTextInRectangle(const char *text, Rectangle rectangle, Theme theme, TextSettings textSettings, GuiElementState state = Normal, bool drawOutline = false){
        Vector2 offset = {rectangle.x, rectangle.y};
        Vector2 textSize = MeasureTextEx(theme.font, text, textSettings.fontSize, textSettings.spacing);

        switch(textSettings.horizontalAlign){
            case TextAlign::Start:
                offset.x += rectangle.width * textSettings.fontMargin.x;
                break;
            case TextAlign::Center:
                offset.x +=  rectangle.width / 2 - textSize.x / 2;
                break;
            case TextAlign::End:
                offset.x += rectangle.width - textSize.x - textSettings.fontMargin.x * rectangle.width;
                break;
        }
        switch(textSettings.verticalAlign){
            case TextAlign::Start:
                offset.y += rectangle.height * textSettings.fontMargin.y;
                break;
            case TextAlign::Center:
                offset.y += rectangle.height / 2 - textSize.y / 2;
                break;
            case TextAlign::End:
                offset.y += rectangle.height - textSize.y - textSettings.fontMargin.y * rectangle.height;
                break;
        }
        DrawTextEx(theme.font, text, offset, textSettings.fontSize, textSettings.spacing, theme.text[state]);
        if(drawOutline) DrawRectangleLines(offset.x,offset.y,textSize.x,textSize.y,GREEN);

    }

    int FindLargestCharacterSize(Font font, bool (*filterFunction)(int) = IsAscii){
        int maxSize = 0;
        for(unsigned char key = 0; key < 255; key++){
            if(filterFunction(key)){
                int index;
                int codepoint = GetCodepoint((char*)&key,&index); //don't care about codepointSize but can't do null due to segfault
                index = GetGlyphIndex(font,codepoint);
                if(key != '\n'){
                    int width;
                    if(font.glyphs[index].advanceX!=0){
                        width = font.glyphs[index].advanceX;
                    }
                    else{
                        width = font.recs[index].width + font.glyphs[index].offsetX;
                    }
                    maxSize = std::max(maxSize,width);
                }
            }
        }
        return maxSize;
    }

    unsigned char FindLargestCharacter(Font font, bool (*filterFunction)(int) = IsAscii){
        int maxSize = 0;
        unsigned char biggest = 0;
        for(unsigned char key = 0; key < 255; key++){
            if(filterFunction(key)){
                int index;
                int codepoint = GetCodepoint((char*)&key,&index); //don't care about codepointSize but can't do null due to segfault
                index = GetGlyphIndex(font,codepoint);
                if(key != '\n'){
                    int width;
                    if(font.glyphs[index].advanceX!=0){
                        width = font.glyphs[index].advanceX;
                    }
                    else{
                        width = font.recs[index].width + font.glyphs[index].offsetX;
                    }
                    if(width > maxSize){
                        maxSize = width;
                        biggest = key;
                    }
                }
            }
        }
        return biggest;
    }

GuiElementState MouseDetection(Rectangle rect){
    GuiElementState state = Normal;
    if(CheckCollisionPointRec(GetMousePosition(),rect)){
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
            state = Pressed;
        }
        else{
            state = Focused;
        }
    }
    return state;

}

    class GuiElement{
        /* The base class from which all gui elements inherit*/
    private:
    protected:
        Rectangle m_rect; // The display rectangle. Where it appears on screen and size
        Theme m_theme = LoadDefaultTheme(); // The theme that affects how the element looks
        GuiElementState m_state = Normal; // enable, focus (mouse hover), pressed, disabled

    public:
        GuiElement(Rectangle rect = {0,0,800,450}, Theme theme = LoadDefaultTheme(), GuiElementState state = Normal){
            m_theme = theme;
            m_state = state;
            m_rect = rect;
        }

        virtual ~GuiElement(){};

        virtual void Draw(){}
        virtual void Update(){}


        [[nodiscard]] const Rectangle &GetRect() {
            return m_rect;
        }

        [[nodiscard]] Vector2 GetPosition(){
            return {m_rect.x,m_rect.y};
        }

        [[nodiscard]] Vector2 GetSize(){
            return {m_rect.width,m_rect.height};
        }

        void SetRect(const Rectangle &mRect) {
            m_rect = mRect;
        }

        [[nodiscard]] const Theme &GetTheme() const {
            return m_theme;
        }

        void SetTheme(const Theme &mTheme) {
            m_theme = mTheme;
        }

        [[nodiscard]] GuiElementState GetState() const {
            return m_state;
        }

        void Enable() {
            if(m_state==Disabled) m_state = Normal;
        }

        void Disable() {
            m_state = Disabled;
        }

        virtual void ShiftRect(Vector2 translation){
            m_rect = {m_rect.x + translation.x, m_rect.y + translation.y, m_rect.width, m_rect.height};
        }

        void ScaleRect(Rectangle window){
            m_rect = {window.x + m_rect.x / window.width, window.y + m_rect.y / window.height, m_rect.width / window.width, m_rect.height / window.height};
        }

        void MouseDetection(Rectangle rect){
            if(CheckCollisionPointRec(GetMousePosition(),rect)){
                if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
                    m_state = Pressed;
                }
                else{
                    m_state = Focused;
                }
            }
            else{
                if(m_state!=Pressed) m_state = Normal;
            }
        }

        void MouseDetection(){
            if(CheckCollisionPointRec(GetMousePosition(),m_rect)){
                if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
                    m_state = Pressed;
                }
                else{
                    m_state = Focused;
                }
            }
            else{
                m_state = Normal;
            }

        }

        //set state functions
        void SetStateNormal(){
            m_state = Normal;
        }

        void SetStateFocused(){
            m_state = Focused;
        }

        void SetStatePressed(){
            m_state = Pressed;
        }

        void SetStateDisabled(){
            m_state = Disabled;
        }

        void SetState(GuiElementState state){
            m_state = state;
        }

        void ToggleState(){
            if(m_state == Disabled) m_state = Normal;
            else m_state = Disabled;
        }

        void GuiElementJsonFields(json &j){
            JsonFromRectangle(j,m_rect);
            j["state"] = m_state;
            j["theme"] = m_theme;
        }

        virtual void ToJson(json &j){
            json temp;
            GuiElementJsonFields(j);
            j["GuiElement"] =  temp;

        }

        void WriteDebugInfo(FILE *stream = stdout){
            fprintf(stream,"Rect: %f %f %f %f\n",m_rect.x,m_rect.y,m_rect.width,m_rect.height);
            fwrite(this, sizeof(*this), 1, stream);
        }

    };

    class TextGuiElement : public GuiElement{
    protected:
        TextSettings m_textSettings = LoadDefaultTextSettings();
    public:
        std::string m_text;

        explicit TextGuiElement(Rectangle rect, std::string &text) : GuiElement(rect){
            m_text = text;
        }

        ~TextGuiElement() override{};

        void SetAlign(TextAlign textAlign){
            m_textSettings.verticalAlign = textAlign;
            m_textSettings.horizontalAlign = textAlign;
        }

        void SetHorizontalAlign(TextAlign textAlign){
            m_textSettings.horizontalAlign = textAlign;
        }

        void SetVerticalAlign(TextAlign textAlign) {m_textSettings.verticalAlign = textAlign;}

        void SetFontSize(float size) {m_textSettings.fontSize = size;}

        [[nodiscard]] float GetFontSize() const {return m_textSettings.fontSize;}

        [[nodiscard]] const TextSettings &GetTextSettings() const {return m_textSettings;}

        void SetTextSettings(const TextSettings &mTextSettings) {m_textSettings = mTextSettings;}

        void SetFontMargin(Vector2 margin) {m_textSettings.fontMargin = margin;}

        [[nodiscard]] Vector2 GetFontMargin() const {return m_textSettings.fontMargin;}

        void TextWrap(float marginForError = 0.95f){
            //Any newlines put manually into the text are overwritten.
            //inserts newline characters at optimal locations in the text string to make it wrap.
            if(m_text.empty()) return;
            struct Word{ //Implementing a stupid list because std::list encapsulates too much for me to understand
                std::string s;
                float width = 0;
                Word *next = nullptr;
            };
            struct WordList{
                Word *head;
                Word *tail;

                void append(Word *newWord){
                    if(tail){
                        tail->next = newWord;
                        tail = newWord; //Assumes newWord has a nullptr next
                    }
                    else{
                        head = newWord;
                        tail = newWord;
                    }
                }

                void remove(Word *targetToRemove){
                    Word *prev = nullptr;
                    for(Word *it = head; it!= nullptr; it=it->next){
                        if(it==targetToRemove){
                            if(it==head){
                                head = head->next;
                            }
                            if(it==tail){
                                tail = prev;
                                if(tail) tail->next = nullptr;
                            }
                            if(prev){
                                prev->next = it->next;
                            }
                            delete it;
                            break;
                        }
                        prev = it;
                    }
                }
            };

            float maximumRowWidth = marginForError * m_rect.width * (1 - 2 * m_textSettings.fontMargin.x);
            WordList words = {nullptr, nullptr};
            int wordSize = 0;

            //Converts words into a list of strings and their widths
            for(int i = 0; i <= m_text.size(); i++){
                if(m_text[i] == '\n' || m_text[i] == ' ' || i == m_text.size()){
                    if(m_text[i] == '\n') m_text[i] = ' ';
                    char temp[wordSize+2];
                    memcpy(temp, &m_text[i - wordSize], wordSize + 1);
                    temp[wordSize+1] = 0;
                    float width = MeasureTextEx(m_theme.font, temp, m_textSettings.fontSize, m_textSettings.spacing).x;
                    Word *word = new Word;
                    word->s = temp;
                    word->width = width;
                    if(word->width != -1) {
                        //printf("Word: \"%s\" was just appended\n",word->s.c_str());
                        words.append(word);
                    }
                    wordSize = 0;
                }
                else{
                    wordSize++;
                }
            }

            //iterate through words and add to text
            for(Word *it = words.head; it->next != nullptr;){
                auto next = it->next;
                if(it->width + next->width <= maximumRowWidth){
                    it->s += next->s;
                    it->width += next->width;
                    words.remove(next);
                }
                else{
                    it->s.pop_back();
                    it->s += '\n';
                    it = it->next;
                }
            }


            m_text.clear();
            for(Word *it = words.head; it != nullptr;it = it->next){
                m_text += it->s;
            }



        }

        void DrawTextInRectangle(Rectangle rectangle,bool drawLines = false){
            Vector2 offset = {rectangle.x, rectangle.y};
            Vector2 textSize = MeasureTextEx(m_theme.font, m_text.c_str(), m_textSettings.fontSize, m_textSettings.spacing);

            switch(m_textSettings.horizontalAlign){
                case TextAlign::Start:
                    offset.x += rectangle.width * m_textSettings.fontMargin.x;
                    break;
                case TextAlign::Center:
                    offset.x += rectangle.width / 2 - textSize.x / 2;
                    break;
                case TextAlign::End:
                    offset.x += rectangle.width - textSize.x - m_textSettings.fontMargin.x * rectangle.width;
                    break;
            }
            switch(m_textSettings.verticalAlign){
                case TextAlign::Start:
                    offset.y += rectangle.height * m_textSettings.fontMargin.y;
                    break;
                case TextAlign::Center:
                    offset.y += rectangle.height / 2 - textSize.y / 2;
                    break;
                case TextAlign::End:
                    offset.y += rectangle.height - textSize.y - m_textSettings.fontMargin.y * rectangle.height;
                    break;
            }
            DrawTextEx(m_theme.font, m_text.c_str(), offset, m_textSettings.fontSize, m_textSettings.spacing, m_theme.text[m_state]);
            if(drawLines) DrawRectangleLines(offset.x,offset.y,textSize.x,textSize.y,GREEN);

        }

        void DrawTextInRectangle(bool drawLines = false){
            Vector2 offset = {m_rect.x, m_rect.y};
            Vector2 textSize = MeasureTextEx(m_theme.font, m_text.c_str(), m_textSettings.fontSize, m_textSettings.spacing);

            switch(m_textSettings.horizontalAlign){
                case TextAlign::Start:
                    offset.x += m_rect.width * m_textSettings.fontMargin.x;
                    break;
                case TextAlign::Center:
                    offset.x += m_rect.width / 2 - textSize.x / 2;
                    break;
                case TextAlign::End:
                    offset.x += m_rect.width - textSize.x - m_textSettings.fontMargin.x * m_rect.width;
                    break;
            }
            switch(m_textSettings.verticalAlign){
                case TextAlign::Start:
                    offset.y += m_rect.height * m_textSettings.fontMargin.y;
                    break;
                case TextAlign::Center:
                    offset.y += m_rect.height / 2 - textSize.y / 2;
                    break;
                case TextAlign::End:
                    offset.y += m_rect.height - textSize.y - m_textSettings.fontMargin.y * m_rect.height;
                    break;
            }
            DrawTextEx(m_theme.font, m_text.c_str(), offset, m_textSettings.fontSize, m_textSettings.spacing, m_theme.text[m_state]);
            if(drawLines) DrawRectangleLines(offset.x,offset.y,textSize.x,textSize.y,GREEN);

        }

        void FindMaxFontSize(Rectangle rectangle, float minimumFontSize = 0) {
            m_textSettings.fontSize = rectangle.height;

            Vector2 measuredSize = MeasureTextEx(m_theme.font, m_text.c_str(), m_textSettings.fontSize, m_textSettings.spacing);
            while (measuredSize.x > rectangle.width * (1 - 2 * m_textSettings.fontMargin.x) ||
                   measuredSize.y > rectangle.height * (1 - 2 * m_textSettings.fontMargin.y)) {
                m_textSettings.fontSize -= 2;
                m_textSettings.spacing = GET_SPACING(m_textSettings.fontSize);
                measuredSize = MeasureTextEx(m_theme.font, m_text.c_str(), m_textSettings.fontSize, m_textSettings.spacing);
            }
            if(m_textSettings.fontSize < minimumFontSize) {
                m_textSettings.fontSize = minimumFontSize;
                m_textSettings.spacing = GET_SPACING(m_textSettings.fontSize);
            }
        }

        void FindMaxFontSize(float minimumFontSize = 0) {
            m_textSettings.fontSize = m_rect.height;

            Vector2 measuredSize = MeasureTextEx(m_theme.font, m_text.c_str(), m_textSettings.fontSize, m_textSettings.spacing);
            while (measuredSize.x > m_rect.width * (1 - 2 * m_textSettings.fontMargin.x) ||
                   measuredSize.y > m_rect.height * (1 - 2 * m_textSettings.fontMargin.y)) {
                m_textSettings.fontSize -= 2;
                m_textSettings.spacing = GET_SPACING(m_textSettings.fontSize);
                measuredSize = MeasureTextEx(m_theme.font, m_text.c_str(), m_textSettings.fontSize, m_textSettings.spacing);
            }
            if(m_textSettings.fontSize < minimumFontSize) {
                m_textSettings.fontSize = minimumFontSize;
                m_textSettings.spacing = GET_SPACING(m_textSettings.fontSize);
            }
        }

        int FindLargestCharacterSize(bool (*filterFunction)(int) = IsAscii){
            int maxSize = 0;
            for(unsigned char key = 0; key < 255; key++){
                if(filterFunction(key)){
                    int index;
                    int codepoint = GetCodepoint((char*)&key,&index); //don't care about codepointSize but can't do null due to segfault
                    index = GetGlyphIndex(m_theme.font, codepoint);
                    if(key != '\n'){
                        int width;
                        if(m_theme.font.glyphs[index].advanceX != 0){
                            width = m_theme.font.glyphs[index].advanceX;
                        }
                        else{
                            width = m_theme.font.recs[index].width + m_theme.font.glyphs[index].offsetX;
                        }
                        maxSize = std::max(maxSize,width);
                    }
                }
            }
            return maxSize;
        }

        unsigned char FindLargestCharacter(bool (*filterFunction)(int) = IsAscii){
            int maxSize = 0;
            unsigned char biggest = 0;
            for(unsigned char key = 0; key < 255; key++){
                if(filterFunction(key)){
                    int index;
                    int codepoint = GetCodepoint((char*)&key,&index); //don't care about codepointSize but can't do null due to segfault
                    index = GetGlyphIndex(m_theme.font,codepoint);
                    if(key != '\n'){
                        int width;
                        if(m_theme.font.glyphs[index].advanceX!=0){
                            width = m_theme.font.glyphs[index].advanceX;
                        }
                        else{
                            width = m_theme.font.recs[index].width + m_theme.font.glyphs[index].offsetX;
                        }
                        if(width > maxSize){
                            maxSize = width;
                            biggest = key;
                        }
                    }
                }
            }
            return biggest;
        }

        TextSettings GetTextSettings(){
            return m_textSettings;
        }

        void SetTextSettings(TextSettings settings){
            m_textSettings = settings;
        }

        void TextGuiElementJsonFields(json &j){
            GuiElementJsonFields(j);
            j["text"] = m_text;
            j["textSettings"] = m_textSettings;
        }

        void ToJson(json &j)override{
            json temp;
            TextGuiElementJsonFields(temp);
            j["TextGuiElement"] = temp;

        }

    };

    class TextBox : public TextGuiElement{
    public:
        bool m_drawBorder = false;
        bool m_doAutoTextWrap = false;
        bool m_doAutoTextResize = true;
        bool m_wrapAtMinFontSize = true;
        bool m_hasTextChanged = true;
        bool m_stringIsFull = false;
        bool m_drawCharacterCount = false;

        int m_keyRepeatCount = 0;
        int m_lastKey = 0;
        float m_minimumFontSize = 24;
        int m_characterLimit = 0;

        bool (*m_filterFunction)(int) = nullptr;


        TextBox(Rectangle rect, std::string &text) : TextGuiElement(rect, text){
            m_filterFunction = &IsAscii;
        };

        ~TextBox() override{};

        void Update() override{
            if(m_state == Disabled) return;

            if(m_state==Pressed){
                //Handle user input
                if(!CheckCollisionPointRec(GetMousePosition(),m_rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    m_state = Normal;
                }
                else if(IsKeyPressed(KEY_ENTER) && !IsKeyDown(KEY_LEFT_SHIFT)){
                    //m_isTyping = false;
                    m_state = Normal;
                }
                else{

                    int input = GetKeyPressed();
                    if(m_filterFunction(m_lastKey)){
                        if(IsKeyDown(m_lastKey)){
                            m_keyRepeatCount++;
                        }
                        else{
                            m_keyRepeatCount = 0;
                            m_lastKey = 0;
                        }

                        if(m_keyRepeatCount>=CONTINUOUS_TYPING_DELAY && !m_filterFunction(input)){
                            input = m_lastKey;
                        }
                    }
                    else{
                        m_keyRepeatCount = 0;
                        m_lastKey = 0;
                    }

                    if(input == KEY_BACKSPACE){
                        if(IsKeyDown(KEY_LEFT_CONTROL)){
                            m_text.clear();
                        }
                        else if(!m_text.empty()) m_text.pop_back();
                        m_hasTextChanged = true;
                    }
                    else if(m_characterLimit > 0 && m_text.size() >= m_characterLimit){
                        m_text.resize(m_characterLimit);
                        return;
                    }
                    else if(MACRO_SHIFT(m_filterFunction(KEY_ENTER)) &&!m_stringIsFull){
                        m_text.push_back('\n');
                        m_hasTextChanged = true;
                    }
                    else{
                        if(m_filterFunction(input) && !m_stringIsFull){
                            int shift = 32 * !IsKeyDown(KEY_LEFT_SHIFT);
                            m_text.push_back((char)(input | shift));
                            m_lastKey = input;
                            m_hasTextChanged = true;
                        }

                    }
                }
            }
            else{
                //Check for mouse collision
                if(CheckCollisionPointRec(GetMousePosition(),m_rect)){
                    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                        m_state = Pressed;
                    }
                    else{
                        m_state = Focused;
                    }
                }
                else{
                    m_state = Normal;
                }
            }

            if(m_hasTextChanged){
                if(m_doAutoTextResize) {
                    FindMaxFontSize(m_minimumFontSize);
                    if(m_wrapAtMinFontSize && m_textSettings.fontSize <= m_minimumFontSize) m_doAutoTextWrap = true;
                }
                if(m_doAutoTextWrap) {
                    TextWrap();
                    if(m_wrapAtMinFontSize &&  m_textSettings.fontSize > m_minimumFontSize) EnableAutoTextResize();
                }
                Vector2 measuredSize = MeasureTextEx(m_theme.font, m_text.c_str(), m_textSettings.fontSize, m_textSettings.spacing);
                if(m_text.back() == ' ') {
                    //If the text ends in a space, it is ignored to prevent "too big" triggering in weird cases
                    measuredSize.x -= MeasureTextEx(m_theme.font," ",m_textSettings.fontSize,m_textSettings.spacing).x;
                }
                if(measuredSize.x > m_rect.width * (1 - 2 * m_textSettings.fontMargin.x) ||
                   measuredSize.y > m_rect.height * (1 - 2 * m_textSettings.fontMargin.y)) {
                    m_stringIsFull = true;
                    printf("too big\n");
                }
                else{
                    m_stringIsFull = false;
                }


                m_hasTextChanged = false;
            }

        }

        void Draw() override{
            if(m_drawBorder){
                DrawRectangleRec(m_rect,m_theme.line[m_state]); //outer
                DrawRectangle(m_rect.x+m_rect.width*m_textSettings.fontMargin.x,
                              m_rect.y+m_rect.height*m_textSettings.fontMargin.y,
                              m_rect.width*(1- 2 * m_textSettings.fontMargin.x),
                              m_rect.height*(1- 2 * m_textSettings.fontMargin.y),
                              m_theme.base[m_state]);
                }
            else{
                DrawRectangleRec(m_rect, m_theme.base[m_state]);
                DrawRectangleLinesEx(m_rect, m_theme.lineWidth, m_theme.line[m_state]);
            }
            DrawTextInRectangle();

            if(m_drawCharacterCount){
                const char* buffer;
                if(m_characterLimit == 0 ) buffer = TextFormat("%d",m_text.size());
                else buffer = TextFormat("%d/%d",m_text.size(),m_characterLimit);

                RTK::DrawTextInRectangle(buffer,
                                         {m_rect.x + m_rect.width * 0.9f, m_rect.y + m_rect.height * 0.9f,  m_rect.width * 0.1f,m_rect.height * 0.1f},
                                         m_theme,{TextAlign::End,TextAlign::End,m_rect.height/10,{0.25,0.25},GET_SPACING(m_rect.height/10)},
                                         m_state);
            }
        }

        void PrintDebugInfo(FILE *stream = stdout, bool showTextSettings = false, bool showConfigurableBool = false, bool showLastKeyInfo = false){
            fprintf(stream,"Text: \"%s\"\n",m_text.c_str());
            Vector2 measuredSize = MeasureTextEx(m_theme.font, m_text.c_str(), m_textSettings.fontSize, m_textSettings.spacing);
            fprintf(stream,"Measured Size: %f %f\n",measuredSize.x,measuredSize.y);
            fprintf(stream,"Rect: %f %f %f %f\n",m_rect.x,m_rect.y,m_rect.width,m_rect.height);
            switch (m_state) {
                case Normal:
                    fprintf(stream,"State: Normal\n");
                    break;
                case Focused:
                    fprintf(stream,"State: Focused\n");
                    break;
                case Pressed:
                    fprintf(stream,"State: Pressed\n");
                    break;
                case Disabled:
                    fprintf(stream,"State: Disabled\n");
                    break;
            }
            fprintf(stream,"StringIsFull: %d\n",m_stringIsFull);
            if(showConfigurableBool){
                fprintf(stream,"DrawBorder: %d\n",m_drawBorder);
                fprintf(stream,"DoAutoTextWrap: %d\n",m_doAutoTextWrap);
                fprintf(stream,"DoAutoTextResize: %d\n",m_doAutoTextResize);
                fprintf(stream,"WrapAtMinFontSize: %d\n",m_wrapAtMinFontSize);
                fprintf(stream,"MinimumFontSize: %f\n",m_minimumFontSize);
            }
            if(showTextSettings){
                fprintf(stream,"FontSize: %f\n",m_textSettings.fontSize);
                switch(m_textSettings.verticalAlign){
                    case TextAlign::Start:
                        fprintf(stream,"Vertical Align: Start\n");
                        break;
                    case TextAlign::Center:
                        fprintf(stream,"Vertical Align: Center\n");
                        break;
                    case TextAlign::End:
                        fprintf(stream,"Vertical Align: End\n");
                        break;
                }
                switch(m_textSettings.horizontalAlign){
                    case TextAlign::Start:
                        fprintf(stream,"Horizontal Align: Start\n");
                        break;
                    case TextAlign::Center:
                        fprintf(stream,"Horizontal Align: Center\n");
                        break;
                    case TextAlign::End:
                        fprintf(stream,"Horizontal Align: End\n");
                        break;
                }
                fprintf(stream,"FontMargin: %f %f\n",m_textSettings.fontMargin.x,m_textSettings.fontMargin.y);
                fprintf(stream,"Spacing: %f\n",m_textSettings.spacing);
            }
            if(showLastKeyInfo){
                fprintf(stream,"KeyRepeatCount: %d\n",m_keyRepeatCount);
                fprintf(stream,"LastKey: %d\n",m_lastKey);
            }

        }

        void EnableAutoTextWrap(){
            m_doAutoTextResize = false;
            m_doAutoTextWrap = true;
        }

        void EnableAutoTextResize(){
            m_doAutoTextWrap = false;
            m_doAutoTextResize = true;
        }

        void DisableAutoTextWrap(){
            m_doAutoTextWrap = false;
        }

        void DisableAutoTextResize(){
            m_doAutoTextResize = false;
        }

        void SetMinimumFontSize(float minimumFontSize){
            m_minimumFontSize = minimumFontSize;
        }

        void EnableWrapAtMinimumFontSize(){
            m_wrapAtMinFontSize = true;
        }

        void DisableWrapAtMinimumFontSize(){
            m_wrapAtMinFontSize = false;
        }

        void EnableDrawBorder(){
            m_drawBorder = true;
        }

        void DisableDrawBorder(){
            m_drawBorder = false;
        }

        bool IsTyping(){
            return false;
        }

        void SetText(std::string &text){
            m_text = text;
        }

        void SetTextLiteral(char *text){
            m_text = text;
        }

        void SetFilterFunction(bool (*function)(int)){
            m_filterFunction = function;
        }

        auto GetFilterFunction(){
            return m_filterFunction;
        }

        void SetCharacterLimit(int limit){
            m_characterLimit = limit;
        }

        int GetCharacterLimit(){
            return m_characterLimit;
        }

        void EnableDrawCharacterCount(){
            m_drawCharacterCount = true;
        }

        void DisableDrawCharacterCount(){
            m_drawCharacterCount = false;
        }

        void FixedTextSize(){
            DisableWrapAtMinimumFontSize();
            DisableAutoTextResize();
            DisableAutoTextWrap();
            if(m_characterLimit == 0){
                FindMaxFontSize();
            }
            else{
                auto buffer = m_text;
                char big = FindLargestCharacter(m_filterFunction);
                m_text.clear();
                for(int i = 0; i < m_characterLimit; i++) m_text += big;
                FindMaxFontSize();
                m_text = buffer;
            }
        }

        void TextBoxJsonFields(json &j){
            TextGuiElementJsonFields(j);
            j["drawBorder"] = m_drawBorder;
            j["doAutoTextWrap"] = m_doAutoTextWrap;
            j["doAutoTextResize"] = m_doAutoTextResize;
            j["wrapAtMinFontSize"] = m_wrapAtMinFontSize;
            j["hasTextChanged"] = m_hasTextChanged;
            j["stringIsFull"] = m_stringIsFull;
            j["drawCharacterCount"] = m_drawCharacterCount;
            j["keyRepeatCount"] = m_keyRepeatCount;
            j["lastKey"] = m_lastKey;
            j["minimumFontSize"] = m_minimumFontSize;
            j["characterLimit"] = m_characterLimit;
        }

        void ToJson(json &j)override{
            json temp;
            TextBoxJsonFields(temp);
            j["TextBox"] = temp;
        }
    };

    class CheckBox : public GuiElement{
        bool m_isChecked = false;
    public:

        CheckBox(Rectangle rect): GuiElement(rect){};

        ~CheckBox() override{};

        void Draw() override{
            DrawRectangleRec(m_rect,m_theme.base[m_state]);
            DrawRectangleLinesEx(m_rect,m_theme.lineWidth,m_theme.line[m_state]);
        }

        void Update() override{
            bool hover = false;
            if(CheckCollisionPointRec(GetMousePosition(),m_rect)){
                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    Toggle();
                }
                else{
                    hover = true;
                }
            }
            if(m_isChecked) m_state = Pressed;
            else if(hover) m_state = Focused;
            else m_state = Normal;
        }

        void Toggle(){
            m_isChecked = !m_isChecked;
        }

        bool IsChecked(){
            return m_isChecked;
        }

        void SetChecked(bool checked){
            m_isChecked = checked;
        }

        void CheckBoxJsonFields(json &j){
            GuiElementJsonFields(j);
            j["isChecked"] = m_isChecked;
        }

        void ToJson(json &j){
            json temp;
            CheckBoxJsonFields(temp);
            j["CheckBox"] = temp;
        }
    };

    class Button : public TextGuiElement{

        void (*m_function)() = nullptr;

    public:

        Button(Rectangle rect, std::string text, void (*function)()) : TextGuiElement(rect, text){
            m_function = function;
            FindMaxFontSize();
        }

        void Draw() override{
            DrawRectangleRec(m_rect,m_theme.base[m_state]);
            DrawRectangleLinesEx(m_rect,m_theme.lineWidth,m_theme.line[m_state]);
            if(!m_text.empty()) DrawTextInRectangle();
        }
        void Update() override{
            if(m_state == Pressed && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) m_function();
            if(CheckCollisionPointRec(GetMousePosition(),m_rect)){
                if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
                    m_state = Pressed;
                }
                else{
                    m_state = Focused;
                }

            }
            else m_state = Normal;
        }

        void CallFunction(){
            m_function();
        }

        void SetFunction(void (*function)()){
            m_function = function;
        }

        void ButtonJsonFields(json &j){
            TextGuiElementJsonFields(j);
        }

        void ToJson(json &j){
            json temp;
            ButtonJsonFields(temp);
            j["Button"] = temp;
        }


    };

    template <typename T, typename A>
    class ButtonPro : public TextGuiElement{

        T (*m_function)(A) = nullptr;
        A m_defaultArgument;

    public:

        ButtonPro(Rectangle rect, std::string text, T (*function)(A), A defaultArgument) : TextGuiElement(rect, text){
            m_defaultArgument = defaultArgument;
            m_function = function;
            FindMaxFontSize();
        }

        void Draw() override{
            DrawRectangleRec(m_rect,m_theme.base[m_state]);
            DrawRectangleLinesEx(m_rect,m_theme.lineWidth,m_theme.line[m_state]);
            if(!m_text.empty()) DrawTextInRectangle();
        }
        void Update() override{
            if(m_state == Pressed && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) m_function(m_defaultArgument);
            if(CheckCollisionPointRec(GetMousePosition(),m_rect)){
                if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
                    m_state = Pressed;
                }
                else{
                    m_state = Focused;
                }

            }
            else m_state = Normal;
        }

        void SetDefaultArgs(A arg){
            m_defaultArgument = arg;
        }

        T CallFunction(){
            return m_function(m_defaultArgument);
        }

        T CallFunction(A arg){
            return m_function(arg);
        }

        void SetFunction(T (*function)(A)){
            m_function = function;
        }

        void ButtonProJsonFields(json &j){
            TextGuiElementJsonFields(j);
            j["defaultArgument"] = m_defaultArgument;
        }

        void ToJson(json &j){
            json temp;
            ButtonProJsonFields(temp);
            j["ButtonPro"] = temp;
        }


    };

    class ButtonPoll : public TextGuiElement{

        bool m_hasBeenpressed = false;

    public:

        ButtonPoll(Rectangle rect, std::string text) : TextGuiElement(rect, text){
            FindMaxFontSize();
        }

        void Draw() override{
            DrawRectangleRec(m_rect,m_theme.base[m_state]);
            DrawRectangleLinesEx(m_rect,m_theme.lineWidth,m_theme.line[m_state]);
            if(!m_text.empty()) DrawTextInRectangle();
        }
        void Update() override{
            if(CheckCollisionPointRec(GetMousePosition(),m_rect)){
                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    m_state = Pressed;
                    m_hasBeenpressed = true;
                }
                else{
                    m_state = Focused;
                }

            }
            else m_state = Normal;
            return;
            if(m_state == Pressed && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) m_hasBeenpressed = true;
            if(CheckCollisionPointRec(GetMousePosition(),m_rect)){
                if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
                    m_state = Pressed;
                }
                else{
                    m_state = Focused;
                }

            }
            else m_state = Normal;
        }

        [[nodiscard]] bool Poll(){
            bool temp = m_hasBeenpressed;
            m_hasBeenpressed = false;
            return temp;
        }

        void ButtonPollJsonFields(json &j){
            TextGuiElementJsonFields(j);
            j["hasBeenPressed"] = m_hasBeenpressed;
        }

        void ToJson(json &j){
            json temp;
            ButtonPollJsonFields(temp);
            j["ButtonPoll"] = temp;
        }

    };
    GuiElementState ToggleGuiElementState(GuiElement *element){
        element->ToggleState();
        return element->GetState();
    }

    class Window : public TextGuiElement{
    protected:
        std::vector<GuiElement*> m_elements;
        bool m_drawWindow = false;
        float m_headerSize = 0.075f;


    public:

        Window(Rectangle rect, std::string &text) : TextGuiElement(rect, text) {
            SetHorizontalAlign(TextAlign::Start);
            SetVerticalAlign(TextAlign::Center);
            SetFontMargin({0.05,0.05});
            FindMaxFontSize(GetHeaderRectangle());
        }

        ~Window(){
            for(auto e : m_elements){
                delete e;
            }
        }

        void Draw(){
            if(m_drawWindow){
                DrawRectangleRec(m_rect,m_theme.background);
                DrawRectangle(m_rect.x,m_rect.y,+m_rect.width,m_rect.height*m_headerSize,m_theme.base[m_state]);
                DrawRectangleLinesEx(m_rect,m_theme.lineWidth,m_theme.line[m_state]);
                DrawTextInRectangle(GetHeaderRectangle());
            }
            for(auto e : m_elements){
                e->Draw();
            }
        }

        void Update() override{
            for(auto e : m_elements){
                e->Update();
            }
        }

        void AddElement(GuiElement *element){
            m_elements.push_back(element);
            element->ShiftRect({m_rect.x,m_rect.y});
        }

        void RemoveElement(GuiElement *element){
            for(auto it = m_elements.begin(); it != m_elements.end(); it++){
                if(*it == element){
                    m_elements.erase(it);
                    break;
                }
            }
        }

        void EnableDrawWindow(){
            m_drawWindow = true;
        }

        void DisableDrawWindow(){
            m_drawWindow = false;
        }

        Rectangle GetHeaderRectangle(){
            return {m_rect.x,m_rect.y,+m_rect.width,m_rect.height*m_headerSize};
        }

        float GetHeaderOffset(){
            return m_rect.height*m_headerSize;
        }

        void ShiftRect(Vector2 translation) override{
            m_rect = {m_rect.x + translation.x, m_rect.y + translation.y, m_rect.width, m_rect.height};

            for(auto e : m_elements){
                e->ShiftRect(translation);
            }
        }

        void UpdateSizes(){
            FindMaxFontSize(GetHeaderRectangle());

        }

        void SetHeaderSize(float size){
            m_headerSize = size;
            UpdateSizes();
        }

        void WriteDebugInfo(FILE *stream = stdout){
            fprintf(stream,"Window: %s\n",m_text.c_str());
            fwrite(this, sizeof(*this), 1, stream);
            for(auto e : m_elements){
                e->WriteDebugInfo(stream);
            }
        }

        void WindowJsonFields(json &j){
            TextGuiElementJsonFields(j);
            j["headerSize"] = m_headerSize;
            j["drawWindow"] = m_drawWindow;
            json elements;
            for(auto e : m_elements){
                json temp;
                e->ToJson(temp);
                elements.push_back(temp);
            }
            j["elements"] = elements;
        }

        void ToJson(json &j)override{
            json temp;
            WindowJsonFields(temp);
            j["Window"] = temp;
        }

    };

    class DynamicWindow : public Window{
        bool m_enableButtons = false;
        ButtonPoll m_delete = {{0,0,0,0},""};
        ButtonPoll m_minimize = {{0,0,0,0},""};
    public:
        explicit DynamicWindow(Rectangle rect, std::string &name) : Window(rect, name) {
            m_delete = ButtonPoll({m_rect.x + m_rect.width * 0.875f, m_rect.y,m_rect.width * 0.125f, m_headerSize * m_rect.height},"X");
            m_minimize= ButtonPoll({m_rect.x + m_rect.width * 0.75f, m_rect.y,m_rect.width * 0.125f, m_headerSize * m_rect.height},"_");
            m_drawWindow = true;
        }

        void EnableButtons(){
            m_enableButtons = true;
        }

        Rectangle GetHeaderRectangle(){
            if(m_enableButtons){
                return {m_rect.x,m_rect.y,0.75f*m_rect.width,m_rect.height*m_headerSize};
            }
            return {m_rect.x,m_rect.y,+m_rect.width,m_rect.height*m_headerSize};
        }

        void Draw() override{
            if(m_drawWindow){
                DrawRectangleRec(m_rect,m_theme.background);
                DrawRectangle(m_rect.x,m_rect.y,+m_rect.width,m_rect.height*m_headerSize,m_theme.base[m_state]);
                DrawRectangleLinesEx(m_rect,m_theme.lineWidth,m_theme.line[m_state]);
                DrawTextInRectangle(GetHeaderRectangle());
                if(m_enableButtons){
                    m_delete.Draw();
                    m_minimize.Draw();
                }
            }
            for(auto e : m_elements){
                e->Draw();
            }
        }

        void Update() override{
            if(m_state == Disabled) return;

            m_delete.Update();
            m_minimize.Update();

            MouseDetection(GetHeaderRectangle());
            Vector2 shift = GetMouseDelta();

            if(m_state == Pressed){
                m_rect = {m_rect.x + shift.x, m_rect.y + shift.y, m_rect.width, m_rect.height};
                m_delete.ShiftRect(shift);
                m_minimize.ShiftRect(shift);
            }
            for(auto e : m_elements){
                if(m_state == Pressed){
                    e->ShiftRect(shift);
                }
                e->Update();
            }
        }

        bool PollDelete(){
            return m_delete.Poll();
        }

        bool PollMinimize(){
            return m_minimize.Poll();
        }

        void UpdateSizes(){
            FindMaxFontSize(GetHeaderRectangle());
            m_delete.SetRect({m_rect.x + m_rect.width * 0.875f, m_rect.y,m_rect.width * 0.125f, m_headerSize * m_rect.height});
            m_minimize.SetRect({m_rect.x + m_rect.width * 0.75f, m_rect.y,m_rect.width * 0.125f, m_headerSize * m_rect.height});
            m_delete.FindMaxFontSize();
            m_minimize.FindMaxFontSize();
        }

        void SetHeaderSize(float size){
            m_headerSize = size;
            UpdateSizes();
        }

        void DynamicWindowJsonFields(json &j){
            WindowJsonFields(j);
            j["enableButtons"] = m_enableButtons;
            json deleteButton;
            m_delete.ToJson(deleteButton);
            j["deleteButton"] = deleteButton;
            json minimizeButton;
            m_minimize.ToJson(minimizeButton);
            j["minimizeButton"] = minimizeButton;
        }

        void ToJson(json &j)override{
            json temp;
            DynamicWindowJsonFields(temp);
            j["DynamicWindow"] = temp;
        }

    };

    class WindowManager : public GuiElement {
    private:
        struct WindowModule{
            DynamicWindow *window;
            ButtonPoll *button;
        };


        std::vector<WindowModule> m_windows;
        float m_footerSize = 0.05f;
        int m_maxWindows = 10;
    public:
        WindowManager(Rectangle rectangle, float footerSize, int maxWindows) : GuiElement(rectangle){
            m_footerSize = footerSize;
            m_maxWindows = maxWindows;
        }

        ~WindowManager() override{
            for(auto m : m_windows){
                delete m.window;
                delete m.button;
            }
        }

        void Draw() override{
            if(m_state==Disabled)return;
            for(auto m_window : m_windows){

                Rectangle r = m_window.window->GetRect();

                switch(m_window.window->GetState()){
                    case Disabled:
                        break;
                    case Normal:
                    case Focused:
                    case Pressed:
                        m_window.window->Draw();
                        break;
                }

                m_window.button->Draw();
            }
            EndTextureMode();

            DrawRectangleLinesEx(m_rect, m_theme.lineWidth, m_theme.line[Normal]);

        }

        void Update() override{
            if(m_state==Disabled)return;
            for (auto it = m_windows.begin(); it != m_windows.end(); ) {

                it->window->Update();
                if (it->window->PollMinimize()) {
                    it->window->Disable();
                    it->button->SetState(GuiElementState::Focused);
                }
                it->button->Update();
                if (it->button->Poll()) {
                    it->window->ToggleState();
                }
                if (it->window->PollDelete()) {
                    // Properly delete the element and advance the iterator
                    it = m_windows.erase(it);
                } else {
                    ++it;
                }
            }
        }



        bool AddWindow(DynamicWindow *window){
            if(m_windows.size() == m_maxWindows) return false;
            window->EnableButtons();
            auto size = window->GetSize();
            window->ShiftRect({m_rect.x, m_rect.y});
            Rectangle buttonRec = {m_windows.size() * (m_rect.width / m_maxWindows), (1 - m_footerSize) * m_rect.height, (m_rect.width / m_maxWindows), m_rect.height * m_footerSize};
            auto but = new ButtonPoll(buttonRec,window->m_text);
            m_windows.push_back({window,but});
            return true;
        }


        void WriteDebugInfo(FILE *stream = stdout){
            fprintf(stream,"Window Manager\n");
            fwrite(this, sizeof(*this), 1, stream);
            for(auto m : m_windows){
                m.window->WriteDebugInfo(stream);
            }
        }

        void WindowManagerJsonFields(json &j){
            GuiElementJsonFields(j);
            j["footerSize"] = m_footerSize;
            j["maxWindows"] = m_maxWindows;
            json windows;
            for(auto m : m_windows){
                json temp;
                m.window->ToJson(temp);
                windows.push_back(temp);
            }
            j["windows"] = windows;
        }

        void ToJson(json &j)override{
            json temp;
            WindowManagerJsonFields(temp);
            j["WindowManager"] = temp;
        }


    };

    class Dropdown : public TextGuiElement{
        struct ButtonNode{
            ButtonPoll *button;
            ButtonNode *next;
        };
        struct ButtonList{
            ButtonNode *head;
            ButtonNode *tail;
        };

    public:
        ButtonList m_options;
        bool m_isExpanded = false;
        int m_maxOptions = 0;
        bool m_enableScrollBar = false;
        bool m_enableScrollBarWhenFull = false; //TODO: Implement scroll bar


        Dropdown(Rectangle rect, std::string text) : TextGuiElement(rect, text){
            FindMaxFontSize();
            m_options.head = nullptr;
            m_options.tail = nullptr;
        }

        ~Dropdown() override{
            while(m_options.head){
                delete m_options.head->button;
                auto temp = m_options.head;
                m_options.head = m_options.head->next;
                delete temp;
            }
        }

        void Draw() override{
            if(m_isExpanded){
                for(ButtonNode *node = m_options.head; node != nullptr; node = node->next){
                    node->button->Draw();
                }
            }
            else{
                if(m_options.head) m_options.head->button->Draw();
            }

        }

        void ShiftRect(Vector2 translation) override{
            m_rect = {m_rect.x + translation.x, m_rect.y + translation.y, m_rect.width, m_rect.height};

            for(ButtonNode *node = m_options.head; node != nullptr; node = node->next){
                node->button->ShiftRect(translation);
            }
        }

        void OrderOptions(){
            Rectangle rectangle = m_rect;
            for(ButtonNode *node = m_options.head; node != nullptr; node = node->next){
                node->button->SetRect(rectangle);
                rectangle.y += rectangle.height;

            }
        }

        void AddOption(std::string &optionName){
            Rectangle rectangle = m_rect;
            int size = 0;
            for(ButtonNode *node = m_options.head; node != nullptr; node = node->next){size++;}
            if(m_maxOptions != 0 && size >= m_maxOptions) return;
            rectangle.y += rectangle.height * size;
            auto but = new ButtonPoll(rectangle,optionName);
            auto node = new ButtonNode;
            node->button = but;
            node->next = nullptr;
            if(!m_options.head) m_options.head = node;
            if(m_options.tail) m_options.tail->next = node;
            m_options.tail = node;
        }

        void Update() override{
            MouseDetection();

            if(m_isExpanded){
                ButtonNode *prev = nullptr;
                for(ButtonNode *node = m_options.head; node != nullptr; node = node->next){
                    node->button->Update();
                    if(node->button->Poll()){
                        if(prev) prev->next = node->next;
                        if(node!=m_options.head) {
                            node->next = m_options.head;
                            m_options.head = node;
                        }
                        OrderOptions();
                        m_isExpanded = false;
                        break;
                    }
                    prev = node;
                }
            }
            else{
                if(m_options.head) {
                    m_options.head->button->Update();
                    if(m_options.head->button->Poll()){
                        m_isExpanded = true;
                    }
                }

            }


        }

        void WriteDebugInfo(FILE *stream = stdout){
            fwrite(this,sizeof(Dropdown),1,stream);
            for(ButtonNode *node = m_options.head; node != nullptr; node = node->next){
                node->button->WriteDebugInfo(stream);
            }
        }

        void DropdownJsonFields(json &j){
            TextGuiElementJsonFields(j);
            j["isExpanded"] = m_isExpanded;
            j["maxOptions"] = m_maxOptions;
            j["enableScrollBar"] = m_enableScrollBar;
            j["enableScrollBarWhenFull"] = m_enableScrollBarWhenFull;
            json options;
            for(ButtonNode *node = m_options.head; node != nullptr; node = node->next){
                json temp;
                node->button->ToJson(temp);
                options.push_back(temp);
            }
            j["options"] = options;
        }

        void ToJson(json &j)override{
            json temp;
            DropdownJsonFields(temp);
            j["Dropdown"] = temp;
        }



    };



    class RTKRuntime{
    public:
        //Not necessary, but simplifies the process and allows for easy use of json files

        //static std::unordered_map<std::string,GuiElement*(*)(const json&)> m_constructorMap;
        std::vector<GuiElement*> m_elements;
        std::unordered_map<std::string,std::fstream> m_files;
        json m_json;


        RTKRuntime(){
           // if(m_constructorMap.empty()) InitializeConstructorMap();
        }

    private:
        void InitializeConstructorMap(){
            //m_constructorMap["GuiElement"] = [](const json &j)->GuiElement*{ return new GuiElement(j["rectangle"],j["theme"]); };
        }

    public:



        ~RTKRuntime(){
            for(auto &f : m_files){
                f.second.close();
            }
        }


        void AddElement(GuiElement *element){
            m_elements.push_back(element);
        }

        void RegisterFile(const std::string &path, const std::string &alias){
            m_files.insert({alias,std::fstream(path)});
        }

        void CloseFiles(){
            for(auto &f : m_files){
                f.second.close();
            }
        }

        void CloseFile(const std::string &alias){
            m_files[alias].close();
        }

        void LoadJson(const std::string &alias){
            //Json to guielements
            m_files[alias] >> m_json;
            FromJson();
        }

        void FromJson(){
            m_json = m_json["RTKRuntime"];

            for(auto &element : m_json.items()){
                //m_elements.push_back(m_constructorMap[element.key()](element.value()));
            }
        }

        void SaveJson(const std::string &alias){
            //GuiElements to json
            ToJson();
            m_files[alias] << m_json.dump(4);
        }

        void ToJson(){
            m_json.clear();
            json temp;
            for(auto &e: m_elements){
                e->ToJson(temp);
            }
            m_json["RTKRuntime"]["elements"] = temp;
        }



        void SaveJsonEverywhere(){
            ToJson();
            for(auto &f : m_files){
                f.second << m_json.dump(4);
            }
        }

        void Update(){
            for(auto &e: m_elements) e->Update();
        }

        void Draw(){
            for(auto &e: m_elements) e->Draw();
        }




    };

// TODO: Implement the following elements:
// Separate classes into different files, ideally header and implementation files.
/*
    Dropdown
    Hamburger
    Tooltip
    Carousel
    Slot machine picker
    Popup
    Radio button / lever / toggle
    Slider
    Incrementer/stepper
 */

}





#endif //RTK_RTK_H
