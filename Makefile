# Output of 'sdl2-config --static-libs' goes here, minus the '-lSDL2'
LIBS=-Wl,--no-undefined -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lsndio -lX11 -lXext -lXcursor -lXinerama -lXi -lXrandr -lXss -lXxf86vm -lwayland-egl -lwayland-client -lwayland-cursor -lxkbcommon -lpthread -lrt
# Compiler for linux
CC=gcc -std=c11
# Where our source files are located
SRCDIR=src
# Where our header files are located
INCDIR=include
# Where SDL2 is located, for mingw
SDL=SDL2-2.0.14

dynamic:
	$(CC) -Wall -Iinclude src/*.c -lSDL2 -lm -o slashescape
linux64:
	$(eval ARCH=x86_64-linux-gnu)
	$(CC) -Wall -I$(INCDIR) $(SRCDIR)/*.c -o slashescape /usr/lib/$(ARCH)/libSDL2.a $(LIBS)
linux32:
	$(eval ARCH=i386-linux-gnu)
	$(CC) -Wall -I$(INCDIR) $(SRCDIR)/*.c -o slashescape /usr/lib/$(ARCH)/libSDL2.a $(LIBS)

win64:
	$(eval MINGW=x86_64-w64-mingw32)
	$(eval CC=$(MINGW)-gcc)
	$(CC) -Wall -I$(INCDIR) -I$(SDL)/$(MINGW)/include/ $(SRCDIR)/*.c -o slashescape -L$(SDL)/$(MINGW)/lib -L/usr/$(MINGW)/lib -lmingw32 -lSDL2main -lSDL2 -lm -w -Wl,-subsystem,windows
	if ! test -f "SDL.dll"; then cp $(SDL)/$(MINGW)/bin/SDL2.dll .; fi
win32:
	$(eval MINGW=i686-w64-mingw32)
	$(eval CC=$(MINGW)-gcc)
	$(CC) -Wall -I$(INCDIR) -I$(SDL)/$(MINGW)/include/ $(SRCDIR)/*.c -o slashescape -L$(SDL)/$(MINGW)/lib -L/usr/$(MINGW)/lib -lmingw32 -lSDL2main -lSDL2 -lm -w -Wl,-subsystem,windows
	if ! test -f "SDL.dll"; then cp $(SDL)/$(MINGW)/bin/SDL2.dll .; fi

clean:
	rm -f slashescape*
	rm -f *.dll
