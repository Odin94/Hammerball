#ifndef StringInput_H
#define StringInput_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class StringInput {
  private:
    TTF_Font *font;

  public:
    // The storage string
    std::string str;

    // determine if string has changed
    bool rerenderpls;

    // Initializes variables
    StringInput(SDL_Surface *screen, int w, int h);

    // Does clean up
    ~StringInput();

    // Handles input
    void handle_input(SDL_Event event);
};

#endif