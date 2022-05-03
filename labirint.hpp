#include <iostream>
#include <cmath>

using namespace std;
const int wall = 0, pass = 1;

void make_exit(int height, int width, string &ret) { // Делаем выход
    int a = rand() % 3; // Рандомный выбор стены для выхода
    int i, value, increment; // Переменные для for'а

    if (a == 0) {
        i = 0;
        value = width;
        increment = 1;
    } else if (a == 1) {
        i = 0;
        value = 1;
        increment = width;
    } else {
        i = width - 1;
        value = -1;
        increment = width;
    }
    for (i; i < height * width; i += increment) {
        if (ret[i] == '#' and ret[i + value] == '.') {
            ret[i] = '@';
            break;
        }
    }
}

bool dead_end(int x, int y, int **maze, int height, int width) { // Проверка на тупик
    int a = 0;

    if (x != 1) {
        if (maze[y][x - 2] == pass)
            a += 1;
    } else a += 1;

    if (y != 1) {
        if (maze[y - 2][x] == pass)
            a += 1;
    } else a += 1;

    if (x != width - 2) {
        if (maze[y][x + 2] == pass)
            a += 1;
    } else a += 1;

    if (y != height - 2) {
        if (maze[y + 2][x] == pass)
            a += 1;
    } else a += 1;

    if (a == 4)
        return 1;
    else
        return 0;
}

string visual(int **maze, int height, int width) { // Отрисовка лабиринта в строчку
    string ret;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (maze[i][j] == wall) ret += '#';
            else ret += '.';
        }
    }
    return ret; // Возвращаем лабиринт строкой
}

void mazemake(int **maze, int height, int width) { // Алгос лабиринта
    int x = 3, y = 3, a = 0, c; // Точка приземления крота, счетчик, переменная для рандома

    for (int i = 0; i < height; i++) // Массив заполняется землей-ноликами
        for (int j = 0; j < width; j++)
            maze[i][j] = wall;

    while (a < 10000) { // Небольшой костыль
        maze[y][x] = pass;
        a++;
        while (1) { // Бесконечный цикл, который прерывается только тупиком
            c = rand() % 4; // Крот прорывает по две клетки в одном направлении за прыжок
            if(c == 0){
                if (y != 1)
                    if (maze[y - 2][x] == wall) { // вверх
                        maze[y - 1][x] = pass;
                        maze[y - 2][x] = pass;
                        y -= 2;
                    }
            }
            else if(c == 1){
                if (y != height - 2)
                    if (maze[y + 2][x] == wall) { // вниз
                        maze[y + 1][x] = pass;
                        maze[y + 2][x] = pass;
                        y += 2;
                    }
            }
            else if(c == 2){
                if (x != 1)
                    if (maze[y][x - 2] == wall) { // налево
                        maze[y][x - 1] = pass;
                        maze[y][x - 2] = pass;
                        x -= 2;
                    }
            }
            else{
                if (x != width - 2)
                    if (maze[y][x + 2] == wall) { // направо
                        maze[y][x + 1] = pass;
                        maze[y][x + 2] = pass;
                        x += 2;
                    }
            }
            if (dead_end(x, y, maze, height, width))//проверка на тупик
                break;
        }

        if (dead_end(x, y, maze, height, width)) // Вытаскиваем крота из тупика
            do {
                x = 2 * (rand() % ((width - 1) / 2)) + 1;
                y = 2 * (rand() % ((height - 1) / 2)) + 1;
            } while (maze[y][x] != pass);
    }
}

string Generate(int h = 17, int w = 17) { // Работает только с нечетными аргументами
    srand((unsigned) time(NULL));
    int height = h, width = w;

    int **maze = new int *[height]; // Делаем лабиринт
    for (int i = 0; i < height; i++)
        maze[i] = new int[width];
    mazemake(maze, height, width);

    string ret = visual(maze, height, width); // Визуализируем и делаем выход
    make_exit(height, width, ret);

    for (int i = 0; i < height; i++) // Чистим память
        delete[] maze[i];
    delete[] maze;
    return ret;
}

//int main() {
//    std::cout << Generate();
//    string out = Generate();
//    for(int i = 0; i < out.length(); i++) {
//        if (i % 17 == 0)
//            cout << endl;
//        cout << out[i];
//    }
//    return 0;
//}
