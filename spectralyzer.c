#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "spectralyzer_display.c"

#include "rvfft_sorenson.c"

#define PI 3.1415

#define WIDTH 1920
#define HEIGHT 1080

static uint32_t texturebuffer[WIDTH*HEIGHT];

static bool fullscreen;

#define RGBA(r,g,b,a) ((uint32_t)(a || b<<8 || g<<16 || r<<24))

int grabmouse  = 0;
int grabmousex = 0;
int grabmousey = 0;
int grabmousexlocal = 0;
int grabmouseylocal = 0;

int active   = 1;
int awt      = 0;
int focus    = 0;
int topmost  = 0;

int showmenu = 0;
int showstat = 0;
int showsub  = 0;

int windowx  = 0;
int windowy  = 0;
int menux    = 0;
int menuy    = 0;
int menuw    = 160;
int menuh    = 190;
int subx     = 0;
int suby     = 0;
int subw     = 160;
int subh     = 110;
int mousex   = 0;
int mousey   = 0;
int mousedx  = 0;
int mousedy  = 0;

int subnumberofentries;
int subdefaultselection;
int subselection;
char *subarrayofstrings;

int minimizer  = 0;
int collapser  = 0;
int quitter    = 0;
int minimized  = 0;
int collapsed  = 0;

unsigned char *sounddata = NULL;
int soundlength = 0;

void DrawAnalyzer() {

  double fftdata[8192];

  for(int x=0; x<WIDTH; x++) {

    for(int i=0; i<512; i++) fftdata[i]=sounddata[x];//*1024;

    realfft_split_unshuffled( fftdata, 1024);

    for(int y=0; y<HEIGHT; y++) {
//      if(fftdata[y]<0) fftdata[y]*=-1;
//      uint32_t color=(float)(3*sqrt(fftdata[y]));
        uint32_t color = fftdata[y]*0xFF;

        texturebuffer[(x+y*WIDTH)] = color; // plot pixel!
    }
  }
}

int main(int argc, char* argv[]) {

	if(XWinCreate("X Window")) {
		puts("Failed to create window");
		return -1;
	}

  InitGL();

	while(!quit) {
		XWinProc();

		if(keys[_ESC_] || keys[_Q_])
			quit = 1;
    if(keys[_F_]) { keys[_F_] = 0;
      XWinFullscreen((fullscreen=!fullscreen));

    }

    Draw();
	}

	XWinDestroy();

	return 0;
}

