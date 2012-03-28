/***************************************************************************
                          DCMappy.cpp  -  description
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

////////////////////////////////////////////////////////////////////////////////////
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

#include "DCMappy.h"

// ---------------------------------------------------------------------------------
// Convert image data from big-endian to little-endian using this function. Taken 
// from Mappy V1.01 C source (provided on http://tilemap.co.uk/mappy.php)
// ---------------------------------------------------------------------------------
unsigned short int byteswaps (unsigned short int i) {
    unsigned short int j;
	j = 0; 
    j = i&0xFF; 
    j <<= 8; 
    i >>= 8; 
    j |= i&0xFF; 
    return j;
}

// ---------------------------------------------------------------------------------
// Convert long numbers from big-endian to little-endian using this function.
// Used for headsize of headers of sections of the *.fmp file and converting from
// 24bpp or 32bpp to 16bpp. Taken from Mappy V1.01 C source 
// (provided on http://tilemap.co.uk/mappy.php)
// ---------------------------------------------------------------------------------
unsigned long int byteswapl (unsigned long int i) {
         
	unsigned long int j;

	j   = i&0xFF;
	j <<= 8;
	i >>= 8;
	j  |= i&0xFF;
	j <<= 8;
	i >>= 8;
	j  |= i&0xFF;
	j <<= 8;
	i >>= 8;
	j  |= i&0xFF;	
	return j;
}

// ---------------------------------------------------------------------------------
// Convert image data from big-endian RGB565 to little-endian ARGB1555. Thanks BlackAura.
// ---------------------------------------------------------------------------------
void swap_convert_RGB565_to_ARGB1555 (uint16 *src, uint16 *dst, int count, uint16 key) {
   int i;
   uint16 src_colour = 0;
   uint16 colour = 0;
   uint16 pixel = 0;
   
   for(i = 0; i < count; i++) {
      // Swap the bytes while we're reading them
      src_colour = *src++;
      colour = byteswaps(src_colour);

      pixel = ((colour & 0xF800) >> 11 ) | (((colour & 0x07E0) >> 6) << 5) | ((colour & 0x001F) << 10 );
      
      // Convert from RGB565 to ARGB1555
      if(pixel == key)      // If color key ...
         *dst++ = 0x0000;   // ... make transparent instead
      else
         *dst++ = (colour & 0x001F) | ((colour & 0xFFC0) >> 1) | 0x8000;
   }
}

// ---------------------------------------------------------------------------------
// Convert image data from big-endian RGB888 to little-endian ARGB1555. 24bpp is 
// stored as RGB
// ---------------------------------------------------------------------------------
void swap_convert_RGB888_to_ARGB1555 (FILE *mapfpt, uint16 *dst, int count, uint16 key) {
   int i;
   uint32 src_colour = 0;
   uint32 colour = 0;
   Color  pixel;
   
   for(i = 0; i < count; i++) {
      // Swap the bytes while we're reading them
      fread(&src_colour, 3, 1, mapfpt);
      colour = byteswapl(src_colour);

      // Convert from RGB888 to ARGB1555
      
      // These right shift numbers(24,16,8) aren't normally the case but I had to 
      // account for the big endian to little endian swap.    
      pixel.b = (colour >> 24) & 0xFF;
      pixel.g = (colour >> 16) & 0xFF;
      pixel.r = (colour >> 8) & 0xFF;    
      if(makecolor16(pixel.r,pixel.g,pixel.b) == key) {    // If color key ...
         *dst++ = 0x0000;                                  // ... make transparent instead
      } else {
         *dst++ = ARGB16(pixel.r, pixel.g, pixel.b);
      }
   }
}

// ---------------------------------------------------------------------------------
// Convert image data from big-endian ARGB8888 to little-endian ARGB1555. 32bpp is 
// stored as RGBA 
// ---------------------------------------------------------------------------------
void swap_convert_ARGB8888_to_ARGB1555 (FILE *mapfpt, uint16 *dst, int count, uint16 key) {
   int i;
   uint32 src_colour = 0;
   uint32 colour = 0;
   Color  pixel;
   
   for(i = 0; i < count; i++) {
      // Swap the bytes while we're reading them
      fread(&src_colour, 4, 1, mapfpt);
      colour = byteswapl(src_colour);

      // Convert from ARGB8888 to ARGB1555
      
      // These right shift numbers(16,8,0) aren't normally the case but I had to 
      // account for the big endian to little endian swap.    
      pixel.b = (colour >> 16) & 0xFF;   
      pixel.g = (colour >> 8) & 0xFF;
      pixel.r = (colour >> 0) & 0xFF;     
      
      if(makecolor16(pixel.r,pixel.g,pixel.b) == key) {   // If color key ...
         *dst++ = 0x0000;                                 // ... make transparent instead
      } else {
         *dst++ = ARGB16(pixel.r, pixel.g, pixel.b);
      }
   }
}

// ---------------------------------------------------------------------------------
// Destructor of this class, deletes all allocated memory
// ---------------------------------------------------------------------------------
DCMappy::~DCMappy (void) { 
    if(mapdepth == 8) { DCMappy::palette--; }
    MapFreeMem(); 
}

// ---------------------------------------------------------------------------------
// Constructor of this class, inits all members
// ---------------------------------------------------------------------------------
DCMappy::DCMappy (void) {
	int	   i;

	MMOX = 0;
	MMOY = 0;

	MSCRW = 0;
	MSCRH = 0;

	maperror		= 0;
	mapwidth		= 0;
	mapheight		= 0;
	mapblockwidth	= 0;
	mapblockheight	= 0;
	mapdepth		= 0;
	mapislsb        = 0;  

	mapblockstrsize	= 0;
	mapnumblockstr	= 0;
	mapnumblockgfx	= 0;

	mapfilept		= NULL;
	mappt			= NULL;
	maparraypt		= NULL; //
	mapcmappt		= NULL;
	mapblockstrpt	= NULL;
	mapanimstrpt	= NULL;
	mapanimstrendpt = NULL;
	
	ColorKey = BLACK;       

    maplpDDSTiles_OP = NULL;
    maplpDDSTiles_PT = NULL;
    tileGraphics = NULL;

	for(i = 0; i < 8; i++ ) { mapmappt[i] = NULL; mapmaparraypt[i] = NULL; } 

	XPosition = YPosition = 0;

	ParallaxSurface  = NULL;
	ParallaxFilename = NULL;
	ParallaxGraphic = NULL;
}

// Start off entering values at palette 0
int DCMappy::palette = -1; 

// ---------------------------------------------------------------------------------
// Deletes all allocated memory. 
// ---------------------------------------------------------------------------------
void DCMappy::MapFreeMem (void) {
     
    int i;
    
	for(i = 0;i < 8; i++) { 
        if (mapmappt[i] != NULL) { 
            free(mapmappt[i]); 
            mapmappt[i] = NULL; 
        } 
    }
	mappt = NULL;
	
	for (i = 0;i < 8; i++) { 
        if (mapmaparraypt[i]!=NULL) { 
            free (mapmaparraypt[i]); 
            mapmaparraypt[i] = NULL; 
        } 
    }
	maparraypt = NULL;
	
	if(mapcmappt != NULL) { 
        free(mapcmappt); 
        mapcmappt = NULL; 
    }
	if(mapblockstrpt != NULL) { 
        free(mapblockstrpt); 
        mapblockstrpt = NULL; 
    }
	if(mapanimstrpt != NULL) { 
        free(mapanimstrpt); 
        mapanimstrpt = NULL; 
    }

    if(maplpDDSTiles_OP != NULL) {
        free(maplpDDSTiles_OP);
        maplpDDSTiles_OP = NULL;
    }
    
    if(maplpDDSTiles_PT != NULL) {
        free(maplpDDSTiles_PT);
        maplpDDSTiles_PT = NULL;
    }
    
    if(tileGraphics != NULL) {
        for(i = 0; i < mapnumblockgfx; i++) {
           pvr_mem_free(tileGraphics[i]); 
           tileGraphics[i] = NULL;
        }
        free(tileGraphics);
    }
    
    if(ParallaxSurface != NULL) {
        free(ParallaxSurface);
        ParallaxSurface = NULL;
    }
    
    if(ParallaxFilename != NULL) {
        free(ParallaxFilename);
        ParallaxFilename = NULL;
    }
    
    if(ParallaxGraphic != NULL) {
        pvr_mem_free(ParallaxGraphic);
        ParallaxGraphic = NULL;
    }

    mapanimstrendpt = NULL; 
    
    #ifdef DEBUG_MAP
    printf("MapFreeMem(): Deallocated all allocated Memory.\n");
    #endif
}

// ---------------------------------------------------------------------------------
// Loads a map, width and height are visible map dimensions. 
// ---------------------------------------------------------------------------------
int DCMappy::LoadMap (char * Filename, int width, int height, uint16 Key) {
    
	int result;

	MSCRW = width;
	MSCRH = height;
	ColorKey = Key;

	XPosition = YPosition = 0;
	
	// load the map
	result = MapLoad(Filename);

    // if the map loaded ok, proceed, else return error code
	if( result != -1 ) {
        
        // RB
        // Since the draw routines will end the application if you try to draw
        // outside the map area, the width and height must be =< the map
        if( ( mapwidth * mapblockwidth ) < MSCRW )
            MSCRW = ( mapwidth * mapblockwidth );

        if( ( mapheight * mapblockheight ) < MSCRH )
            MSCRH = ( mapheight * mapblockheight );

        // Also, the drawn area must be at least 3*3 tiles big
        if( MSCRW < ( mapblockwidth * 3 ) )
            MSCRW = mapblockwidth * 3;

        if( MSCRH < ( mapblockheight * 3 ) )
            MSCRH = mapblockheight * 3;
	}

	return result;
}

// ---------------------------------------------------------------------------------
// Loads the *.fmp mappy file into RAM and decodes it
// ---------------------------------------------------------------------------------
int DCMappy::MapLoad (char * mapname) {

	int		 i;
    int      buflen;
    char     pfn[64];
    uint8    * buffer;
	long int mapfilesize;

	MapFreeMem();
	maperror = MER_NONE;
	
	buflen = fs_load(mapname, (void **)&buffer);
	
	sprintf(pfn, "/map.fmp");
	
	if( fs_ramdisk_attach(pfn, buffer, buflen) < 0 ) {
        #ifdef DEBUG_MAP
        printf("MapLoad(): Unable to attach mappy file to ramdisk at /ram/map.fmp\n");
        #endif 
        return false;
    }
    
    sprintf(pfn, "/ram/map.fmp");

	mapfilept = fopen (pfn, "rb");
	if(mapfilept == NULL) { 
        #ifdef DEBUG_MAP
        printf("MapLoad(): Could not open /ram/map.fmp\n");
        #endif 
        return -1; 
    }
    
	fseek(mapfilept, 0, SEEK_END);
	mapfilesize = ftell(mapfilept);
	fseek(mapfilept, 0, SEEK_SET);
	
    if(fread(&mapgenheader, 1, sizeof(GENHEAD), mapfilept) != sizeof(GENHEAD))  { 
        maperror = MER_MAPLOADERROR; 
        fclose(mapfilept); 
        return -1; 
    }
    
	if(mapgenheader.id1 != 'F') maperror = MER_MAPLOADERROR;
	if(mapgenheader.id2 != 'O') maperror = MER_MAPLOADERROR;
	if(mapgenheader.id3 != 'R') maperror = MER_MAPLOADERROR;
	if(mapgenheader.id4 != 'M') maperror = MER_MAPLOADERROR;

	if((mapfilesize == -1) || (mapfilesize != ((byteswapl(mapgenheader.headsize))+ 8))) { 
        maperror = MER_MAPLOADERROR;
    }

	if(maperror) { 
        #ifdef DEBUG_MAP
        printf("MapLoad(): Something is wrong with the header of the %s file. After FORM\n", mapname);
        #endif         
        fclose (mapfilept); 
        return -1; 
    }
  
	if (fread(&mapgenheader, 1, sizeof(GENHEAD)-4, mapfilept) != sizeof(GENHEAD)-4) { 
        maperror = MER_MAPLOADERROR; 
        fclose(mapfilept); 
        return -1; 
    } 
    
	maperror = MER_MAPLOADERROR;
	
	if(mapgenheader.id1 == 'F') { 
    if(mapgenheader.id2 == 'M') { 
    if(mapgenheader.id3 == 'A') { 
    if(mapgenheader.id4 == 'P') maperror = MER_NONE; 
    } } }
    
	if(maperror) { 
        #ifdef DEBUG_MAP
        printf("MapLoad(): Something is wrong with the header of the %s file. After FMAP\n", mapname);
        #endif           
        fclose(mapfilept); 
        return -1; 
    }
    
    // All errors are past, read the file!
	while (mapfilesize!=(ftell(mapfilept))) {
          
		if (fread (&mapgenheader, 1, sizeof(GENHEAD),mapfilept) != sizeof(GENHEAD)) {  
            #ifdef DEBUG_MAP
            printf("MapLoad(): Something is wrong with the size of a header of one of the chunks(MPHD, BODY, CMAP, etc).\n");
            #endif  
            fclose (mapfilept); 
            return -1; 
        }

		i = 0;
		
		if (mapgenheader.id1=='M')  if (mapgenheader.id2=='P')  if (mapgenheader.id3=='H')
			if (mapgenheader.id4=='D')
				{ MapDecodeMPHD(); i = 1; }	
		
		if (mapgenheader.id1=='C')  if (mapgenheader.id2=='M')  if (mapgenheader.id3=='A')
			if (mapgenheader.id4=='P')
				{ MapDecodeCMAP(); i = 1; }
		
		if (mapgenheader.id1=='B')  if (mapgenheader.id2=='K')  if (mapgenheader.id3=='D')
			if (mapgenheader.id4=='T')
				{ MapDecodeBKDT(); i = 1; }
		
		if (mapgenheader.id1=='A')  if (mapgenheader.id2=='N')  if (mapgenheader.id3=='D')
			if (mapgenheader.id4=='T')
				{ MapDecodeANDT(); i = 1; }
		
		if (mapgenheader.id1=='B')  if (mapgenheader.id2=='G')  if (mapgenheader.id3=='F')
			if (mapgenheader.id4=='X')
				{ MapDecodeBGFX(); i = 1; }
		
		if (mapgenheader.id1=='B')  if (mapgenheader.id2=='O')  if (mapgenheader.id3=='D')
			if (mapgenheader.id4=='Y')
				{ MapDecodeBODY(); i = 1; }
		
		if (mapgenheader.id1=='L')  if (mapgenheader.id2=='Y')  if (mapgenheader.id3=='R')
			if (mapgenheader.id4=='1')
				{ MapDecodeLYR1(); i = 1; }
		
		if (mapgenheader.id1=='L')  if (mapgenheader.id2=='Y')  if (mapgenheader.id3=='R')
			if (mapgenheader.id4=='2')
				{ MapDecodeLYR2(); i = 1; }
		
		if (mapgenheader.id1=='L')  if (mapgenheader.id2=='Y')  if (mapgenheader.id3=='R')
			if (mapgenheader.id4=='3')
				{ MapDecodeLYR3(); i = 1; }
		
		if (mapgenheader.id1=='L')  if (mapgenheader.id2=='Y')  if (mapgenheader.id3=='R')
			if (mapgenheader.id4=='4')
				{ MapDecodeLYR4(); i = 1; }
		
		if (mapgenheader.id1=='L')  if (mapgenheader.id2=='Y')  if (mapgenheader.id3=='R')
			if (mapgenheader.id4=='5')
				{ MapDecodeLYR5(); i = 1; }
		
		if (mapgenheader.id1=='L')  if (mapgenheader.id2=='Y')  if (mapgenheader.id3=='R')
			if (mapgenheader.id4=='6')
				{ MapDecodeLYR6(); i = 1; }
		
		if (mapgenheader.id1=='L')  if (mapgenheader.id2=='Y')  if (mapgenheader.id3=='R')
			if (mapgenheader.id4=='7')
				{ MapDecodeLYR7(); i = 1; }
		
		if (!i) MapDecodeNULL();
		if (maperror) { fclose (mapfilept); return -1; }
	}
	fclose (mapfilept);
	fs_unlink(pfn);	
	
	return 0;
}

// ---------------------------------------------------------------------------------
// Calling this may speed up searching for tiles
// ---------------------------------------------------------------------------------
int DCMappy::MapGenerateYLookup (void) {
	int i, j;

	for (i=0;i<8;i++) {
		if (mapmaparraypt[i]!=NULL) { free (mapmaparraypt[i]); mapmaparraypt[i] = NULL; }
		if (mapmappt[i]!=NULL) {
			mapmaparraypt[i] = (short int **) malloc (mapheight*sizeof(short int *));
			if (mapmaparraypt[i] == NULL) return -1;
			for (j=0;j<mapheight;j++) mapmaparraypt[i][j] = (mapmappt[i]+(j*mapwidth));
			if (mapmappt[i] == mappt) maparraypt = mapmaparraypt[i];
		}
	}
	return 0;
}

int DCMappy::MapGetshort (unsigned char * locpt) {
    
    int rval;

	if (mapislsb)
	rval = ((((int) (locpt[1]))<<8)|((int) (locpt[0])));
	else
	rval = ((((int) (locpt[0]))<<8)|((int) (locpt[1])));
	if (rval & 0x8000) rval -= 0x10000;
	return rval;
}

int DCMappy::MapGetlong (unsigned char * locpt) {
    
	if (mapislsb)
	return ((((int) (locpt[3]))<<24)|(((int) (locpt[2]))<<16)|
		(((int) (locpt[1]))<<8)|((int) (locpt[0])));
	else
	return ((((int) (locpt[0]))<<24)|(((int) (locpt[1]))<<16)|
		(((int) (locpt[2]))<<8)|((int) (locpt[3])));
}

// ---------------------------------------------------------------------------------
// Used to skip over useless information such as the Author of the *.fmp file and 
// version of the editor used to create it.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeNULL (void) {
    
	char * mynllpt;

	mynllpt = (char *) malloc(byteswapl(mapgenheader.headsize));
	if (mynllpt == NULL) { 
        #ifdef DEBUG_MAP 
        printf("MapDecodeNULL(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1; 
    }
	fread(mynllpt, byteswapl(mapgenheader.headsize), 1, mapfilept);
	free(mynllpt);
	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Map Header (See MPHD struct in dcmappy.h)
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeMPHD (void) {
    
	MPHD	* hdrmempt;

	hdrmempt = (MPHD *) malloc (byteswapl(mapgenheader.headsize));
	if (hdrmempt==NULL) {
        #ifdef DEBUG_MAP 
        printf("MapDecodeMPHD(): Ran out of memory to allocate.\n"); 
        #endif
        return -1;
    }
	fread (hdrmempt, byteswapl(mapgenheader.headsize), 1, mapfilept);
	mapislsb = (hdrmempt->lsb) ? 1 : 0;
	mapwidth = hdrmempt->mapwidth;
	mapheight = hdrmempt->mapheight;
	mapblockwidth = hdrmempt->blockwidth;
	mapblockheight = hdrmempt->blockheight;
	mapdepth = hdrmempt->blockdepth;
	mapblockstrsize = hdrmempt->blockstrsize;
	mapnumblockstr = hdrmempt->numblockstr;
	mapnumblockgfx = hdrmempt->numblockgfx;
	free (hdrmempt);

    // Changes which palette is selected.
    if(mapdepth == 8){ palette++; }
    
	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes an array of short ints containing positive offsets into BKDT, and negative 
// offsets into ANDT. 
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeBODY (void) {
    
	mappt = (short int *) malloc (byteswapl(mapgenheader.headsize));
	if (mappt==NULL) { 
        #ifdef DEBUG_MAP 
        printf("MapDecodeBODY(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	fread (mappt, byteswapl(mapgenheader.headsize), 1, mapfilept);
	mapmappt[0] = mappt;

	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Colour palette for 8bit maps, red byte, green byte, blue byte for however 
// many colours are needed (so usually 256*3 bytes = 768 bytes).
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeCMAP (void) {
     
	int i;
	unsigned char * index;
    
	mapcmappt = (unsigned char *) malloc(byteswapl(mapgenheader.headsize));
	if (mapcmappt==NULL) { 
        #ifdef DEBUG_MAP 
        printf("MapDecodeCMAP(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	fread (mapcmappt, byteswapl(mapgenheader.headsize), 1, mapfilept);

	index = mapcmappt;
	
	for(i = 0;i < 256; i++) {
		mappept[i].r = (char)*index; 
        index++;
		mappept[i].g = (char)*index; 
        index++;
		mappept[i].b = (char)*index; 
        index++;
	}
	
	for (i = 0; i < 256; i++) {
        if((makecolor16(mappept[i].r, mappept[i].g, mappept[i].b)) == ColorKey) {   // If it matches ColorKey ...
            pvr_set_pal_entry(i + 256*palette, PACK_ARGB8888(0, 0, 0, 0));          // ... make it transparent
        } else {
            pvr_set_pal_entry(i + 256*palette, PACK_ARGB8888(255, mappept[i].r, mappept[i].g, mappept[i].b));    
        } 
    }

	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Block data. Contains BLKSTR structures for however many block structures 
// were made.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeBKDT (void) {
    
    int i, j;
    unsigned char * mdatpt;
    unsigned char * temp;
    BLKSTR * myblkpt;

	mapblockstrpt = (char *) malloc (mapnumblockstr*sizeof(BLKSTR));
	mdatpt = (unsigned char *) malloc (mapnumblockstr*sizeof(BLKSTR));
	temp = mdatpt;
	if (mapblockstrpt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mdatpt, byteswapl(mapgenheader.headsize), 1, mapfilept);
	myblkpt = (BLKSTR *) mapblockstrpt;
    j = (mapblockwidth*mapblockheight*((mapdepth+1)/8));
	i = 0; while (i < (mapnumblockstr*mapblockstrsize)) {
		myblkpt->bgoff = (int) MapGetlong(mdatpt+i);
		myblkpt->fgoff = (int) MapGetlong(mdatpt+i+4);
		myblkpt->fgoff2 = (int) MapGetlong(mdatpt+i+8);
		myblkpt->fgoff3 = (int) MapGetlong(mdatpt+i+12);

        myblkpt->bgoff /= j;
        myblkpt->fgoff /= j;
        myblkpt->fgoff2 /= j;
        myblkpt->fgoff3 /= j;

		myblkpt->user1 = (int) MapGetlong(mdatpt+i+16);
		myblkpt->user2 = (int) MapGetlong(mdatpt+i+20);
		myblkpt->user3 = (short int) MapGetshort(mdatpt+i+24);
		myblkpt->user4 = (short int) MapGetshort(mdatpt+i+26);
		myblkpt->user5 = mdatpt[i+28];
		myblkpt->user6 = mdatpt[i+29];
		myblkpt->user7 = mdatpt[i+30];
		myblkpt->unused3 = (mdatpt[i+31]&0x80) ? 1 : 0;
	    myblkpt->unused2 = (mdatpt[i+31]&0x40) ? 1 : 0;
	    myblkpt->unused1 = (mdatpt[i+31]&0x20) ? 1 : 0;
	    myblkpt->trigger = (mdatpt[i+31]&0x10) ? 1 : 0;
	    myblkpt->br = (mdatpt[i+31]&0x08) ? 1 : 0;
	    myblkpt->bl = (mdatpt[i+31]&0x04) ? 1 : 0;
	    myblkpt->tr = (mdatpt[i+31]&0x02) ? 1 : 0;
	    myblkpt->tl = (mdatpt[i+31]&0x01) ? 1 : 0;
		i += mapblockstrsize;
		myblkpt++;
	}
	/*
	for(i=0; i<mapnumblockstr; i++) {				
		 printf ("B%d: %d,%d,%d,%d\n", i, ((BLKSTR *) mapblockstrpt)[i].tl,((BLKSTR *) mapblockstrpt)[i].tr,((BLKSTR *) mapblockstrpt)[i].bl,((BLKSTR *) mapblockstrpt)[i].br );  
	}*/
	
	free(temp);
 
	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Animation data. Contains ANISTR structures for however many animation 
// structures were made, and also animation data.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeANDT (void) {
    
	ANDTSize =  byteswapl(mapgenheader.headsize);
	mapanimstrpt = (char *) malloc(ANDTSize);
	
	if (mapanimstrpt==NULL) {
        #ifdef DEBUG_MAP 
        printf("MapDecodeANDT(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	mapanimstrendpt = (char *) (mapanimstrpt+(byteswapl(mapgenheader.headsize)));
	fread (mapanimstrpt, byteswapl(mapgenheader.headsize), 1, mapfilept);
	
	MapInitAnims();
	
	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes the raw graphics in whatever format the map is in. If mapdepth == 8 we setup
// the dreamcast palette. If mapdepth == 16 we convert the raw RGB565 data to ARGB1555
// data. If mapdepth == 24 we convert the raw RGB888 data to ARGB1555 data..and so forth
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeBGFX (void) {
    
    int               i;
    int               bytes;
    int               textureFormat;
    uint16            * rawimagedata = NULL;   // Holds the raw graphics if the format the map is [8bit;16bit]
	pvr_ptr_t         mapblockgfxpt = NULL; 
	pvr_poly_cxt_t    cxt;
	
	maplpDDSTiles_OP = (pvr_poly_hdr_t *) malloc(sizeof(pvr_poly_hdr_t) * mapnumblockgfx);
	maplpDDSTiles_PT = (pvr_poly_hdr_t *) malloc(sizeof(pvr_poly_hdr_t) * mapnumblockgfx);
	tileGraphics = (pvr_ptr_t *) malloc(sizeof(pvr_ptr_t) * mapnumblockgfx);
	
    // Allocated raw image data memory(one block at a time) depending on the mapdepth[8:1, 16:2] 
    if(mapdepth == 8) {
        bytes = mapblockwidth*mapblockheight;        
        rawimagedata = (uint16 *)malloc(bytes);
        pvr_set_pal_format(PVR_PAL_ARGB8888);
        textureFormat = PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(palette); // Palette: (0)-1st (1)-2nd (2)-3rd (3)-4th 
    } else if(mapdepth == 16 | mapdepth == 24 | mapdepth == 32) {  // Will be converted from 24bpp or 32bpp to 16bpp(ARGB1555)
        bytes = mapblockwidth*mapblockheight*2; 
        rawimagedata = (uint16 *)malloc(bytes);
        textureFormat = PVR_TXRFMT_ARGB1555;
    } else {
        #ifdef DEBUG_MAP   
        printf("MapDecodeBGFX(): Map Depth %d is not supported.\n", mapdepth);
        #endif
        return -1;
    } 
    
    if (rawimagedata == NULL) { 
        #ifdef DEBUG_MAP   
        printf("MapDecodeBGFX(): Ran out of memory to allocate for rawimagedata.\n" );
        #endif
        return -1; 
    }
    
    // One block at a time filling up maplpDDSTiles
    for(i = 0; i < mapnumblockgfx; i++) {
        
         // Allocated PVR Memory(one block at a time) depending on the mapdepth 
        if(mapdepth == 8) {
            fread(rawimagedata, bytes, 1, mapfilept);      
            mapblockgfxpt = pvr_mem_malloc(bytes);
            if(mapblockgfxpt==NULL) {  
                #ifdef DEBUG_MAP   
                printf("MapDecodeBGFX(): Ran out of TEXTURE memory to allocate for mapblockgfxpt.\n" );
                #endif
                return -1; 
            }
            pvr_txr_load_ex((void *)rawimagedata, mapblockgfxpt, mapblockwidth, mapblockheight, PVR_TXRLOAD_8BPP);
        } else if(mapdepth == 16) {
            fread(rawimagedata, bytes, 1, mapfilept); 
            mapblockgfxpt = pvr_mem_malloc(bytes);
            if(mapblockgfxpt==NULL) {  
                #ifdef DEBUG_MAP   
                printf("MapDecodeBGFX(): Ran out of TEXTURE memory to allocate for mapblockgfxpt.\n" );
                #endif
                return -1; 
            }
            swap_convert_RGB565_to_ARGB1555(rawimagedata, rawimagedata, mapblockwidth*mapblockheight, ColorKey);
            pvr_txr_load_ex((void *)rawimagedata, mapblockgfxpt, mapblockwidth, mapblockheight, PVR_TXRLOAD_16BPP);
        } else if (mapdepth == 24) { 
            mapblockgfxpt = pvr_mem_malloc(bytes);  
            if(mapblockgfxpt==NULL) {  
                #ifdef DEBUG_MAP   
                printf("MapDecodeBGFX(): Ran out of TEXTURE memory to allocate for mapblockgfxpt.\n" );
                #endif
                return -1; 
            }
            swap_convert_RGB888_to_ARGB1555(mapfilept, rawimagedata, mapblockwidth*mapblockheight, ColorKey);
            pvr_txr_load_ex((void *)rawimagedata, mapblockgfxpt, mapblockwidth, mapblockheight, PVR_TXRLOAD_16BPP);
        } else if (mapdepth == 32) { 
            mapblockgfxpt = pvr_mem_malloc(bytes);  
            if(mapblockgfxpt==NULL) {  
                #ifdef DEBUG_MAP   
                printf("MapDecodeBGFX(): Ran out of TEXTURE memory to allocate for mapblockgfxpt.\n" );
                #endif
                return -1; 
            }
            swap_convert_ARGB8888_to_ARGB1555(mapfilept, rawimagedata, mapblockwidth*mapblockheight, ColorKey);
            pvr_txr_load_ex((void *)rawimagedata, mapblockgfxpt, mapblockwidth, mapblockheight, PVR_TXRLOAD_16BPP);
        } 
        
        // Setup a pvr header struct to hold each gfx block info; OP
        pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, textureFormat, mapblockwidth, mapblockheight, mapblockgfxpt, PVR_FILTER_NONE);
        pvr_poly_compile(&maplpDDSTiles_OP[i], &cxt);
        
        // Setup a pvr header struct to hold each gfx block info; PT
        pvr_poly_cxt_txr(&cxt, PVR_LIST_PT_POLY, textureFormat, mapblockwidth, mapblockheight, mapblockgfxpt, PVR_FILTER_NONE);
        pvr_poly_compile(&maplpDDSTiles_PT[i], &cxt);
        
        tileGraphics[i] = mapblockgfxpt;
        
        mapblockgfxpt = NULL;
    } // End of mapnumblockgfx
    free(rawimagedata);

	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Layer Data(LYR1). These are the same size and format as BODY, and allow 
// object layers to be used.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeLYR1 (void) {
    
	mapmappt[1] = (short int *) malloc (byteswapl(mapgenheader.headsize));
	if (mapmappt[1]==NULL) {
        #ifdef DEBUG_MAP 
        printf("MapDecodeLYR1(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	fread (mapmappt[1], byteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Layer Data(LYR2). These are the same size and format as BODY, and allow 
// object layers to be used.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeLYR2 (void) {
    
	mapmappt[2] = (short int *) malloc (byteswapl(mapgenheader.headsize));
	if (mapmappt[2]==NULL) {
        #ifdef DEBUG_MAP 
        printf("MapDecodeLYR2(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	fread (mapmappt[2], byteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Layer Data(LYR3). These are the same size and format as BODY, and allow 
// object layers to be used.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeLYR3 (void) {
    
	mapmappt[3] = (short int *) malloc (byteswapl(mapgenheader.headsize));
	if (mapmappt[3]==NULL) { 
        #ifdef DEBUG_MAP 
        printf("MapDecodeLYR3(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	fread (mapmappt[3], byteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Layer Data(LYR4). These are the same size and format as BODY, and allow 
// object layers to be used.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeLYR4 (void) {
    
	mapmappt[4] = (short int *) malloc (byteswapl(mapgenheader.headsize));
	if (mapmappt[4]==NULL) {
        #ifdef DEBUG_MAP 
        printf("MapDecodeLYR4(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	fread (mapmappt[4], byteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Layer Data(LYR5). These are the same size and format as BODY, and allow 
// object layers to be used.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeLYR5 (void) {
    
	mapmappt[5] = (short int *) malloc (byteswapl(mapgenheader.headsize));
	if (mapmappt[5]==NULL) {
        #ifdef DEBUG_MAP 
        printf("MapDecodeLYR5(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	fread (mapmappt[5], byteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Layer Data(LYR6). These are the same size and format as BODY, and allow 
// object layers to be used.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeLYR6 (void) {
    
	mapmappt[6] = (short int *) malloc (byteswapl(mapgenheader.headsize));
	if (mapmappt[6]==NULL) {
        #ifdef DEBUG_MAP 
        printf("MapDecodeLYR6(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	fread (mapmappt[6], byteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

// ---------------------------------------------------------------------------------
// Decodes Layer Data(LYR7). These are the same size and format as BODY, and allow 
// object layers to be used.
// ---------------------------------------------------------------------------------
int DCMappy::MapDecodeLYR7 (void) {
    
	mapmappt[7] = (short int *) malloc (byteswapl(mapgenheader.headsize));
	if (mapmappt[7]==NULL) {
        #ifdef DEBUG_MAP 
        printf("MapDecodeLYR7(): Ran out of memory to allocate.\n"); 
        #endif 
        return -1;
    }
	fread (mapmappt[7], byteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

// ---------------------------------------------------------------------------------
// Returns a BLKSTR pointer, useful for collision detection and examining a block structure.
// ---------------------------------------------------------------------------------
BLKSTR * DCMappy::MapGetBlock (int x, int y) {
       
    int numanim;   
	short int * mymappt;
	ANISTR * myanpt;
    
   	if (maparraypt!= NULL) {
	    mymappt = maparraypt[y]+x;
	} else {
        mymappt = mappt;
	    mymappt += x;
	    mymappt += y*mapwidth;
    }

	if (*mymappt>=0) return (BLKSTR*) (mapblockstrpt + *mymappt);
	else { 
        myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);	
        numanim = mapanimstrpt[ANDTSize+myanpt->ancuroff+1]&0XFF;
        numanim <<= 8;
        numanim |= mapanimstrpt[ANDTSize+myanpt->ancuroff]&0XFF;	    
	    return (BLKSTR *) (mapblockstrpt+numanim); 
    }
}

//----------------------------------------------------------------------------------
// Same as above, except the user can specify the LAYER he/she wishes to
// examine.
// Auto-resets the layer to 0
//-----------------------------------------------------------------------------------
BLKSTR* DCMappy::MapGetBlockLayer (int x, int y, int layer) { 
        
  BLKSTR* tempblock;
  MapChangeLayer(layer);
  tempblock = MapGetBlock(x, y);
  MapChangeLayer(0);
  return tempblock;
}

// ---------------------------------------------------------------------------------
// The x and y paramaters are the offset from the left and top of the map in BLOCKS, 
// NOT pixels.
// ---------------------------------------------------------------------------------
void DCMappy::MapSetBlock (int x, int y, int strvalue) {
     
	short int * mymappt;

    if (maparraypt!= NULL) {
		mymappt = maparraypt[y]+x;
	} else {
        mymappt = mappt;
        mymappt += x;
        mymappt += y*mapwidth;
    }
	
	if (strvalue>=0) *mymappt = strvalue*sizeof(BLKSTR);
	else *mymappt = strvalue*sizeof(ANISTR);
}

// ------------------------------------------------------------
// Returns the content of a block
// if return is positive then the block is a normal tile
// if return is negative then the block is an animation 
// ------------------------------------------------------------
short int DCMappy::MapGetTile (int x, int y) {
      
	short int	Content;

	Content = mappt[x+(y*mapwidth)];

	if( Content >= 0 )
		Content /= sizeof(BLKSTR);
	else
	{
		Content = -Content / sizeof(ANISTR) ;
		Content = -Content + 1;
	}

	return Content;
}

// ---------------------------------------------------------------------------------
// Returns the width of the map in pixels
// ---------------------------------------------------------------------------------
unsigned int DCMappy::GetMapWidth (void) { return mapwidth * mapblockwidth; }

// ---------------------------------------------------------------------------------
// Returns the width of the map in pixels
// ---------------------------------------------------------------------------------
unsigned int DCMappy::GetMapHeight (void) { return mapheight * mapblockheight; }

// ---------------------------------------------------------------------------------
// Sets the location of the uppermost top of the map
// ---------------------------------------------------------------------------------
void DCMappy::MapSetLocation(int x, int y) {
	MMOX = x;
	MMOY = y;
}

// ---------------------------------------------------------------------------------
// Changes the part of the map that we view on the screen
// ---------------------------------------------------------------------------------
void DCMappy::MapMoveTo (int x, int y) {
     
    int Width , Height;

    XPosition = x;
    YPosition = y;

    Width = GetMapWidth();

    if( XPosition > (Width - 1 - MSCRW))
        XPosition = Width-1-MSCRW;

    if( XPosition < 0 )
        XPosition = 0;

    Height = GetMapHeight();

    if( YPosition > (Height - 1 - MSCRH))
        YPosition = Height-1-MSCRH;

    if( YPosition < 0 )
        YPosition = 0;
}

// ---------------------------------------------------------------------------------
// Changes the number of the layer (0 to 7) that the drawing functions will concentrate
// on.
// ---------------------------------------------------------------------------------
int DCMappy::MapChangeLayer (int newlyr) {
    
	if (newlyr < 0 || newlyr > 7 || mapmappt[newlyr] == NULL) return -1;
	mappt = mapmappt[newlyr]; maparraypt = mapmaparraypt[newlyr];
	return newlyr;
}

// ---------------------------------------------------------------------------------
// Draws the background layer WITHOUT Transparency
// Should only be called between pvr_list_begin(PVR_LIST_OP_POLY) and pvr_list_finish().
// ---------------------------------------------------------------------------------
int DCMappy::MapDrawBG (void) {
    
    #ifdef DEBUG_MAP
    if(pvr_state.list_reg_open != PVR_LIST_OP_POLY) {
        printf("MapDrawBG() can only be called after pvr_list_begin(PVR_LIST_OP_POLY) and NOT after pvr_list_begin(PVR_LIST_TR_POLY).\n");
        return -1;
    }
    #endif
    
    int			i, j, mapvclip, maphclip;
    int         x_max, y_max, x_min, y_min;
    int 		numtile, numanim;
    float       temp;
    ANISTR		* anim;
	Block_Rect  TileDestRect;
	
	x_min = XPosition/mapblockwidth;
    y_min = YPosition/mapblockheight;
    x_max = x_min+(MSCRW/mapblockwidth)+1;
    y_max = y_min+(MSCRH/mapblockheight)+1;
    
	maphclip=XPosition%mapblockwidth;
	mapvclip=YPosition%mapblockheight;	
	
	for(i = 0; i <= y_max; i++) {
		for(j = 0; j < x_max; j++)
		{	
            TileDestRect.x	= j*mapblockwidth-maphclip + MMOX;
			TileDestRect.y	= i*mapblockheight-mapvclip + MMOY;
			TileDestRect.h	= mapblockheight; 
            TileDestRect.w	= mapblockwidth;  
            
			// Left
			if(TileDestRect.x < MMOX) {                            
                TileDestRect.w = mapblockwidth - (MMOX - TileDestRect.x);            
                if(TileDestRect.w <= 0) continue;
                TileDestRect.x	= MMOX;
                temp = mapblockwidth - TileDestRect.w;
                temp = temp / mapblockwidth;
                TileDestRect.u1 = temp;
                
                // Handle Corners(Top Left then Bottom Left)
                if(TileDestRect.y < MMOY){
                    TileDestRect.h = mapblockheight - (MMOY - TileDestRect.y);
                    TileDestRect.y	= MMOY;
                    temp = mapblockheight - TileDestRect.h;
                    temp = temp / mapblockheight;
                    TileDestRect.v1 = temp;
                } else TileDestRect.v1 = 0.0f;
                
                if((TileDestRect.y+mapblockheight) > (MMOY+MSCRH)) {
                        TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                        if(TileDestRect.h > mapblockheight) continue;
                        temp = TileDestRect.h;
			            temp = temp/mapblockheight;
                        TileDestRect.v2 = temp;
                } else TileDestRect.v2 = 1.0f;
			    TileDestRect.u2 = 1.0f; 
			    
			    // Right
            } else if((TileDestRect.x+mapblockwidth) > (MMOX+MSCRW)) {
                TileDestRect.w = (MMOX+MSCRW) - TileDestRect.x;
                if(TileDestRect.w > mapblockwidth) continue; 
                TileDestRect.u1 = 0.0f;
                
                // Handle Corners(Top Right then Bottom Right)
                if(TileDestRect.y < MMOY){
                    TileDestRect.h = mapblockheight - (MMOY - TileDestRect.y);
                    TileDestRect.y	= MMOY;
                    temp = mapblockheight - TileDestRect.h;
                    temp = temp / mapblockheight;
                    TileDestRect.v1 = temp;
                } else TileDestRect.v1 = 0.0f;
                
                if((TileDestRect.y+mapblockheight) > (MMOY+MSCRH)) {
                        TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                        if(TileDestRect.h > mapblockheight) continue;
                        temp = TileDestRect.h;
			            temp = temp/mapblockheight;
                        TileDestRect.v2 = temp;
                } else TileDestRect.v2 = 1.0f;
                temp = TileDestRect.w;
                temp = temp / mapblockwidth;
			    TileDestRect.u2 = temp;
                
                // Top
            } else if(TileDestRect.y < MMOY) {
                TileDestRect.h = mapblockheight - (MMOY - TileDestRect.y);
                if( TileDestRect.h <= 0) continue;
                TileDestRect.y	= MMOY;
                temp = mapblockheight - TileDestRect.h;
                temp = temp / mapblockheight;
                TileDestRect.u1 = 0.0f;
                TileDestRect.v1 = temp;
			    TileDestRect.u2 = TileDestRect.v2 = 1.0f;
			    
			    // Bottom
            } else if((TileDestRect.y+mapblockheight) > (MMOY+MSCRH)) {
                TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                if(TileDestRect.h > mapblockheight) continue;   
                TileDestRect.u1 = TileDestRect.v1 = 0.0f;
			    TileDestRect.u2 = 1.0f;
			    temp = TileDestRect.h;
			    temp = temp/mapblockheight;
                TileDestRect.v2 = temp;
                
                // Default
            } else {			
			    TileDestRect.u1 = TileDestRect.v1 = 0.0f;
			    TileDestRect.u2 = TileDestRect.v2 = 1.0f;
            } 
			
			numtile = mappt[x_min+j+((y_min+i)*mapwidth)];
			
			if (((x_min+j)<mapwidth) && ((y_min+i)<mapheight))
			    if (numtile>=0)
					MapDrawBlock(&maplpDDSTiles_OP[((BLKSTR *) (mapblockstrpt+numtile))->bgoff], &TileDestRect); 
			else {
			    anim = (ANISTR *) (mapanimstrendpt + numtile);
			    numanim = mapanimstrpt[ANDTSize+anim->ancuroff+1]&0XFF;
			    numanim <<= 8;
			    numanim |= mapanimstrpt[ANDTSize+anim->ancuroff]&0XFF;
			    MapDrawBlock(&maplpDDSTiles_OP[((BLKSTR *) (mapblockstrpt+numanim))->bgoff], &TileDestRect); 
			}
		}
    }
	
	return 0;
}

// ---------------------------------------------------------------------------------
// Draws the background layer WITH Transparency 
// Should only be called between pvr_list_begin(PVR_LIST_TR_POLY) and pvr_list_finish().
// ---------------------------------------------------------------------------------
int DCMappy::MapDrawBGT (void) {
    
    #ifdef DEBUG_MAP
    if(pvr_state.list_reg_open != PVR_LIST_PT_POLY) {
        printf("MapDrawBGT() can only be called after pvr_list_begin(PVR_LIST_PT_POLY) and NOT after pvr_list_begin(PVR_LIST_OP_POLY).\n");      
        return -1;
    }
    #endif
    
	int			i, j, mapvclip, maphclip;
    int         x_max, y_max, x_min, y_min;
    int 		numtile, numanim;
    float       temp;
    ANISTR		* anim;
	Block_Rect  TileDestRect;
	
	x_max = y_max = x_min = y_min = 0;
	
	x_min = XPosition/mapblockwidth;
    y_min = YPosition/mapblockheight;
    x_max = (MSCRW/mapblockwidth) + 1;
    y_max = (MSCRH/mapblockheight) + 1;
    
	maphclip=XPosition%mapblockwidth;
	mapvclip=YPosition%mapblockheight;	
	
	for(i = 0; i <= y_max; i++) {
		for(j = 0; j < x_max; j++)
		{	
			TileDestRect.x	= j*mapblockwidth-maphclip + MMOX;
			TileDestRect.y	= i*mapblockheight-mapvclip + MMOY;
			TileDestRect.h	= mapblockheight; 
            TileDestRect.w	= mapblockwidth;  
            
			// Left
			if(TileDestRect.x < MMOX) {                            
                TileDestRect.w = mapblockwidth - (MMOX - TileDestRect.x);            
                if(TileDestRect.w <= 0) continue;
                TileDestRect.x	= MMOX;
                temp = mapblockwidth - TileDestRect.w;
                temp = temp / mapblockwidth;
                TileDestRect.u1 = temp;
                
                // Handle Corners(Top Left then Bottom Left)
                if(TileDestRect.y < MMOY){
                    TileDestRect.h = mapblockheight - (MMOY - TileDestRect.y);
                    TileDestRect.y	= MMOY;
                    temp = mapblockheight - TileDestRect.h;
                    temp = temp / mapblockheight;
                    TileDestRect.v1 = temp;
                } else TileDestRect.v1 = 0.0f;
                
                if((TileDestRect.y+mapblockheight) > (MMOY+MSCRH)) {
                        TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                        if(TileDestRect.h > mapblockheight) continue;
                        temp = TileDestRect.h;
			            temp = temp/mapblockheight;
                        TileDestRect.v2 = temp;
                } else TileDestRect.v2 = 1.0f;
			    TileDestRect.u2 = 1.0f; 
			    
			    // Right
            } else if((TileDestRect.x+mapblockwidth) > (MMOX+MSCRW)) {
                TileDestRect.w = (MMOX+MSCRW) - TileDestRect.x;
                if(TileDestRect.w > mapblockwidth) continue; 
                TileDestRect.u1 = 0.0f;
                
                // Handle Corners(Top Right then Bottom Right)
                if(TileDestRect.y < MMOY){
                    TileDestRect.h = mapblockheight - (MMOY - TileDestRect.y);
                    TileDestRect.y	= MMOY;
                    temp = mapblockheight - TileDestRect.h;
                    temp = temp / mapblockheight;
                    TileDestRect.v1 = temp;
                } else TileDestRect.v1 = 0.0f;
                
                if((TileDestRect.y+mapblockheight) > (MMOY+MSCRH)) {
                        TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                        if(TileDestRect.h > mapblockheight) continue;
                        temp = TileDestRect.h;
			            temp = temp/mapblockheight;
                        TileDestRect.v2 = temp;
                } else TileDestRect.v2 = 1.0f;
                temp = TileDestRect.w;
                temp = temp / mapblockwidth;
			    TileDestRect.u2 = temp;
                
                // Top
            } else if(TileDestRect.y < MMOY) {
                TileDestRect.h = mapblockheight - (MMOY - TileDestRect.y);
                if( TileDestRect.h <= 0) continue;
                TileDestRect.y	= MMOY;
                temp = mapblockheight - TileDestRect.h;
                temp = temp / mapblockheight;
                TileDestRect.u1 = 0.0f;
                TileDestRect.v1 = temp;
			    TileDestRect.u2 = TileDestRect.v2 = 1.0f;
			    
			    // Bottom
            } else if((TileDestRect.y+mapblockheight) > (MMOY+MSCRH)) {
                TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                if(TileDestRect.h > mapblockheight) continue;   
                TileDestRect.u1 = TileDestRect.v1 = 0.0f;
			    TileDestRect.u2 = 1.0f;
			    temp = TileDestRect.h;
			    temp = temp/mapblockheight;
                TileDestRect.v2 = temp;
                
                // Default
            } else {			
			    TileDestRect.u1 = TileDestRect.v1 = 0.0f;
			    TileDestRect.u2 = TileDestRect.v2 = 1.0f;
            } 
			
			numtile = mappt[x_min+j+((y_min+i)*mapwidth)];
			
			if (((x_min+j)<mapwidth) && ((y_min+i)<mapheight))
			    if (numtile>=0)
					MapDrawBlock(&maplpDDSTiles_PT[((BLKSTR *) (mapblockstrpt+numtile))->bgoff], &TileDestRect); 
			else {
			    anim = (ANISTR *) (mapanimstrendpt + numtile);
			    numanim = mapanimstrpt[ANDTSize+anim->ancuroff+1]&0XFF;
			    numanim <<= 8;
			    numanim |= mapanimstrpt[ANDTSize+anim->ancuroff]&0XFF;
			    MapDrawBlock(&maplpDDSTiles_PT[((BLKSTR *) (mapblockstrpt+numanim))->bgoff], &TileDestRect); 
			}
		}
    }
    
	return 0;
}

// ---------------------------------------------------------------------------------
// Draws a foreground layer WITH Transparency; Specify layers with 0, 1, 2 
// Should only be called between pvr_list_begin(PVR_LIST_TR_POLY) and pvr_list_finish().
// ---------------------------------------------------------------------------------
int DCMappy::MapDrawFG (int layer) {
    
    #ifdef DEBUG_MAP
    if(pvr_state.list_reg_open != PVR_LIST_PT_POLY) {
        printf("MapDrawFG() can only be called after pvr_list_begin(PVR_LIST_PT_POLY) and NOT after pvr_list_begin(PVR_LIST_OP_POLY).\n"); 
        return -1;
    }
    #endif 
    
    if(layer > 2) { printf("MapDrawFG(N): The input variable 'N' can only be a value 0 to 2.\n"); return -1; }
    
	int			i, j, mapvclip, maphclip;
    int         x_max, y_max, x_min, y_min;
    int 		numtile, numanim;
    float       temp;
    long int    mapgfxnum;
    ANISTR		* anim;
	Block_Rect  TileDestRect;
	
	x_min = XPosition/mapblockwidth;
    y_min = YPosition/mapblockheight;
    x_max = x_min + (MSCRW/mapblockwidth) + 2;
    y_max = y_min + (MSCRH/mapblockwidth) + 2;
    
	maphclip=XPosition%mapblockwidth;
	mapvclip=YPosition%mapblockheight;	
	
	for(i = 0; i <= y_max; i++)
		for(j = 0; j < x_max; j++)
		{	
			TileDestRect.x	= j*mapblockwidth-maphclip + MMOX;
			TileDestRect.y	= i*mapblockheight-mapvclip + MMOY;
			TileDestRect.h	= mapblockheight; 
            TileDestRect.w	= mapblockwidth;  
            
			// Left
			if(TileDestRect.x < MMOX) {                            
                TileDestRect.w = mapblockwidth - (MMOX - TileDestRect.x);            
                if(TileDestRect.w <= 0) continue;
                TileDestRect.x	= MMOX;
                temp = mapblockwidth - TileDestRect.w;
                temp = temp / mapblockwidth;
                TileDestRect.u1 = temp;
                
                // Handle Corners(Top Left then Bottom Left)
                if(TileDestRect.y < MMOY){
                    TileDestRect.h = mapblockheight - (MMOY - TileDestRect.y);
                    TileDestRect.y	= MMOY;
                    temp = mapblockheight - TileDestRect.h;
                    temp = temp / mapblockheight;
                    TileDestRect.v1 = temp;
                } else TileDestRect.v1 = 0.0f;
                
                if((TileDestRect.y+mapblockheight) > (MMOY+MSCRH)) {
                        TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                        if(TileDestRect.h > mapblockheight) continue;
                        temp = TileDestRect.h;
			            temp = temp/mapblockheight;
                        TileDestRect.v2 = temp;
                } else TileDestRect.v2 = 1.0f;
			    TileDestRect.u2 = 1.0f; 
			    
			    // Right
            } else if((TileDestRect.x+mapblockwidth) > (MMOX+MSCRW)) {
                TileDestRect.w = (MMOX+MSCRW) - TileDestRect.x;
                if(TileDestRect.w > mapblockwidth) continue; 
                TileDestRect.u1 = 0.0f;
                
                // Handle Corners(Top Right then Bottom Right)
                if(TileDestRect.y < MMOY){
                    TileDestRect.h = mapblockheight - (MMOY - TileDestRect.y);
                    TileDestRect.y	= MMOY;
                    temp = mapblockheight - TileDestRect.h;
                    temp = temp / mapblockheight;
                    TileDestRect.v1 = temp;
                } else TileDestRect.v1 = 0.0f;
                
                if((TileDestRect.y+mapblockheight) > (MMOY+MSCRH)) {
                        TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                        if(TileDestRect.h > mapblockheight) continue;
                        temp = TileDestRect.h;
			            temp = temp/mapblockheight;
                        TileDestRect.v2 = temp;
                } else TileDestRect.v2 = 1.0f;
                temp = TileDestRect.w;
                temp = temp / mapblockwidth;
			    TileDestRect.u2 = temp;
                
                // Top
            } else if(TileDestRect.y < MMOY) {
                TileDestRect.h = mapblockheight - (MMOY - TileDestRect.y);
                if( TileDestRect.h <= 0) continue;
                TileDestRect.y	= MMOY;
                temp = mapblockheight - TileDestRect.h;
                temp = temp / mapblockheight;
                TileDestRect.u1 = 0.0f;
                TileDestRect.v1 = temp;
			    TileDestRect.u2 = TileDestRect.v2 = 1.0f;
			    
			    // Bottom
            } else if((TileDestRect.y+mapblockheight) > (MMOY+MSCRH)) {
                TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                if(TileDestRect.h > mapblockheight) continue;   
                TileDestRect.u1 = TileDestRect.v1 = 0.0f;
			    TileDestRect.u2 = 1.0f;
			    temp = TileDestRect.h;
			    temp = temp/mapblockheight;
                TileDestRect.v2 = temp;
                
                // Default
            } else {			
			    TileDestRect.u1 = TileDestRect.v1 = 0.0f;
			    TileDestRect.u2 = TileDestRect.v2 = 1.0f;
            } 
			
			numtile = mappt[x_min+j+((y_min+i)*mapwidth)];
			
			if (((x_min+j)<mapwidth) && ((y_min+i)<mapheight))
			    if (numtile>=0) {
			        if(!layer) mapgfxnum = ((BLKSTR *) (mapblockstrpt+numtile))->fgoff;
			        else if(layer == 1) mapgfxnum = ((BLKSTR *) (mapblockstrpt+numtile))->fgoff2;
			        else mapgfxnum = ((BLKSTR *) (mapblockstrpt+numtile))->fgoff3;
					MapDrawBlock(&maplpDDSTiles_PT[mapgfxnum], &TileDestRect); 
                }
			else {
			    anim = (ANISTR *) (mapanimstrendpt + numtile);
			    numanim = mapanimstrpt[ANDTSize+anim->ancuroff+1]&0XFF;
			    numanim <<= 8;
			    numanim |= mapanimstrpt[ANDTSize+anim->ancuroff]&0XFF;
			    
			    if(!layer) mapgfxnum = ((BLKSTR *) (mapblockstrpt+numanim))->fgoff;
                else if(layer == 1) mapgfxnum = ((BLKSTR *) (mapblockstrpt+numanim))->fgoff2;
                else mapgfxnum = ((BLKSTR *) (mapblockstrpt+numanim))->fgoff3;
			    MapDrawBlock(&maplpDDSTiles_PT[mapgfxnum], &TileDestRect); 
			}
		}
		
	return 0;
}

// ---------------------------------------------------------------------------------
// Initiates the block animations. Automatically called by DecodeANDT() since there was
// animation data to decode.
// ---------------------------------------------------------------------------------
void DCMappy::MapInitAnims (void) {
     
	ANISTR	* myanpt;

	if (mapanimstrpt==NULL) return;
	myanpt = (ANISTR *) mapanimstrendpt; myanpt--;
	while (myanpt->antype!=-1)
	{
		if (myanpt->antype==AN_PPFR) myanpt->antype = AN_PPFF;
		if (myanpt->antype==AN_PPRF) myanpt->antype = AN_PPRR;
		if (myanpt->antype==AN_ONCES) myanpt->antype = AN_ONCE;
		if ((myanpt->antype==AN_LOOPR) || (myanpt->antype==AN_PPRR))
		{
		myanpt->ancuroff = myanpt->anstartoff;
		if ((myanpt->anstartoff)!=(myanpt->anendoff)) myanpt->ancuroff=(myanpt->anendoff)-4; 
		} else {
		myanpt->ancuroff = myanpt->anstartoff;
		}
		myanpt->ancount = myanpt->andelay;
		myanpt--;
	}
}

// ---------------------------------------------------------------------------------
// Updates the animations. Call during the game loop.
// ---------------------------------------------------------------------------------
void DCMappy::MapUpdateAnims (void) {
     
	ANISTR	* myanpt;

	if (mapanimstrpt==NULL) return;
	myanpt = (ANISTR *) mapanimstrendpt; myanpt--;
	while (myanpt->antype!=-1)
	{
		if (myanpt->antype!=AN_NONE) { myanpt->ancount--; if (myanpt->ancount<0) {
		myanpt->ancount = myanpt->andelay;
		if (myanpt->antype==AN_LOOPF)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff+=4;
			if (myanpt->ancuroff==myanpt->anendoff) myanpt->ancuroff = myanpt->anstartoff;
		} }
		if (myanpt->antype==AN_LOOPR)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff-=4;
			if (myanpt->ancuroff==((myanpt->anstartoff)-4))
				myanpt->ancuroff = (myanpt->anendoff)-4;
		} }
		if (myanpt->antype==AN_ONCE)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff+=4;
			if (myanpt->ancuroff==myanpt->anendoff) { myanpt->antype = AN_ONCES;
				myanpt->ancuroff = myanpt->anstartoff; }
		} }
		if (myanpt->antype==AN_ONCEH)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) {
			if (myanpt->ancuroff!=((myanpt->anendoff)-4)) myanpt->ancuroff+=4;
		} }
		if (myanpt->antype==AN_PPFF)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff+=4;
			if (myanpt->ancuroff==myanpt->anendoff) { myanpt->ancuroff -= 8;
			myanpt->antype = AN_PPFR;
			if (myanpt->ancuroff<myanpt->anstartoff) myanpt->ancuroff +=4; }
		} } else {
		if (myanpt->antype==AN_PPFR)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff-=4;
			if (myanpt->ancuroff==((myanpt->anstartoff)-4)) { myanpt->ancuroff += 8;
			myanpt->antype = AN_PPFF;
			if (myanpt->ancuroff>myanpt->anendoff) myanpt->ancuroff -=4; }
		} } }
		if (myanpt->antype==AN_PPRR)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff-=4;
			if (myanpt->ancuroff==((myanpt->anstartoff)-4)) { myanpt->ancuroff += 8;
			myanpt->antype = AN_PPRF;
			if (myanpt->ancuroff>myanpt->anendoff) myanpt->ancuroff -=4; }
		} } else {
		if (myanpt->antype==AN_PPRF)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff+=4;
			if (myanpt->ancuroff==myanpt->anendoff) { myanpt->ancuroff -= 8;
			myanpt->antype = AN_PPRR;
			if (myanpt->ancuroff<myanpt->anstartoff) myanpt->ancuroff +=4; }
		} } }
	} } myanpt--; }
}

// ---------------------------------------------------------------------------------
// Creates a parallax surface from a kmg file
// Returns true, if an error occurs, returns FALSE
// caution: the parallax image size(width or height) must be a power of 2 (ex. 2, 4,
// 8, 16, 32, 64, 128, ..., 1024)
// ---------------------------------------------------------------------------------
bool DCMappy::CreateParallax (char * Filename) {
     
    ParallaxFilename = (char *)malloc(strlen(Filename)+1);
    if(ParallaxFilename == NULL) {
        #ifdef DEBUG_MAP 
        printf("CreateParallax(): Ran out of memory to allocate for ParallaxFilename.\n"); 
        #endif 
        return -1;
    }
    ParallaxSurface = (pvr_poly_hdr_t *)malloc(sizeof(pvr_poly_hdr_t)); 
    strcpy(ParallaxFilename, Filename); 

    return RestoreParallax();
}

// ---------------------------------------------------------------------------------
// Creates a parallax surface from a kmg file
// Returns true, if an error occurs, returns FALSE
// caution: the parallax image size(width or height) must be a power of 2 (ex. 2, 4,
// 8, 16, 32, 64, 128, ..., 1024)
// ---------------------------------------------------------------------------------
bool DCMappy::RestoreParallax (void) {
     
    pvr_ptr_t      ParallaxTemp = NULL;
    kos_img_t      * SurfaceTemp = NULL;
    pvr_poly_cxt_t cxt;

    // when no map is loaded, return an error
    if( GetMapWidth() == 0 )
        return false;

    // load the bitmap into a surface
	if(ParallaxFilename == NULL)
		return false;
		
	SurfaceTemp = (kos_img_t *)malloc(sizeof(kos_img_t)); 
	
	kmg_to_img(ParallaxFilename, SurfaceTemp); 
	
	#ifdef DEBUG_MAP
    printf("ParallaxSurface:\nWidth: %d \nHeight: %d \nByte_Count: %d\n", SurfaceTemp->w, SurfaceTemp->h, SurfaceTemp->byte_count);
    #endif
    
	ParallaxTemp = pvr_mem_malloc(SurfaceTemp->byte_count);
	if ( ParallaxTemp == NULL ) {
        #ifdef DEBUG_MAP 
        printf("RestoreParallax(): Ran out of TEXTURE memory to allocate for ParallaxTemp.\n"); 
        #endif 
		return false;
    }
    
	pvr_txr_load_kimg(SurfaceTemp, ParallaxTemp, PVR_TXRLOAD_16BPP);
		
    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, SurfaceTemp->w, SurfaceTemp->h, ParallaxTemp, PVR_FILTER_NONE);
    pvr_poly_compile(ParallaxSurface, &cxt);

    PARAW = SurfaceTemp->w;
    PARAH = SurfaceTemp->h;
    ParallaxGraphic = ParallaxTemp;
    
    kos_img_free(SurfaceTemp, 1); 

    return true;
}

// ---------------------------------------------------------------------------------
// Draws the Parallax layer WITHOUT Transparency 
// Should only be called between pvr_list_begin(PVR_LIST_OP_POLY) and pvr_list_finish().
// ---------------------------------------------------------------------------------
int DCMappy::DrawParallax (void) {
    
    #ifdef DEBUG_MAP
    if(pvr_state.list_reg_open != PVR_LIST_OP_POLY) { 
        printf("DrawParallax() can only be called after pvr_list_begin(PVR_LIST_OP_POLY) and NOT after pvr_list_begin(PVR_LIST_TR_POLY).\n"); 
        return 0;
    }
    if(ParallaxSurface == NULL) {
        printf("You did not create a parallax (ie. CreateParallax(filename.kmg)) so there is no Parallax to draw.\n"); 
        return 0;
    }
    #endif 
    
	int			i, j, mapvclip, maphclip;
    int         x_min, y_min, x_max, y_max;
    float       temp;
	Block_Rect  TileDestRect;
	
	x_min = XPosition;
	y_min = YPosition;
	mapvclip = ((x_min/2)%PARAW);
	maphclip = ((y_min/2)%PARAH);
	x_max = (MSCRW/PARAW)+1;
    y_max = (MSCRH/PARAH)+3;
	
	for(i=0; i < y_max;i++)
		for(j=0; j < x_max;j++)
		{	
            TileDestRect.x	= j*PARAW-mapvclip + MMOX; 
			TileDestRect.y	= i*PARAH-maphclip + MMOY; 
			TileDestRect.h	= PARAH;
			TileDestRect.w	= PARAW;
			
			
			// Left
			if(TileDestRect.x < MMOX) {                            
                TileDestRect.w = PARAW - (MMOX - TileDestRect.x);            
                if(TileDestRect.w <= 0) continue;
                TileDestRect.x	= MMOX;
                temp = PARAW - TileDestRect.w;
                temp = temp / PARAW;
                TileDestRect.u1 = temp;
                
                // Handle Corners(Top Left then Bottom Left)
                if(TileDestRect.y < MMOY){
                    TileDestRect.h = PARAH - (MMOY - TileDestRect.y);
                    TileDestRect.y	= MMOY;
                    temp = PARAH - TileDestRect.h;
                    temp = temp / PARAH;
                    TileDestRect.v1 = temp;
                } else TileDestRect.v1 = 0.0f;
                
                if((TileDestRect.y+PARAH) > (MMOY+MSCRH)) {
                        TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                        if(TileDestRect.h > PARAH) continue;
                        temp = TileDestRect.h;
			            temp = temp/PARAH;
                        TileDestRect.v2 = temp;
                } else TileDestRect.v2 = 1.0f;
			    TileDestRect.u2 = 1.0f; 
			    
			    // Right
            } else if((TileDestRect.x+PARAW) > (MMOX+MSCRW)) {
                TileDestRect.w = (MMOX+MSCRW) - TileDestRect.x;
                if(TileDestRect.w > PARAW) continue; 
                TileDestRect.u1 = 0.0f;
                
                // Handle Corners(Top Right then Bottom Right)
                if(TileDestRect.y < MMOY){
                    TileDestRect.h = PARAH - (MMOY - TileDestRect.y);
                    TileDestRect.y	= MMOY;
                    temp = PARAH - TileDestRect.h;
                    temp = temp / PARAH;
                    TileDestRect.v1 = temp;
                } else TileDestRect.v1 = 0.0f;
                
                if((TileDestRect.y+PARAH) > (MMOY+MSCRH)) {
                        TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                        if(TileDestRect.h > PARAH) continue;
                        temp = TileDestRect.h;
			            temp = temp/PARAH;
                        TileDestRect.v2 = temp;
                } else TileDestRect.v2 = 1.0f;
                temp = TileDestRect.w;
                temp = temp / PARAW;
			    TileDestRect.u2 = temp;
                
                // Top
            } else if(TileDestRect.y < MMOY) {
                TileDestRect.h = PARAH - (MMOY - TileDestRect.y);
                if( TileDestRect.h <= 0) continue;
                TileDestRect.y	= MMOY;
                temp = PARAH - TileDestRect.h;
                temp = temp / PARAH;
                TileDestRect.u1 = 0.0f;
                TileDestRect.v1 = temp;
			    TileDestRect.u2 = TileDestRect.v2 = 1.0f;
			    
			    // Bottom
            } else if((TileDestRect.y+PARAH) > (MMOY+MSCRH)) {
                TileDestRect.h = (MMOY+MSCRH) - TileDestRect.y;
                if(TileDestRect.h > PARAH) continue;   
                TileDestRect.u1 = TileDestRect.v1 = 0.0f;
			    TileDestRect.u2 = 1.0f;
			    temp = TileDestRect.h;
			    temp = temp/PARAH;
                TileDestRect.v2 = temp;
                
                // Default
            } else {			
			    TileDestRect.u1 = TileDestRect.v1 = 0.0f;
			    TileDestRect.u2 = TileDestRect.v2 = 1.0f;
            } 
			
			MapDrawBlock(ParallaxSurface, &TileDestRect);
		}
	
	return 0;
}

// ---------------------------------------------------------------------------------
// Draws a Block 
// ---------------------------------------------------------------------------------
void DCMappy::MapDrawBlock(pvr_poly_hdr_t * Block, Block_Rect * Coordinates ) {
	
    pvr_vertex_t vert;
    pvr_prim(Block, sizeof(*Block));

    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);    
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;
    
    vert.x = Coordinates->x;
    vert.y = Coordinates->y;
    vert.z = 1;
    vert.u = Coordinates->u1;  
    vert.v = Coordinates->v1;  
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = Coordinates->w + Coordinates->x;
    vert.u = Coordinates->u2; 
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = Coordinates->x;
    vert.y = Coordinates->h + Coordinates->y; 
    vert.u = Coordinates->u1; 
    vert.v = Coordinates->v2; 
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = Coordinates->w + Coordinates->x; 
    vert.u = Coordinates->u2; 
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}

