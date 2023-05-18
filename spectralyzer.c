#include <stdio.h>
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "spectralyzer_audio.c"
#include "spectralyzer_display.c"

#include "rvfft_sorenson.c"

#define WIDTH 1920
#define HEIGHT 1080

static uint32_t texturebuffer[WIDTH*HEIGHT];

static int fullscreen;

#define RGBA(r,g,b,a) ((uint32_t)(a<<24 | b<<16 | g<<8 | r))

const float cols[] = {
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f
};

uint32_t HSV2RGBA( float H, float S, float V ) {
    H = fmodf(H, M_PI*2);
    float Slice = H / (M_PI*2) * 6.f;
    float SliceInt = floorf( Slice );
    float SliceFrac = Slice - SliceInt;
    int i = (int)SliceInt*3;
    int j = (i+3);

    uint32_t R = (cols[i+0] + (cols[j+0] - cols[i+0]) * SliceFrac ) * 255.f * V;
    uint32_t G = (cols[i+1] + (cols[j+1] - cols[i+1]) * SliceFrac ) * 255.f * V;
    uint32_t B = (cols[i+2] + (cols[j+2] - cols[i+2]) * SliceFrac ) * 255.f * V;

    uint32_t rgb = RGBA(R, G, B, 0);

    printf("H%.2f R%03d G%03d B%03d / 0x%08X \r", H, R, G, B, rgb);

    return rgb;
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

        for(int i=0; i<40; i++)
            putchar(i<max*400?'-':' ');
        printf("%.4f %.4f \r", max, max*100); fflush(stdout);

        float *src = snd->data;
        float *dst = tex->img->data+(loops%512)*8192;

        float l[1024];
        float r[1024];

        for(int i=0; i<2048; i++) {
            l[i] = *src++;
            r[i] = *src++;
        }

        float fftdata[1024];

        realfft(l, fftdata, 1024);

        uint32_t *texdata = tex->img->data+(loops%1024)*4096; // ABGR
        uint32_t *cursor = tex->img->data+((loops+1)%1024)*4096;

        for(int i=0; i<1024; i++) {
            texdata[i] = HSV2RGBA(fftdata[i]*2*pi, 1.f, fftdata[i]*10);
            if(i%32==0) cursor[i] = RGBA(0,0,0xFF,0);
        }

	    UploadTexture(tex);

  	    glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex->id);

        Draw();

        loops++;
	}

    XWinDestroy();

	return 0;
}

