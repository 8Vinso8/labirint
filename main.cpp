#include <cmath>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <ncurses.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
using namespace std;

int screenWidth = 180;
int screenHeight = 60;

int mapWidth = 16;
int mapHeight = 16;

int FPS = 60;

float playerX = 2;
float playerY = 2;
float playerAngle = 0.0;
float fov = numbers::pi / 4.0;
float playerSpeed = 0.1f / ((float) FPS / 60.0f);

float renderDistance = 16.0;

int main()
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    auto *screen = new wchar_t[screenWidth * screenHeight];

    string map;
    map += "#########.......";
    map += "#...............";
    map += "#.......########";
    map += "#..............#";
    map += "#......##......#";
    map += "#......##......#";
    map += "#..............#";
    map += "###............#";
    map += "##.............#";
    map += "#......####..###";
    map += "#......#.......#";
    map += "#......#.......#";
    map += "#..............#";
    map += "#......#########";
    map += "#..............#";
    map += "################";


    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    using clock = std::chrono::steady_clock;
    auto next_frame = clock::now();

    bool runGame = true;

    vector<int> keysPressed;

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

        if (find(keysPressed.begin(), keysPressed.end(), KEY_LEFT) != keysPressed.end())
        {
            playerAngle -= (playerSpeed * 0.75f);
        }

        if (find(keysPressed.begin(), keysPressed.end(), KEY_RIGHT) != keysPressed.end())
        {
            playerAngle += (playerSpeed * 0.75f);
        }

        if (find(keysPressed.begin(), keysPressed.end(), KEY_UP) != keysPressed.end())
        {
            playerX += sinf(playerAngle) * playerSpeed;
            playerY += cosf(playerAngle) * playerSpeed;

            if (map.c_str()[(int) playerX * mapWidth + (int) playerY] != '.')
            {
                playerX -= sinf(playerAngle) * playerSpeed;
                playerY -= cosf(playerAngle) * playerSpeed;
            }
        }

        if (find(keysPressed.begin(), keysPressed.end(), KEY_DOWN) != keysPressed.end())
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

            int ceilingDistance = (int) ((float) (screenHeight / 2.0) -
                                         (float) screenHeight / ((float) distanceToWall));
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
        //printw(L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", playerX, playerY, playerAngle, 1.0f/timeDifferenceNumber);
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