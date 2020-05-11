
#ifndef Actor_H
#define Actor_H

#include "SDL/SDL.h"
#include "Timer.h"
#include "Tile.h"

class Actor{

	public:

		//rect
		int x;
		int y;
		int w;
		int h;

		int powerup;   //not needed, but without this the ball.gethit function will cry   (le uber poor design face)

		float velcapx;
		float velcapy;

		float velx;
		float vely;

		int health;
		int invultime;
		int invulcounter;
		bool invuln;

		bool alive;

		bool inBallRange;

		int damage;

		SDL_Rect clip_rect;

		int statictime2nextframe;
		int time2nextframe;     //dynamic; depends on movementspeed / speedcap
		int timeinthisframe;
		int framecounter;
		int maxframes;

		int direction; //0  = left, 1 = right

		Actor();

		void set(int x, int y, int w, int h,int velx, int vely);

		void move(int time, EventTile BTiles[][15], int ballx = 0, int bally = 0, bool lethal = false);

		void invul(int invultime);

		void takeDamage(int dmg, Actor source);

};

class Player : public Actor
{
	public:

		int speedup;
		int powerup;

		int speedupcap;
		int powerupcap;

		Player();

		void shoot();

};

class Ball : public Actor
{
	public:

		bool lethal;
		float time1;

		Ball();

		void move(int deltaT, EventTile Btiles[][15]);

		void getHit(Actor source);
		

};

class Upgrade : public Actor
{
	public:

		char type;
		bool active;

		Upgrade();
};

class AI : public Actor
{
	public:

		Timer hitspampreventor;

		AI();

		void setDirection(Ball *ball, Player player);

		void hitball(Ball *ball);

		void update(Ball *ball, Player player, int deltaT, EventTile BTiles[][15]);
};


class Enemy : public Actor
{
	public:

		bool active;

		Enemy();

		void setEnemy(int x, int y, int maxframes, int time2nextframe, int dmg = 10);
		
		void move(int time, Actor player, EventTile BTiles[][15]);

		void shoot();
};


class Bullet : public Actor
{
	public: 

		//rect
		int x;
		int y;
		int w;
		int h;

		int velx;
		int vely;

		bool active;

		bool splash;
		bool penetrating;

		Bullet();

		void setBullet(int xz, int y, int mousex, int mousey);

		void move(int time);
};

#endif