#include <kos.h>
#include "DCMappy.h"
#include "warbirds.h"
#include "font.h"
#include <math.h>

DCMappy *Map;	

extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);
KOS_INIT_ROMDISK(romdisk);

static int done = 0;


void loadsprites(void)
{    
    int n;
    kos_img_t      * KTemp = NULL;
    pvr_ptr_t      PTemp = NULL;
    
    KTemp = (kos_img_t *)malloc(sizeof(kos_img_t));
    
    //load bonus shot
    kmg_to_img("/rd/bonusshot.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    bonus_shot_image = (BITMAP*)malloc(sizeof(BITMAP));
    bonus_shot_image->image_data = PTemp;
    bonus_shot_image->width = KTemp->w;
    bonus_shot_image->height = KTemp->h;
    
    bonus_shot = (SPRITE*)malloc(sizeof(SPRITE));
    bonus_shot->alive=0;
    bonus_shot->x = 0;
    bonus_shot->y = 0;
    bonus_shot->width = KTemp->w;
    bonus_shot->height = KTemp->h;
    bonus_shot->xdelay = 0;
    bonus_shot->ydelay = 2;
    bonus_shot->xcount = 0;
    bonus_shot->ycount = 0;
    bonus_shot->xspeed = 0;
    bonus_shot->yspeed = 5;
    bonus_shot->curframe = 0;
    bonus_shot->maxframe = 0;
    bonus_shot->framecount = 0;
    bonus_shot->framedelay = 0;
    
    kos_img_free(KTemp, 0);
    PTemp = NULL;

    // Player image 1
    kmg_to_img("/rd/p381.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[0] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[0]->image_data = PTemp;
    player_images[0]->width = KTemp->w;
    player_images[0]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Player image 2
    kmg_to_img("/rd/p382.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[1] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[1]->image_data = PTemp;
    player_images[1]->width = KTemp->w;
    player_images[1]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Player image 3
    kmg_to_img("/rd/p383.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[2] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[2]->image_data = PTemp;
    player_images[2]->width = KTemp->w;
    player_images[2]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;

    //initialize the player's sprite
    player = (SPRITE*)malloc(sizeof(SPRITE));
    player->x = 320-32;
    player->y = 400;
    player->width = KTemp->w;
    player->height = KTemp->h;
    player->xdelay = 1;
    player->ydelay = 0;
    player->xcount = 0;
    player->ycount = 0;
    player->xspeed = 0;
    player->yspeed = 0;
    player->curframe = 0;
    player->maxframe = 2;
    player->framecount = 0;
    player->framedelay = 10;
    player->animdir = 1;

    //load bullet images
    kmg_to_img("/rd/bullets.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    bullet_images[0] = (BITMAP*)malloc(sizeof(BITMAP));
    bullet_images[0]->image_data = PTemp;
    bullet_images[0]->width = KTemp->w;
    bullet_images[0]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;

    //initialize the bullet sprites
    for (n=0; n<MAX_BULLETS; n++)
    {
        bullets[n] = (SPRITE*)malloc(sizeof(SPRITE));
        bullets[n]->alive = 0;
        bullets[n]->x = 0;
        bullets[n]->y = 0;
        bullets[n]->width = KTemp->w;
        bullets[n]->height = KTemp->h;
        bullets[n]->xdelay = 0;
        bullets[n]->ydelay = 0;
        bullets[n]->xcount = 0;
        bullets[n]->ycount = 0;
        bullets[n]->xspeed = 0;
        bullets[n]->yspeed = -8;
        bullets[n]->curframe = 0;
        bullets[n]->maxframe = 0;
        bullets[n]->framecount = 0;
        bullets[n]->framedelay = 0;
        bullets[n]->animdir = 0;
    }

    // Enemy image 1
    kmg_to_img("/rd/enemyplane1.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    enemy_plane_images[0] = (BITMAP*)malloc(sizeof(BITMAP));
    enemy_plane_images[0]->image_data = PTemp;
    enemy_plane_images[0]->width = KTemp->w;
    enemy_plane_images[0]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Enemy image 2
    kmg_to_img("/rd/enemyplane2.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    enemy_plane_images[1] = (BITMAP*)malloc(sizeof(BITMAP));
    enemy_plane_images[1]->image_data = PTemp;
    enemy_plane_images[1]->width = KTemp->w;
    enemy_plane_images[1]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Enemy image 3
    kmg_to_img("/rd/enemyplane3.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    enemy_plane_images[2] = (BITMAP*)malloc(sizeof(BITMAP));
    enemy_plane_images[2]->image_data = PTemp;
    enemy_plane_images[2]->width = KTemp->w;
    enemy_plane_images[2]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    //initialize the enemy planes
    for (n=0; n<MAX_ENEMIES; n++)
    {
        enemy_planes[n] = (SPRITE*)malloc(sizeof(SPRITE));
        enemy_planes[n]->alive = 0;
        enemy_planes[n]->x = rand() % 100 + 50;
        enemy_planes[n]->y = 0;
        enemy_planes[n]->width = KTemp->w;
        enemy_planes[n]->height = KTemp->h;
        enemy_planes[n]->xdelay = 2;
        enemy_planes[n]->ydelay = 1;
        enemy_planes[n]->xcount = 0;
        enemy_planes[n]->ycount = 0;
        enemy_planes[n]->xspeed = (rand() % 2 - 3);
        enemy_planes[n]->yspeed = 1;
        enemy_planes[n]->curframe = 0;
        enemy_planes[n]->maxframe = 2;
        enemy_planes[n]->framecount = 0;
        enemy_planes[n]->framedelay = 10;
        enemy_planes[n]->animdir = 1;
    }
    
    // Explosion image 1
    kmg_to_img("/rd/explosion1.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    explosion_images[0] = (BITMAP*)malloc(sizeof(BITMAP));
    explosion_images[0]->image_data = PTemp;
    explosion_images[0]->width = KTemp->w;
    explosion_images[0]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 2
    kmg_to_img("/rd/explosion2.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    explosion_images[1] = (BITMAP*)malloc(sizeof(BITMAP));
    explosion_images[1]->image_data = PTemp;
    explosion_images[1]->width = KTemp->w;
    explosion_images[1]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 3
    kmg_to_img("/rd/explosion3.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    explosion_images[2] = (BITMAP*)malloc(sizeof(BITMAP));
    explosion_images[2]->image_data = PTemp;
    explosion_images[2]->width = KTemp->w;
    explosion_images[2]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 4
    kmg_to_img("/rd/explosion4.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    explosion_images[3] = (BITMAP*)malloc(sizeof(BITMAP));
    explosion_images[3]->image_data = PTemp;
    explosion_images[3]->width = KTemp->w;
    explosion_images[3]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 5
    kmg_to_img("/rd/explosion5.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    explosion_images[4] = (BITMAP*)malloc(sizeof(BITMAP));
    explosion_images[4]->image_data = PTemp;
    explosion_images[4]->width = KTemp->w;
    explosion_images[4]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 6
    kmg_to_img("/rd/explosion6.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    explosion_images[5] = (BITMAP*)malloc(sizeof(BITMAP));
    explosion_images[5]->image_data = PTemp;
    explosion_images[5]->width = KTemp->w;
    explosion_images[5]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;

    //initialize the sprites
    for (n=0; n<MAX_EXPLOSIONS; n++)
    {
        explosions[n] = (SPRITE*)malloc(sizeof(SPRITE));
        explosions[n]->alive = 0;
        explosions[n]->x = 0;
        explosions[n]->y = 0;
        explosions[n]->width = KTemp->w;
        explosions[n]->height = KTemp->h;
        explosions[n]->xdelay = 0;
        explosions[n]->ydelay = 8;
        explosions[n]->xcount = 0;
        explosions[n]->ycount = 0;
        explosions[n]->xspeed = 0;
        explosions[n]->yspeed = -1;
        explosions[n]->curframe = 0;
        explosions[n]->maxframe = 5;
        explosions[n]->framecount = 0;
        explosions[n]->framedelay = 5;
        explosions[n]->animdir = 1;
    }

     // Explosion image 1
    kmg_to_img("/rd/bigexplosion1.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    bigexp_images[0] = (BITMAP*)malloc(sizeof(BITMAP));
    bigexp_images[0]->image_data = PTemp;
    bigexp_images[0]->width = KTemp->w;
    bigexp_images[0]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 2
    kmg_to_img("/rd/bigexplosion2.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    bigexp_images[1] = (BITMAP*)malloc(sizeof(BITMAP));
    bigexp_images[1]->image_data = PTemp;
    bigexp_images[1]->width = KTemp->w;
    bigexp_images[1]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 3
    kmg_to_img("/rd/bigexplosion3.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    bigexp_images[2] = (BITMAP*)malloc(sizeof(BITMAP));
    bigexp_images[2]->image_data = PTemp;
    bigexp_images[2]->width = KTemp->w;
    bigexp_images[2]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 4
    kmg_to_img("/rd/bigexplosion4.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    bigexp_images[3] = (BITMAP*)malloc(sizeof(BITMAP));
    bigexp_images[3]->image_data = PTemp;
    bigexp_images[3]->width = KTemp->w;
    bigexp_images[3]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 5
    kmg_to_img("/rd/bigexplosion5.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    bigexp_images[4] = (BITMAP*)malloc(sizeof(BITMAP));
    bigexp_images[4]->image_data = PTemp;
    bigexp_images[4]->width = KTemp->w;
    bigexp_images[4]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 6
    kmg_to_img("/rd/bigexplosion6.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    bigexp_images[5] = (BITMAP*)malloc(sizeof(BITMAP));
    bigexp_images[5]->image_data = PTemp;
    bigexp_images[5]->width = KTemp->w;
    bigexp_images[5]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;
    
    // Explosion image 7
    kmg_to_img("/rd/bigexplosion7.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    bigexp_images[6] = (BITMAP*)malloc(sizeof(BITMAP));
    bigexp_images[6]->image_data = PTemp;
    bigexp_images[6]->width = KTemp->w;
    bigexp_images[6]->height = KTemp->h;

    PTemp = NULL;

    //initialize the sprites
    bigexp = (SPRITE*)malloc(sizeof(SPRITE));
    bigexp->alive = 0;
    bigexp->x = 0;
    bigexp->y = 0;
    bigexp->width = KTemp->w;
    bigexp->height = KTemp->h;
    bigexp->xdelay = 0;
    bigexp->ydelay = 8;
    bigexp->xcount = 0;
    bigexp->ycount = 0;
    bigexp->xspeed = 0;
    bigexp->yspeed = -1;
    bigexp->curframe = 0;
    bigexp->maxframe = 6;
    bigexp->framecount = 0;
    bigexp->framedelay = 5;
    bigexp->animdir = 1;
    
    kos_img_free(KTemp, 1);
}

void draw_sprite(BITMAP *image, float x, float y) {
     
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444, image->width, image->height, image->image_data, PVR_FILTER_NONE);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);    
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;
    
    vert.x = x;
    vert.y = y;
    vert.z = 3;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x+image->width;
    vert.u = 1.0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x;
    vert.y = y + image->height;
    vert.u = 0.0;
    vert.v = 1.0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x+image->width;
    vert.u = 1.0;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}


int inside(int x,int y,int left,int top,int right,int bottom)
{
    if (x > left && x < right && y > top && y < bottom)
        return 1;
    else
        return 0;
}

void updatesprite(SPRITE *spr)
{
    //update x position
    if (++spr->xcount > spr->xdelay)
    {
        spr->xcount = 0;
        spr->x += spr->xspeed;
    }

    //update y position
    if (++spr->ycount > spr->ydelay)
    {
        spr->ycount = 0;
        spr->y += spr->yspeed;
    }

    //update frame based on animdir
    if (++spr->framecount > spr->framedelay)
    {
        spr->framecount = 0;
        if (spr->animdir == -1)
        {
            if (--spr->curframe < 0)
                spr->curframe = spr->maxframe;
        }
        else if (spr->animdir == 1)
        {
            if (++spr->curframe > spr->maxframe)
                spr->curframe = 0;
        }
    }
}

void startexplosion(int x, int y)
{
    int n;
    for (n=0; n<MAX_EXPLOSIONS; n++)
    {
        if (!explosions[n]->alive)
        {
            explosions[n]->alive++;
            explosions[n]->x = x;
            explosions[n]->y = y;
            break;
        }
    }

    //launch bonus shot if ready
    if (!bonus_shot->alive)
    {
        bonus_shot->alive++;
        bonus_shot->x = x;
        bonus_shot->y = y;
    }
}

void updateexplosions()
{
    int n, c=0;

    for (n=0; n<MAX_EXPLOSIONS; n++)
    {
        if (explosions[n]->alive)
        {
            c++;
            updatesprite(explosions[n]);
            draw_sprite(explosion_images[explosions[n]->curframe],
                explosions[n]->x, explosions[n]->y);

            if (explosions[n]->curframe >= explosions[n]->maxframe)
            {
                explosions[n]->curframe=0;
                explosions[n]->alive=0;
            }
        }
    }
    gpfont_printf(&bios_font, 0, 350, "explosions %d", c);

    //update the big "player" explosion if needed
    if (bigexp->alive)
    {
        updatesprite(bigexp);
        draw_sprite(bigexp_images[bigexp->curframe], 
            bigexp->x, bigexp->y);
        if (bigexp->curframe >= bigexp->maxframe)
        {
            bigexp->curframe=0;
            bigexp->alive=0;
        }
    }
}

void updatebonuses()
{
    int x,y,x1,y1,x2,y2;

    //add more bonuses here

    //update bonus shot if alive
    if (bonus_shot->alive)
    {
        updatesprite(bonus_shot);
        draw_sprite(bonus_shot_image, bonus_shot->x, bonus_shot->y);
        if (bonus_shot->y > Map->GetMapScreenHeight())
            bonus_shot->alive=0;

        //see if player got the bonus
        x = bonus_shot->x + bonus_shot->width/2;
        y = bonus_shot->y + bonus_shot->height/2;
        x1 = player->x;
        y1 = player->y;
        x2 = x1 + player->width;
        y2 = y1 + player->height;

        if (inside(x,y,x1,y1,x2,y2))
        {
            //increase firing rate
            if (firedelay>20) firedelay-=2;

            bonus_shot->alive=0;
        }
    }

}

void updatebullet(SPRITE *spr)
{
    int n,x,y;
    int x1,y1,x2,y2;

    //move the bullet
    updatesprite(spr);

    //check bounds
    if (spr->y < 0)
    {
        spr->alive = 0;
        return;
    }

    for (n=0; n<MAX_ENEMIES; n++)
    {
        if (enemy_planes[n]->alive)
        {
            //find center of bullet
            x = spr->x + spr->width/2;
            y = spr->y + spr->height/2;

            //get enemy plane bounding rectangle
            x1 = enemy_planes[n]->x;
            y1 = enemy_planes[n]->y - yoffset;
            x2 = x1 + enemy_planes[n]->width;
            y2 = y1 + enemy_planes[n]->height;

            //check for collisions
            if (inside(x, y, x1, y1, x2, y2))
            {
                enemy_planes[n]->alive=0;
                spr->alive=0;
                startexplosion(spr->x+16, spr->y);
                score+=2;
                break;
            }
        }
    }
}

void updatebullets()
{
    int n;
    //update/draw bullets
    for (n=0; n<MAX_BULLETS; n++)
        if (bullets[n]->alive)
        {
            updatebullet(bullets[n]);
            draw_sprite(bullet_images[0], bullets[n]->x, bullets[n]->y);
        }
}

void bouncex_warpy(SPRITE *spr)
{
    //bounces x off bounds
    if (spr->x < 0 - spr->width)
    {
        spr->x = 0 - spr->width + 1;
        spr->xspeed *= -1;
    }

    else if (spr->x > Map->GetMapScreenWidth())
    {
        spr->x = Map->GetMapScreenWidth() - spr->xspeed;
        spr->xspeed *= -1;
    }

    //warps y if plane has passed the player
    if (spr->y > yoffset + 2000)
    {
        //respawn enemy plane
        spr->y = yoffset - 1000 - rand() % 1000;
        spr->alive++;
        spr->x = rand() % Map->GetMapScreenWidth();
    }

    //warps y from bottom to top of level
    if (spr->y < 0)
    {
        spr->y = 0;
    }

    else if (spr->y > 48000)
    {
        spr->y = 0;
    }

}

void fireatenemy()
{
    int n;
    for (n=0; n<MAX_BULLETS; n++)
    {
        if (!bullets[n]->alive)
        {
            bullets[n]->alive++;
            bullets[n]->x = player->x;
            bullets[n]->y = player->y;
            return;
        }
    }
}

void updateenemyplanes()
{
    int n, c=0;

    //update/draw enemy planes
    for (n=0; n<MAX_ENEMIES; n++)
    {
        if (enemy_planes[n]->alive)
        {
            c++;
            updatesprite(enemy_planes[n]);
            bouncex_warpy(enemy_planes[n]);

            //is plane visible on screen?
            if (enemy_planes[n]->y > yoffset-32 && enemy_planes[n]->y < yoffset + Map->GetMapScreenHeight()+32)
            {
                //draw enemy plane
                draw_sprite(enemy_plane_images[enemy_planes[n]->curframe],
                    enemy_planes[n]->x, enemy_planes[n]->y - yoffset);
            }
        }
        //reset plane
        else
        {
            enemy_planes[n]->alive++;
            enemy_planes[n]->x = rand() % 100 + 50;
            enemy_planes[n]->y = yoffset - 2000 + rand() % 2000;
        }
    }
    gpfont_printf(&bios_font, 0, 375, "enemies %d", c);
}

void updatescroller()
{
    //make sure it doesn't scroll beyond map edge
    if (yoffset <= 0) 
    {
        //level is over
        yoffset = 0;
        gpfont_print(&bios_font, 320, 240, "END OF LEVEL");
    }
    else if (yoffset > BOTTOM) yoffset = BOTTOM;
    else yoffset-=5;

    //draw map with single layer
    Map->MapMoveTo(0, yoffset);


}

void updateplayer()
{
    int n,x,y,x1,y1,x2,y2;

    //update/draw player sprite
    updatesprite(player);
    draw_sprite(player_images[player->curframe], player->x, player->y);

    //check for collision with enemy planes
    x = player->x + player->width/2;
    y = player->y + player->height/2;
    for (n=0; n<MAX_ENEMIES; n++)
    {
        if (enemy_planes[n]->alive)
        {
            x1 = enemy_planes[n]->x;
            y1 = enemy_planes[n]->y - yoffset;
            x2 = x1 + enemy_planes[n]->width;
            y2 = y1 + enemy_planes[n]->height;
            if (inside(x,y,x1,y1,x2,y2))
            {
                enemy_planes[n]->alive=0;
                if (health > 0) health--;
                bigexp->alive++;
                bigexp->x = player->x;
                bigexp->y = player->y;
                score++;
            }
        }
    }
}


void displaystats()
{
    //display some status information
    gpfont_printf(&bios_font, 0, 400, "firing rate %d", firedelay);
    gpfont_printf(&bios_font, 0, 425, "yoffset %d", yoffset);
    gpfont_printf(&bios_font, 0, 450, "counter %d", counter);

    //display score
    gpfont_printf(&bios_font, 22, 22, "SCORE: %d", score);

}

void checkinput() {
    
    cont_cond_t cond;
    cont_get_cond( maple_addr( 0, 0 ), &cond);
    
    if  ((!(cond.buttons & CONT_START))||(!(cond.buttons & CONT_A))||(!(cond.buttons & CONT_DPAD_UP))||(!(cond.buttons & CONT_DPAD_DOWN))||(!(cond.buttons & CONT_DPAD_LEFT))||(!(cond.buttons & CONT_DPAD_RIGHT)))
	{ 
        if(!(cond.buttons & CONT_DPAD_UP)) {
            player->y -= 5;
            if (player->y < 100)
                player->y = 100;
        }
        if(!(cond.buttons & CONT_DPAD_DOWN)) {
            player->y += 5;
            if (player->y > Map->GetMapScreenHeight()-65)
                player->y = Map->GetMapScreenHeight()-65;
        }
        if(!(cond.buttons & CONT_DPAD_LEFT)) {
            player->x -= 5;
            if (player->x < 0)
                player->x = 0;
        }
        if(!(cond.buttons & CONT_DPAD_RIGHT)) {
            player->x += 5;
            if (player->x > Map->GetMapScreenWidth()-65)
                player->x = Map->GetMapScreenWidth()-65;
        }

        if(!(cond.buttons & CONT_A)) {
                          
            if (firecount > firedelay) {
                firecount = 0;
                fireatenemy();
            }
        }
        if(!(cond.buttons & CONT_START)) {
              done = 1;	
        }
    } 

}

pvr_init_params_t params = {
	/* Enable opaque and translucent polygons with size 16 */
	{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16 },
	
	/* Vertex buffer size 512K */
	512*1024
};

int main(int argc, char **argv) {
    
	/* Init PVR API */
    if (pvr_init(&params) < 0)
	    return -1;
    
    /* Initialise the font system */
    gpfont_init();
    
    // map loading
	Map = new DCMappy();

	if (Map->LoadMap("/rd/warbirds.fmp", 640, 480, BLACK) == -1) {
		printf("Can't load the Map...\n");
    }
    Map->MapChangeLayer(0);   
    
    loadsprites();
    
	// Game Loop
	while (!done)
	{
        checkinput();  

        updatescroller(); 

        pvr_wait_ready();
        pvr_scene_begin();
    
        pvr_list_begin(PVR_LIST_OP_POLY);
        Map->MapDrawBG();
        pvr_list_finish();
        
        pvr_list_begin(PVR_LIST_TR_POLY);
        updateplayer();
        updateenemyplanes();

        updatebullets();            
        updateexplosions();
        updatebonuses();
        displaystats(); 
        pvr_list_finish();
        
        pvr_scene_finish();
        
        firecount += 4;
	}
	
	delete Map;

	return 0;
}
