#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include "Utility.h"
#include "Actors.h"
#include "Timer.h"
#include "Tile.h"
#include "StringInput.h"
#include <SDL_ttf.h>
#include <Math.h>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iostream>
using namespace std;

void initStage();

const int SCREEN_WIDTH = 1600; // 25 tiles
const int SCREEN_HEIGHT = 960; // 15 tiles
const int SCREEN_BPP = 32;

const int GROUND_TILE = 0;
const int SPIKES_TILE = 1;
const int WALL_TILE = 2;
const int FALLING_TILE = 3;

const int EVENT_TILE = 0;

const int FRAMES_PER_SECOND = 60;

int mousex;
int mousey;

fstream map;

EventTile ATiles[25][15]; // background
EventTile BTiles[25][15]; // things that lie on the ground

bool inTitle = true;

// TCPWrapper connection
TCPsocket TCPServer, TCPClient = NULL, TCPClient2 = NULL, TCPClient3 = NULL;
IPaddress Client2Adr, Client3Adr, Client4Adr;
char TCPBufferClt[100];
IPaddress TCPip;
bool TCPConnectionDone = false;
int playercount; // amount of players
int playernumber = 1; // your number (host is 1)

UDPsocket clientSocket;
UDPsocket serverSocket;
UDPpacket *sendPac;
UDPpacket *recPac; // really wanna call one of the tuPac.. :S

char hitball = '0';
Timer hitStackPreventor;

stringstream sstream;
string connectionIP;

IPaddress address;
IPaddress UDPaddress;

bool isHost = true; // true if you dont play MP - used to check whether this
                    // entity has to calculate game logic
bool multiplayer = false;

bool localmultiplayer = false;

Timer fps;
Timer totaltime;
int deltaT; // time since last frame

Timer clickspamprevent0r;
Timer clickspamprevent0r2;

Timer UpgradeSpawner;
Upgrade upgrade;

TTF_Font *font = NULL;
TTF_Font *fontSmall = NULL;
SDL_Color textColor = {30, 30, 30};
SDL_Color bgcolor = {0, 0, 0};

Bullet newbullet;
Enemy newenemy;

Bullet bullets[32];
int bulletcounter;

Enemy enemies[32];
int enemycounter;

SDL_Surface *surfaces[10];

SDL_Surface *screen = NULL;
SDL_Surface *TitleScreen = NULL;
SDL_Surface *HostingScreen = NULL;
SDL_Surface *SettingsBG = NULL;
SDL_Surface *ClientScreen = NULL;
SDL_Surface *selection = NULL;
SDL_Surface *ATile_ground = NULL;
SDL_Surface *BTile_Spikes = NULL;
SDL_Surface *BTile_Wall = NULL;
SDL_Surface *PlayerSprite = NULL;
SDL_Surface *PlayerDead = NULL;
SDL_Surface *SpeedUp = NULL;
SDL_Surface *PowerUp = NULL;
SDL_Surface *Enemy_Knife = NULL;
SDL_Surface *HealthBar = NULL;
SDL_Surface *HealthBarFrame = NULL;
SDL_Surface *Bullet = NULL;
SDL_Surface *FallingTile = NULL;

SDL_Surface *Borders = NULL;

SDL_Surface *TEST = NULL;
SDL_Surface *TEST2 = NULL;

SDL_Surface *BallLethal = NULL;
SDL_Surface *BallNonLethal = NULL;

SDL_Rect healthrect;

StringInput StrIn(screen, SCREEN_WIDTH, SCREEN_HEIGHT);
StringInput StrInName(screen, SCREEN_WIDTH, SCREEN_HEIGHT);
int inputcounter = 0;
bool inputdone = false;
SDL_Surface *text = NULL;
SDL_Surface *name = NULL;
SDL_Surface *name2 = NULL;

Mix_Music *Stage1music = NULL;

bool running = false;

Player player;
Player player2;
Player player3;
Player player4;

AI ai;

Ball ball;

// The event structure
SDL_Event event;

void init() {
  SDL_Init(SDL_INIT_EVERYTHING);

  TTF_Init();
  SDL_EnableUNICODE(SDL_ENABLE);

  // init networkstuff  (UDP stuff is in separate function)
  SDLNet_Init();

  // read options
  fstream settings;
  settings.open("Settings.txt", ios::in);
  if (settings.is_open()) {
    settings.ignore(256, '\n'); // skips till \n character aka next line  (first
                                // argument = max characters skipped)
    getline(settings, connectionIP);

    string tempplayercount;
    getline(settings, tempplayercount);
    playercount = atoi(tempplayercount.c_str());
    playercount = min(playercount, 4);

    string tempplayernumber;
    getline(settings, tempplayernumber);
    playernumber = atoi(tempplayernumber.c_str());
  } else {
    cout << "Settings.txt didnt exist and was created, please start again\n";
    settings.open("Settings.txt", ios::out);
    settings << "IP:"
             << "\n"
             << "localhost"
             << "\n"
             << "2"
             << "\n"
             << "1"
             << "\n";
    settings.close();
    exit(0);
  }

  // read map
  map.open("Map.txt", ios::in);
  if (!map.is_open()) {
    cout << "map.txt didn't exist and was created, please start again\n";
    map.open("Map.txt", ios::out);
    for (int i = 0; i < 25; i++) {
      for (int j = 0; j < 15; j++) {
        if (j == 0 || j == 14 || i == 0 || i == 24) {
          map << "-1,0," << WALL_TILE << ",0,0,0,1,0,0,0 ";
        } else if (j == 12 && i == 12) {
          map << "8,1," << FALLING_TILE << ",9,100,1,1,2000,3000,2500 ";
        } else {
          map << "-1,0," << GROUND_TILE << ",0,0,1,1,0,0,0 ";
        }
        if (j == 14) {
          map << "\n";
        }
      }
    }
    map.close();
    exit(0);
  }

  // server, used for receiving

  recPac = SDLNet_AllocPacket(512);
  sendPac = SDLNet_AllocPacket(512);

  // client, used for sending
  clientSocket = SDLNet_UDP_Open(0);

  SDLNet_ResolveHost(&TCPip, NULL, 8134);
  SDLNet_ResolveHost(&address, connectionIP.c_str(),
                     8134); // address, hostIPString, (server's)port
  // TCPServer
  // Resolving the host using NULL make network interface to listen
  TCPServer = SDLNet_TCP_Open(&TCPip);

  font = TTF_OpenFont("res/Demonized.ttf", 28);
  fontSmall = TTF_OpenFont("res/Demonized.ttf", 12);

  screen =
      SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);

  Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096); // init mixer

  ATile_ground = load_image("res/ground.png");
  BTile_Wall = load_image("res/Wall.png");
  FallingTile = load_image("res/falltile.png");

  PlayerSprite = load_image("res/Ironman.png");
  PlayerDead = load_image("res/PlayerDead.png");

  TitleScreen = load_image("res/Title.png");
  HostingScreen = load_image("res/Hosting.png");
  ClientScreen = load_image("res/Client.png");
  SettingsBG = load_image("res/Settings.png");

  SpeedUp = load_image("res/SpeedUp.png");
  PowerUp = load_image("res/PowerUp.png");

  HealthBar = load_image("res/healthbar.png");
  HealthBarFrame = load_image("res/HealthBarFrame.png");

  BallLethal = load_image("res/Ball.png");
  BallNonLethal = load_image("res/Ballnonlethal.png");

  Borders = load_image("res/Borders.png");

  TEST = load_image("res/asphaltdecal.png");
  TEST2 = load_image("res/asphaltdecal2.png");

  string xxy = "";
  text = TTF_RenderText_Solid(font, xxy.c_str(), textColor);
  name = TTF_RenderText_Solid(font, xxy.c_str(), textColor);
  name2 = TTF_RenderText_Solid(fontSmall, xxy.c_str(), textColor);

  surfaces[0] = ATile_ground;
  surfaces[1] = BTile_Spikes;
  surfaces[2] = BTile_Wall;
  surfaces[3] = FallingTile;
  // score = TTF_RenderUTF8_Shaded(font, toString(travelled).c_str(), textColor,
  // bgcolor);

  bulletcounter = 0; // counts how many player-bullets are on the field
  enemycounter = 0; // counts enemies

  player.set(100, 270, 66, 66, 0, 0);
  player2.set(500, 270, 66, 66, 0, 0);
  player3.set(100, 600, 66, 66, 0, 0);
  player4.set(500, 600, 66, 66, 0, 0);

  ai.set(500, 270, 66, 66, 0, 0);

  ball.set(500, 500, 66, 66, 0, 0);

  healthrect.x = 0;
  healthrect.y = 0;
  healthrect.w = 105;
  healthrect.h = 27;

  // Stage1music = Mix_LoadMUS("res/MysteriousSpace.wav");

  initStage();
}

void setupServer(string ip) // doesnt setup clientsocket, since clientsocket is
                            // opened on rnd available port (0) anyways
{
  serverSocket = SDLNet_UDP_Open(8133);

  SDLNet_ResolveHost(&UDPaddress, ip.c_str(),
                     8134); // TODO change port for every client..?

  player.set(100, 270, 66, 66, 0, 0);
  player2.set(500, 270, 66, 66, 0, 0);
}

void setupClient(string ip) {
  int port = 8132 + playernumber; // playernumber for clients is between 2 and 4
                                  // -> ports are 8134 - 8136
  serverSocket = SDLNet_UDP_Open(port);

  SDLNet_ResolveHost(&UDPaddress, ip.c_str(), 8133);

  player.set(500, 270, 66, 66, 0, 0); // set player location for clientside
  player2.set(100, 270, 66, 66, 0, 0);
}

// setE: xywh, dmgfr, dmgfrdmg, img, maxframes, time2nextframe, traversable,
// active, minTime, maxTime, TimeTillReset
void initStage() {
  srand((unsigned)time(0));
  int rnd;

  string mapContent;
  for (int i = 0; i < 25; i++) {
    for (int j = 0; j < 15; j++) {
      rnd = (rand() % 3);
      ATiles[i][j].clip_rect.x = rnd * 66 + 1;
      if (ATiles[i][j].clip_rect.x == 1) {
        ATiles[i][j].clip_rect.x = 0;
      }
      ATiles[i][j].clip_rect.y = 0;
      map >> mapContent;

      int *mpc = charAToIntA(separateString(mapContent, 10, ","), 10);

      ATiles[i][j].setE(i * 66, j * 66, 66, 66, mpc[0], mpc[1], mpc[2], mpc[3],
                        mpc[4], mpc[5] != 0, mpc[6] != 0, mpc[7], mpc[8],
                        mpc[9]);

      if (ATiles[i][j].img == FALLING_TILE) {
        ATiles[i][j].generateTime();
        ATiles[i][j].clip_rect.x = 0;
      }

      mapContent.clear();

      BTiles[i][j].setE(i * 66, j * 66, 66, 66, -1, 0, -1, 0, 1, true, true, 0,
                        0, 0);
      BTiles[i][j].clip_rect.x = 0;
      BTiles[i][j].clip_rect.y = 0;
    }
  }
  map.close();

  ATiles[11][12].setE(11 * 66, 12 * 66, 66, 66, 8, 1, FALLING_TILE, 9, 100,
                      true, true, 2000, 3000, 2500);
  ATiles[11][12].generateTime();
  ATiles[11][12].clip_rect.x = 0;

  ATiles[11][11].setE(11 * 66, 11 * 66, 66, 66, 8, 1, FALLING_TILE, 9, 250,
                      true, true, 2000, 3000, 2500);
  ATiles[11][11].generateTime();
  ATiles[11][11].clip_rect.x = 0;
}

void draw() {
  if (multiplayer && inTitle) {
    if (isHost) {
      apply_surface(0, 0, HostingScreen, screen);
    } else {
      apply_surface(0, 0, ClientScreen, screen);
    }
  } else if (inTitle) {
    if (!inputdone) {
      apply_surface(0, 0, SettingsBG, screen);
      apply_surface(600, 200, text, screen);
      apply_surface(600, 680, name, screen);
    } else {
      apply_surface(0, 0, TitleScreen, screen);
    }
  } else {
    for (int i = 0; i < 25; i++) {
      for (int j = 0; j < 15; j++) {
        if (ATiles[i][j].active) {
          apply_surface(ATiles[i][j].x, ATiles[i][j].y,
                        surfaces[ATiles[i][j].img], screen,
                        &ATiles[i][j].clip_rect);
        }
        if (ATiles[i][j].needsBorder) // check if borders from top, left, right
                                      // or bottom need to be drawn
        {
          if (!ATiles[i - 1][j].needsBorder) {
            apply_surface(ATiles[i][j].x, ATiles[i][j].y, Borders, screen, 0, 0,
                          66, 66);
          }
          if (!ATiles[i + 1][j].needsBorder) {
            apply_surface(ATiles[i][j].x, ATiles[i][j].y, Borders, screen, 67,
                          0, 66, 66);
          }
          if (!ATiles[i][j - 1].needsBorder) {
            apply_surface(ATiles[i][j].x, ATiles[i][j].y, Borders, screen, 134,
                          0, 66, 66);
          }
          if (!ATiles[i][j + 1].needsBorder) {
            apply_surface(ATiles[i][j].x, ATiles[i][j].y, Borders, screen, 201,
                          0, 66, 66);
          }
        }
        if (BTiles[i][j].active) {
          apply_surface(BTiles[i][j].x, BTiles[i][j].y,
                        surfaces[BTiles[i][j].img], screen,
                        &BTiles[i][j].clip_rect);
        }
      }
    }

    if (upgrade.active) {
      if (upgrade.type == 's') {
        apply_surface(upgrade.x, upgrade.y, SpeedUp, screen);
      }
      if (upgrade.type == 'p') {
        apply_surface(upgrade.x, upgrade.y, PowerUp, screen);
      }
    }

    if (!multiplayer && !localmultiplayer) {
      if (ai.alive) {
        apply_surface(ai.x, ai.y, PlayerSprite, screen, &ai.clip_rect);
      } else {
        apply_surface(ai.x, ai.y, PlayerDead, screen);
      }
    }

    if (player.alive) {
      apply_surface(player.x, player.y, PlayerSprite, screen,
                    &player.clip_rect);
    } else {
      apply_surface(player.x, player.y, PlayerDead, screen);
    }
    if (multiplayer || localmultiplayer) {
      if (player2.alive) {
        apply_surface(player2.x, player2.y, PlayerSprite, screen,
                      &player2.clip_rect);
      } else {
        apply_surface(player2.x, player2.y, PlayerDead, screen);
      }
      if (playercount > 2 && player3.alive) {
        apply_surface(player3.x, player3.y, PlayerSprite, screen,
                      &player3.clip_rect);
        if (playercount > 3 && player4.alive) {
          apply_surface(player4.x, player4.y, PlayerSprite, screen,
                        &player4.clip_rect);
        }
      }
    }

    if (ball.lethal) {
      apply_surface(ball.x, ball.y, BallLethal, screen);
    } else {
      apply_surface(ball.x, ball.y, BallNonLethal, screen);
    }

    // draw UI
    apply_surface(player.x, player.y - 25, name, screen);
    apply_surface(player2.x, player2.y - 25, name2, screen);
    apply_surface(SCREEN_WIDTH - 132, 0, HealthBarFrame, screen);

    healthrect.w = 105 - (100 - player.health);
    apply_surface(SCREEN_WIDTH - 132, 0, HealthBar, screen, &healthrect);

    //    apply_surface(66*5, 66*5, TEST, screen);
    //   apply_surface(66*2,66*2, TEST2, screen);
  }
  SDL_Flip(screen);
}

void handleMP() {
  if (isHost) // gets updates immediatly
  {
    // RECV
    if (SDLNet_UDP_Recv(serverSocket, recPac) > 0) {
      char *temp[16];

      temp[0] = strtok((char *)recPac->data, ",");
      temp[1] = strtok(NULL, ",");
      temp[2] = strtok(NULL, ",");
      temp[3] = strtok(NULL, ",");
      temp[4] = strtok(NULL, ",");

      if (temp[0][0] == '2') {
        player2.x = atoi(temp[1]);
        player2.y = atoi(temp[2]);

        if (!hitStackPreventor.is_started()) {
          if (temp[3][0] == '2') {
            ball.getHit(player2);
            hitball = '1'; // send hitball = 1 back to confirm that ball was hit
                           // TODO: prevent ball from getting hit twice due to
                           // packet loss("recently hit ball" variable dependant
                           // on timer?)
            hitStackPreventor.start();
          }
          if (temp[3][0] == '3') {
            ball.getHit(player3);
            hitball = '1';
            hitStackPreventor.start();
          }
          if (temp[3][0] == '4') {
            ball.getHit(player4);
            hitball = '1';
            hitStackPreventor.start();
          }
        }
        if (hitStackPreventor.is_started() &&
            hitStackPreventor.get_ticks() > 500) {
          hitStackPreventor.stop();
        }
      } else if (temp[0][0] == '3') {
        player3.x = atoi(temp[1]);
        player3.y = atoi(temp[2]);

        if (temp[3][0] == '1' && !hitStackPreventor.is_started()) {
          ball.getHit(player3);
          hitball = '1';
          hitStackPreventor.start();
        }
        if (hitStackPreventor.is_started() &&
            hitStackPreventor.get_ticks() > 500) {
          hitStackPreventor.stop();
        }
      } else if (temp[0][0] == '4') {
        player4.x = atoi(temp[1]);
        player4.y = atoi(temp[2]);

        if (temp[3][0] == '1' && !hitStackPreventor.is_started()) {
          ball.getHit(player4);
          hitball = '1';
          hitStackPreventor.start();
        }
        if (hitStackPreventor.is_started() &&
            hitStackPreventor.get_ticks() > 500) {
          hitStackPreventor.stop();
        }
      }
    }

    char islethal;
    if (ball.lethal) {
      islethal = '1';
    } else {
      islethal = '0';
    }

    // SEND
    sstream.str(
        ""); // reset stream to empty TODO: send information to all players
    sstream << player.x << "," << player.y << "," << ball.x << "," << ball.y
            << "," << islethal << "," << hitball << "," << player2.x << ","
            << player2.y << "," << player3.x << "," << player3.y << "\0";
    hitball = '0'; // reset hitball
    string result = sstream.str();

    // Fleischerei HACK ?
    const char *datatemp = result.c_str();

    int size = 1;
    while (datatemp[size] != '\0') {
      size++;
    }
    char data[size + 1];

    int i = 0;
    while (datatemp[i] != '\0') {
      data[i] = datatemp[i];
      i++;
    }
    data[i + 1] = '\0';

    sendPac->data = (Uint8 *)&data;
    sendPac->address.host = Client2Adr.host; // set destination host
    sendPac->address.port = Client2Adr.port; // and port
    sendPac->len =
        strlen((char *)sendPac->data) +
        1; // length of data when cast to string; +1 for finish-character
    SDLNet_UDP_Send(clientSocket, -1,
                    sendPac); // sets packet->channel (socket,channel,packet)

    if (playernumber > 2) {
      sendPac->address.host = Client3Adr.host;
      sendPac->address.port =
          Client3Adr.port; // port# is wrong! its the TCP port number
      SDLNet_UDP_Send(clientSocket, -1, sendPac);

      if (playernumber > 3) {
        sendPac->address.host = Client4Adr.host;
        sendPac->address.port = Client4Adr.port;
        SDLNet_UDP_Send(clientSocket, -1, sendPac);
      }
    }

  } else {
    // SEND
    sstream.str(""); // reset stream to empty
    sstream << playernumber << "," << player.x << "," << player.y << ","
            << hitball << "\0";
    string result = sstream.str();

    // FUCKING HACKY AS SHIT AKA //HACK
    const char *datatemp = result.c_str();

    int size = 1;
    while (datatemp[size] != '\0') {
      size++;
    }
    char data[size + 1];

    int i = 0;
    while (datatemp[i] != '\0') {
      data[i] = datatemp[i];
      i++;
    }
    data[i + 1] = '\0';

    sendPac->data = (Uint8 *)&data;
    sendPac->address.host = UDPaddress.host; // set destination host
    sendPac->address.port = UDPaddress.port; // and port
    sendPac->len =
        strlen((char *)sendPac->data) +
        1; // length of data when cast to string; +1 for finish-character
    SDLNet_UDP_Send(clientSocket, -1,
                    sendPac); // sets packet->channel (socket,channel,packet)

    // RECV
    if (SDLNet_UDP_Recv(serverSocket, recPac) > 0) {
      // cout << "UDP Packet incoming\n";
      // cout << ("Data:    %s \n",(char *) recPac->data);

      char *temp[12];

      temp[0] = strtok((char *)recPac->data, ",");
      temp[1] = strtok(NULL, ",");
      temp[2] = strtok(NULL, ",");
      temp[3] = strtok(NULL, ",");
      temp[4] = strtok(NULL, ",");
      temp[5] = strtok(NULL, ",");

      if (playercount > 2) {
        temp[6] = strtok(NULL, ","); // server's player2 x
        temp[7] = strtok(NULL, ",");
        temp[8] = strtok(NULL, ","); // server's player3 x
        temp[9] = strtok(NULL, ",");

        if (playernumber == 2) {
          player3.x = atoi(temp[8]);
          player3.y = atoi(temp[9]);
        }
        if (playernumber == 3) {
          player3.x = atoi(temp[6]);
          player3.y = atoi(temp[7]);
        }
      }

      player2.x = atoi(temp[0]);
      player2.y = atoi(temp[1]);

      ball.x = atoi(temp[2]);
      ball.y = atoi(temp[3]);

      if (temp[4][0] == '1') {
        ball.lethal = true;
      } else {
        ball.lethal = false;
      }

      if (temp[5][0] == '1') // temp is an array of char arrays - fuck me
      {
        hitball = '0';
      }
    }
  }
}

void update(Uint32 delta) {
  // PLAYER STUFF
  if (player.alive) {
    player.move(delta, ATiles, ball.x, ball.y, ball.lethal);

    if (abs(player.x + player.w / 2 - (ball.x + ball.w / 2)) <= 120 &&
        abs(player.y + player.h / 2 - (ball.y + ball.h / 2)) <= 120) {
      player.inBallRange = true;
    } else {
      player.inBallRange = false;
    }

    if (BTiles[(player.x + player.w / 2) / 66][(player.y + player.h / 2) / 66]
                .damage > 0 ||
        ATiles[(player.x + player.w / 2) / 66][(player.y + player.h / 2) / 66]
                .damage > 0) {
      player.alive = false;
    }
  }

  if (localmultiplayer && player2.alive) {
    player2.move(delta, ATiles, ball.x, ball.y, ball.lethal);

    if (abs(player2.x + player2.w / 2 - (ball.x + ball.w / 2)) <= 120 &&
        abs(player2.y + player2.h / 2 - (ball.y + ball.h / 2)) <= 120) {
      // player2.clip_rect.x = 67;
    } else {
      player2.clip_rect.x = 0;
    }
  }

  // Tiles
  for (int i = 0; i < 25; i++) {
    for (int j = 0; j < 15; j++) {
      ATiles[i][j].update(delta);
      BTiles[i][j].update(delta);
    }
  }

  // UPGRADE STUFF
  if (!upgrade.active &&
      UpgradeSpawner.get_ticks() >
          10000) // if 10sec since last upgrade got taken
  {
    int ux = 0; // rand() % 25;
    int uy = rand() % 15;

    int x = 0;
    while (!ATiles[ux][uy].traversable || !BTiles[ux][uy].traversable) {
      ux = rand() % 25;
      uy = rand() % 15;
    }

    upgrade.set(ux * 66, uy * 66, 66, 66, 0, 0);
    int typerand = rand() % 2;

    if (typerand == 0) {
      upgrade.type = 's';
    } else {
      upgrade.type = 'p';
    }

    upgrade.active = true;
  }

  if (upgrade.active) {
    if (abs(player.x + player.w / 2 - (upgrade.x + upgrade.w / 2)) < 58 &&
        abs(player.y + player.h / 2 - (upgrade.y + upgrade.h / 2)) < 58) {
      upgrade.active = false;
      if (upgrade.type == 's') {
        if (player.speedup < player.speedupcap) {
          player.speedup++;
        };
      }
      if (upgrade.type == 'p') {
        if (player.powerup < player.powerupcap) {
          player.powerup++;
        };
      }
    }
    if (multiplayer || localmultiplayer) {
      if (abs(player2.x + player2.w / 2 - (upgrade.x + upgrade.w / 2)) < 58 &&
          abs(player2.y + player2.h / 2 - (upgrade.y + upgrade.h / 2)) < 58) {
        upgrade.active = false;
        if (upgrade.type = 's') {
          if (player2.speedup < player2.speedupcap) {
            player2.speedup++;
          };
        }
        if (upgrade.type = 'p') {
          if (player2.powerup < player2.powerupcap) {
            player2.powerup++;
          };
        }
      }
      if (playercount > 2) {
        if (abs(player3.x + player3.w / 2 - (upgrade.x + upgrade.w / 2)) < 58 &&
            abs(player3.y + player3.h / 2 - (upgrade.y + upgrade.h / 2)) < 58) {
          upgrade.active = false;
          if (upgrade.type = 's') {
            if (player3.speedup < player3.speedupcap) {
              player3.speedup++;
            };
          }
          if (upgrade.type = 'p') {
            if (player3.powerup < player3.powerupcap) {
              player3.powerup++;
            };
          }
        }
        if (playercount > 3) {
          if (abs(player4.x + player4.w / 2 - (upgrade.x + upgrade.w / 2)) <
                  58 &&
              abs(player4.y + player4.h / 2 - (upgrade.y + upgrade.h / 2)) <
                  58) {
            upgrade.active = false;
            if (upgrade.type = 's') {
              if (player4.speedup < player4.speedupcap) {
                player4.speedup++;
              };
            }
            if (upgrade.type = 'p') {
              if (player4.powerup < player4.powerupcap) {
                player4.powerup++;
              };
            }
          }
        }
      }
    }
    if (!upgrade.active) {
      UpgradeSpawner.start();
    }
  }

  // AI STUFF
  if ((!multiplayer && !localmultiplayer)) {
    if (ai.alive) {
      ai.update(&ball, player, delta, ATiles);
    }

    if (ball.lethal && checkCollision(ai.x, ai.y, ai.w, ai.h, ball.x, ball.y,
                                      ball.w, ball.h)) {
      ai.alive = false;
    }
  }

  // BALL STUFF (huehue)
  if (isHost) {
    ball.move(delta, ATiles);
    if (abs(ball.velx) > 10 || abs(ball.vely) > 10) {
      ball.lethal = true;
    }
  }

  // check for hit-by-ball
  if (ball.lethal && checkCollision(player.x, player.y, player.w, player.h,
                                    ball.x, ball.y, ball.w, ball.h)) {
    player.alive = false;
  }

  if (localmultiplayer) {
    if (ball.lethal &&
        checkCollision(player2.x, player2.y, player2.w, player2.h, ball.x,
                       ball.y, ball.w, ball.h)) {
      player2.alive = false;
    }
  }

  if (clickspamprevent0r.started && clickspamprevent0r.get_ticks() > 500) {
    clickspamprevent0r.stop();
  }
  if (localmultiplayer && clickspamprevent0r2.started &&
      clickspamprevent0r2.get_ticks() > 500) {
    clickspamprevent0r2.stop();
  }
}

void run() {
  while (inTitle == true) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        inTitle = false;
        running = false;
      }

      if (event.type == SDL_KEYDOWN) {
        if (!inputdone) {
          if (inputcounter == 0) {
            StrIn.handle_input(event);
            if (StrIn.rerenderpls) {
              text = TTF_RenderText_Solid(font, StrIn.str.c_str(), textColor);
              StrIn.rerenderpls = false;
            }
          } else if (inputcounter == 1) {
            StrInName.handle_input(event);
            if (StrInName.rerenderpls) {
              name =
                  TTF_RenderText_Solid(font, StrInName.str.c_str(), textColor);
              StrInName.rerenderpls = false;
            }
          }

          if (event.key.keysym.sym == SDLK_ESCAPE) {
            running = false;
            inTitle = false;
          }
          if (event.key.keysym.sym == SDLK_RETURN) {
            if (inputcounter == 0) {
              inputcounter++;
            } else if (inputcounter == 1) {
              if (StrIn.str.length() > 0) {
                SDLNet_ResolveHost(&address, StrIn.str.c_str(), 8134);
                connectionIP = StrIn.str;
                if (StrIn.str == "local") {
                  localmultiplayer = true;
                }
              }
              name = TTF_RenderText_Solid(fontSmall, StrInName.str.c_str(),
                                          textColor);
              inputdone = true;
            }
          }
          break;
        }
        switch (event.key.keysym.sym) {
        case SDLK_RETURN:
          inTitle = false;
          UpgradeSpawner.start();
          break;
        case SDLK_ESCAPE:
          if (multiplayer) {
            multiplayer = false;
            isHost = true;
            SDLNet_UDP_Close(serverSocket);
          } else {
            running = false;
            inTitle = false;
          }
          break;
        case SDLK_c:
          screen =
              SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
                               SDL_SWSURFACE | SDL_RESIZABLE | SDL_FULLSCREEN);
          break;
        case SDLK_v:
          screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
                                    SDL_SWSURFACE | SDL_RESIZABLE);
          break;

        case SDLK_h:
          isHost = true;
          multiplayer = true;
          setupServer(connectionIP);
          break;
        case SDLK_j:
          isHost = false;
          multiplayer = true;
          setupClient(connectionIP);
          draw();
          break;
        }
      }
    }

    if (multiplayer && !TCPConnectionDone) // TODO: Multithread dis shiz (or set
                                           // nonblocking..?)
    {
      if (isHost) {
        TCPClient = SDLNet_TCP_Accept(TCPServer);
        if (TCPClient) {
          if (playercount > 2) {
            while (TCPClient2 == NULL) {
              TCPClient2 = SDLNet_TCP_Accept(TCPServer);
            }
            if (playercount > 3) {
              while (TCPClient3 == NULL) {
                TCPClient3 = SDLNet_TCP_Accept(TCPServer);
              }
            }
          }
          string TCPmessage = "H" + StrInName.str;
          SDLNet_TCP_Send(TCPClient, TCPmessage.c_str(),
                          strlen(TCPmessage.c_str()) + 1);
          SDLNet_TCP_Recv(TCPClient, TCPBufferClt, 16);
          name2 = TTF_RenderText_Solid(fontSmall, TCPBufferClt, textColor);

          if (playercount > 2) {
            SDLNet_TCP_Send(TCPClient2, TCPmessage.c_str(),
                            strlen(TCPmessage.c_str()) + 1);
            if (playercount > 3) {
              SDLNet_TCP_Send(TCPClient3, TCPmessage.c_str(),
                              strlen(TCPmessage.c_str()) + 1);
            }
          }

          TCPConnectionDone = true;
          inTitle = false;

          // get IPAddress of clients and close TCPConnection

          cout << toDottedIP(SDLNet_TCP_GetPeerAddress(TCPClient)->host);

          // cout << toDottedIP(SDLNet_TCP_GetPeerAddress(TCPClient)->host);
          SDLNet_ResolveHost(
              &Client2Adr,
              toDottedIP(SDLNet_TCP_GetPeerAddress(TCPClient)->host).c_str(),
              8134);
          SDLNet_TCP_Close(TCPClient);

          if (playercount > 2) {
            SDLNet_ResolveHost(
                &Client3Adr,
                toDottedIP(SDLNet_TCP_GetPeerAddress(TCPClient2)->host).c_str(),
                8135);
            SDLNet_TCP_Close(TCPClient2);

            if (playercount > 3) {
              SDLNet_ResolveHost(
                  &Client4Adr,
                  toDottedIP(SDLNet_TCP_GetPeerAddress(TCPClient3)->host)
                      .c_str(),
                  8136);
              SDLNet_TCP_Close(TCPClient3);
            }
          }
        } else {
          fprintf(stderr, "SDLNet_TCP_Accept: %s\n", SDLNet_GetError());
        }
      } else {
        if ((TCPClient = SDLNet_TCP_Open(&address)) != NULL) {
          SDLNet_TCP_Recv(TCPClient, TCPBufferClt, 100);
          SDLNet_TCP_Send(TCPClient, StrInName.str.c_str(),
                          strlen(StrInName.str.c_str()) + 1);
          if (TCPBufferClt[0] == 'H') {
            TCPConnectionDone = true;
            inTitle = false;
            SDLNet_TCP_Close(TCPClient);
            char name2array[16];
            for (int i = 0; i < 16; i++) {
              name2array[i] = TCPBufferClt[i + 1];
            }
            name2 = TTF_RenderText_Solid(fontSmall, name2array, textColor);
          }
        } else {
          fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        }
      }
    }
    draw();
  }

  Timer delta;
  delta.start();

  while (running) {
    fps.start();

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_w:
          if (localmultiplayer) {
            player2.velcapy = -(10 + player2.speedup);
            break;
          }
        case SDLK_UP:
          player.velcapy = -(10 + player.speedup);
          break;
        case SDLK_s:
          if (localmultiplayer) {
            player2.velcapy = 10 + player2.speedup;
            break;
          }
        case SDLK_DOWN:
          player.velcapy = (10 + player.speedup);
          break;
        case SDLK_a:
          if (localmultiplayer) {
            player2.velcapx = -(10 + player2.speedup);
            break;
          }
        case SDLK_LEFT:
          player.velcapx = -(10 + player.speedup);
          break;
        case SDLK_d:
          if (localmultiplayer) {
            player2.velcapx = (10 + player2.speedup);
            break;
          }
        case SDLK_RIGHT:
          player.velcapx = (10 + player.speedup);
          break;

        case SDLK_SPACE:
          if (localmultiplayer) {
            if (!clickspamprevent0r2.started) {
              clickspamprevent0r2.start();
            }

            if (abs(player2.x + player2.w / 2 - (ball.x + ball.w / 2)) <= 120 &&
                abs(player2.y + player2.h / 2 - (ball.y + ball.h / 2)) <= 120 &&
                (clickspamprevent0r2.get_ticks() == 0 ||
                 clickspamprevent0r2.get_ticks() > 500)) // half a second cd
            {
              ball.getHit(player2);
            }
          }
          break;

        // reset game state for SP
        case SDLK_r:
          if (localmultiplayer) {
            player.set(100, 270, 66, 66, 0, 0);
            player.alive = true;
            player2.set(500, 270, 66, 66, 0, 0);
            player2.alive = true;
            ball.set(500, 500, 66, 66, 0, 0);
          } else if (!multiplayer) {
            player.set(100, 270, 66, 66, 0, 0);
            player.alive = true;
            ai.set(500, 270, 66, 66, 0, 0);
            ai.alive = true;
            ball.set(500, 500, 66, 66, 0, 0);
          }
          break;

        case SDLK_ESCAPE:
          running = false;
          break;
        case SDLK_c:
          screen =
              SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
                               SDL_SWSURFACE | SDL_RESIZABLE | SDL_FULLSCREEN);
          break;
        case SDLK_v:
          screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
                                    SDL_SWSURFACE | SDL_RESIZABLE);
          break;
        }
      }
      if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
        case SDLK_w:
          if (localmultiplayer) {
            if (player2.velcapy < 0) {
              player2.velcapy = 0;
            }
            break;
          }
        case SDLK_UP:
          if (player.velcapy < 0) {
            player.velcapy = 0;
          }
          break;
        case SDLK_s:
          if (localmultiplayer) {
            if (player2.velcapy > 0) {
              player2.velcapy = 0;
            }
            break;
          }
        case SDLK_DOWN:
          if (player.velcapy > 0) {
            player.velcapy = 0;
          }
          break;
        case SDLK_a:
          if (localmultiplayer) {
            if (player2.velcapx < 0) {
              player2.velcapx = 0;
            }
            break;
          }
        case SDLK_LEFT:
          if (player.velcapx < 0) {
            player.velcapx = 0;
          }
          break;
        case SDLK_d:
          if (localmultiplayer) {
            if (player2.velcapx > 0) {
              player2.velcapx = 0;
            }
            break;
          }
        case SDLK_RIGHT:
          if (player.velcapx > 0) {
            player.velcapx = 0;
          }
          break;
        }
      }
      if (event.type == SDL_MOUSEMOTION) {
        // Get the mouse offsets
        mousex = event.motion.x;
        mousey = event.motion.y;
      }
      // If a mouse button was pressed
      if (event.type == SDL_MOUSEBUTTONDOWN) {
        // If the left mouse button was pressed
        if (event.button.button == SDL_BUTTON_LEFT) {
          // Get the mouse offsets
          mousex = event.button.x;
          mousey = event.button.y;

          // implementing cd on clicking
          if (!clickspamprevent0r.started) {
            clickspamprevent0r.start();
          }

          // hit the ball if in range and not on cd
          if (abs(player.x + player.w / 2 - (ball.x + ball.w / 2)) <= 120 &&
              abs(player.y + player.h / 2 - (ball.y + ball.h / 2)) <= 120 &&
              (clickspamprevent0r.get_ticks() == 0 ||
               clickspamprevent0r.get_ticks() > 500)) // half a second cd
          {
            if (isHost) {
              ball.getHit(player);
            } else {
              if (playernumber == 2) {
                hitball = '2'; //(char) playernumber doesnt work T_T
              }
              if (playernumber == 3) {
                hitball = '3';
              }
              if (playernumber == 4) {
                hitball = '4';
              }
            }
          }
        }
      }
    }

    update(delta.get_ticks());
    delta.start(); // restart timer

    if (multiplayer) {
      handleMP();
    }

    draw();

    if (fps.get_ticks() < 1000 / FRAMES_PER_SECOND) {
      SDL_Delay((1000 / FRAMES_PER_SECOND) - fps.get_ticks());
    }
  }
}

int main(int argc, char *args[]) {
  init();

  running = true;
  run();
}