#include <ncurses.h>
#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

WINDOW *create_newwin(int height, int width, int starty, int startx);
void remove_win(WINDOW *cur_win);

int game_win_height = 20;
int game_win_width = 25;

int next_win_height = 10;
int next_win_width = 14;

WINDOW *game_win, *next_win, *score_win;
//REMEMBER TO COME BACK

int key;

int get_rand(int min, int max) {
    return (min + rand() % (max - min + 1));
}

WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *cur_win;
    cur_win = newwin(height, width, starty, startx);
    box(cur_win, 0, 0);
    wrefresh(cur_win);
    
    //return the pointer
    return cur_win;
}

void remove_win(WINDOW *cur_win) {
    wborder(cur_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(cur_win);
    delwin(cur_win);
}

class Piece {
    public:
        int score;

        int x;
        int y;
        
        int width;
        int height;
        int shape;
        int box_shape[4][4];

        int next_width;
        int next_height;
        int next_shape;
        int next_box_shape[4][4];

        int box_map[30][45];

        int is_over;

        //int game_status;

    public:
        void initialize();
        void set_shape(int &cshape, int box_shape[4][4], int &width, int &height);
        void score_next();
        void judge();
        void move();
        void rotate();
        //void switch_status();
        bool is_overlap();
        bool is_row_occupied(int row);
};

void Piece::initialize() {
    score = 0;
    is_over = false;

    for (int i = 0; i < game_win_height; i++)
        for (int j = 0; j < game_win_width; j++) {
            if(i == 0 || i == game_win_height - 1 || j == 0 || j == game_win_width - 1)
                box_map[i][j] = 1;
            else
                box_map[i][j] = 0;
        }

    //Initialize shapes
    srand((unsigned)time(0));
    shape = get_rand(0, 6);
    set_shape(shape, box_shape, width, height);

    //Print shapes
    next_shape = get_rand(0, 6);
    set_shape(next_shape, next_box_shape, next_width, next_height);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if(next_box_shape[i][j] == 1)
                mvwaddch(next_win, (next_win_height - next_height)/2+i, (next_win_width - next_width)/2+j, '#');
    wrefresh(next_win);

    mvwprintw(score_win, next_win_height, next_win_width, "%d", score);
    wrefresh(score_win);
}

//update score_win and next_win
void Piece::score_next() {
    score++;
    judge();

    mvwprintw(score_win, next_win_height/2, next_win_width/2, "%d", score);
    wrefresh(score_win);

    //Update next shape
    set_shape(next_shape, box_shape, width, height);
    next_shape = get_rand(0, 6);
    set_shape(next_shape, next_box_shape, next_width, next_height);

    //clean up score window
    for (int i = 1; i < next_win_height - 1; i++)
        for (int j = 1; j < next_win_width - 1; j++) {
            mvwaddch(next_win, i, j, ' ');
            wrefresh(next_win);
        }
    
    //print next shape
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if(next_box_shape[i][j] == 1) {
                mvwaddch(next_win, (next_win_height - next_height)/2+i, (next_win_width - next_width)/2+j, '#');
                wrefresh(next_win);
            }
}

//set box_shape and width height according to shape
void Piece::set_shape(int &pshape, int box_shape[4][4], int &width, int &height) {
    int i,j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            box_shape[i][j] = 0;

    switch(pshape) {
        case 0:
            height = 1;
            width = 4;
            box_shape[0][0] = 1;
            box_shape[0][1] = 1;
            box_shape[0][2] = 1;
            box_shape[0][3] = 1;
            break;
        case 1:
            height = 2;
            width = 3;
            box_shape[0][0] = 1;
            box_shape[1][0] = 1;
            box_shape[1][1] = 1;
            box_shape[1][2] = 1;
            break;
        case 2:
            height = 2;
            width = 3;
            box_shape[0][2] = 1;
            box_shape[1][0] = 1;
            box_shape[1][1] = 1;
            box_shape[1][2] = 1;
            break;
        case 3:
            height = 2;
            width = 3;
            box_shape[0][1] = 1;
            box_shape[0][2] = 1;
            box_shape[1][0] = 1;
            box_shape[1][1] = 1;
            break;
        case 4:
            height = 2;
            width = 3;
            box_shape[0][0] = 1;
            box_shape[0][1] = 1;
            box_shape[1][1] = 1;
            box_shape[1][2] = 1;
            break;
        case 5:
            height = 2;
            width = 2;
            box_shape[0][0] = 1;
            box_shape[0][1] = 1;
            box_shape[1][0] = 1;
            box_shape[1][1] = 1;
            break;
        case 6:
            height = 2;
            width = 3;
            box_shape[0][1] = 1;
            box_shape[1][0] = 1;
            box_shape[1][1] = 1;
            box_shape[1][2] = 1;
            break;
    }
    x = game_win_width / 2;
    y = 1;

    if (is_overlap())
        is_over = true;
}

void Piece::rotate() {
    int temp[4][4] = {0};
    int original_piece[4][4] = {0};
    int temp_height, temp_width;

    int original_height = height;
    int original_width = width;
    
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            original_piece[i][j] = box_shape[i][j];
    
    //Transpose
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            temp[j][i] = box_shape[i][j];
    
    int t = height;
    height = width;
    width = t;
    
    //Refresh
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            box_shape[i][j] = 0;
    
    //Flip horizontally
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            box_shape[i][width - j - 1] = temp[i][j];

    //Rotation is not a valid move -> get back to original
    if(is_overlap()) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                box_shape[i][j] = original_piece[i][j];
        height = original_height;
        width = original_width;
    }

    //Rotation is valid
    else {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) {
                if(original_piece[i][j] == 1) {
                    mvwaddch(game_win, y+i, x+j, ' ');
                    wrefresh(game_win);
                }
            }
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) {
                if(box_shape[i][j] == 1) {
                    mvwaddch(game_win, y+i, x+j, '#');
                    wrefresh(game_win);
                }
            }
    }
}

/**
Piece::switch_status() {
    "stub"
}
**/

//keyboard
void Piece::move() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 400000; //.4 sec
    
    //select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptionfds, struct timeval *timeout)
    if(select(1, &fds, NULL, NULL, &timeout) == 0) {
        y++;
        if(is_overlap()) {
            y--;
            for (int i = 0; i < height; i++)
                for (int j = 0; j < width; j++)
                    if(box_shape[i][j] == 1)
                        box_map[y+i][x+j] = 1;
            score_next();
        }
        else {
            for (int i = height - 1; i >= 0; i--)
                for (int j = 0; j < width; j++) {
                    if (box_shape[i][j] == 1) {
                        mvwaddch(game_win, y-1+i, x+j, ' ');
                        mvwaddch(game_win, y+i, x+j, '#');
                    }
                }
            wrefresh(game_win);
        }
    }

    //check if fd 0 is in fds
    //aka if a signal is received
    if(FD_ISSET(0, &fds)) {
        key = getch();
        while (key == -1);

        if (key == KEY_LEFT) {
            x--;
            if (is_overlap())
                x++;
            else {
                for (int i = 0; i < height; i++)
                    for (int j = 0; j < width; j++) {
                        if (box_shape[i][j] == 1) {
                            mvwaddch(game_win, y+i, x+j+1, ' ');
                            mvwaddch(game_win, y+i, x+j, '#');
                        }
                    }
                wrefresh(game_win);
            }
        }

        if (key == KEY_RIGHT) {
            x++;
            if (is_overlap())
                x--;
            else {
                //MIND THE ORDER OF ERASING
                for (int i = 0; i < height; i++)
                    for (int j = width - 1; j >= 0; j--) {
                        if (box_shape[i][j] == 1) {
                            mvwaddch(game_win, y+i, x+j-1, ' ');
                            mvwaddch(game_win, y+i, x+j, '#');
                        }
                    }
                wrefresh(game_win);
            }
        }

        if(key == KEY_DOWN) {
            y++;
            if (is_overlap()) {
                y--;
                for (int i = 0; i < height; i++)
                    for (int j = 0; j < width; j++)
                        box_map[y+i][x+j] = 1;
            }
            else {
                //MIND THE ORDER OF ERASING
                for (int i = height - 1; i >= 0; i--)
                    for (int j = 0; j < width; j++) {
                        if(box_shape[i][j] == 1) {
                            mvwaddch(game_win, y+i-1, x+j, ' ');
                            mvwaddch(game_win, y+i, x+j, '#');
                        }
                    }
                wrefresh(game_win);
            }
        }

        if(key == KEY_UP) {
            rotate();
        }
    }
}

bool Piece::is_overlap() {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            if (box_shape[i][j] == 1) {
                if (y + i  > game_win_height - 2)
                    return true;
                if (x + j  > game_win_width - 2 || x + j < 1)
                    return true;
                if (box_map[y+i][x+j] == 1)
                    return true;
            }
        }
    return false;
}

bool Piece::is_row_occupied(int row) {
    for (int j = 1; j < game_win_width - 1; j++)
        if(box_map[row][j] == 1)
            return true;
    return false;
}

void Piece::judge() {
    int filled_line = 0;
    bool is_full;

    for (int i = 1; i < game_win_height - 1; i++) {
        is_full = true;
        for (int j = 1; j < game_win_width - 1; j++) {
            if (box_map[i][j] == 0)
                is_full = false;
        }
        //Empty the row first, and get rid of empty rows
        if (is_full) {
            score += 5;
            filled_line++;
            for (int j = 1; j < game_win_width - 1; j++) 
                box_map[i][j] = 0;
        }
    }

    if (filled_line != 0) {
        for (int i = game_win_height - 2; i > 1; i--) {
            int temp = i;
            if (!is_row_occupied(i)) {
                while(temp > 1 && (!is_row_occupied(--temp)));
                //Until a row that has something
                for(int j = 1; j < game_win_width - 1; j++) {
                    box_map[i][j] = box_map[temp][j];
                    box_map[temp][j] = 0;
                }
            }
        }

        for (int i = 1; i < game_win_height - 1; i++)
            for (int j = 1; j < game_win_width - 1; j++) {
                if(box_map[i][j] == 1)
                    mvwaddch(game_win, i, j, '#');
                else
                    mvwaddch(game_win, i, j, ' ');
            }
        wrefresh(game_win);
    }
}

//Game loop
int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    refresh();

    //Windows
    game_win = create_newwin(game_win_height, game_win_width, 0, 0);
    wborder(game_win, '|', '|', '-', '-', '+', '+', '+', '+');
    wrefresh(game_win);
    
    score_win = create_newwin(next_win_height, next_win_width, 10, game_win_width+5);
    mvprintw(10, game_win_width+5, "%s", "Score");
    refresh();

    next_win = create_newwin(next_win_height, next_win_width, 0, game_win_width+5);
    mvprintw(0, game_win_width+5, "%s", "Next");
    refresh();

    Piece *p = new Piece;
    p -> initialize();

    while(!p -> is_over)
        p -> move();
    
    // Clean up
    remove_win(game_win);
    remove_win(next_win);
    remove_win(score_win);
    delete p;
    system("clear");

    int terminal_height, terminal_width;
    getmaxyx(stdscr, terminal_height, terminal_width);
    mvprintw(terminal_height/2, terminal_width/2, "%s", "GAME OVER!\n");
    refresh();

    sleep(2);
    endwin();
    
    return 0;
}
