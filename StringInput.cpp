#include "StringInput.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
#include "Utility.h"

StringInput::StringInput() {
    // Initialize the string
    str = "";
}

StringInput::~StringInput() {}

void StringInput::handle_input(SDL_Event event) {
    SDL_Color textColor = {255, 255, 255};

    std::string temp = str;

    //Special key input
    if (event.type == SDL_KEYDOWN) {
        //Handle backspace
        if (event.key.keysym.sym == SDLK_BACKSPACE && str.length() > 0) {
            //lop off character
            str.pop_back();
        }
        //Handle copy
        else if (event.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL) {
            SDL_SetClipboardText(str.c_str());
        }
        //Handle paste
        else if (event.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL) {
            str = SDL_GetClipboardText();
        }
    }
    //Special text input event
    else if (event.type == SDL_TEXTINPUT) {
        //Not copy or pasting
        if (!(SDL_GetModState() & KMOD_CTRL && (event.text.text[0] == 'c' || event.text.text[0] == 'C' || event.text.text[0] == 'v' || event.text.text[0] == 'V'))) {
            str += event.text.text;
        }
    }

    // If the string was changed
    if (str != temp) {
        rerenderpls = true;
    }
}
