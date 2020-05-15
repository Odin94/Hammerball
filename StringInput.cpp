#include "StringInput.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
#include "Utility.h"

StringInput::StringInput(SDL_Surface *screen, int w, int h) {
    // Initialize the string
    str = "";

    mainscreen = screen;

    SCREEN_WIDTH = w;
    SCREEN_HEIGHT = h;
}

StringInput::~StringInput() {}

void StringInput::handle_input(SDL_Event event) {
    SDL_Color textColor = {255, 255, 255};

    // If a key was pressed
    if (event.type == SDL_KEYDOWN) {
        // Keep a copy of the current version of the string
        std::string temp = str;

        // If the string less than maximum size
        if (str.length() <= 16) {
            // If the key is a space
            if (event.key.keysym.unicode == (Uint16)' ' ||
                event.key.keysym.unicode == (Uint16)'.') {
                // Append the character
                str += (char)event.key.keysym.unicode;
            }

            // If the key is a number
            else if ((event.key.keysym.unicode >= (Uint16)'0') &&
                     (event.key.keysym.unicode <= (Uint16)'9')) {
                // Append the character
                str += (char)event.key.keysym.unicode;
            }

            // If the key is a uppercase letter
            else if ((event.key.keysym.unicode >= (Uint16)'A') &&
                     (event.key.keysym.unicode <= (Uint16)'Z')) {
                // Append the character
                str += (char)event.key.keysym.unicode;
            }

            // If the key is a lowercase letter
            else if ((event.key.keysym.unicode >= (Uint16)'a') &&
                     (event.key.keysym.unicode <= (Uint16)'z')) {
                // Append the character
                str += (char)event.key.keysym.unicode;
                // std::cout << "String should be: " << str;
            }
        }

        // If backspace was pressed and the string isn't blank
        if ((event.key.keysym.sym == SDLK_BACKSPACE) && (str.length() != 0)) {
            // Remove a character from the end
            str.erase(str.length() - 1);
        }

        // If the string was changed
        if (str != temp) {
            rerenderpls = true;
        }
    }
}