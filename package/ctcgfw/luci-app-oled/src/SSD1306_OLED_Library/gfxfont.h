#ifndef _GFXFONT_H_
#define _GFXFONT_H_

typedef struct {                                 // Data stored PER GLYPH
	unsigned short  bitmapOffset;                // Pointer into GFXfont->bitmap
	unsigned char  width, height;                // Bitmap dimensions in pixels
	unsigned char  xAdvance;                     // Distance to advance cursor (x axis)
	char   xOffset, yOffset;                     // Dist from cursor pos to UL corner
} GFXglyphT, *GFXglyphPtr;

typedef struct {                                 // Data stored for FONT AS A WHOLE:
	unsigned char  *bitmap;                      // Glyph bitmaps, concatenated
	GFXglyphPtr glyph;                           // Glyph array
	unsigned char   first, last;                 // ASCII extents
	unsigned char   yAdvance;                    // Newline distance (y axis)
} GFXfontT, *GFXfontPtr;

#endif // _GFXFONT_H_
