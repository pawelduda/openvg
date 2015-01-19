#ifndef OPENVG_FONTINFO_H
#define OPENVG_FONTINFO_H

#if defined(__cplusplus)
extern "C" {
#endif
	typedef struct {
		const short *CharacterMap;
		const int *GlyphAdvances;
		int Count;
                VGPath Glyphs[400];
	} Fontinfo;

        extern Fontinfo SansTypeface;

#if defined(__cplusplus)
}
#endif				// OPENVG_FONTINFO_H
#endif
