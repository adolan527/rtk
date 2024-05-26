//
// Created by Aweso on 5/19/2024.
//

#ifndef RTK_RTK_H
#define RTK_RTK_H

#include "raylib.h"
#include <string>
#include <list>

namespace RTK{
#define SPACING 16 //Based off of default parameters in DrawText()
#define GET_SPACING(_size) (_size < SPACING ? 1 : _size/SPACING)

#define MACRO_CTRL(_key) (IsKeyPressed(_key) && IsKeyDown(KEY_LEFT_CONTROL))
#define MACRO_SHIFT(_key) (IsKeyPressed(_key) && IsKeyDown(KEY_LEFT_SHIFT))
#define MACRO_ALT(_key) (IsKeyPressed(_key) && IsKeyDown(KEY_LEFT_ALT))


#define CONTINUOUS_TYPING_DELAY 35 // The amount of frames of holding down a key before it is spammed
#define ISASCIICHAR(_key) ((_key >= 32 && _key <= 96) || (_key >= 256 && _key <= 265))
    //Defines the range of all ascii characters + SPACE, BS, TAB, ENTER, ESC, DEL, and arrows



    typedef enum{
        Normal,
        Focused,
        Pressed,
        Disabled
    }GuiElementState;

    typedef enum {
        TA_Start,
        TA_Center,
        TA_End
    }TextAlign;
#define RTK_STATES_COUNT 4

    typedef struct{
        Color line[RTK_STATES_COUNT];
        Color text[RTK_STATES_COUNT];
        Color base[RTK_STATES_COUNT];
        Color background;
        float lineWidth;

        Font font;
    }Theme;

    typedef struct{
        TextAlign horizontalAlign;
        TextAlign verticalAlign;
        float fontSize;
        Vector2 fontMargin; // percentage based. 200x200 rectangle with fontmargin 0.2,0.1, will have borders of
                            // 40px and 20px. Domain of (0,0.5).
        float spacing;
    }TextSettings;


Theme LoadDefaultTheme(){
    Theme theme;
    theme.font = GetFontDefault();
    theme.line[Normal] = BLACK;
    theme.line[Focused] = ColorAlpha(BLACK,0.9f);
    theme.line[Pressed] = ColorTint(BLACK,GREEN);
    theme.line[Disabled] = DARKGRAY;

    theme.text[Normal] = BLACK;
    theme.text[Focused] = ColorAlpha(BLACK,0.9f);
    theme.text[Pressed] = ColorTint(BLACK,GREEN);
    theme.text[Disabled] = DARKGRAY;

    theme.base[Normal] = WHITE;
    theme.base[Focused] = LIGHTGRAY;
    theme.base[Pressed] = ColorTint(WHITE,GREEN);
    theme.base[Disabled] = GRAY;

    theme.background = {220,220,220,255};

    theme.lineWidth = 5;

    return theme;
}

TextSettings LoadDefaultTextSettings(){
    TextSettings settings = {
            .horizontalAlign = TA_Center,
            .verticalAlign = TA_Center,
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

    void DrawTextInRectangle(const char *text, Rectangle rectangle, Theme theme, TextSettings textSettings, GuiElementState state = Normal){
        Vector2 offset = {rectangle.x, rectangle.y};
        Vector2 textSize = MeasureTextEx(theme.font, text, textSettings.fontSize, textSettings.spacing);

        switch(textSettings.horizontalAlign){
            case TA_Start:
                offset.x += rectangle.width * textSettings.fontMargin.x;
                break;
            case TA_Center:
                offset.x +=  rectangle.width / 2 - textSize.x / 2;
                break;
            case TA_End:
                offset.x += rectangle.width - textSize.x - textSettings.fontMargin.x * rectangle.width;
                break;
        }
        switch(textSettings.verticalAlign){
            case TA_Start:
                offset.y += rectangle.height * textSettings.fontMargin.y;
                break;
            case TA_Center:
                offset.y += rectangle.height / 2 - textSize.y / 2;
                break;
            case TA_End:
                offset.y += rectangle.height - textSize.y - textSettings.fontMargin.y * rectangle.height;
                break;
        }
        DrawTextEx(theme.font, text, offset, textSettings.fontSize, textSettings.spacing, theme.text[state]);
        DrawRectangleLines(offset.x,offset.y,textSize.x,textSize.y,GREEN);

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
        GuiElement(Rectangle rect = {0,0,1000,1000}, Theme theme = LoadDefaultTheme(), GuiElementState state = Normal){
            m_theme = theme;
            m_state = state;
            m_rect = rect;
        }

        virtual ~GuiElement(){};

        virtual void Draw() = 0;
        virtual void Update() = 0;


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

        void ShiftRect(Vector2 translation){
            m_rect = {m_rect.x + translation.x, m_rect.y + translation.y, m_rect.width, m_rect.height};
        }

        void ScaleRect(Rectangle window){
            m_rect = {window.x + m_rect.x / window.width, window.y + m_rect.y / window.height, m_rect.width / window.width, m_rect.height / window.height};
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
                case TA_Start:
                    offset.x += rectangle.width * m_textSettings.fontMargin.x;
                    break;
                case TA_Center:
                    offset.x += rectangle.width / 2 - textSize.x / 2;
                    break;
                case TA_End:
                    offset.x += rectangle.width - textSize.x - m_textSettings.fontMargin.x * rectangle.width;
                    break;
            }
            switch(m_textSettings.verticalAlign){
                case TA_Start:
                    offset.y += rectangle.height * m_textSettings.fontMargin.y;
                    break;
                case TA_Center:
                    offset.y += rectangle.height / 2 - textSize.y / 2;
                    break;
                case TA_End:
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
                case TA_Start:
                    offset.x += m_rect.width * m_textSettings.fontMargin.x;
                    break;
                case TA_Center:
                    offset.x += m_rect.width / 2 - textSize.x / 2;
                    break;
                case TA_End:
                    offset.x += m_rect.width - textSize.x - m_textSettings.fontMargin.x * m_rect.width;
                    break;
            }
            switch(m_textSettings.verticalAlign){
                case TA_Start:
                    offset.y += m_rect.height * m_textSettings.fontMargin.y;
                    break;
                case TA_Center:
                    offset.y += m_rect.height / 2 - textSize.y / 2;
                    break;
                case TA_End:
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

    };

    class TextBox : public TextGuiElement{
    public:
        bool m_drawBorder = false;
        //bool m_isTyping = false;
        bool m_doAutoTextWrap = false;
        bool m_doAutoTextResize = true;
        bool m_wrapAtMinFontSize = true;
        bool m_hasTextChanged = true;
        bool m_stringIsFull = false;

        int m_keyRepeatCount = 0;
        int m_lastKey = 0;
        float m_minimumFontSize = 24;

        TextBox(Rectangle rect, std::string &text) : TextGuiElement(rect, text){};

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
                    if(ISASCIICHAR(m_lastKey)){
                        if(IsKeyDown(m_lastKey)){
                            m_keyRepeatCount++;
                        }
                        else{
                            m_keyRepeatCount = 0;
                            m_lastKey = 0;
                        }

                        if(m_keyRepeatCount>=CONTINUOUS_TYPING_DELAY && !ISASCIICHAR(input)){
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
                    else if(MACRO_SHIFT(KEY_ENTER) &&!m_stringIsFull){
                        m_text.push_back('\n');
                        m_hasTextChanged = true;
                    }
                    else{
                        if(ISASCIICHAR(input) && !m_stringIsFull){
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
                    case TA_Start:
                        fprintf(stream,"Vertical Align: Start\n");
                        break;
                    case TA_Center:
                        fprintf(stream,"Vertical Align: Center\n");
                        break;
                    case TA_End:
                        fprintf(stream,"Vertical Align: End\n");
                        break;
                }
                switch(m_textSettings.horizontalAlign){
                    case TA_Start:
                        fprintf(stream,"Horizontal Align: Start\n");
                        break;
                    case TA_Center:
                        fprintf(stream,"Horizontal Align: Center\n");
                        break;
                    case TA_End:
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

        bool Poll(){
            bool temp = m_hasBeenpressed;
            m_hasBeenpressed = false;
            return temp;
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
            SetHorizontalAlign(TA_Start);
            SetVerticalAlign(TA_Center);
            SetFontMargin({0.01,0.15});
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

        void ShiftRect(Vector2 translation) {
            m_rect = {m_rect.x + translation.x, m_rect.y + translation.y, m_rect.width, m_rect.height};

            for(auto e : m_elements){
                e->ShiftRect(translation);
            }

        }

    };

    class DynamicWindow : public Window{
        bool m_enableButtons = false;
        ButtonPoll m_delete = {{0,0,0,0},""};
        ButtonPoll m_minimize = {{0,0,0,0},""};
    public:
        explicit DynamicWindow(Rectangle rect, std::string &name) : Window(rect, name) {
            m_delete = ButtonPoll({m_rect.x + m_rect.width * 0.875f, m_rect.y,m_rect.width * 0.125f, m_headerSize * m_rect.height},"Close");
            m_minimize= ButtonPoll({m_rect.x + m_rect.width * 0.75f, m_rect.y,m_rect.width * 0.125f, m_headerSize * m_rect.height},"Minimize");
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

            m_state = MouseDetection(GetHeaderRectangle());
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


    };

    class WindowManager {
    private:
        struct WindowModule{
            DynamicWindow *window;
            ButtonPoll *button;


        };


        std::vector<WindowModule> m_windows;
        Theme m_theme;
        Rectangle m_screenRect = {0,0,1920,1080};
        float m_footerSize = 0.05f;
        int m_maxWindows = 10;
    public:
        WindowManager(Rectangle rectangle, float footerSize, int maxWindows){
            m_theme = LoadDefaultTheme();
            m_screenRect = rectangle;
            m_footerSize = footerSize;
            m_maxWindows = maxWindows;
        }

        ~WindowManager(){
            for(auto m : m_windows){
                delete m.window;
                delete m.button;
            }
        }

        void Draw(){

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

            DrawRectangleLinesEx(m_screenRect,m_theme.lineWidth,m_theme.line[Normal]);

        }

        void Update() {
            for (auto it = m_windows.begin(); it != m_windows.end(); ) {
                it->window->Update();
                if (it->window->PollMinimize()) {
                    it->window->Disable();
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
            window->ShiftRect({m_screenRect.x,m_screenRect.y});
            Rectangle buttonRec = {m_windows.size() * (m_screenRect.width/m_maxWindows), (1-m_footerSize) * m_screenRect.height, (m_screenRect.width/m_maxWindows),m_screenRect.height * m_footerSize};
            auto but = new ButtonPoll(buttonRec,window->m_text);
            m_windows.push_back({window,but});
            return true;
        }


    };

}





#endif //RTK_RTK_H
