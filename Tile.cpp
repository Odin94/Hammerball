#include "Tile.h"
#include <SDL.h>
#include <Math.h>

Tile::Tile() { needsBorder = false; }

void Tile::update(int deltaT) {
    if (maxframes > 1) {
        timeinthisframe += deltaT;

        if (timeinthisframe >= time2nextframe) {
            framecounter++;
            if (framecounter > maxframes) {
                framecounter = 0;
            }

            // 4 tiles per row in tilesheet //67 134 201 268
            clip_rect.y = 66 * (framecounter % (maxframes + 1)) + framecounter;
            if (clip_rect.y < 20) {
                clip_rect.y = 0;
            }
            // clip_rect.x = 33 * (framecounter%10);
            timeinthisframe = 0;
        }
    }

    if (framecounter == dmgframe) {
        damage = dmgframedmg;
        needsBorder = true;
    } else {
        damage = 0;
        needsBorder = false;
    }
}

void Tile::set(int x, int y, int w, int h, int dmgfr, int dmgfrdmg, int img, int maxframes,
               int time2nextframe, bool traversable, bool active) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;

    dmgframe = dmgfr;
    dmgframedmg = dmgfrdmg;

    this->img = img;
    this->time2nextframe = time2nextframe;
    this->maxframes = maxframes;

    this->active = active;
    this->traversable = traversable;
    foreground = false;

    damage = 0;
}

void EventTile::generateTime() { TimeTillEvent = rand() % (maxTime - minTime + 1) + minTime; }

void EventTile::update(int deltaT) {
    if (EventActive) {
        timeinthisframe += deltaT;

        if (!WaitingForReset) // if we're not done with initial(falling)
                              // animation,
                              // keep doing that
        {
            if (timeinthisframe >= time2nextframe) {
                framecounter++;
                if (framecounter > maxframes) {
                    framecounter = 0;
                }

                clip_rect.y = 66 * (framecounter % (maxframes + 1)) +
                              framecounter; // 4 tiles per row in tilesheet //67
                                            // 134 201 268
                if (clip_rect.y < 20) {
                    clip_rect.y = 0;
                }
                // clip_rect.x = 33 * (framecounter%10);
                timeinthisframe = 0;
            }
        } else // if were done with initial animation wait until we want to
               // reset
               // animation
        {
            if (timeinthisframe >= TimeTillReset) {
                timeinthisframe = 0;
                framecounter = 0;
                WaitingForReset = false;
                EventActive = false;
                clip_rect.y = 66 * (framecounter % (maxframes + 1)) + framecounter;
            }
        }
    } else {
        timeinthisframe += deltaT;
        if (timeinthisframe >= TimeTillEvent && TimeTillEvent != -1) {
            timeinthisframe = 0;
            EventActive = true;
        }
    }

    if (framecounter == dmgframe) {
        damage = dmgframedmg;
        needsBorder = true;
        WaitingForReset = true;
    } else {
        damage = 0;
        needsBorder = false;
    }
}

void EventTile::setE(int x, int y, int w, int h, int dmgfr, int dmgfrdmg, int img, int maxframes,
                     int time2nextframe, bool traversable, bool active, int minTime, int maxTime,
                     int TimeTillReset) {
    set(x, y, w, h, dmgfr, dmgfrdmg, img, maxframes, time2nextframe, traversable, active);
    this->minTime = minTime;
    this->maxTime = maxTime;
    this->TimeTillReset = TimeTillReset;

    TimeTillEvent = -1;

    EventActive = false;
    WaitingForReset = false;

    clip_rect.w = 66;
    clip_rect.h = 66;
}

Tile EventTile::toTile() {
    Tile returnTile;
    returnTile.set(x, y, w, h, dmgframe, dmgframedmg, img, maxframes, time2nextframe, traversable, active);
    return returnTile;
}