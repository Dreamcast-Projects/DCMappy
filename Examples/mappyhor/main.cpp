/////////////////////////////////////////////////////////
// Game Programming All In One, Third Edition
// Chapter 16 - PlatformScroller
/////////////////////////////////////////////////////////

#include <kos.h>
#include <stdio.h>
#include "DCMappy.h"

#define JUMPIT 1600

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

//declare the bitmaps and sprites
BITMAP *player_images[8];
SPRITE *player;
DCMappy *Map;

extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);
KOS_INIT_ROMDISK(romdisk);

int collided(int x, int y)
{
    BLKSTR *blockdata;
	blockdata = Map->MapGetBlock(x/Map->GetMapBlockWidth(), y/Map->GetMapBlockHeight());
	return blockdata->bl;
}

void loadsprites(void)
{
    kos_img_t      * KTemp = NULL;
    pvr_ptr_t      PTemp = NULL;
    
    KTemp = (kos_img_t *)malloc(sizeof(kos_img_t));
    
     // Player image 1
    kmg_to_img("/rd/man1.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[0] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[0]->image_data = PTemp;
    player_images[0]->width = KTemp->w;
    player_images[0]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;   
    
     // Player image 2
    kmg_to_img("/rd/man2.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[1] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[1]->image_data = PTemp;
    player_images[1]->width = KTemp->w;
    player_images[1]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;   
    
     // Player image 3
    kmg_to_img("/rd/man3.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[2] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[2]->image_data = PTemp;
    player_images[2]->width = KTemp->w;
    player_images[2]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;   
    
     // Player image 4
    kmg_to_img("/rd/man4.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[3] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[3]->image_data = PTemp;
    player_images[3]->width = KTemp->w;
    player_images[3]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;   
    
     // Player image 5
    kmg_to_img("/rd/man5.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[4] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[4]->image_data = PTemp;
    player_images[4]->width = KTemp->w;
    player_images[4]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;   
    
     // Player image 6
    kmg_to_img("/rd/man6.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[5] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[5]->image_data = PTemp;
    player_images[5]->width = KTemp->w;
    player_images[5]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;   
    
     // Player image 7
    kmg_to_img("/rd/man7.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[6] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[6]->image_data = PTemp;
    player_images[6]->width = KTemp->w;
    player_images[6]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;   
    
    // Player image 8
    kmg_to_img("/rd/man8.kmg", KTemp); 
    PTemp = pvr_mem_malloc(KTemp->byte_count);
    pvr_txr_load_kimg(KTemp, PTemp, PVR_TXRLOAD_16BPP);
    
    player_images[7] = (BITMAP*)malloc(sizeof(BITMAP));
    player_images[7]->image_data = PTemp;
    player_images[7]->width = KTemp->w;
    player_images[7]->height = KTemp->h;

    kos_img_free(KTemp, 0);
    PTemp = NULL;   
    
    
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
    vert.u = 0.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x+image->width;
    vert.u = 1.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x;
    vert.y = y + image->height;
    vert.u = 0.0f;
    vert.v = 1.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x+image->width;
    vert.u = 1.0f;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}

void draw_sprite_h_flip(BITMAP *image, float x, float y) {
     
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
    vert.u = 1.0f;
    vert.v = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x+image->width;
    vert.u = 0.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x;
    vert.y = y + image->height;
    vert.u = 1.0f;
    vert.v = 1.0f;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x+image->width;
    vert.u = 0.0f;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}


pvr_init_params_t params = {
	/* Enable opaque and translucent polygons with size 16 */
	{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16 },
	
	/* Vertex buffer size 512K */
	512*1024
};

int main(int argc, char **argv) {
    
    int mapxoff, mapyoff;
    int oldpy, oldpx;
    int facing = 0;
    int jump = JUMPIT;
    cont_cond_t cond;
    
    
    cont_get_cond( maple_addr( 0, 0 ), &cond);  
    
	/* Init PVR API */
    if (pvr_init(&params) < 0)
	    return -1;
	    
    loadsprites();
   
    player = (SPRITE *)malloc(sizeof(SPRITE));
    player->x = 80;
    player->y = 100;
    player->curframe=0;
    player->framecount=0;
    player->framedelay=6;
    player->maxframe=7;
    player->width= player_images[0]->width;
    player->height= player_images[0]->height;
    
     // map loading
	Map = new DCMappy();

	if (Map->LoadMap("/cd/mpgame.fmp", 640, 480, BLACK) == -1) {
		printf("Can't load the Map...\n");
    }
     
    Map->MapChangeLayer(0);   
    Map->MapGenerateYLookup();
    
    //main loop
	while ((cond.buttons & CONT_START))
	{
        cont_get_cond( maple_addr( 0, 0 ), &cond);  

		oldpy = player->y; 
        oldpx = player->x;

		if (!(cond.buttons & CONT_DPAD_RIGHT)) 
        { 
            facing = 1; 
            player->x+=2; 
            if (++player->framecount > player->framedelay)
            {
                player->framecount=0;
                if (++player->curframe > player->maxframe)
                    player->curframe=1;
            }
        }
        else if (!(cond.buttons & CONT_DPAD_LEFT)) 
        { 
            facing = 0; 
            player->x-=2; 
            if (++player->framecount > player->framedelay)
            {
                player->framecount=0;
                if (++player->curframe > player->maxframe)
                    player->curframe=1;
            }
        }
        else player->curframe=0;

        //handle jumping
        if (jump==JUMPIT)
        { 
            if (!collided(player->x + player->width/2, 
                player->y + player->height + 5))
                jump = 0; 

		    if (!(cond.buttons & CONT_A)) 
                jump = 30;
        }
        else
        {
            player->y -= jump/3; 
            jump--; 
        }

		if (jump<0) 
        { 
            if (collided(player->x + player->width/2, 
                player->y + player->height))
			{ 
                jump = JUMPIT; 
                while (collided(player->x + player->width/2, 
                    player->y + player->height))
                    player->y -= 2; 
            } 
        }

        //check for collided with foreground tiles
		if (!facing) 
        { 
            if (collided(player->x, player->y + player->height)) 
                player->x = oldpx; 
        }
		else 
        { 
            if (collided(player->x + player->width, 
                player->y + player->height)) 
                player->x = oldpx; 
        }
		
        //update the map scroll position
		mapxoff = player->x + player->width/2 - Map->GetMapScreenWidth()/2 + 10;
		mapyoff = player->y + player->height/2 - Map->GetMapScreenHeight()/2 + 10;

        //avoid moving beyond the map edge
		if (mapxoff < 0) mapxoff = 0;
		if (mapxoff > (Map->GetMapWidthInBlocks() * Map->GetMapBlockWidth() - Map->GetMapScreenWidth()))
            mapxoff = Map->GetMapWidthInBlocks() * Map->GetMapBlockWidth() - Map->GetMapScreenWidth();
		if (mapyoff < 0) 
            mapyoff = 0;
		if (mapyoff > (Map->GetMapHeightInBlocks() * Map->GetMapBlockHeight() - Map->GetMapScreenHeight())) 
            mapyoff = Map->GetMapHeightInBlocks() * Map->GetMapBlockHeight() - Map->GetMapScreenHeight();
            
        Map->MapMoveTo(mapxoff, mapyoff);    

        pvr_wait_ready();
        pvr_scene_begin();
        
        Map->MapUpdateAnims();
    
        pvr_list_begin(PVR_LIST_OP_POLY);
        Map->MapDrawBG();
        pvr_list_finish();
        
        pvr_list_begin(PVR_LIST_PT_POLY);
       //draw foreground tiles
		Map->MapDrawFG(0);
		pvr_list_finish();
		//draw the player's sprite
        pvr_list_begin(PVR_LIST_TR_POLY);
		if (facing) 
            draw_sprite(player_images[player->curframe], 
               (player->x-mapxoff), (player->y-mapyoff+1));
		else 
            draw_sprite_h_flip(player_images[player->curframe], 
                (player->x-mapxoff), (player->y-mapyoff+1));
		
        pvr_list_finish();
        pvr_scene_finish();

        
	} 

    free(player);
	delete Map;
	
	return 0;
}

