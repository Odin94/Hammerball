#ifndef TILE_H
#define TILE_H

#include "SDL/SDL.h"

class Tile{
	public:
		int x;
		int y;
		int w;
		int h;

		int dmgframe;      //the frame of the animation that does damage
		int dmgframedmg;

		int  img; //number of img
		SDL_Rect clip_rect;

		int time2nextframe;
		int timeinthisframe;
		int framecounter;
		int maxframes;

		bool active;
		bool traversable;
		bool foreground; // like the "*" tag from RPGMayxx0r

		bool needsBorder;

		int damage;// if 0 -> not a damaging tile

		Tile();

		void set(int x, int y, int w, int h, int dmgfr, int dmgfrdmg, int img, int time2nextframe, int maxframes, bool traversable, bool active);

		void update(int deltaT);

};

class EventTile : public Tile{
	public:
		int TimeTillEvent;
		int TimeTillReset;
		int minTime;
		int maxTime;

		bool EventActive;
		bool WaitingForReset;

		Tile toTile();
		void update(int deltaT);
		void generateTime();
		void setE(int x, int y, int w, int h, int dmgfr, int dmgfrdmg, int img, int time2nextframe, int maxframes, bool traversable, bool active, int minTime, int maxTime, int TimeTillReset);
};

#endif