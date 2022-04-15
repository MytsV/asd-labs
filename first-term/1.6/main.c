#include <stdio.h>
#include <unistd.h>
#include <curses.h>

const int ROW_COUNT = 24;
const int COLUMN_COUNT = 80;
const char SYMBOL = '*';

void printSymbol(int x, int y, int symbol) {
    mvaddch(y, x, symbol);
    refresh();
    usleep(5000);
}

void printSpiral() {
    int xSize = COLUMN_COUNT - ROW_COUNT + 1;
    int ySize = 1;
    int x = (ROW_COUNT / 2);
    int y = (ROW_COUNT / 2);
    int directionX = 1;

    while (x >= 1 && y <= ROW_COUNT) {
        int currentSize = 0;
        while(currentSize != xSize) {
            printSymbol(x - 1, y - 1, SYMBOL);
            currentSize++;
            x += directionX;
        }
        if (x < 1) break;
        currentSize = 0;
        while (currentSize != ySize) {
            printSymbol(x - 1, y - 1, SYMBOL);
            currentSize++;
            y += directionX;
        }
        directionX = -directionX;
        xSize += 1;
        ySize += 1;
    }

}

int main() {
    initscr();
    cbreak();
    noecho();
    
    clear();

    printSpiral();

    getch();
    endwin();
    return 0;
}
