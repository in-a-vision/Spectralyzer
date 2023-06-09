#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define PI M_PI
#define TAU M_2_PI

#include <time.h>

void Sleep(uint32_t ms) {
	struct timespec req, rem;
	req.tv_sec = ms/1000;
	req.tv_nsec = ms*1000000;
	nanosleep(&req, &rem);	
}

static float GetTime() {
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
	static struct timespec freq, spec, smek;
	static char init;
	if(!init) { init=1;
		clock_getres(CLOCK_REALTIME, &freq);
		clock_gettime(CLOCK_REALTIME, &smek);
	}
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec - smek.tv_sec + spec.tv_nsec * .000000001;
}

#ifdef __linux__

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>

#include <stdio.h>

typedef struct XWin_t {
	Display *dpy;
	Window win;
	XVisualInfo *vi;
	XEvent xev;
	Atom wm_delete_window;
	GLXContext glx;
} XWin;

static XWin xw;
static int quit;

static char keys[256];
static char mouse;

enum KeyCode {
	_0x0_,_0x1_,_0x2_,_0x3_,_0x4_,_0x5_,_0x6_,_0x7_,_0x8_,
	_ESC_,_1_,_2_,_3_,_4_,_5_,_6_,_7_,_8_,_9_,_0_,_SUB_,_EQ_,_BKSPC_,
	_TAB_,_Q_,_W_,_E_,_R_,_T_,_Y_,_U_,_I_,_O_,_P_,_LBRAK_,_BRAKR_,_RETURN_,
	_LCTL_,_A_,_S_,_D_,_F_,_G_,_H_,_J_,_K_,_L_,_SEMI_,_APOS_,_TILDE_,
	_LSHFT_,_BSLASH_,_Z_,_X_,_C_,_V_,_B_,_N_,_M_,_COMMA_,_DOT_,_SLASH_,_RSHFT_,
	_KPMUL_,_LALT_,_SPACE_,_0x42_,_F1_,_F2_,_F3_,_F4_,_F5_,_F6_,_F7_,_F8_,_F9_,_F10_,
	_0x4D_,_0x4E_,_KP7_,_KP8_,_KP9_,_KPSUB_,_KP4_,_KP5_,_KP6_,_KPADD_,_KP1_,_KP2_,_KP3_,
	_KP0_,_KPDOT_,_0x5C_,_0x5D_,_ANGBR_,_F11_,_F12_,_0x61_,_0x62_,_0x63_,_0x64_,_0x65_,_0x66_,_0x67_,
	_KPENTR_,_RCTL_,_KPDIV_,_0x6B_,_ALTR_,_0x6D_,_HOME_,_UP_,_PGUP_,_LEFT_,_RIGHT_,_END_,_DOWN_,_PGDN_,_INS_,_DEL_,_0x78_
};

// https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html


#include <GL/glx.h>

static int XWinCreate(char *title) {

	xw.dpy = XOpenDisplay(0);
	if(!xw.dpy) {
 		perror("No connect X server.");
		return -1;
 	}

	int scr = DefaultScreen(xw.dpy);
	int w = DisplayWidth(xw.dpy, scr) / 8;
	int h = DisplayHeight(xw.dpy, scr) / 8;
	int x = w / 2;
	int y = h / 2;

	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	xw.vi = glXChooseVisual(xw.dpy, 0, att);
	if(!xw.vi) { perror("No choose glX visual."); return -1; }

	Window root = RootWindow(xw.dpy, xw.vi->screen);

	XSetWindowAttributes swa;
	swa.colormap = XCreateColormap(xw.dpy, root, xw.vi->visual, AllocNone);
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | StructureNotifyMask |
		SubstructureNotifyMask | SubstructureRedirectMask | VisibilityChangeMask | PropertyChangeMask;
	swa.override_redirect = True;
	swa.border_pixel = 0;

	xw.win = XCreateWindow(xw.dpy, root, x, y, w, h, 0, xw.vi->depth,
						InputOutput, xw.vi->visual, CWColormap | CWEventMask, &swa);
	if(!xw.win) { perror("No create X window."); return -1; }

	xw.wm_delete_window = XInternAtom(xw.dpy, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(xw.dpy, xw.win, &xw.wm_delete_window, 1);
	XSetWindowBackground(xw.dpy, xw.win, 0x002F0000); // ARGB

	XClearWindow(xw.dpy, xw.win);
	XStoreName(xw.dpy, xw.win, title);

	xw.glx = glXCreateContext(xw.dpy, xw.vi, 0, GL_TRUE);
	glXMakeCurrent(xw.dpy, xw.win, xw.glx);

	XSizeHints xsh = {0};

	xsh.flags = USSize|PAspect|PPosition;
	xsh.x = x;
	xsh.y = y;
	xsh.min_aspect.x = xsh.max_aspect.x = w;
	xsh.min_aspect.y = xsh.max_aspect.y = h;
	XSetWMSizeHints(xw.dpy, xw.win, &xsh, XA_WM_NORMAL_HINTS);

	XMapWindow(xw.dpy, xw.win);
    XRaiseWindow(xw.dpy, xw.win);
	XFlush(xw.dpy);
	XSync(xw.dpy, 0);

	return 0;
}


#define _NET_WM_STATE_REMOVE 0 //
#define _NET_WM_STATE_ADD    1 //
#define _NET_WM_STATE_TOGGLE 2 //

void XWinFullscreen(int fullscreen) {

    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.window = xw.win;
    xev.xclient.message_type = XInternAtom(xw.dpy, "_NET_WM_STATE", 0);
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = (fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE);
    xev.xclient.data.l[1] = XInternAtom(xw.dpy, "_NET_WM_STATE_FULLSCREEN", 0);
    xev.xclient.data.l[2] = 0;
    XSendEvent(xw.dpy, DefaultRootWindow(xw.dpy), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

void XWinRestore() {
	XClientMessageEvent ev = {};
	ev.type = ClientMessage;
	ev.window = xw.win;
	ev.message_type = XInternAtom(xw.dpy, "_NET_ACTIVE_WINDOW", True);
	ev.format = 32;
	ev.data.l[0] = 1;
	ev.data.l[1] = CurrentTime;
	ev.data.l[2] = ev.data.l[3] = ev.data.l[4] = 0;
	XSendEvent( xw.dpy, RootWindow(xw.dpy, XDefaultScreen(xw.dpy)), False, SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*)&ev );
	XFlush( xw.dpy );
}

void XWinProc() {

	while(XPending(xw.dpy)) {
 		XNextEvent(xw.dpy, &xw.xev);
		switch(xw.xev.type) {
    	case VisibilityNotify:
    	    XRaiseWindow(xw.dpy, xw.win);
    	    XFlush(xw.dpy);
			break;
		case Expose: {
			XWindowAttributes gwa;
			XGetWindowAttributes(xw.dpy, xw.win, &gwa);
			glViewport(0, 0, gwa.width, gwa.height);
		}	break;
		case KeyPress:
		case KeyRelease: {
			XID key = ((XKeyEvent*)&xw.xev)->keycode;
			keys[key&0xFF] = xw.xev.type==KeyPress?1:0;
		}	break;
		case ButtonPress:
			break;
		case MapNotify:
			break;
        case ClientMessage:
			if((Atom)xw.xev.xclient.data.l[0] == xw.wm_delete_window)
				quit = 1;
    		break;	
		}
	}

}

void XWinDestroy() {
	XDestroyWindow(xw.dpy, xw.win);
 	XCloseDisplay(xw.dpy);
}

static const char* fs_txt_ =
	"#version 310 es"
	"precision highp float;"
	"in vec2 v_UV1;"
	"layout(location=0) out vec4 o_Col;"
	"uniform sampler2D u_ColTex;"
	"uniform float u_Dissolve;"
	"void main() {"
	"	vec4 col = texture(u_ColTex, v_UV1);"
	"	col.a *= 1.0 - u_Dissolve;"
	"	o_Col = col;"
	"}";

static const char* vs_txt =
	"precision lowp float;"
	"uniform mat4 uMVP;"
	"attribute vec4 aPos;"
	"attribute vec2 aUV1;"
	"varying vec2 vUV1;"
	"void main()"
	"{"
		"vUV1 = aUV1;"
		"gl_Position = uMVP * aPos;"
	"}";

static const char* fs_txt =
	"precision lowp float;"
	"varying vec2 vUV1;"
	"uniform sampler2D uTex;"
	"void main()"
	"{"
		"gl_FragColor = texture(uTex, vUV1);"
	"}";

static GLuint prg, vtxbuf;
static GLint uMVP_loc, aPos_loc, aCol_loc, uTex_loc, aUV1_loc;

typedef struct Vtx {
	float x, y, z, r, g, b;
} Vtx;

typedef struct Image {
	GLuint w, h, stride, vstride, format, type, datasize, flags;
	uint8_t *data;
} Image;

typedef struct Texture {
	GLuint id;
	GLint min, mag, wrap_s, wrap_t, mipmap;
	GLuint internalformat;
	Image *img;
} Texture;


Texture *NewTexture(int w, int h) {
	Texture *tex = malloc(sizeof(Texture));
	assert(tex);
	tex->img = malloc(sizeof(Image));
	assert(tex->img);

	tex->img->w = w;
	tex->img->h = h;
	tex->img->datasize = w*h*4;
	tex->img->data = malloc(tex->img->datasize);
	tex->internalformat = GL_RGBA;

	glGenTextures(1, &tex->id);
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR/*GL_NEAREST*/);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->img->w);
	
	return tex;
}

void UploadTexture(Texture *tex) {
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexImage2D(GL_TEXTURE_2D, 0, tex->internalformat, tex->img->w, tex->img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->img->data);
//	glTexSubImage2D(GL_TEXTURE_2D, 0, dx, dy, w, h, img->format, img->type, img->data);

	if(tex->mipmap) glGenerateMipmap(GL_TEXTURE_2D);
}


int InitGL() {

	GLint res = glewInit();
	if(res != GLEW_OK) {
		printf("GLEW initialization failed: %s\n", glewGetErrorString(res));
		return -1;
	}

	glXSwapIntervalEXT(xw.dpy, xw.win, 1);

	glViewport(0, 0, 640, 480);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_txt, NULL);
	glCompileShader(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_txt, NULL);
	glCompileShader(fs);

	prg = glCreateProgram();
	glAttachShader(prg, vs);
	glAttachShader(prg, fs);
	glLinkProgram(prg);

	uMVP_loc = glGetUniformLocation(prg, "uMVP");
	aPos_loc = glGetAttribLocation(prg, "aPos");
	aUV1_loc = glGetAttribLocation(prg, "aUV1");
	uTex_loc = glGetAttribLocation(prg, "uTex");

	glGenBuffers(1, &vtxbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vtxbuf);

	glEnableVertexAttribArray(aPos_loc);
	glVertexAttribPointer(aPos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)0);
	glEnableVertexAttribArray(aUV1_loc);
	glVertexAttribPointer(aUV1_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)(sizeof(float) * 3));

	return 0;
}

void Draw() {

	static int loops;

	float t = GetTime();

	glClear(GL_COLOR_BUFFER_BIT);

	Vtx v[6] = {
		{-1.f, 1.f, 0.f, 1.f, 0.f, },
		{-1.f,-1.f, 0.f, 0.f, 0.f, },
		{ 1.f,-1.f, 0.f, 0.f, 1.f, },
		{-1.f, 1.f, 0.f, 1.f, 0.f, },
		{ 1.f, 1.f, 0.f, 1.f, 1.f, },
		{ 1.f,-1.f, 0.f, 0.f, 1.f, },
	};

	glBindBuffer(GL_ARRAY_BUFFER, vtxbuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

	GLfloat mvp[4*4] = { 1, 0, 0, 0,
						 0, 1, 0, 0,
						 0, 0,-1, 0,
						 0, 0, 0, 1 };

//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, tex->id);

	GLint loc = glGetUniformLocation(prg, "uTex");
	glUniform1i(loc, 0);

	glUseProgram(prg);
	glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, mvp);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glXSwapBuffers(xw.dpy, xw.win);

}

#endif // __linux__