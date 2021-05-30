# Architecture of the system you're compiling on
ARCH=x86_64-linux-gnu
# Output of "sdl2-config --static-libs" goes here
LIBS="-Wl,--no-undefined -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lsndio -lX11 -lXext -lXcursor -lXinerama -lXi -lXrandr -lXss -lXxf86vm -lwayland-egl -lwayland-client -lwayland-cursor -lxkbcommon -lpthread -lrt"

gcc -Wall main.c -o slashescape /usr/lib/$ARCH/libSDL2.a $LIBS
