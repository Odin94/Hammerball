#include "Actors.h"
#include "Tile.h"
#include "Utility.h"
#include <math.h>
#include "Timer.h"

Actor::Actor() {
    alive = true;
    powerup = 0;
}

void Actor::set(int x, int y, int w, int h, int velx = 0, int vely = 0) {

    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->velx = velx;
    this->vely = vely;

    health = 100;
    invulcounter = 0;
    invuln = false;

    clip_rect.x = 0;
    clip_rect.y = 0;
    clip_rect.w = w;
    clip_rect.h = h;

    statictime2nextframe = 100;
    timeinthisframe = 0;
    framecounter = 0;
    maxframes = 4;

    inBallRange = false;
}

void Actor::takeDamage(int dmg, Actor source) {
    health -= dmg;

    int px = source.x + source.w / 2;
    int py = source.y + source.h / 2;

    int ex = x + w / 2;
    int ey = y + h / 2;

    int tempvelx = px - ex;
    int tempvely = py - ey;

    // set vectorlength 9
    float length = sqrt(tempvelx * tempvelx + tempvely * tempvely);

    float value = 20; // desired length of velx/vely movement vector

    if (length != 0) {
        tempvelx *= value / length;
        tempvely *= value / length;
    }

    x -= tempvelx;
    y -= tempvely;
}

void Actor::invul(int invultim) {
    invuln = true;
    invultime = invultim;
}

void Actor::move(int deltaT, EventTile BTiles[][15], int ballx, int bally, bool lethal) {
    // Acceleration:  .25sec to reach velcap; /250 to turn .25sec into ms
    if (velcapx != 0) {
        velx += velcapx * (deltaT / 200.f);
        if (abs(velx) > abs(velcapx)) {
            velx = velcapx;
        }
    }
    if (velcapy != 0) {
        vely += velcapy * (deltaT / 200.f);
        if (abs(vely) > abs(velcapy)) {
            vely = velcapy;
        }
    }

    // Deceleration
    if (velcapx == 0 && velx != 0) {
        velx -= velx * (deltaT / 200.f);
        if (abs(velx) < 0.3) {
            velx = 0;
        }
    }
    if (velcapy == 0 && vely != 0) {
        vely -= vely * (deltaT / 200.f);
        if (abs(vely) < 0.3) {
            vely = 0;
        }
    }

    // x+=velx* ( deltaT / 60.f );//.f interprets it a float; divide by expected
    // MS since last frame (30fps -> /30)
    y += vely * (deltaT / 60.f);

    // check map boundaries
    if (y + h > 960) {
        y = 960 - h;
    }
    if (y < 0) {
        y = 0;
    }

    // check walls

    if (vely > 0) {
        if (!BTiles[int(ceil((x + w) / 64))][int((y + h) / 64)].traversable && y + h > BTiles[int((x + w) / 64)][int(ceil(y / 64 + h))].y) {
            y = BTiles[int((x + w) / 64)][int(ceil((y + h) / 64))].y - h - 1;
        }

        if (!BTiles[int(ceil(x / 64))][int((y + h) / 64)].traversable && y + h > BTiles[int(x / 64)][int(ceil(y / 64 + h))].y) {
            y = BTiles[int(x / 64)][int(ceil((y + h) / 64))].y - h - 1;
        }
    }

    if (vely < 0) {
        if (!BTiles[int(floor((x + w) / 64))][int(y / 64)].traversable && y < BTiles[int(floor((x + w) / 64))][int(y / 64)].y + BTiles[int(floor((x + w) / 64))][int(y / 64)].h) {
            y = BTiles[int(floor((x + w) / 64))][int(y / 64)].y +
                BTiles[int(floor((x + w) / 64))][int(y / 64)].h + 1;
        }

        if (!BTiles[int(floor(x / 64))][int(y / 64)].traversable && y < BTiles[int(floor(x / 64))][int(y / 64)].y + BTiles[int(floor(x / 64))][int(y / 64)].h) {
            y = BTiles[int(floor(x / 64))][int(y / 64)].y +
                BTiles[int(floor(x / 64))][int(y / 64)].h + 1;
        }
    }

    x += velx * (deltaT / 60.f);

    // check map boundaries
    if (x + w > 1600) {
        x = 1600 - w;
    }
    if (x < 0) {
        x = 0;
    }

    if (velx > 0) {
        if (!BTiles[int(ceil((x + w) / 64))][int((y + h) / 64)].traversable && x + w > BTiles[int(ceil(x + w) / 64)][int((y + h) / 64)].x) {
            x = BTiles[int(ceil((x + w) / 64))][int((y + h) / 64)].x - w - 1;
        }

        if (!BTiles[int(ceil((x + w) / 64))][int(y / 64)].traversable && x + w > BTiles[int(ceil(x + w) / 64)][int(y / 64)].x) {
            x = BTiles[int(ceil((x + w) / 64))][int((y) / 64)].x - w - 1;
        }
    }

    if (velx < 0) {
        if (!BTiles[int(floor(x / 64))][int((y + h) / 64)].traversable && x < BTiles[int(floor(x / 64))][int((y + h) / 64)].x + BTiles[int(floor(x / 64))][int((y + h) / 64)].w) {
            x = BTiles[int(ceil(x / 64))][int((y + h) / 64)].x +
                BTiles[int(ceil(x / 64))][int((y + h) / 64)].w;
        }

        if (!BTiles[int(floor(x / 64))][int(y / 64)].traversable && x < BTiles[int(floor(x / 64))][int(y / 64)].x + BTiles[int(floor(x / 64))][int(y / 64)].w) {
            x = BTiles[int(ceil(x / 64))][int(y / 64)].x +
                BTiles[int(ceil(x / 64))][int(y / 64)].w;
        }
    }

    // check collision with ball
    if (!lethal && ballx != 0) {
        if (abs(x + w / 2 - (ballx + w / 2)) < w && abs(y + h / 2 - (bally + h / 2)) < h) // if ball and player intersect  x + w/2 -= ballx+w/2 - x+w/2
        {
            x -= 0.1 * ((ballx + w / 2) - (x + w / 2));
            y -= 0.1 * ((bally + h / 2) - (y + h / 2));
        }
    }

    // if moving update cliprect for animation
    // time2nextframe = abs(statictime2nextframe * 1/2 *((velx+.5) / (velcapx+1)
    // +  (vely+.5) / (velcapy+1)));  //+1 to prevent division by 0
    time2nextframe = statictime2nextframe; // TODO: get dis shit workin

    if (velx != 0 || vely != 0) {

        timeinthisframe += deltaT;

        if (timeinthisframe >= time2nextframe) {
            framecounter++;
            if (framecounter > maxframes) {
                framecounter = 0;
            }

            clip_rect.y =
                64 * (framecounter % (maxframes)) +
                framecounter; // 4 tiles per row in tilesheet //0, 65, 130,
            if (clip_rect.y < 20) {
                clip_rect.y = 0;
            }
            // clip_rect.x = 33 * (framecounter%10);
            timeinthisframe = 0;
        }
    }

    if (invuln) {
        // clip_rect.y = 33;
        invulcounter += deltaT;
        if (invulcounter > invultime) {
            invulcounter = 0;
            invuln = false;
            // clip_rect.y = 0;
        }
    }
}

Player::Player() {
    speedup = 0;
    powerup = 0;

    speedupcap = 5;
    powerupcap = 5;
}

Upgrade::Upgrade() { active = false; }

Ball::Ball() {
    lethal = false;
    time1 = 0;
}

void Ball::move(int deltaT, EventTile Btiles[][15]) {
    bool didcollide = false;
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 15; j++) {
            if (!Btiles[i][j].traversable && checkCollision(x + velx * (deltaT / 60.f), y, h, w,
                                                            Btiles[i][j].x, Btiles[i][j].y, Btiles[i][j].w,
                                                            Btiles[i][j].h)) {
                velx = -velx;
                didcollide = true;
            }
            if (!Btiles[i][j].traversable && checkCollision(x, y + vely * (deltaT / 60.f), h, w,
                                                            Btiles[i][j].x, Btiles[i][j].y, Btiles[i][j].w,
                                                            Btiles[i][j].h)) {
                vely = -vely;
                didcollide = true;
            }
            if (didcollide) {
                break;
            }
        }
        if (didcollide) {
            break;
        }
    }

    x += velx * (deltaT / 60.f);
    y += vely * (deltaT / 60.f);

    // make the ball slow down by itself with time
    // TODO: Make deceleration affect the vector length; right now the ball
    // moves unnaturally if velx hits 0 while y is still > 0 in ms
    time1 += deltaT;
    if (time1 >= 100) {

        // Decel with vectorlength
        float veclength = abs(velx) * abs(vely);

        if (velx > 0) {
            if (velx < 1) { // if velx is between 0 and 1; checked so we dont swing between -.5 and +.5 or sth
                velx = 0;
            } else {
                velx--;
            }

        } else if (velx < 0) {
            if (velx > -1) { // if velx is between 0 and -1; checked so we dont swing between -.5 and +.5 or sth

                velx = 0;
            } else {
                velx++;
            }
        }

        if (vely > 0) {
            if (vely < 1) { // if vely is between 0 and -1; checked so we dont swing between -.5 and +.5 or sth
                vely = 0;
            } else {
                vely--;
            }

        } else if (vely < 0) {
            if (vely > -1) { // if vely is between 0 and -1; checked so we dont swing between -.5 and +.5 or sth
                vely = 0;
            } else {
                vely++;
            }
        }

        time1 -= 100;
    }

    if (x > 1600) {
        x = 1400;
    } else if (x < 0) {
        x = 200;
    }

    if (y > 960) {
        y = 760;
    } else if (y < 0) {
        y = 200;
    }

    if (abs(velx) > 3 || abs(vely) > 3) {
        lethal = true;
    } else {
        lethal = false;
    }
}

void Ball::getHit(Actor source) {
    float addedvelx = x - source.x;
    float addedvely = y - source.y;

    addedvelx += 25;
    addedvely += 25;

    if (abs(addedvelx) > 50) {
        addedvelx = 50;
    }
    if (abs(addedvely) > 50) {
        addedvely = 50;
    }

    int velxtemp = abs(velx / 2) + abs(addedvelx + source.powerup * 3);
    int velytemp = abs(vely / 2) + abs(addedvely + source.powerup * 3);

    // if it is hit from the right
    if (x < source.x) {
        velx = -velxtemp;
    } else {
        velx = velxtemp;
    }

    if (y < source.y) {
        vely = -velytemp;
    } else {
        vely = velytemp;
    }
}

Bullet::Bullet() { active = false; }
void Bullet::setBullet(int xz, int y, int targetx, int targety) {
    x = xz;
    this->y = y;

    w = 6;
    h = 5;

    int px = targetx;
    int py = targety;

    int ex = x + w / 2;
    int ey = y + h / 2;

    velx = px - ex;
    vely = py - ey;

    damage = 50;

    splash = false;
    penetrating = false;

    // set vectorlength 9
    float length = sqrt(velx * velx + vely * vely);

    float value = 30; // desired length of velx/vely movement vector

    if (length != 0) {
        velx *= value / length;
        vely *= value / length;
    }

    active = true;
}

void Bullet::move(int deltaT) {
    x += velx * (deltaT / 60.f);
    y += vely * (deltaT / 60.f);

    if (x > 1600 || x < -6 || y > 2000 || y < -6) {
        active = false;
    }
}

Enemy::Enemy() { active = false; }

void Enemy::setEnemy(int x, int y, int mframes, int time2next, int damage) {
    this->x = x;
    this->y = y;

    active = true;

    velcapx = 8;
    velcapy = 8;

    this->w = 32;
    this->h = 32;

    this->damage = damage;

    health = 100;

    clip_rect.x = 0;
    clip_rect.y = 0;
    clip_rect.w = 32;
    clip_rect.h = 32;

    maxframes = mframes;
    time2nextframe = time2next;
}

void Enemy::move(int deltaT, Actor player, EventTile BTiles[][15]) {
    int px = player.x + player.w / 2;
    int py = player.y + player.h / 2;

    int ex = x + w / 2;
    int ey = y + h / 2;

    velx = px - ex;
    vely = py - ey;

    // set vectorlength 9
    float length = sqrt(velx * velx + vely * vely);

    float value = 9; // desired length of velx/vely movement vector

    if (length != 0) {
        velx *= value / length;
        vely *= value / length;
    }

    if (velx > velcapx) {
        velx = velcapx;
    }
    if (velx < -velcapx) {
        velx = -velcapx;
    }
    if (vely < -velcapy) {
        vely = -velcapy;
    }
    if (vely > velcapy) {
        vely = velcapy;
    }

    if (velx > 0) {
        direction = 67; // probably needs to become a var at some point
    } else if (velx < 0) {
        direction = 0;
    }

    x += velx * (deltaT / 60.f);
    y += vely * (deltaT / 60.f);

    // check walls
    if (velx > 0) {
        if (!BTiles[int(ceil((x + w) / 64))][int((y + h / 2) / 64)].traversable && x + w > BTiles[int(ceil(x / 64 + w))][int((y + y / 2) / 64)].x) {
            x = BTiles[int(ceil((x + w) / 64))][int((y + h / 2) / 64)].x - w;
        }
    }
    if (velx < 0) {
        if (!BTiles[int(floor(x / 64))][int((y + h / 2) / 64)].traversable && x < BTiles[int(floor(x / 64))][int((y + h / 2) / 64)].x + BTiles[int(floor(x / 64))][int((y + h / 2) / 64)].w) {
            x = BTiles[int(ceil(x / 64))][int((y + h / 2) / 64)].x + BTiles[int(ceil(x / 64))][int((y + h / 2) / 64)].w;
        }
    }

    if (vely > 0) {
        if (!BTiles[int(ceil((x + w / 2) / 64))][int((y + h) / 64)].traversable && y + h > BTiles[int((x + w / 2) / 64)][int(ceil(y / 64 + h))].y) {
            y = BTiles[int((x + w / 2) / 64)][int(ceil((y + h) / 64))].y - h;
        }
    }
    if (vely < 0) {
        if (!BTiles[int(floor((x + w / 2) / 64))][int(y / 64)].traversable && y < BTiles[int(floor((x + w / 2) / 64))][int(y / 64)].y + BTiles[int(floor((x + w / 2) / 64))][int(y / 64)].h) {
            y = BTiles[int(floor((x + w / 2) / 64))][int(y / 64)].y + BTiles[int(floor((x + w / 2) / 64))][int(y / 64)].h;
        }
    }

    if (velx != 0 || vely != 0) {
        if (maxframes > 0) {
            timeinthisframe += deltaT;

            if (timeinthisframe >= time2nextframe) {
                framecounter++;
                if (framecounter > maxframes) {
                    framecounter = 0;
                }

                clip_rect.x = 33 * (framecounter % 10) + 1; // 10 tiles per row in tilesheet
                if (clip_rect.x == 1) {
                    clip_rect.x = 0;
                }
                // clip_rect.y = 33 * (framecounter%10);

                timeinthisframe = 0;
            }
        }
    }
}

AI::AI() {}

void AI::setDirection(Ball *ball, Player player) {

    int px = player.x + player.w / 2;
    int py = player.y + player.h / 2;

    int ex = x + w / 2;
    int ey = y + h / 2;

    int bx = ball->x + ball->w / 2;
    int by = ball->y + ball->h / 2;

    float value = 15; // desired length of velx/vely movement vector

    // if ball poses a threat, move away
    if (ball->lethal && abs(ball->velx) + abs(ball->vely) > 30) {
        if (abs(ex - bx) + abs(ey - by) > 400) {
            velx = bx - ex;
            vely = by - ey;

            float length = sqrt(velx * velx + vely * vely);

            if (length != 0) {
                velx *= value / length;
                vely *= value / length;
            }
        } else {
            velx = ex - bx;
            vely = ey - by;

            float length = sqrt(velx * velx + vely * vely);

            if (length != 0) {
                velx *= value / length;
                vely *= value / length;
            }
        }
    } else {
        // if player is ready to hit the ball, move away from player
        if (abs(player.x + player.w / 2 - bx) <= 120 &&
            abs(player.y + player.h / 2 - by) <= 120) {
            velx = ex - px;
            vely = ey - py;

            // set vectorlength 20
            float length = sqrt(velx * velx + vely * vely);

            if (length != 0) {
                velx *= value / length;
                vely *= value / length;
            }
        }
        // if player not ready to hit the ball, move towards ball
        else {
            velx = bx - ex;
            vely = by - ey;

            float length = sqrt(velx * velx + vely * vely);

            if (length != 0) {
                velx *= value / length;
                vely *= value / length;
            }
        }
    }
}

void AI::hitball(Ball *ball) {
    if (!hitspampreventor.started || hitspampreventor.get_ticks() > 500) {
        if (abs(x + w / 2 - (ball->x + ball->w / 2)) <= 120 && abs(y + h / 2 - (ball->y + ball->h / 2)) <= 120) {
            ball->getHit(*this);
            hitspampreventor.start();
        }
    }
}

void AI::update(Ball *ball, Player player, int deltaT, EventTile BTiles[][15]) {
    setDirection(ball, player);
    hitball(ball);
    move(deltaT, BTiles);
}