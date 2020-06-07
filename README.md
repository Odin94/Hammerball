# Hammerball
This is an old game project I wrote during university for fun and practice. 

I've recently discovered the code on my old hard drive and am now looking to clean it up a bit and make it run with SDL2 instead of SDL1.2.


## Dependencies
These instructions are made for building on Windows 10. They likely work on other Windows versions as well.

Unpack the 64bit version (has to match your mingw installation) of the following dependencies into `C:\mingw_dev_lib`

* SDL 2 (http://libsdl.org/download-2.0.php SDL2-devel-2.0.12-mingw.tar.gz)
* SDL_net (https://www.libsdl.org/projects/SDL_net/ SDL2_net-devel-2.0.1-mingw.tar.gz)
* SDL_image (https://www.libsdl.org/projects/SDL_image/ SDL2_image-devel-2.0.5-mingw.tar.gz)
* SDL_ttf (https://www.libsdl.org/projects/SDL_ttf/ SDL2_ttf-devel-2.0.15-mingw.tar.gz)
* SDL_mixer(https://www.libsdl.org/projects/SDL_mixer/ SDL2_mixer-devel-2.0.4-mingw.tar.gz)

Install `mingw` and `make`, run `make` in the base folder of this repository to build.

## Planned Features
* Visual and/or audio cue when ball is in hitting range
* Visual cue for ball hitting cooldown
* Fix multiplayer
* Make upgrades work