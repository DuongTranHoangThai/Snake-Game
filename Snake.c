#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

#define WIDTH 40
#define HEIGHT 20
#define SNAKE_MAXLENGTH (WIDTH * HEIGHT)

typedef enum
{
	LEFT = 0,
	RIGHT,
	UP,
	DOWN,
	INVALID
} Direction_t;

struct
{
	int x;
	int y;
} typedef Snake_pos_t;

typedef struct {
    int length;
    Snake_pos_t pos[SNAKE_MAXLENGTH];
    int direction;
} Snake_t;

Snake_t Snake;

typedef struct {
    Snake_pos_t pos;
} Food_t;

Food_t food;

int random_number (int min, int max)
{
	return min + rand() % (max - min + 1);
}

void food_set(int row, int col)
{
	food.pos.x = col;
	food.pos.y = row;
}

void enable_nonblocking_input(void) {
    struct termios newt;
    tcgetattr(STDIN_FILENO, &newt);
    newt.c_lflag &= ~(ICANON | ECHO); // Disable buffering & echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK); // Set non-blocking mode
}

void check_input(void)
{
    int ch = getchar();
    if (ch == 27 && getchar() == 91) { // Detect arrow key sequence
        switch (getchar()) {
            case 65: Snake.direction = UP; break;
            case 66: Snake.direction = DOWN; break;
            case 67: Snake.direction = RIGHT; break;
            case 68: Snake.direction = LEFT; break;
        }
    }
}

void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void cursor_position(int row, int col) {
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
}

void draw_border(void) {
    clear_screen();
    for (int i = 0; i < HEIGHT + 2; i++) {
        for (int j = 0; j < WIDTH + 2; j++) {
            if (i == 0 || i == HEIGHT + 1 || j == 0 || j == WIDTH + 1)
                printf("#");
            else
                printf(" ");
        }
        printf("\n");
    }
}

void snake_init(int row, int col, int length) {
    Snake.length = length;
    for (int i = 0; i < length; i++) {
        Snake.pos[i].x = col + i;
        Snake.pos[i].y = row;
    }
    Snake.direction = RIGHT;
}

void snake_draw(void) {
    //Draw the food
    cursor_position(food.pos.y + 1, food.pos.x + 1);
    printf("@");

	//Draw the Snake
    for (int i = 0; i < Snake.length; i++) {
        cursor_position(Snake.pos[i].y + 1, Snake.pos[i].x + 1);
        printf("0");
    }



    fflush(stdout);
}

void snake_move(Direction_t direction) {
    // Move the body
    for (int i = 0; i < Snake.length - 1; i++) {
    	Snake.pos[i].x = Snake.pos[i+1].x;
    	Snake.pos[i].y = Snake.pos[i+1].y;
    }

    //New head
    switch(direction)
    {
    case RIGHT:
		Snake.pos[Snake.length - 1].x += 1; break;
    case DOWN:
    	Snake.pos[Snake.length - 1].y += 1; break;
    case UP:
    	Snake.pos[Snake.length - 1].y -= 1; break;
    case LEFT:
		Snake.pos[Snake.length - 1].x -= 1; break;
    case INVALID:
		break;
    default:
    	break;
    }

    // Wrap-around at borders
    if (Snake.pos[Snake.length - 1].x > WIDTH) Snake.pos[Snake.length - 1].x = 1;
    if (Snake.pos[Snake.length - 1].x < 1) Snake.pos[Snake.length - 1].x = WIDTH;
    if (Snake.pos[Snake.length - 1].y > HEIGHT) Snake.pos[Snake.length - 1].y = 1;
    if (Snake.pos[Snake.length - 1].y < 1) Snake.pos[Snake.length - 1].y = HEIGHT;

    //Check if snake take a self collision
    for(int i = 0 ; i < Snake.length  -1; i++)
    {
    	if((Snake.pos[Snake.length -1].x == Snake.pos[i].x) &&
    	   (Snake.pos[Snake.length -1].y == Snake.pos[i].y) )
		{
			//Snake have selft collision
            printf("\nGame Over! Snake hit the wall.\n");
            printf("\033[?25h"); // Show cursor
    		   fflush(stdout);

            exit(0);
		}
   }

    //Check is snake take the food
    if( (Snake.pos[Snake.length -1].x == food.pos.x ) && (Snake.pos[Snake.length -1].y == food.pos.y ) )
    {
    	//Snake eat the food, he grow
    	if(Snake.length < SNAKE_MAXLENGTH)
    	{
    		Snake.pos[Snake.length] = Snake.pos[Snake.length - 1];
    		Snake.length ++;
    	}


    	//Make a new food
    	//TO DO : food should not be in the snake
    	food_set( random_number(1,HEIGHT-1),random_number(1,WIDTH-1) );
    }
}

void debug()
{
	cursor_position(50,50);
	printf("Snake :%d,%d",Snake.pos[Snake.length - 1].x,Snake.pos[Snake.length -1].y);
	printf("Food :%d,%d",food.pos.x,food.pos.y);
}

void hide_cursor(void)
{
    printf("\033[?25l");
    fflush(stdout);
}
void show_cursor(void) {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ECHO;  // Enable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    printf("\033[?25h");  // Show cursor
    fflush(stdout);
}

void handle_exit(int sig) {
	clear_screen();
	cursor_position(1,1);
    show_cursor();  // Ensure cursor is restored
    exit(0);
}

int main(void) {

	hide_cursor();
	enable_nonblocking_input();
    // Ensure cursor is shown on normal exit
    atexit(show_cursor);
    signal(SIGINT, handle_exit);

    clear_screen();
    memset(&Snake, 0, sizeof(Snake));
    snake_init(10, 5, 5);
    food_set(10,10);

    while (1) {
    	check_input();
        draw_border();
        snake_move(Snake.direction);
        debug();
        snake_draw();
        usleep(200000);

    }

    printf("\033[?25h"); // Show cursor
	fflush(stdout);
    return EXIT_SUCCESS;
}

