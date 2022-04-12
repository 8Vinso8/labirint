#include <cmath>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <windows.h>

#include "labirint.hpp"


using namespace std;

int main()
{
    int screenWidth = 180;
    int screenHeight = 60;

    int mapWidth = 17;
    int mapHeight = 17;

    int FPS = 60;

    float playerX = 2;
    float playerY = 2;
    float playerAngle = 0.0;
    float fov = numbers::pi / 4.0;
    float playerSpeed = 0.05f;

    int renderDistance = 17.0;
    bool miniMap = true;
    wstring mapStr = L"YES";

    auto *screen = new wchar_t[screenWidth * screenHeight];
    HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                               0,
                                               nullptr,
                                               CONSOLE_TEXTMODE_BUFFER,
                                               nullptr);
    SetConsoleActiveScreenBuffer(console);
    DWORD bytesWritten = 0;

    string map;

    using clock = std::chrono::steady_clock;
    auto next_frame = clock::now();

    bool runGame = true;

    int menuIndex = 0;

    vector<wstring>menuName =   {L"Start",  L"MapWidth: ",      L"MapHeight: ",     L"FPS: ",    L"ScreenWidth: ",    L"ScreenHeight: " , L"RenderDistance:2", L"Minimap:" };
    vector<int*>menuVar =       {nullptr,   &mapWidth,          &mapHeight,         &FPS,        &screenWidth,        &screenHeight  , &renderDistance, nullptr};
    vector<int>increment =      {NULL,      2,                  2,                  30,          10,                  10                };

    wstring blank = wstring(screenWidth, ' ');
    const wchar_t *wblank = blank.c_str();

    while(runGame)
    {
        next_frame += std::chrono::milliseconds(1000 / 10);

        if (GetAsyncKeyState(VK_SPACE))
        {
            if (menuName[menuIndex] == L"Start")
            {
                runGame = false;
                continue;
            }
            if (menuName[menuIndex] == L"Minimap:"){
                mapStr = mapStr == L"YES" ? L"NO" : L"YES";
                miniMap = mapStr == L"YES";
            }
        }

        if (GetAsyncKeyState((unsigned short) 'A') & 0x8000)
        {
            if (menuVar[menuIndex] != nullptr)
            {
                if (*menuVar[menuIndex] - increment[menuIndex] > 0)
                {
                    *menuVar[menuIndex] -= increment[menuIndex];
                }
            }
        }

        if (GetAsyncKeyState((unsigned short) 'D') & 0x8000)
        {
            if (menuVar[menuIndex] != nullptr)
            {
                *menuVar[menuIndex] += increment[menuIndex];
            }
        }

        if (GetAsyncKeyState((unsigned short) 'W') & 0x8000)
        {
            menuIndex  = menuIndex - 1 < 0 ? menuName.size() - 1 : menuIndex - 1;
        }

        if (GetAsyncKeyState((unsigned short) 'S') & 0x8000)
        {
            menuIndex  = (menuIndex + 1) % menuName.size();
        }
        // clear();
        for (int i = 0; i < menuName.size(); i++)
        {
            wstring name = menuName[i];
            if (menuVar[i] != nullptr)
            {
                name += to_wstring(*menuVar[i]);
            }
            if (menuName[i] == L"Minimap:"){
                name += mapStr;
            }
            const wchar_t *wname = name.c_str();
            WriteConsoleOutputCharacterW(console,
                                         wblank,
                                         screenWidth,
                                         {0, static_cast<SHORT>(i)},
                                         &bytesWritten);
            WriteConsoleOutputCharacterW(console,
                                         wname,
                                         name.size(),
                                         {0, static_cast<SHORT>(i)},
                                         &bytesWritten);
        }
        WriteConsoleOutputCharacterW(console,
                                     L"<---",
                                     4,
                                     {40, static_cast<SHORT>(menuIndex)},
                                     &bytesWritten);
        std::this_thread::sleep_until(next_frame);
    }

    map = Generate(mapHeight, mapWidth);

    runGame = true;

    playerSpeed = playerSpeed / ((float) FPS / 60.0f);

    map = Generate();

    using clock = std::chrono::steady_clock;
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();
    next_frame = clock::now();

    runGame = true;

    while (runGame)
    {
        next_frame += std::chrono::milliseconds(1000 / FPS);

        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        if (GetAsyncKeyState(VK_ESCAPE))
        {
            runGame = false;
            continue;
        }

        if (GetAsyncKeyState((unsigned short) 'A') & 0x8000)
        {
            playerAngle -= (playerSpeed * 0.5f);
        }

        if (GetAsyncKeyState((unsigned short) 'D') & 0x8000)
        {
            playerAngle += (playerSpeed * 0.5f);
        }

        if (GetAsyncKeyState((unsigned short) 'W') & 0x8000)
        {
            playerX += sinf(playerAngle) * playerSpeed;
            playerY += cosf(playerAngle) * playerSpeed;

            if (map.c_str()[(int) playerX * mapWidth + (int) playerY] != '.')
            {
                playerX -= sinf(playerAngle) * playerSpeed;
                playerY -= cosf(playerAngle) * playerSpeed;
            }
        }

        if (GetAsyncKeyState((unsigned short) 'S') & 0x8000)
        {
            playerX -= sinf(playerAngle) * playerSpeed;
            playerY -= cosf(playerAngle) * playerSpeed;

            if (map.c_str()[(int) playerX * mapWidth + (int) playerY] != '.')
            {
                playerX += sinf(playerAngle) * playerSpeed;
                playerY += cosf(playerAngle) * playerSpeed;
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

            while (!hitWall and distanceToWall < renderDistance)
            {
                distanceToWall += stepSize;
                int testX = (int) (playerX + eyeX * distanceToWall);
                int testY = (int) (playerY + eyeY * distanceToWall);

                if (testX < 0 or testX >= mapWidth or testY < 0 or testY >= mapHeight)
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

            int ceilingDistance = (int) (((float) screenHeight / 2.0f) -
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

        swprintf_s(screen, 40, L"FPS=%3.2f ", 1.0f / fElapsedTime);

        if (miniMap) {
            for (int nx = 0; nx < mapWidth; nx++)
                for (int ny = 0; ny < mapWidth; ny++) {
                    screen[(ny + 1) * screenWidth + nx] = map[ny * mapWidth + nx];
                }
            screen[((int) playerX + 1) * screenWidth + (int) playerY] = 'P';
        }
        WriteConsoleOutputCharacterW(console,
                                     screen,
                                     screenWidth * screenHeight,
                                     {0, 0},
                                     &bytesWritten);

        std::this_thread::sleep_until(next_frame);
    }
    return 0;
}