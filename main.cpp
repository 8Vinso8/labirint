#include <iostream>
#include <cmath>
#include <vector>
#include <windows.h>

using namespace std;

int screenWidth = 120;
int screenHeight = 40;
int mapWidth = 16;
int mapHeight = 16;

float playerX = 14.7;
float playerY = 5.09;
float playerAngle = 0.0;
float fov = numbers::pi / 4.0;
float renderDistance = 16.0;


int main()
{
    auto *screen = new char[screenWidth * screenHeight];
    HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                               0,
                                               nullptr,
                                               CONSOLE_TEXTMODE_BUFFER,
                                               nullptr);
    SetConsoleActiveScreenBuffer(console);
    DWORD bytesWritten = 0;


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

    while (true)
    {
        playerAngle += 0.00015f;
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

            short shade = ' ';
            if (distanceToWall <= renderDistance / 4.0f)
                shade = 0x2588;
            else if (distanceToWall < renderDistance / 3.0f)
                shade = 0x2593;
            else if (distanceToWall < renderDistance / 2.0f)
                shade = 0x2592;
            else if (distanceToWall < renderDistance)
                shade = 0x2591;
            else
                shade = ' ';

            if (hitBoundary)
                shade = ' ';

            for (int y = 0; y < screenHeight; y++)
            {
                if (y <= ceilingDistance)
                    screen[y * screenWidth + x] = ' ';
                else if (y > ceilingDistance && y <= floorDistance)
                    screen[y * screenWidth + x] = shade;
                else
                {
                    float b = 1.0f - (((float) y - screenHeight / 2.0f) / ((float) screenHeight / 2.0f));
                    if (b < 0.25)
                        shade = '#';
                    else if (b < 0.5)
                        shade = 'x';
                    else if (b < 0.75)
                        shade = '.';
                    else if (b < 0.9)
                        shade = '-';
                    else
                        shade = ' ';
                    screen[y * screenWidth + x] = shade;
                }
            }
        }
        WriteConsoleOutputCharacterA(console,
                                     screen,
                                     screenWidth * screenHeight,
                                     {0, 0},
                                     &bytesWritten);
    }
    return 0;
}