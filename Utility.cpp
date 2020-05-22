//#include "Utility.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>


SDL_Texture *load_texture(std::string path, SDL_Renderer* renderer) {
    //The final texture
    SDL_Texture *newTexture = NULL;

    //Load image at specified path
    SDL_Surface *loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
    } else {
        //Color key image
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
        }

        //Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    //Return success
    return newTexture;
}

void apply_surface(float drawscale, int x, int y, SDL_Surface *source, SDL_Surface *destination, SDL_Rect *clip) // clip is defaulted to NULL!!
{
    // Holds offsets
    SDL_Rect offset;

    // Get offsets
    offset.x = x * drawscale;
    offset.y = y * drawscale;

    if (clip == NULL) {
        // Blit
        SDL_BlitSurface(source, clip, destination, &offset);
    } else {
        SDL_Rect clipScaled = {clip->x * drawscale, clip->y * drawscale, clip->w * drawscale, clip->h * drawscale};
        SDL_BlitSurface(source, &clipScaled, destination, &offset);
    }
}

void apply_surface(float drawscale, int x, int y, SDL_Surface *source, SDL_Surface *destination, int xr, int yr, int w, int h) {
    SDL_Rect rekt = {xr, yr, w, h};
    apply_surface(drawscale, x, y, source, destination, &rekt);
}

std::string toStr(int number) {
    std::stringstream convert; // stream used for the conversion
    convert << number;         // insert the textual representation of 'Number' in the
                               // characters in the stream

    return convert.str();
}

// returns weird hex number, but it works so w/e
std::string toDottedIP(Uint32 rawIP) {
    SDL_SwapBE32(rawIP); // switches big endian to "normal" byte order

    unsigned char octet[4] = {0, 0, 0, 0};

    octet[0] = rawIP >> 24;
    octet[1] = (rawIP >> 16) & 0xff;
    octet[2] = (rawIP >> 8) & 0xff, octet[3] = rawIP & 0xff;

    std::stringstream result;
    result << octet[0] << "." << octet[1] << "." << octet[2] << "." << octet[3];

    return result.str();
    ;
}

bool checkCollision(int x, int y, int w, int h, int x2, int y2, int w2, int h2) {
    // The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    // Calculate the sides of rect A
    leftA = x;
    rightA = x + w;
    topA = y;
    bottomA = y + h;

    // Calculate the sides of rect B
    leftB = x2;
    rightB = x2 + w2;
    topB = y2;
    bottomB = y2 + h2;

    // If any of the sides from A are outside of B
    if (bottomA <= topB) {
        return false;
    }

    if (topA >= bottomB) {
        return false;
    }

    if (rightA <= leftB) {
        return false;
    }

    if (leftA >= rightB) {
        return false;
    }

    // If none of the sides from A are outside B
    return true;
}

char **separateString(std::string input, int arraySize, const char *sepChar) {
    char *in = (char *)input.c_str();
    char **output = new char *[arraySize + 1]; //+1 for null character

    output[0] = strtok(in, sepChar);
    for (int i = 1; i < arraySize; i++) {
        output[i] = strtok(NULL, sepChar);
    }

    return output;
}

int *charAToIntA(char **in, int size) {
    int *output = new int[size + 1];
    for (int i = 0; i < size; i++) {
        output[i] = atoi(in[i]);
    }

    return output;
}
