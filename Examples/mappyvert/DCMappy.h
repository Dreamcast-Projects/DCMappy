/***************************************************************************
                          DCMappy.h  -  description
                             -------------------
    begin                : Thu Jun 11 2009
    copyright            : (C) 2009 by BB Hood
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//////////////////////////////////////////////////////////////////////////////////
//
// DCMappy
//
// (c) based on sources of Robin Burrows / rburrows@bigfoot.com.
// Mappy homepage. The current Homepage for Mappy is:
// http://www.tilemap.co.uk/mappy.php
// 
// Ported to Dreamcast(using PVR) by BB Hood 
//
// Special Thanks to BlackAura, BlueCrab, Quzar, OneThirty8
//

#ifndef DCMAPPY_H
#define DCMAPPY_H

#include <kos.h>
#include <malloc.h>
#include <kmg/kmg.h> // For Parallax images
#include <dc/../../hardware/pvr/pvr_internal.h> // So I can use pvr_state (global: pvr_globals.c)

// Leave commented when you are finished writing your program.
// Uncomment the next line if you have problems loading/using a Mappy file (*.fmp)  
//#define DEBUG_MAP

#define BLACK    makecolor16(0,0,0)      
#define MAGENTA  makecolor16(255,0,255)

#define makecolor16(red, green, blue) ( ((red >> 3) << 10) | ((green >> 3) << 5) | ((blue >> 3) << 0) | 0x0000 )

// When 8-bit maps are used...Huge thanks to BlackAura
#define PACK_ARGB8888(a,r,g,b) ( ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF) )

// Taken from a post by Big Sassy (http://www.gamedev.net/community/forums/topic.asp?topic_id=84026&whichpage=1&#424630)
// and modified to make ARGB1555 instead of RGB565
#define ARGB16(red, green, blue) ( ((blue >> 3) << 10) | ((green >> 3) << 5) | (red >> 3) | 0x8000 )


#define MER_NONE 0			        /* All the horrible things that can go wrong */
#define MER_OUTOFMEM 1
#define MER_MAPLOADERROR 2
#define MER_NOOPEN 3

#define AN_END -1					/* Animation types, AN_END = end of anims */
#define AN_NONE 0					/* No anim defined */
#define AN_LOOPF 1					/* Loops from start to end, then jumps to start etc */
#define AN_LOOPR 2					/* As above, but from end to start */
#define AN_ONCE 3					/* Only plays once */
#define AN_ONCEH 4					/* Only plays once, but holds end frame */
#define AN_PPFF 5					/* Ping Pong start-end-start-end-start etc */
#define AN_PPRR 6					/* Ping Pong end-start-end-start-end etc */
#define AN_PPRF 7					/* Used internally by playback */
#define AN_PPFR 8					/* Used internally by playback */
#define AN_ONCES 9					/* Used internally by playback */

/* Useful data types */
typedef struct Block_Rect {
	signed short x, y;
	unsigned short w, h;
	float	u1, v1, u2, v2;
} Block_Rect;

typedef struct {
	uint8 r;
	uint8 g;
	uint8 b;
} Color;

unsigned short int byteswaps (unsigned short int i);
unsigned long int byteswapl (unsigned long int i);
void swap_convert_RGB565_to_ARGB1555(uint16 *src, uint16 *dst, int count, uint16 key);
void swap_convert_RGB888_to_ARGB1555(FILE *mapfpt, uint16 *dst, int count, uint16 key);
void swap_convert_ARGB8888_to_ARGB1555(FILE *mapfpt, uint16 *dst, int count, uint16 key); 

typedef struct {					/* Structure for data blocks */
 long int bgoff , fgoff;            /* offsets from start of graphic blocks */
 long int fgoff2, fgoff3;           /* more overlay blocks */
 long int  user1, user2;	        /* user long data */
 short int user3, user4;	        /* user short data */
 char user5, user6, user7;	        /* user byte data */
 unsigned char tl : 1;				/* bits for collision detection */
 unsigned char tr : 1;
 unsigned char bl : 1;
 unsigned char br : 1;
 unsigned char trigger : 1;			/* bit to trigger an event */
 unsigned char unused1 : 1;
 unsigned char unused2 : 1;
 unsigned char unused3 : 1;
} BLKSTR;

typedef struct {					/* Animation control structure */
    signed char antype;				/* Type of anim, AN_? */
    signed char andelay;			/* Frames to go before next frame */
    signed char ancount;			/* Counter, decs each frame, till 0, then resets to andelay */
    signed char anuser;				/* User info */
    long int    ancuroff;			/* Points to current offset in list */
    long int    anstartoff;			/* Points to start of blkstr offsets list, AFTER ref. blkstr offset */
    long int    anendoff;			/* Points to end of blkstr offsets list */
} ANISTR;

typedef struct {					/* Generic structure for chunk headers */
    char id1, id2, id3, id4;		/* 4 byte header id. */
    long int headsize;				/* size of header chunk. */
} GENHEAD;

typedef struct {					/* Map header structure */
    char mapverhigh;				/* map version number to left of . (ie X.0). */
	char mapverlow;					/* map version number to right of . (ie 0.X). */
	char lsb;						/* if 1, data stored LSB first, otherwise MSB first. */
	char reserved;
	short int mapwidth;				/* width in blocks. */
	short int mapheight;			/* height in blocks. */
	short int reserved1;
	short int reserved2;
	short int blockwidth;			/* width of a block (tile) in pixels. */
	short int blockheight;			/* height of a block (tile) in pixels. */
	short int blockdepth;			/* depth of a block (tile) in planes (ie. 256 colours is 8) */
	short int blockstrsize;			/* size of a block data structure */
	short int numblockstr;			/* Number of block structures in BKDT */
	short int numblockgfx;			/* Number of 'blocks' in graphics (BGFX/BODY) */
} MPHD;

class DCMappy
{
private:
	// These define how far from the left and top of the surface the drawn area is (in pixels)
	int			MMOX;
	int			MMOY;

	// These define the width and height of the drawn area... (in pixels)
	int			MSCRW;
	int			MSCRH;
	
    // These are for the parallax surface
    int         PARAW;              // width of the parallax surface - blockwidth
    int         PARAH;              // height of the parallax surface - blockheight

	short int	mapwidth,           // width of the map in blocks
				mapheight,          // height of the map in blocks
				mapdepth;           // color depth of the map
	unsigned int mapblockwidth,     // width of one map block in pixels
				 mapblockheight;    // height of one map block in pixels	
	short int	mapblockstrsize,    // size of a block data structure
				mapnumblockstr,     // total number of blocks (Number of block structures in BKDT)
				mapnumblockgfx;     // number of 'blocks' in graphics (BGFX/BODY) 
				
	GENHEAD		mapgenheader;

    int         XPosition , YPosition;          // X/Y position of the map in pixels

	short int	* mappt;             // number of tiles in each layer (an array of short int (16bit) values that reference Block Structures (when positive) and Anim Structures (when negative))
	short int	** maparraypt;       //
	short int 	* mapmappt[8];       // number of tiles in 8 layers (There can be up to 8 layers of the map array for objects, ingame changes, etc.)
	short int   ** mapmaparraypt[8]; //
	
	unsigned char * mapcmappt;       // Colour palette for 8 bit maps
	Color	      mappept[256];      // The RGB values of the 256 colors for 8bpp files 
    
	char		* mapblockstrpt;     // Block data. Contains BLKSTR structures...
								     // ...for however many block structures were made
    int         * mapanimseqpt;
	char		* mapanimstrpt;      // Animation data. Contains ANISTR structures for however many...
	char		* mapanimstrendpt;   // ...animation structures were made, and also animation data

	int			maperror;				// Set to a MER_ error if something wrong happens
	int         mapislsb;               // if 1, data stored LSB first, otherwise MSB first.
	FILE		* mapfilept;            // File that contains the map
	
	pvr_poly_hdr_t	* maplpDDSTiles_OP;  // Compiled headers. One for each gfx tile. Opaque
	pvr_poly_hdr_t  * maplpDDSTiles_PT;  // Compiled headers. One for each gfx tile. Transparent
	pvr_ptr_t       * tileGraphics;      // Keeps track of the allocated PVR memory(for each tile) so it can be freed
	
	unsigned long   ANDTSize;		   // Animation data size
	uint16		    ColorKey;          // Holds the Color Key
	
    pvr_poly_hdr_t   * ParallaxSurface;         // parallax surface header
    char             * ParallaxFilename;        // filename of the parallax .kmg
    pvr_ptr_t        ParallaxGraphic;           // Keeps track of the allocated PVR memory(for Parallax tile) so it can be freed 
    
    static int        palette;    // Which palette to start entering rgb values in
    
    // PRIVATE FUNCTIONS
    int			      MapLoad (char * mapname);
	int			      MapGetshort (unsigned char *);
	int			      MapGetlong (unsigned char *);
	int 		      MapDecodeNULL (void);   
	int 		      MapDecodeMPHD (void);   // Decodes Map Header (See MPHD struct)
	int		 	      MapDecodeBODY (void);   // Decodes an array of short ints containing positive offsets into BKDT, and negative offsets into ANDT. 
	int			      MapDecodeCMAP (void);   // Decodes Colour palette for 8bit maps, red byte, green byte, blue byte for however many colours are needed (so usually 256*3 bytes).
	int			      MapDecodeBKDT (void);   // Decodes Block data. Contains BLKSTR structures for however many block structures were made.
	int			      MapDecodeANDT (void);   // Decodes Animation data. Contains ANISTR structures for however many animation structures were made, and also animation data.
	int			      MapDecodeBGFX (void);   // Decodes the raw graphics in whatever format the map is in. 
	int			      MapDecodeLYR1 (void);   // Decodes Layer Data(LYR?) where ? is an ASCII number form 1 to 7. These are the same size and format as BODY, and allow object layers to be used.
	int			      MapDecodeLYR2 (void);
	int			      MapDecodeLYR3 (void);
	int			      MapDecodeLYR4 (void);
	int			      MapDecodeLYR5 (void);
	int		      	  MapDecodeLYR6 (void);
	int			      MapDecodeLYR7 (void);
	void 		      MapInitAnims (void);
	void              MapDrawBlock (pvr_poly_hdr_t * block, Block_Rect * coordinates);
  
public:
	~DCMappy();
	DCMappy(void);

	void		MapFreeMem (void);
	int 		LoadMap (char * Filename, int width , int height, uint16 Key);
	int		    MapGenerateYLookup (void);
	
	BLKSTR * 	MapGetBlock (int x, int y);
    BLKSTR *    MapGetBlockLayer (int x, int y, int layer);
	void 		MapSetBlock (int x, int y, int strvalue);
	short int	MapGetTile (int x, int y);
	
    int		    	MapGetXPosition (void) { return XPosition; };
    int			    MapGetYPosition (void) { return YPosition; };
	unsigned int	GetMapWidth (void); 
	unsigned int 	GetMapHeight (void);
	short int       GetMapScreenWidth (void) { return MSCRW; };             // Returns the width(visible dimension) of the map in pixels
	short int       GetMapScreenHeight (void) { return MSCRH; };            // Returns the height(visible dimension) of the map in pixels
	short int	    GetMapWidthInBlocks (void) { return mapwidth; };        // Returns the width of the map in blocks
	short int	    GetMapHeightInBlocks (void) { return mapheight; };      // Returns the height of the map in blocks
	short int	    GetMapBlockWidth (void) { return mapblockwidth; };      // Returns the width of one map block in pixels
	short int	    GetMapBlockHeight (void) { return mapblockheight; };    // Returns the height of one map block in pixels
	short int	    GetMapBPP (void) { return mapdepth; };                  // Returns the color depth of the map
	short int       GetMapBlockStrSize (void) { return mapblockstrsize; };  // Returns the size of a block data structure
	short int       GetMapNumBlockStr (void) { return mapnumblockstr; };    // Returns the total number of blocks (Number of block structures in BKDT)
	short int       GetMapNumBlockGfx (void) { return mapnumblockgfx; };    // Returns the number of 'blocks' in graphics (BGFX/BODY)
		
	void        MapSetLocation(int x, int y);	
	void 		MapMoveTo (int x, int y);
	int			MapChangeLayer (int newlyr);
	int 		MapDrawBG (void);
	int		    MapDrawBGT (void);
	int		    MapDrawFG (int layer);
	void 		MapUpdateAnims (void);
	
    bool    CreateParallax (char * Filename);
    bool    RestoreParallax (void);
    int     DrawParallax (void);
    
};

#endif
