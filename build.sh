source /opt/toolchains/dc/kos/environ.sh
rm -f TicTacToe.elf
kos-cc  -c main.c -o main.o
kos-cc -o TicTacToe.elf main.o -lSDL