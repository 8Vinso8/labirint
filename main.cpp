#include <cmath>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <ncurses.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
using namespace std;

int screenWidth = 120;
int screenHeight = 40;

int mapWidth = 16;
int mapHeight = 16;

float playerX = 14.7;
float playerY = 5.09;
float playerAngle = 0.0;
float fov = numbers::pi / 4.0;
float playerSpeed = 5.0;

float renderDistance = 16.0;
float msDelay = 15;


int main()
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(FALSE);
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

    chrono::system_clock::time_point timePoint1 = chrono::system_clock::now();
    chrono::system_clock::time_point timePoint2 = chrono::system_clock::now();

    while (true)
    {
        timePoint1 = chrono::system_clock::now();
        chrono::duration<float> timeDifference = timePoint1 - timePoint2;
        float timeDifferenceNumber = timeDifference.count();

        if (false)
        {
            playerAngle -= (playerSpeed * 0.75f) * timeDifferenceNumber;
        }

        if (false)
        {
            playerAngle += (playerSpeed * 0.75f) * timeDifferenceNumber;
        }

        if (false)
        {
            playerX += sinf(playerAngle) * playerSpeed * timeDifferenceNumber;
            playerY += cosf(playerAngle) * playerSpeed * timeDifferenceNumber;

            if (map.c_str()[(int)playerX * mapWidth + (int)playerY] != '.')
            {
                playerX -= sinf(playerAngle) * playerSpeed * timeDifferenceNumber;
                playerY -= cosf(playerAngle) * playerSpeed * timeDifferenceNumber;
            }
        }

        if (false)
        {
            playerX -= sinf(playerAngle) * playerSpeed * timeDifferenceNumber;
            playerY -= cosf(playerAngle) * playerSpeed * timeDifferenceNumber;

            if (map.c_str()[(int)playerX * mapWidth + (int)playerY] != '.')
            {
                playerX += sinf(playerAngle) * playerSpeed * timeDifferenceNumber;
                playerY += cosf(playerAngle) * playerSpeed * timeDifferenceNumber;
            }
        }


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
                                float vy = (float) testY + ty - playerY;
                                float vx = (float) testX + tx - playerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (eyeX * vx / d) + (eyeY * vy / d);
                                p.emplace_back(d, dot);
                            }

                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {
                            return left.first < right.first;
                        });

                        float bound = 0.01;
                        if (acos(p.at(0).second) < bound or acos(p.at(1).second) < bound or
                            acos(p.at(2).second) < bound)
                            hitBoundary = true;
                    }
                }
            }

            int ceilingDistance = (float) (screenHeight / 2.0) - screenHeight / ((float) distanceToWall);
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
                    float b = 1.0f - (((float) y - screenHeight / 2.0f) / ((float) screenHeight / 2.0f));
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

        timePoint2 = chrono::system_clock::now();
        //printw(L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", playerX, playerY, playerAngle, 1.0f/timeDifferenceNumber);
        mvaddwstr(0, 0, screen);
        refresh();
    }
    return 0;
}
#pragma clang diagnostic pop