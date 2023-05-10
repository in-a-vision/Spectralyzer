#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "spectralyzer_audio.c"
#include "spectralyzer_display.c"

#include "rvfft_sorenson.c"

//#define PI 3.1415

#define WIDTH 1920
#define HEIGHT 1080

static uint32_t texturebuffer[WIDTH*HEIGHT];

static bool fullscreen;

#define RGBA(r,g,b,a) ((uint32_t)(a || b<<8 || g<<16 || r<<24))
#define BGRA(r,g,b,a) ((uint32_t)(a || r<<8 || g<<16 || b<<24))


uint32_t HSV2RGBA(float H, float s,float v) {

    float C = s*v;
    float X = C*(1-abs(fmod(H/60.f, 2)-1));
    float m = v-C;
    float r,g,b;
    if(H >=   0 && H <  60) { r=C, g=X, b=0; } else
    if(H >=  60 && H < 120) { r=X, g=C, b=0; } else
    if(H >= 120 && H < 180) { r=0, g=C, b=X; } else
    if(H >= 180 && H < 240) { r=0, g=X, b=C; } else
    if(H >= 240 && H < 300) { r=X, g=0, b=C; } else
                            { r=C, g=0, b=X; }
    int R = (r+m)*255.f;
    int G = (g+m)*255.f;
    int B = (b+m)*255.f;

    printf("R:%d G:%d B:%d ", R, G, B);

    return BGRA( R, G, B, 0 );
}

int main(int argc, char* argv[]) {

	if(XWinCreate("X Window")) {
		puts("Failed to create window");
		return -1;
	}

  InitSnd();
  InitGL();

  Texture *tex = NewTexture(1024, 1024);

  Sound *snd = NewSound(10.0);

  int loops=0;

  float fftdata[8192];

	while(!quit) {

		XWinProc();

		if(keys[_ESC_] || keys[_Q_])
			quit = 1;
    if(keys[_F_]) { keys[_F_] = 0;
      XWinFullscreen((fullscreen=!fullscreen));
    }

    Rec(snd->data, 1024);

	  float *f = snd->data, max = .0f;
	  for(int i=0; i<1024; i++)
		  if(*f > max) max = *f;

	  for(int i=0; i<40; i++) {
		  putchar(i<max*400?'-':' ');
	  } printf("%.4f %.4f \r", max, max*100); fflush(stdout);

    float *src = snd->data;
    float *dst = tex->img->data+(loops%512)*8192;

    float l[1024];
    float r[1024];

    for(int i=0; i<2048; i++) {
      l[i] = *src++;
      r[i] = *src++;


    }

    float fftdata[1024];

    realfft_split_unshuffled(l, fftdata, 1024);

    float *texdata = tex->img->data+(loops%1024)*4096;

    for(int i=0; i<1024; i++) {

        texdata[i] = //HSV2RGBA(10,1,.25f);
                    ((uint8_t)(fftdata[i]*0xFF)) << 24 |
                    ((uint8_t)(fftdata[i]*0xFF)) << 16 |
                    ((uint8_t)(fftdata[i]*0xFF)) << 8;


//        int n = snd->data[i]*4096;
//        if(n>511) n=511; else if(n<-511) n=-511;
//        texdata[i*1024+512+n] = 0xFF00FF00;
//      texdata[i*1024+(int)snd->data[i]] = max*400;

    }

	  UploadTexture(tex);

//    if(loops%8==0)
//    memset(tex->img->data, 0x00, 1024*1024*4);

  	glActiveTexture(GL_TEXTURE0);
	  glBindTexture(GL_TEXTURE_2D, tex->id);

    Draw();

    loops++;

	}
	XWinDestroy();

	return 0;
}

