#include <cmath>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <ncurses.h>
#include "labirint.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
using namespace std;

int screenWidth = 120;
int screenHeight = 40;

int mapWidth = 17;
int mapHeight = 17;

int FPS = 60;

float playerX = 2;
float playerY = 2;
float playerAngle = 0.0;
float fov = numbers::pi / 4.0;
float playerSpeed = 0.1f;

float renderDistance = 16.0;

int main()
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    string map;

    using clock = std::chrono::steady_clock;
    auto next_frame = clock::now();

    bool runGame = true;

    vector<int> keysPressed;

    int menuIndex = 0;

    vector<wstring>menuName =   {L"Start",  L"MapWidth: ",      L"MapHeight: ",     L"FPS: ",    L"ScreenWidth: ",    L"ScreenHeight: " };
    vector<int*>menuVar =       {nullptr,   &mapWidth,          &mapHeight,         &FPS,        &screenWidth,        &screenHeight     };
    vector<int>increment =      {NULL,      2,                  2,                  30,          10,                  10                };

    while(runGame)
    {
        next_frame += std::chrono::milliseconds(1000 / FPS);

        keysPressed.clear();
        int c;
        while ((c = getch()) != ERR)
        {
            keysPressed.push_back(c);
        }

        if (find(keysPressed.begin(), keysPressed.end(), 10) != keysPressed.end())  // 27 - ESC
        {
            if (menuName[menuIndex] == L"Start")
            {
                runGame = false;
                continue;
            }
        }

        if (find(keysPressed.begin(), keysPressed.end(), 97) != keysPressed.end()) // 97 - A
        {
            if (menuVar[menuIndex] != nullptr)
            {
                if (*menuVar[menuIndex] - increment[menuIndex] > 0)
                {
                    *menuVar[menuIndex] -= increment[menuIndex];
                }
            }
        }

        if (find(keysPressed.begin(), keysPressed.end(), 100) != keysPressed.end()) // 100 - D
        {
            if (menuVar[menuIndex] != nullptr)
            {
                *menuVar[menuIndex] += increment[menuIndex];
            }
        }

        if (find(keysPressed.begin(), keysPressed.end(), 119) != keysPressed.end()) // 119 - W
        {
            menuIndex  = menuIndex - 1 < 0 ? menuName.size() - 1 : menuIndex - 1;
        }

        if (find(keysPressed.begin(), keysPressed.end(), 115) != keysPressed.end()) // 115 - S
        {
            menuIndex  = (menuIndex + 1) % menuName.size();
        }
        clear();
        for (int i = 0; i < menuName.size(); i++)
        {
            wstring name = menuName[i];
            if (menuVar[i] != nullptr)
            {
                name += to_wstring(*menuVar[i]);
            }
            const wchar_t *wname = name.c_str();
            mvaddwstr(i, 0, wname);
        }
        mvaddwstr(menuIndex, 40, L"<---");
        std::this_thread::sleep_until(next_frame);
        refresh();
    }

    map = Generate(mapHeight, mapWidth);

    auto *screen = new wchar_t[screenWidth * screenHeight];

    using clock = std::chrono::steady_clock;
    next_frame = clock::now();
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    runGame = true;

    keysPressed.clear();

    //playerSpeed = playerSpeed / ((float) FPS / 60.0f); - не имеет смысла с NCURSES


    while (runGame)
    {
        next_frame += std::chrono::milliseconds(1000 / FPS);

        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();


        int c;
        while ((c = getch()) != ERR)
        {
            keysPressed.push_back(c);
        }

        if (find(keysPressed.begin(), keysPressed.end(), 27) != keysPressed.end())  // 27 - ESC
        {
            runGame = false;
            continue;
        }

        if (find(keysPressed.begin(), keysPressed.end(), 97) != keysPressed.end()) // 97 - A
        {
            playerAngle -= (playerSpeed * 0.75f);
        }

        if (find(keysPressed.begin(), keysPressed.end(), 100) != keysPressed.end()) // 100 - D
        {
            playerAngle += (playerSpeed * 0.75f);
        }

        if (find(keysPressed.begin(), keysPressed.end(), 119) != keysPressed.end()) // 119 - W
        {
            playerX += sinf(playerAngle) * playerSpeed;
            playerY += cosf(playerAngle) * playerSpeed;

            if (map.c_str()[(int) playerX * mapWidth + (int) playerY] != '.')
            {
                playerX -= sinf(playerAngle) * playerSpeed;
                playerY -= cosf(playerAngle) * playerSpeed;
            }
        }

        if (find(keysPressed.begin(), keysPressed.end(), 115) != keysPressed.end()) // 115 - S
        {
            playerX -= sinf(playerAngle) * playerSpeed;
            playerY -= cosf(playerAngle) * playerSpeed;

            if (map.c_str()[(int) playerX * mapWidth + (int) playerY] != '.')
            {
                playerX += sinf(playerAngle) * playerSpeed;
                playerY += cosf(playerAngle) * playerSpeed;
            }
        }

        keysPressed.clear();


        for (int x = 0; x < screenWidth; x++)
        {
            float rayAngle = (playerAngle - fov / 2.0f) + ((float) x / (float) screenWidth) * fov;

            float stepSize = 0.1f;
            float distanceToWall = 0.0f;

            bool hitWall = false;
            bool hitBoundary = false;

            float eyeX = sinf(rayAngle);
            float eyeY = cosf(rayAngle);

            while (!hitWall && distanceToWall < renderDistance)
            {
                distanceToWall += stepSize;
                int testX = (int) (playerX + eyeX * distanceToWall);
                int testY = (int) (playerY + eyeY * distanceToWall);

                if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight)
                {
                    hitWall = true;
                    distanceToWall = renderDistance;
                } else
                {
                    if (map.c_str()[testX * mapWidth + testY] != '.')
                    {
                        hitWall = true;

                        vector<pair<float, float>> p;

                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float) testY + (float) ty - playerY;
                                float vx = (float) testX + (float) tx - playerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (eyeX * vx / d) + (eyeY * vy / d);
                                p.emplace_back(d, dot);
                            }

                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right)
                        {
                            return left.first < right.first;
                        });

                        float bound = 0.01;
                        if (acos(p.at(0).second) < bound or acos(p.at(1).second) < bound or
                            acos(p.at(2).second) < bound)
                            hitBoundary = true;
                    }
                }
            }

            int ceilingDistance = (int) ((float) (screenHeight / 2.0) - (float)screenHeight / ((float) distanceToWall));
            int floorDistance = screenHeight - ceilingDistance;

            short shadeChar = ' ';
            if (!hitBoundary)
            {
                if (distanceToWall <= renderDistance / 4.0f)
                    shadeChar = 0x2588;
                else if (distanceToWall < renderDistance / 3.0f)
                    shadeChar = 0x2593;
                else if (distanceToWall < renderDistance / 2.0f)
                    shadeChar = 0x2592;
                else if (distanceToWall < renderDistance)
                    shadeChar = 0x2591;
            }

            for (int y = 0; y < screenHeight; y++)
            {
                if (y <= ceilingDistance)
                    screen[y * screenWidth + x] = ' ';
                else if (y > ceilingDistance && y <= floorDistance)
                    screen[y * screenWidth + x] = shadeChar;
                else
                {
                    float b = 1.0f - (((float) y - (float) screenHeight / 2.0f) / ((float) screenHeight / 2.0f));
                    if (b < 0.25)
                        shadeChar = '#';
                    else if (b < 0.5)
                        shadeChar = 'x';
                    else if (b < 0.75)
                        shadeChar = '.';
                    else if (b < 0.9)
                        shadeChar = '-';
                    else
                        shadeChar = ' ';
                    screen[y * screenWidth + x] = shadeChar;
                }
            }
        }

        for (int nx = 0; nx < mapWidth; nx++)
            for (int ny = 0; ny < mapHeight; ny++)
            {
                screen[(ny + 1) * screenWidth + nx] = map[ny * mapWidth + nx];
            }
        screen[((int) playerX + 1) * screenWidth + (int) playerY] = 'P';

        mvaddwstr(0, 0, screen);

        wstring fps = to_wstring((int) (1.0f / fElapsedTime));
        const wchar_t *wfps = fps.c_str();
        mvaddwstr(0, 0, wfps);

        std::this_thread::sleep_until(next_frame);

        refresh();

    }
    return 0;
}

#pragma clang diagnostic pop