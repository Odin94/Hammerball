#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

std::string toString(int number);
SDL_Surface *load_image( std::string filename );
void apply_surface( float drawscale, int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL );
void apply_surface( float drawscale, int x, int y, SDL_Surface* source, SDL_Surface* destination, int xr, int yr, int w, int h);
std::string toStr(int number);
std::string toDottedIP(Uint32 rawIP);
bool checkCollision( int x, int y, int w, int h, int x2, int y2, int w2, int h2);
char **separateString(std::string in, int arraySize, const char *sepChar);
int* charAToIntA(char** in, int size);