#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_ttf.h>
#include <Math.h>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cstring>
#include <iostream>
#include <array>
#include "./Utility.h"
#include "./Actors.h"
#include "./Timer.h"
#include "./Tile.h"
#include "./StringInput.h"

using namespace std;

bool init_stage();
bool init_textures();
bool init_settings();

const int SCREEN_WIDTH = 1600; // 25 tiles
const int SCREEN_HEIGHT = 960; // 15 tiles
const int SCREEN_BPP = 32;
const int DTS = 64; // default tile size
const int TILE_COUNT_X = SCREEN_WIDTH / DTS;
const int TILE_COUNT_Y = SCREEN_HEIGHT / DTS;

const int GROUND_TILE = 0;
const int SPIKES_TILE = 1;
const int WALL_TILE = 2;
const int FALLING_TILE = 3;

const int EVENT_TILE = 0;

const string map_path = "Map.txt";
const string settings_path = "Settings.txt";

const int FRAMES_PER_SECOND = 60;

float drawscale;

int mousex;
int mousey;

fstream map;

EventTile ATiles[TILE_COUNT_X][TILE_COUNT_Y]; // background
EventTile BTiles[TILE_COUNT_X][TILE_COUNT_Y]; // things that lie on the ground

bool inTitle = true;

// TCPWrapper connection
TCPsocket TCPServer, TCPClient = NULL, TCPClient2 = NULL, TCPClient3 = NULL;
IPaddress Client2Adr, Client3Adr, Client4Adr;
char TCPBufferClt[100];
IPaddress TCPip;
bool TCPConnectionDone = false;
int playercount;
int playernumber = 1; // your number (host is 1)

UDPsocket clientSocket;
UDPsocket serverSocket;
UDPpacket *sendPac;
UDPpacket *recPac;

char hitball = '0';
Timer hitStackPreventor;

stringstream sstream;
string connectionIP;
string desiredScreenWidthString;
int desiredScreenWidth;

IPaddress address;
IPaddress UDPaddress;

bool isHost = true;
bool multiplayer = false;

bool localmultiplayer = false;

Timer fps;
Timer totaltime;
int deltaT; // time since last frame

Timer clickspamprevent0r;
Timer clickspamprevent0r2;
int ball_hit_cooldown = 500;

Timer UpgradeSpawner;
Upgrade upgrade;

TTF_Font *font = NULL;
TTF_Font *fontSmall = NULL;
SDL_Color textColor = {30, 30, 30};
SDL_Color bgcolor = {0, 0, 0};

SDL_Texture *textures[10];

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Texture *TitleScreen = NULL;
SDL_Texture *HostingScreen = NULL;
SDL_Texture *SettingsBG = NULL;
SDL_Texture *ClientScreen = NULL;

SDL_Texture *selection = NULL;

SDL_Texture *ATile_ground = NULL;
SDL_Texture *BTile_Spikes = NULL;
SDL_Texture *BTile_Wall = NULL;

SDL_Texture *SpeedUp = NULL;
SDL_Texture *PowerUp = NULL;

SDL_Texture *HealthBar = NULL;
SDL_Texture *HealthBarFrame = NULL;
SDL_Texture *FallingTile = NULL;

SDL_Texture *Borders = NULL;

SDL_Texture *TEST = NULL;
SDL_Texture *TEST2 = NULL;

SDL_Texture *PlayerSprite = NULL;
SDL_Texture *PlayerDead = NULL;

SDL_Texture *BallLethal = NULL;
SDL_Texture *BallNonLethal = NULL;

SDL_Rect healthrect;

StringInput StrIn;
StringInput StrInName;
int inputcounter = 0;
bool inputdone = false;
SDL_Texture *text = NULL;
SDL_Texture *name = NULL;
SDL_Texture *name2 = NULL;

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

bool init() {
    bool success = true;

    if (!init_settings()) {
        cout << "Failed to init settings";
        success = false;
    }

    drawscale = (float)desiredScreenWidth / SCREEN_WIDTH;
    cout << "drawscale: " << drawscale;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    }

    SDLNet_Init();

    // server, used for receiving
    recPac = SDLNet_AllocPacket(512);
    sendPac = SDLNet_AllocPacket(512);

    // client, used for sending
    clientSocket = SDLNet_UDP_Open(0);

    SDLNet_ResolveHost(&TCPip, NULL, 8134);
    SDLNet_ResolveHost(&address, connectionIP.c_str(), 8134); // address, hostIPString, (server's)port
    TCPServer = SDLNet_TCP_Open(&TCPip);

    font = TTF_OpenFont("res/Demonized.ttf", 28);
    fontSmall = TTF_OpenFont("res/Demonized.ttf", 12);

    //Set texture filtering to linear
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
        printf("Warning: Linear texture filtering not enabled!");
    }

    window = SDL_CreateWindow("Hammerball", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * drawscale, SCREEN_HEIGHT * drawscale, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        success = false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }

    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);

    init_textures();

    player.set(100, 270, DTS, DTS, 0, 0, PlayerSprite, PlayerDead);
    player2.set(1430, 270, DTS, DTS, 0, 0, PlayerSprite, PlayerDead);
    player3.set(100, 600, DTS, DTS, 0, 0, PlayerSprite, PlayerDead);
    player4.set(1430, 600, DTS, DTS, 0, 0, PlayerSprite, PlayerDead);

    ai.set(1430, 270, DTS, DTS, 0, 0, PlayerSprite, PlayerDead);

    ball.set(730, 500, DTS, DTS, 0, 0, BallNonLethal, BallLethal);

    healthrect.x = 0;
    healthrect.y = 0;
    healthrect.w = 105;
    healthrect.h = 27;

    // Stage1music = Mix_LoadMUS("res/MysteriousSpace.wav");

    if (!init_stage()) {
        cout << "Failed to init stage";
        success = false;
    }

    return success;
}

void apply_settings(fstream *settings) {
    settings->ignore(256, '\n');
    getline(*settings, desiredScreenWidthString);
    cout << desiredScreenWidthString;
    desiredScreenWidth = atoi(desiredScreenWidthString.c_str());

    settings->ignore(256, '\n');
    getline(*settings, connectionIP);

    string tempplayercount;
    getline(*settings, tempplayercount);
    playercount = atoi(tempplayercount.c_str());
    playercount = min(playercount, 4);

    string tempplayernumber;
    getline(*settings, tempplayernumber);
    playernumber = atoi(tempplayernumber.c_str());
}

void create_settings(fstream *settings) {
    settings->open(settings_path, ios::out);
    *settings << "ScreenWidth(Scale always 16:10):"
              << "\n"
              << "1600"
              << "\n"
              << "IP:"
              << "\n"
              << "localhost"
              << "\n"
              << "2"
              << "\n"
              << "1"
              << "\n";
    settings->close();
}

bool init_settings() {
    fstream settings;
    settings.open(settings_path, ios::in);
    if (settings.is_open()) {
        apply_settings(&settings);
    } else {
        cout << settings_path << " didnt exist and was created\n";
        create_settings(&settings);
        settings.open(settings_path, ios::in);
        if (settings.is_open()) {
            apply_settings(&settings);
        } else {
            cout << "failed to open settings after creating it.\n";
            return false;
        }
    }

    return true;
}

void create_map() {
    map.open(map_path, ios::out);
    for (int i = 0; i < TILE_COUNT_X; i++) {
        for (int j = 0; j < TILE_COUNT_Y; j++) {
            if ((i == 6 || i == 18) && j > 4 && j < 11) {
                map << "-1,0," << WALL_TILE << ",0,0,0,1,0,0,0 ";
            } else if (j == 0 || j == 14 || i == 0 || i == 24) {
                map << "-1,0," << WALL_TILE << ",0,0,0,1,0,0,0 ";
            } else if (j == 12 && i == 12) {
                map << "8,1," << FALLING_TILE
                    << ",9,100,1,1,2000,3000,2500 ";
            } else {
                map << "-1,0," << GROUND_TILE << ",0,0,1,1,0,0,0 ";
            }
            if (j == 14) {
                map << "\n";
            }
        }
    }
    map.close();
}

bool init_textures() {
    ATile_ground = load_texture("res/ground.png", renderer);
    BTile_Wall = load_texture("res/Wall.png", renderer);
    FallingTile = load_texture("res/falltile.png", renderer);

    PlayerSprite = load_texture("res/Ironman.png", renderer);
    PlayerDead = load_texture("res/PlayerDead.png", renderer);

    TitleScreen = load_texture("res/Title.png", renderer);
    HostingScreen = load_texture("res/Hosting.png", renderer);
    ClientScreen = load_texture("res/Client.png", renderer);
    SettingsBG = load_texture("res/Settings.png", renderer);

    SpeedUp = load_texture("res/SpeedUp.png", renderer);
    PowerUp = load_texture("res/PowerUp.png", renderer);

    HealthBar = load_texture("res/healthbar.png", renderer);
    HealthBarFrame = load_texture("res/HealthBarFrame.png", renderer);

    BallLethal = load_texture("res/Ball.png", renderer);
    BallNonLethal = load_texture("res/Ballnonlethal.png", renderer);

    Borders = load_texture("res/Borders.png", renderer);

    TEST = load_texture("res/asphaltdecal.png", renderer);
    TEST2 = load_texture("res/asphaltdecal2.png", renderer);

    string xxy = " ";
    text = load_from_rendered_text(font, xxy, textColor, renderer);
    name = load_from_rendered_text(font, xxy, textColor, renderer);
    name2 = load_from_rendered_text(fontSmall, xxy, textColor, renderer);

    textures[0] = ATile_ground;
    textures[1] = BTile_Spikes;
    textures[2] = BTile_Wall;
    textures[3] = FallingTile;

    return true;
}

bool init_stage() {
    map.open(map_path, ios::in);
    if (!map.is_open()) {
        cout << map_path << " didn't exist and was created\n";
        create_map();
        map.open(map_path, ios::in);
        if (!map.is_open()) {
            cout << "failed to open map after creating it.\n";
            return false;
        }
    }

    srand((unsigned)time(0));
    int rnd;

    string mapContent;
    for (int i = 0; i < TILE_COUNT_X; i++) {
        for (int j = 0; j < TILE_COUNT_Y; j++) {
            rnd = (rand() % 3);
            ATiles[i][j].clip_rect.x = rnd * DTS + 1;
            if (ATiles[i][j].clip_rect.x == 1) {
                ATiles[i][j].clip_rect.x = 0;
            }
            ATiles[i][j].clip_rect.y = 0;
            map >> mapContent;

            int *mpc = charAToIntA(separateString(mapContent, 10, ","), 10);

            ATiles[i][j].setE(i * DTS, j * DTS, DTS, DTS, mpc[0], mpc[1], mpc[2], mpc[3], mpc[4], mpc[5] != 0, mpc[6] != 0, mpc[7], mpc[8], mpc[9]);

            if (ATiles[i][j].img == FALLING_TILE) {
                ATiles[i][j].generateTime();
                ATiles[i][j].clip_rect.x = 0;
            }

            mapContent.clear();

            BTiles[i][j].setE(i * DTS, j * DTS, DTS, DTS, -1, 0, -1, 0, 1, true, true, 0, 0, 0);
            BTiles[i][j].clip_rect.x = 0;
            BTiles[i][j].clip_rect.y = 0;
        }
    }
    map.close();

    ATiles[11][12].setE(11 * DTS, 12 * DTS, DTS, DTS, 8, 1, FALLING_TILE, 9, 100, true, true, 2000, 3000, 2500);
    ATiles[11][12].generateTime();
    ATiles[11][12].clip_rect.x = 0;

    ATiles[11][11].setE(11 * DTS, 11 * DTS, DTS, DTS, 8, 1, FALLING_TILE, 9, 250, true, true, 2000, 3000, 2500);
    ATiles[11][11].generateTime();
    ATiles[11][11].clip_rect.x = 0;

    return true;
}

void draw_multiplayer_title() {
    if (isHost) {
        apply_surface(drawscale, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, HostingScreen, renderer);
    } else {
        apply_surface(drawscale, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ClientScreen, renderer);
    }
}

void draw_singleplayer_title() {
    if (!inputdone) {
        apply_surface(drawscale, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SettingsBG, renderer);

        int textW, textH;
        SDL_QueryTexture(text, NULL, NULL, &textW, &textH);
        int nameW, nameH;
        SDL_QueryTexture(name, NULL, NULL, &nameW, &nameH);
        apply_surface(drawscale, 600, 200, textW, textH, text, renderer);
        apply_surface(drawscale, 600, 680, nameW, nameH, name, renderer);
    } else {
        apply_surface(drawscale, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, TitleScreen, renderer);
    }
}

void draw_title() {
    if (multiplayer) {
        draw_multiplayer_title();
    } else {
        draw_singleplayer_title();
    }
}

void draw_tiles() {
    for (int i = 0; i < TILE_COUNT_X; i++) {
        for (int j = 0; j < TILE_COUNT_Y; j++) {
            if (ATiles[i][j].active) {
                apply_surface(drawscale, ATiles[i][j].x, ATiles[i][j].y, DTS, DTS, textures[ATiles[i][j].img], renderer, &ATiles[i][j].clip_rect);
            }
            if (ATiles[i][j].needsBorder) { // check if borders from top, left, right or bottom need to be drawn
                if (!ATiles[i - 1][j].needsBorder) {
                    apply_surface(drawscale, ATiles[i][j].x, ATiles[i][j].y, DTS, DTS, Borders, renderer, 0, 0, DTS, DTS);
                }
                if (!ATiles[i + 1][j].needsBorder) {
                    apply_surface(drawscale, ATiles[i][j].x, ATiles[i][j].y, DTS, DTS, Borders, renderer, 67, 0, DTS, DTS);
                }
                if (!ATiles[i][j - 1].needsBorder) {
                    apply_surface(drawscale, ATiles[i][j].x, ATiles[i][j].y, DTS, DTS, Borders, renderer, 134, 0, DTS, DTS);
                }
                if (!ATiles[i][j + 1].needsBorder) {
                    apply_surface(drawscale, ATiles[i][j].x, ATiles[i][j].y, DTS, DTS, Borders, renderer, 201, 0, DTS, DTS);
                }
            }
            if (BTiles[i][j].active) {
                apply_surface(drawscale, BTiles[i][j].x, BTiles[i][j].y, DTS, DTS, textures[BTiles[i][j].img], renderer, &BTiles[i][j].clip_rect);
            }
        }
    }
}

void draw_ui() {
    int nameW, nameH;
    SDL_QueryTexture(name, NULL, NULL, &nameW, &nameH);
    int name2W, name2H;
    SDL_QueryTexture(name2, NULL, NULL, &name2W, &name2H);
    apply_surface(drawscale, player.x, player.y - 25, nameW, nameH, name, renderer);
    apply_surface(drawscale, player2.x, player2.y - 25, name2W, name2H, name2, renderer);

    apply_surface(drawscale, SCREEN_WIDTH - 132, 0, 109, 32, HealthBarFrame, renderer);
    healthrect.w = 105 - (100 - player.health);
    apply_surface(drawscale, SCREEN_WIDTH - 132, 0, 109, 32, HealthBar, renderer, &healthrect);
}

void draw_active_game() {
    draw_tiles();

    upgrade.draw(drawscale, renderer);

    if (!multiplayer && !localmultiplayer) {
        ai.draw(drawscale, renderer);
    }

    player.draw(drawscale, renderer);

    if (multiplayer || localmultiplayer) {
        player2.draw(drawscale, renderer);
        if (playercount > 2 && player3.alive) {
            player3.draw(drawscale, renderer);
            if (playercount > 3 && player4.alive) {
                player4.draw(drawscale, renderer);
            }
        }
    }
    ball.draw(drawscale, renderer);

    draw_ui();
}

void draw() {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    if (inTitle) {
        draw_title();
    } else {
        draw_active_game();
    }

    SDL_RenderPresent(renderer);
}

void handleMP() {
    if (isHost) {
        if (SDLNet_TCP_Recv(TCPClient, TCPBufferClt, 100) > 0) {
            char *temp[16];

            temp[0] = strtok((char *)TCPBufferClt, ",");
            temp[1] = strtok(NULL, ",");
            temp[2] = strtok(NULL, ",");
            temp[3] = strtok(NULL, ",");
            temp[4] = strtok(NULL, ",");

            if (temp[0][0] == '2') {
                player2.x = atoi(temp[1]);
                player2.y = atoi(temp[2]);
                player2.alive = atoi(temp[4]);

                if (!hitStackPreventor.is_started()) {
                    if (temp[3][0] == '2') {
                        ball.getHit(player2);
                        hitball = '1';
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
                    hitStackPreventor.get_ticks() > ball_hit_cooldown) {
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
        sstream.str("");
        sstream << player.x << "," << player.y << "," << ball.x << "," << ball.y
                << "," << islethal << "," << hitball << "," << player2.x << ","
                << player2.y << "," << player3.x << "," << player3.y << ","
                << player.alive << "\0";
        hitball = '0';
        string result = sstream.str();

        SDLNet_TCP_Send(TCPClient, result.c_str(), strlen(result.c_str()) + 1);
    } else {
        // SEND
        sstream.str("");
        sstream << playernumber << "," << player.x << "," << player.y << "," << hitball << "," << player.alive << "\0";
        string result = sstream.str();

        if (SDLNet_TCP_Send(TCPClient, result.c_str(), strlen(result.c_str()) + 1) < 1) {
            printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
        }

        if (SDLNet_TCP_Recv(TCPClient, TCPBufferClt, 100) > 0) {
            char *temp[12];

            temp[0] = strtok((char *)TCPBufferClt, ",");
            temp[1] = strtok(NULL, ",");
            temp[2] = strtok(NULL, ",");
            temp[3] = strtok(NULL, ",");
            temp[4] = strtok(NULL, ",");
            temp[5] = strtok(NULL, ",");
            temp[6] = strtok(NULL, ",");
            temp[7] = strtok(NULL, ",");
            temp[8] = strtok(NULL, ",");
            temp[9] = strtok(NULL, ",");
            temp[10] = strtok(NULL, ",");

            if (playercount > 2) {
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
            player2.alive = atoi(temp[10]);

            ball.x = atoi(temp[2]);
            ball.y = atoi(temp[3]);

            if (temp[4][0] == '1') {
                ball.lethal = true;
            } else {
                ball.lethal = false;
            }

            if (temp[5][0] == '1') {
                hitball = '0';
            }
        }
    }
}

void update_local_player(Uint32 delta) {
    if (player.alive) {
        player.move(delta, ATiles, ball.x, ball.y, ball.lethal);

        if (abs(player.x + player.w / 2 - (ball.x + ball.w / 2)) <= 120 && abs(player.y + player.h / 2 - (ball.y + ball.h / 2)) <= 120) {
            player.inBallRange = true;
        } else {
            player.inBallRange = false;
        }

        if (BTiles[(player.x + player.w / 2) / DTS][(player.y + player.h / 2) / DTS].damage > 0 || ATiles[(player.x + player.w / 2) / DTS][(player.y + player.h / 2) / DTS].damage > 0) {
            player.alive = false;
        }
    }
}

void update_multiplayer_players(Uint32 delta) {
    if (localmultiplayer && player2.alive) {
        player2.move(delta, ATiles, ball.x, ball.y, ball.lethal);

        if (abs(player2.x + player2.w / 2 - (ball.x + ball.w / 2)) <= 120 && abs(player2.y + player2.h / 2 - (ball.y + ball.h / 2)) <= 120) {
            // player2.clip_rect.x = 67;
        } else {
            player2.clip_rect.x = 0;
        }
    }
}

void update_tiles(Uint32 delta) {
    for (int x = 0; x < TILE_COUNT_X; x++) {
        for (int y = 0; y < TILE_COUNT_Y; y++) {
            ATiles[x][y].update(delta);
            BTiles[x][y].update(delta);
        }
    }
}

void update_upgrade() {
    if (!upgrade.active && UpgradeSpawner.get_ticks() > 10000) {
        int ux = 0; // rand() % TILE_COUNT_X;
        int uy = rand() % TILE_COUNT_Y;

        int x = 0;
        while (!ATiles[ux][uy].traversable || !BTiles[ux][uy].traversable) {
            ux = rand() % TILE_COUNT_X;
            uy = rand() % TILE_COUNT_Y;
        }

        upgrade.set(ux * DTS, uy * DTS, DTS, DTS, 0, 0);
        int typerand = rand() % 2;

        if (typerand == 0) {
            upgrade.type = 's';
            upgrade.texture = SpeedUp;
        } else {
            upgrade.type = 'p';
            upgrade.texture = PowerUp;
        }

        upgrade.active = true;
    }

    if (upgrade.active) {
        if (abs(player.x + player.w / 2 - (upgrade.x + upgrade.w / 2)) < 58 && abs(player.y + player.h / 2 - (upgrade.y + upgrade.h / 2)) < 58) {
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
            if (abs(player2.x + player2.w / 2 - (upgrade.x + upgrade.w / 2)) < 58 && abs(player2.y + player2.h / 2 - (upgrade.y + upgrade.h / 2)) < 58) {
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
                if (abs(player3.x + player3.w / 2 - (upgrade.x + upgrade.w / 2)) < 58 && abs(player3.y + player3.h / 2 - (upgrade.y + upgrade.h / 2)) < 58) {
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
                    if (abs(player4.x + player4.w / 2 - (upgrade.x + upgrade.w / 2)) < 58 && abs(player4.y + player4.h / 2 - (upgrade.y + upgrade.h / 2)) < 58) {
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
}

void update_ai(Uint32 delta) {
    if ((!multiplayer && !localmultiplayer)) {
        if (ai.alive) {
            ai.update(&ball, player, delta, ATiles);
        }

        if (ball.lethal && checkCollision(ai.x, ai.y, ai.w, ai.h, ball.x, ball.y, ball.w, ball.h)) {
            ai.alive = false;
        }
    }
}

bool is_player_hit(Player &player, Ball &ball) {
    return ball.lethal && checkCollision(player.x, player.y, player.w, player.h, ball.x, ball.y, ball.w, ball.h);
}

void update_ball(Uint32 delta) {
    if (isHost) {
        ball.move(delta, ATiles);
        if (abs(ball.velx) > 10 || abs(ball.vely) > 10) {
            ball.lethal = true;
        }
    }

    if (is_player_hit(player, ball)) {
        player.alive = false;
    }

    if (localmultiplayer) {
        if (ball.lethal && checkCollision(player2.x, player2.y, player2.w, player2.h, ball.x, ball.y, ball.w, ball.h)) {
            player2.alive = false;
        }
    }
}

void update_clickspamprevent0r() {
    if (clickspamprevent0r.started && clickspamprevent0r.get_ticks() > ball_hit_cooldown) {
        clickspamprevent0r.stop();
    }
    if (localmultiplayer && clickspamprevent0r2.started && clickspamprevent0r2.get_ticks() > ball_hit_cooldown) {
        clickspamprevent0r2.stop();
    }
}

void update(Uint32 delta) {
    update_local_player(delta);
    update_multiplayer_players(delta);
    update_tiles(delta);
    update_ai(delta);
    update_ball(delta);
    update_upgrade();
    update_clickspamprevent0r();
}

bool can_hit_ball(Player &player, Ball &ball, Timer &clickspamprevent0r) {
    return abs(player.x + player.w / 2 - (ball.x + ball.w / 2)) <= 120 &&
           abs(player.y + player.h / 2 - (ball.y + ball.h / 2)) <= 120 &&
           (clickspamprevent0r.get_ticks() == 0 || clickspamprevent0r.get_ticks() > ball_hit_cooldown);
}

void process_input() {
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

                    if (abs(player2.x + player2.w / 2 - (ball.x + ball.w / 2)) <= 120 && abs(player2.y + player2.h / 2 - (ball.y + ball.h / 2)) <= 120 &&
                        (clickspamprevent0r2.get_ticks() == 0 || clickspamprevent0r2.get_ticks() > ball_hit_cooldown)) { // half a second cd
                        ball.getHit(player2);
                    }
                }
                break;

            // reset game state for SP
            case SDLK_r:
                if (localmultiplayer) {
                    player.set(100, 270, DTS, DTS, 0, 0);
                    player.alive = true;
                    player2.set(1430, 270, DTS, DTS, 0, 0);
                    player2.alive = true;
                    ball.set(730, 500, DTS, DTS, 0, 0);
                } else if (!multiplayer) {
                    player.set(100, 270, DTS, DTS, 0, 0);
                    player.alive = true;
                    ai.set(1430, 270, DTS, DTS, 0, 0);
                    ai.alive = true;
                    ball.set(730, 500, DTS, DTS, 0, 0);
                }
                break;

            case SDLK_ESCAPE:
                running = false;
                break;

            case SDLK_c:
                SDL_SetWindowFullscreen(window, SDL_FALSE);
                break;
            case SDLK_v:
                SDL_SetWindowFullscreen(window, SDL_TRUE);
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
            mousex = event.motion.x;
            mousey = event.motion.y;
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                mousex = event.button.x;
                mousey = event.button.y;

                if (!clickspamprevent0r.started) {
                    clickspamprevent0r.start();
                }

                if (can_hit_ball(player, ball, clickspamprevent0r)) {
                    if (isHost) {
                        ball.getHit(player);
                    } else {
                        if (playernumber == 2) {
                            hitball = '2';
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
}

void setup_multiplayer() {
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
            SDLNet_TCP_Send(TCPClient, TCPmessage.c_str(), strlen(TCPmessage.c_str()) + 1);
            SDLNet_TCP_Recv(TCPClient, TCPBufferClt, 16);
            name2 = load_from_rendered_text(fontSmall, TCPBufferClt, textColor, renderer);

            if (playercount > 2) {
                SDLNet_TCP_Send(TCPClient2, TCPmessage.c_str(), strlen(TCPmessage.c_str()) + 1);
                if (playercount > 3) {
                    SDLNet_TCP_Send(TCPClient3, TCPmessage.c_str(), strlen(TCPmessage.c_str()) + 1);
                }
            }

            TCPConnectionDone = true;
            inTitle = false;
        }
    } else {
        if ((TCPClient = SDLNet_TCP_Open(&address)) != NULL) {
            SDLNet_TCP_Recv(TCPClient, TCPBufferClt, 100);
            SDLNet_TCP_Send(TCPClient, StrInName.str.c_str(), strlen(StrInName.str.c_str()) + 1);
            if (TCPBufferClt[0] == 'H') {
                TCPConnectionDone = true;
                inTitle = false;
                char name2array[16];
                for (int i = 0; i < 16; i++) {
                    name2array[i] = TCPBufferClt[i + 1];
                }
                name2 = load_from_rendered_text(fontSmall, name2array, textColor, renderer);
            }
        } else {
            fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        }
    }
}

void process_title_input() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            inTitle = false;
            running = false;
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            SDL_Rect new_viewport = {0, 0, event.window.data1, event.window.data2};
            SDL_RenderSetLogicalSize(renderer, event.window.data1, event.window.data2);
        }

        if (event.type == SDL_KEYDOWN || event.type == SDL_TEXTINPUT) {
            if (!inputdone) {
                if (inputcounter == 0) {
                    StrIn.handle_input(event);
                    if (StrIn.rerenderpls) {
                        text = load_from_rendered_text(font, StrIn.str, textColor, renderer);
                        StrIn.rerenderpls = false;
                    }
                } else if (inputcounter == 1) {
                    StrInName.handle_input(event);
                    if (StrInName.rerenderpls) {
                        name = load_from_rendered_text(font, StrInName.str, textColor, renderer);
                        StrInName.rerenderpls = false;
                    }
                }

                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_StopTextInput();
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
                        name = load_from_rendered_text(fontSmall, StrInName.str, textColor, renderer);
                        inputdone = true;
                        SDL_StopTextInput();
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
                SDL_SetWindowFullscreen(window, SDL_FALSE);
                break;
            case SDLK_v:
                SDL_SetWindowFullscreen(window, SDL_TRUE);
                break;

            case SDLK_h:
                isHost = true;
                multiplayer = true;
                break;
            case SDLK_j:
                isHost = false;
                multiplayer = true;
                draw();
                break;
            }
        }
    }
}

void update_title() {
    process_title_input();

    if (multiplayer && !TCPConnectionDone) {
        setup_multiplayer();
    }

    draw();
}

void run() {
    SDL_StartTextInput();

    while (inTitle == true) {
        update_title();
    }

    Timer delta;
    delta.start();

    while (running) {
        fps.start();

        process_input();

        update(delta.get_ticks());
        delta.start();

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

    return 0;
}