/////////////////////////////////////////////////////////
// Game Programming All In One, Third Edition
// Chapter 15 - Warbirds
/////////////////////////////////////////////////////////

#ifndef _WARBIRDS_H
#define _WARBIRDS_H

#include <kos.h>
#include <stdlib.h>
#include <kmg/kmg.h>
#include "DCMappy.h"

#define MAX_ENEMIES 20
#define MAX_BULLETS 20
#define MAX_EXPLOSIONS 10
#define BOTTOM 47520

//define the sprite structure
typedef struct SPRITE
{
    int dir, alive;
    int x,y;
    int width,height;
    int xspeed,yspeed;
    int xdelay,ydelay;
    int xcount,ycount;
    int curframe,maxframe,animdir;
    int framecount,framedelay;
}SPRITE;

typedef struct BITMAP {
    pvr_ptr_t image_data;
    uint32 width;
    uint32 height;
}BITMAP;

void draw_sprite(pvr_ptr_t image, float x, float y);

//y offset in pixels 
int yoffset = BOTTOM;

//player variables
int firecount = 0;
int firedelay = 60;
int health = 25;
int score = 0;

//timer variables
volatile int counter;
volatile int ticks;
volatile int framerate;

//bitmaps and sprites
BITMAP *explosion_images[6];
SPRITE *explosions[MAX_EXPLOSIONS];
BITMAP *bigexp_images[7];
SPRITE *bigexp;
BITMAP *player_images[3];
SPRITE *player;
BITMAP *bullet_images[3];
SPRITE *bullets[MAX_BULLETS];
BITMAP *enemy_plane_images[3];
SPRITE *enemy_planes[MAX_ENEMIES];
BITMAP *bonus_shot_image;
SPRITE *bonus_shot;

#endif

