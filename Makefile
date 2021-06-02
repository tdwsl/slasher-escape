# Output of "sdl2-config --static-libs" goes here (minus the '-lSDL2')
LIBS=-Wl,--no-undefined -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lsndio -lX11 -lXext -lXcursor -lXinerama -lXi -lXrandr -lXss -lXxf86vm -lwayland-egl -lwayland-client -lwayland-cursor -lxkbcommon -lpthread -lrt
# Compiler
CC=gcc
# Where our source files are located
SRCDIR=src
# Where our header files are located
INCDIR=include
# Where SDL2 is located, for mingw
SDL=~/SDL2-2.0.14
# sdl2-config output for mingw
MGW_LIBS=-lmingw32 -lSDL2main -lSDL2 -mwindows -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc

linux64:
	$(eval ARCH=x86_64-linux-gnu)
	$(CC) -Wall -I$(INCDIR) $(SRCDIR)/*.c -o slashescape /usr/lib/$(ARCH)/libSDL2.a $(LIBS)
linux32:
	$(eval ARCH=i386-linux-gnu)
	$(CC) -Wall -I$(INCDIR) $(SRCDIR)/*.c -o slashescape /usr/lib/$(ARCH)/libSDL2.a $(LIBS)

win32:
	$(eval ARCH=i686)
	$(eval CC=$(ARCH)-w64-mingw32-gcc)
	$(CC) -Wall -I$(INCDIR) -I$(SDL)/$(ARCH)-w64-mingw32/include/ $(SRCDIR)/*.c -o slashescape $(SDL)/$(ARCH)-w64-mingw32/lib/libSDL2.a -L/opt/local/$(ARCH)-w64-mingw32/lib $(MGW_LIBS)
win64:
	$(eval ARCH=x86_64)
	$(eval CC=$(ARCH)-w64-mingw32-gcc)
	$(CC) -Wall -I$(INCDIR) -I$(SDL)/$(ARCH)-w64-mingw32/include/ $(SRCDIR)/*.c -o slashescape $(SDL)/$(ARCH)-w64-mingw32/lib/libSDL2.a -L/opt/local/$(ARCH)-w64-mingw32/lib $(MGW_LIBS)

clean:
	rm slashescape*
