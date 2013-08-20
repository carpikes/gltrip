all:
	gcc -Wall -Werror -pedantic -o particles particles.c -lSDL -lGL -lm -lpthread
