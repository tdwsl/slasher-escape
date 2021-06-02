# Output of "sdl2-config --static-libs" goes here (minus the '-lSDL2')
LIBS=-Wl,--no-undefined -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lsndio -lX11 -lXext -lXcursor -lXinerama -lXi -lXrandr -lXss -lXxf86vm -lwayland-egl -lwayland-client -lwayland-cursor -lxkbcommon -lpthread -lrt
# Compiler
CC=gcc
# Where our source files are located
SRCDIR=src
# Where our header files are located
INCDIR=include

linux64:
	$(eval ARCH=x86_64-linux-gnu)
	$(CC) -Wall -I$(INCDIR) $(SRCDIR)/*.c -o slashescape /usr/lib/$(ARCH)/libSDL2.a $(LIBS)
linux32:
	$(eval ARCH=i386-linux-gnu)
	$(CC) -Wall -I$(INCDIR) $(SRCDIR)/*.c -o slashescape /usr/lib/$(ARCH)/libSDL2.a $(LIBS)

clean:
	rm slashescape*
