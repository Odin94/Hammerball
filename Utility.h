#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

std::string toString(int number);
SDL_Texture *load_texture(std::string path, SDL_Renderer *renderer);
SDL_Texture *load_from_rendered_text(TTF_Font *font, std::string text, SDL_Color text_color, SDL_Renderer *renderer);
void apply_surface(float drawscale, int x, int y, int w, int h, SDL_Texture *texture, SDL_Renderer *renderer, SDL_Rect *srcRect = NULL);
void apply_surface(float drawscale, int x, int y, int w, int h, SDL_Texture *texture, SDL_Renderer *renderer, int xr, int yr, int wr, int hr);
std::string toStr(int number);
std::string toDottedIP(Uint32 rawIP);
bool checkCollision(int x, int y, int w, int h, int x2, int y2, int w2, int h2);
char **separateString(std::string in, int arraySize, const char *sepChar);
int *charAToIntA(char **in, int size);