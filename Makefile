CC        = gcc
CCFLAGS   =  
LDFLAGS   = -lm -lX11 -lGL -lGLU -lGLEW
OBJS      = spectralyzer.obj
PROGS     = spectralyzer
RES       = spectralyzer.res

all:
	gcc spectralyzer.c $(LDFLAGS) -o Spectralyzer

#all: $(PROGS)

#spectralyzer.obj: spectralyzer.c
#	$(CC) $(CCFLAGS) $(LDFLAGS) spectralyzer.c -o $(PROGS)

clean:
	del $(PROGS) $(OBJS) 
