Slasher Escape

![screenshot](https://raw.githubusercontent.com/tdwsl/slasher-escape/main/screenshot.png)

ABOUT

Slasher Escape is a game about running away. Your goal is to gather three car
parts, all while fending off a vicious killer. It was originally made for the
4MB game jam on itch.io:
https://tylerdwsl.itch.io/slasher-escape

HOW TO PLAY

The controls are pretty simple, just the arrow keys, Z and X. Z is used to pick
up items and interact with things, as well as to shoot a ranged weapon. X is
used to switch items, and if you hold it, throw them. Throwing items can get
you out of a pickle in this game, as they can easily smash through windows. The
arrow keys are, of course, used for moving. There is some more detailed 
documentation on the game's itch.io page (as well as a sweet GIF :D).

COMPILING

Being that the whole game has to be included, the makefile has static targets
for Linux and Windows 64 and 32 bit. I have now also added a 'dynamic' target,
which basically just compiles everything normally. Anyhoo, all you'll need is
a C compiler, the SDL2 development libraries (just the base libs, no extensions
are used) and maybe make, though it isn't too hard to look at the makefile and
figure out how to compile.
