#include "user/user.h"
#include "kernel/types.h"
#include "kernel/stat.h"

#define WIDTH 20
#define HEIGHT 10
#define MAX_SNAKE_LEN (WIDTH*HEIGHT)

// 方向
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

struct Point {
    int x, y;
};

struct Snake {
    struct Point body[MAX_SNAKE_LEN];
    int len;
    int dir;
};

void draw(struct Snake *snake, struct Point food) {
    printf("\033[H"); // 光标回到左上角
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int is_snake = 0;
            for (int k = 0; k < snake->len; k++) {
                if (snake->body[k].x == x && snake->body[k].y == y) {
                    is_snake = 1;
                    break;
                }
            }
            if (is_snake) {
                printf("O");
            } else if (food.x == x && food.y == y) {
                printf("*");
            } else if (x == 0 || x == WIDTH-1 || y == 0 || y == HEIGHT-1) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
}

int check_collision(struct Snake *snake, struct Point next) {
    if (next.x <= 0 || next.x >= WIDTH-1 || next.y <= 0 || next.y >= HEIGHT-1)
        return 1;
    for (int i = 0; i < snake->len; i++) {
        if (snake->body[i].x == next.x && snake->body[i].y == next.y)
            return 1;
    }
    return 0;
}

void gen_food(struct Snake *snake, struct Point *food) {
    int ok = 0;
    while (!ok) {
        food->x = (uptime() * 17) % (WIDTH-2) + 1;
        food->y = (uptime() * 31) % (HEIGHT-2) + 1;
        ok = 1;
        for (int i = 0; i < snake->len; i++) {
            if (snake->body[i].x == food->x && snake->body[i].y == food->y) {
                ok = 0;
                break;
            }
        }
    }
}

int main() {
    struct Snake snake;
    snake.len = 3;
    snake.body[0].x = WIDTH/2; snake.body[0].y = HEIGHT/2;
    snake.body[1].x = WIDTH/2-1; snake.body[1].y = HEIGHT/2;
    snake.body[2].x = WIDTH/2-2; snake.body[2].y = HEIGHT/2;
    snake.dir = RIGHT;
    struct Point food;
    gen_food(&snake, &food);
    char buf[1];
    printf("\033[2J"); // 清屏
    while (1) {
        // 非阻塞读取输入
        if (read(0, buf, 1) > 0) {
            if (buf[0] == 'w' && snake.dir != DOWN) snake.dir = UP;
            else if (buf[0] == 's' && snake.dir != UP) snake.dir = DOWN;
            else if (buf[0] == 'a' && snake.dir != RIGHT) snake.dir = LEFT;
            else if (buf[0] == 'd' && snake.dir != LEFT) snake.dir = RIGHT;
        }
        struct Point next = snake.body[0];
        if (snake.dir == UP) next.y--;
        else if (snake.dir == DOWN) next.y++;
        else if (snake.dir == LEFT) next.x--;
        else if (snake.dir == RIGHT) next.x++;
        if (check_collision(&snake, next)) {
            printf("Game Over! Score: %d\n", snake.len-3);
            break;
        }
        // 移动蛇
        for (int i = snake.len; i > 0; i--) {
            snake.body[i] = snake.body[i-1];
        }
        snake.body[0] = next;
        if (next.x == food.x && next.y == food.y) {
            snake.len++;
            gen_food(&snake, &food);
        } else {
            // 蛇长度不变
            // 蛇尾自动被覆盖
        }
        draw(&snake, food);
        sleep(10); // 控制速度
    }
    exit(0);
}
