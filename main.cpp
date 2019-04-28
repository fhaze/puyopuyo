#include <ncurses.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <cstdlib>

#define FIELD_WIDTH 8
#define FIELD_HEIGHT 14

#define PUYO_START_X 3
#define PUYO_START_Y 1

#define PUYO_COLOR_MAX 4

enum {
    CELL_NONE,
    CELL_WALL,
    CELL_PUYO_0,
    CELL_PUYO_1,
    CELL_PUYO_2,
    CELL_PUYO_3,
    CELL_MAX
};

enum {
    PUYO_ANGLE_0,
    PUYO_ANGLE_90,
    PUYO_ANGLE_180,
    PUYO_ANGLE_270,
    PUYO_ANGLE_MAX,
};

int puyoSubPotions[][2] = {
    { 0, -1},// PUYO_ANGLE_0
    {-1,  0},// PUYO_ANGLE_90
    { 0,  1},// PUYO_ANGLE_180
    { 1,  0},// PUYO_ANGLE_270
};

int cells[FIELD_HEIGHT][FIELD_WIDTH],
    displayBuffer[FIELD_HEIGHT][FIELD_WIDTH],
    checked[FIELD_HEIGHT][FIELD_WIDTH];

char cellNames[][4] = {
    "　", // CELL_NONE
    "⬛", // CELL_WALL
    "土", // CELL_PUYO_0
    "水", // CELL_PUYO_1
    "火", // CELL_PUYO_2
    "風", // CELL_PUYO_3
};

int puyoX = PUYO_START_X,
    puyoY = PUYO_START_Y;
int puyoColor;
int puyoAngle;

bool lock = false;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void display() {
    clear();
    memcpy(displayBuffer, cells, sizeof cells);

    if (!lock) {
        int subX = puyoX + puyoSubPotions[puyoAngle][0];
        int subY = puyoY + puyoSubPotions[puyoAngle][1];
        displayBuffer[puyoY][puyoX] =
        displayBuffer[subY][subX]   = CELL_PUYO_0 + puyoColor;
    }

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            attron(COLOR_PAIR(displayBuffer[y][x]));
            mvprintw(y, x, cellNames[displayBuffer[y][x]]);
            attroff(COLOR_PAIR(displayBuffer[y][x]));
        }
    }

    mvprintw(FIELD_HEIGHT, 0, "");
}

bool intersectPuyoToField(int _puyoX, int _puyoY, int _puyoAngle) {
    if (cells[_puyoY][_puyoX] != CELL_NONE) {
        return true;
    }

    int subX = _puyoX + puyoSubPotions[_puyoAngle][0];
    int subY = _puyoY + puyoSubPotions[_puyoAngle][1];

    return cells[subY][subX] != CELL_NONE;

}

int getPuyoConenctedCount(int _x, int _y, int _cell, int _count) {
    if (checked[_y][_x] || cells[_y][_x] != _cell) {
        return _count;
    }
    _count++;
    checked[_y][_x] = true;

    for (int i = 0; i < PUYO_ANGLE_MAX; i++) {
        int x = _x + puyoSubPotions[i][0];
        int y = _y + puyoSubPotions[i][1];
        _count = getPuyoConenctedCount(x, y, _cell, _count);
    }

    return _count;
}

void erasePuyo(int _x, int _y, int _cell) {
    if (cells[_y][_x] != _cell) {
        return;
    }
    cells[_y][_x] = CELL_NONE;

    for (int i = 0; i < PUYO_ANGLE_MAX; i++) {
        int x = _x + puyoSubPotions[i][0];
        int y = _y + puyoSubPotions[i][1];
        erasePuyo(x, y, _cell);
    }
}

int main() {
    srand((unsigned int)time(nullptr));
    setlocale(LC_ALL,"");
    initscr();
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    start_color();
    init_pair(CELL_NONE, COLOR_WHITE, COLOR_BLACK);
    init_pair(CELL_WALL, COLOR_WHITE, COLOR_BLACK);
    init_pair(CELL_PUYO_0, COLOR_YELLOW, COLOR_BLACK);
    init_pair(CELL_PUYO_1, COLOR_BLUE, COLOR_BLACK);
    init_pair(CELL_PUYO_2, COLOR_RED, COLOR_BLACK);
    init_pair(CELL_PUYO_3, COLOR_GREEN, COLOR_BLACK);

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        cells[y][0] =
        cells[y][FIELD_WIDTH - 1] = CELL_WALL;
    }

    for (int x = 0; x < FIELD_WIDTH; x++) {
        cells[FIELD_HEIGHT - 1][x] = CELL_WALL;
    }

    puyoColor = rand() % PUYO_COLOR_MAX;

    time_t t = 0;
    while(true) {
        if (t < time(nullptr)) {
            t = time(nullptr);
            if (!lock) {
                if (!intersectPuyoToField(puyoX, puyoY + 1, puyoAngle)) {
                    puyoY++;
                } else {
                    int subX = puyoX + puyoSubPotions[puyoAngle][0];
                    int subY = puyoY + puyoSubPotions[puyoAngle][1];

                    cells[puyoY][puyoX] =
                    cells[subY][subX]   = CELL_PUYO_0 + puyoColor;

                    puyoX     = PUYO_START_X;
                    puyoY     = PUYO_START_Y;
                    puyoAngle = PUYO_ANGLE_0;
                    puyoColor = rand() % PUYO_COLOR_MAX;

                    lock = true;
                }
            }

            if (lock) {
                lock = false;
                for (int y = FIELD_HEIGHT - 3; y >= 0; y--) {
                    for(int x = 1; x < FIELD_WIDTH; x++) {
                        if (cells[y][x] != CELL_NONE && cells[y+1][x] == CELL_NONE) {
                            cells[y + 1][x] = cells[y][x];
                            cells[y    ][x] = CELL_NONE;
                            lock            = true;
                        }
                    }
                }
                if (!lock) {
                    memset(checked, 0, sizeof checked);
                    for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
                        for (int x = 1; x < FIELD_WIDTH - 1; x++) {
                            if (cells[y][x] != CELL_NONE) {
                                if (getPuyoConenctedCount(x, y, cells[y][x], 0) >= 4) {
                                    erasePuyo(x, y, cells[y][x]);
                                    lock = true;
                                }
                            }
                        }
                    }
                }
            }
            display();
        }

        char key = getch();
        if (key != ERR) {
            if (!lock) {
                int x = puyoX;
                int y = puyoY;
                int angle = puyoAngle;

                switch (key) {
                    case 's': y++; break;
                    case 'a': x--; break;
                    case 'd': x++; break;
                    case ' ': angle = (++angle) % PUYO_ANGLE_MAX; break;
                    default: break;
                }
                if (!intersectPuyoToField(x, y, angle)) {
                    puyoX     = x;
                    puyoY     = y;
                    puyoAngle = angle;
                }
                display();
            }
        }

    }
    endwin();
}
#pragma clang diagnostic pop