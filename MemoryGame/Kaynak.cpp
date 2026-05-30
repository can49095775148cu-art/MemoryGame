#define _CRT_SECURE_NO_WARNINGS
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Ekran boyutu ayarı
#define SCREEN_W 640
#define SCREEN_H 480

// Kart dizilim ayarı
#define GRID_SIZE 4
#define TOTAL_CARDS (GRID_SIZE * GRID_SIZE)

// Kart boyutu ayarı
#define CARD_MAX_SIZE 80

// Kart değerlerini tutar
int cards[TOTAL_CARDS];

// Kart açık mı kapalı mı kontrolü
int revealed[TOTAL_CARDS];

// Flip animasyonu için genişlik değeri
float currentWidths[TOTAL_CARDS];

// Animasyon durumu kontrolü
int animStates[TOTAL_CARDS];

// Animasyon hızı ayarı
const float animSpeed = 8.0f;

// Seçilen kartlar
int first = -1, second = -1;

// Tıklama kontrol sistemi
int canClick = 1;

// Hamle sayacı
int moves = 0;

// Eşleşen çift sayısı
int matchedPairs = 0;

// Toplam çift sayısı
int totalPairs = TOTAL_CARDS / 2;

// Süre sistemi
Uint32 startTime = 0;
int elapsedTime = 0;

// Oyun bitti kontrolü
int gameOver = 0;

// Menü sistemi kontrolü
int gameStarted = 0;

// Yeni rekor kontrolü
int isNewRecord = 0;

// En iyi skor sistemi
int bestMoves = 9999;

// Kart renkleri
int cardColors[8][3] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 0},
    {255, 0, 255},
    {0, 255, 255},
    {255, 128, 0},
    {128, 0, 128}
};

// En iyi skoru dosyadan yükler
void loadHighScore()
{
    FILE* file = fopen("highscore.txt", "r");

    if (file != NULL)
    {
        fscanf(file, "%d", &bestMoves);
        fclose(file);
    }
    else
    {
        bestMoves = 9999;
    }
}

// Yeni rekoru dosyaya kaydeder
void saveHighScore(int newBest)
{
    FILE* file = fopen("highscore.txt", "w");

    if (file != NULL)
    {
        fprintf(file, "%d", newBest);
        fclose(file);
    }
}

// Kartları karıştırır
void shuffle()
{
    for (int i = 0; i < TOTAL_CARDS; i++)
        cards[i] = i % (TOTAL_CARDS / 2);

    srand(time(NULL));

    for (int i = 0; i < TOTAL_CARDS; i++)
    {
        int j = rand() % TOTAL_CARDS;

        int temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    }
}

// Oyunu sıfırlar
void resetGame()
{
    for (int i = 0; i < TOTAL_CARDS; i++)
    {
        revealed[i] = 0;

        currentWidths[i] = CARD_MAX_SIZE;

        animStates[i] = 0;
    }

    first = -1;
    second = -1;

    moves = 0;

    matchedPairs = 0;

    gameOver = 0;

    canClick = 1;

    isNewRecord = 0;

    startTime = SDL_GetTicks();

    shuffle();

    loadHighScore();
}

// Kazanma kontrolü
int checkWin()
{
    return matchedPairs == totalPairs;
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    // Tam ekran pencere oluşturur
    SDL_Window* window = SDL_CreateWindow(
        "Memory Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_W,
        SCREEN_H,
        SDL_WINDOW_FULLSCREEN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_PRESENTVSYNC
    );

    resetGame();

    // Start butonu ayarı
    SDL_Rect startButton = { 220, 180, 200, 80 };

    // Kart kutuları
    SDL_Rect rects[TOTAL_CARDS];

    int margin = 10;

    int origX[TOTAL_CARDS];

    // Kartların ekrandaki yerleri
    for (int i = 0; i < TOTAL_CARDS; i++)
    {
        origX[i] =
            (i % GRID_SIZE) * (CARD_MAX_SIZE + margin) + 100;

        rects[i].y =
            (i / GRID_SIZE) * (CARD_MAX_SIZE + margin) + 80;

        rects[i].w = CARD_MAX_SIZE;

        rects[i].h = CARD_MAX_SIZE;
    }

    SDL_Event e;

    int running = 1;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            // Pencere kapatma kontrolü
            if (e.type == SDL_QUIT)
                running = 0;

            // R tuşu ile yeniden başlatır
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.sym == SDLK_r)
            {
                resetGame();

                gameStarted = 1;
            }

            // Mouse tıklama sistemi
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mx = e.button.x;
                int my = e.button.y;

                // Start menü sistemi
                if (!gameStarted)
                {
                    if (mx > startButton.x &&
                        mx < startButton.x + startButton.w &&
                        my > startButton.y &&
                        my < startButton.y + startButton.h)
                    {
                        gameStarted = 1;

                        startTime = SDL_GetTicks();
                    }
                }

                // Kart tıklama sistemi
                else if (!gameOver && canClick)
                {
                    for (int i = 0; i < TOTAL_CARDS; i++)
                    {
                        if (mx > origX[i] &&
                            mx < origX[i] + CARD_MAX_SIZE &&
                            my > rects[i].y &&
                            my < rects[i].y + rects[i].h &&
                            !revealed[i] &&
                            animStates[i] == 0)
                        {
                            // Kart açma animasyonu
                            animStates[i] = 1;

                            // İlk kart seçimi
                            if (first == -1)
                            {
                                first = i;
                            }

                            // İkinci kart seçimi
                            else if (second == -1)
                            {
                                second = i;

                                // Hamle sayacı
                                moves++;
                            }

                            break;
                        }
                    }
                }
            }
        }

        // Start ekranı çizimi
        if (!gameStarted)
        {
            SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);

            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 70, 70, 200, 255);

            SDL_RenderFillRect(renderer, &startButton);

            SDL_RenderPresent(renderer);

            continue;
        }

        // Kart flip animasyonu sistemi
        for (int i = 0; i < TOTAL_CARDS; i++)
        {
            // Kart kapanırken
            if (animStates[i] == 1)
            {
                currentWidths[i] -= animSpeed;

                if (currentWidths[i] <= 0)
                {
                    currentWidths[i] = 0;

                    animStates[i] = 2;

                    revealed[i] = !revealed[i];
                }
            }

            // Kart açılırken
            else if (animStates[i] == 2)
            {
                currentWidths[i] += animSpeed;

                if (currentWidths[i] >= CARD_MAX_SIZE)
                {
                    currentWidths[i] = CARD_MAX_SIZE;

                    animStates[i] = 0;

                    // Kart eşleşme kontrolü
                    if (i == second &&
                        animStates[first] == 0)
                    {
                        // Doğru eşleşme
                        if (cards[first] == cards[second])
                        {
                            matchedPairs++;

                            first = -1;
                            second = -1;
                        }

                        // Yanlış eşleşme
                        else
                        {
                            canClick = 0;
                        }
                    }
                }
            }
        }

        // Süre hesaplama sistemi
        if (!gameOver)
        {
            elapsedTime =
                (SDL_GetTicks() - startTime) / 1000;
        }

        // Yanlış kart gecikme sistemi
        static Uint32 wrongTime = 0;

        if (!canClick &&
            animStates[first] == 0 &&
            animStates[second] == 0)
        {
            if (wrongTime == 0)
                wrongTime = SDL_GetTicks();

            if (SDL_GetTicks() - wrongTime > 600)
            {
                animStates[first] = 1;

                animStates[second] = 1;

                first = -1;
                second = -1;

                canClick = 1;

                wrongTime = 0;
            }
        }

        // Oyun bitiş kontrolü
        if (checkWin() && !gameOver)
        {
            gameOver = 1;

            // Yeni rekor kontrolü
            if (moves < bestMoves)
            {
                bestMoves = moves;

                saveHighScore(bestMoves);

                isNewRecord = 1;
            }
        }

        // Arka plan rengi
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

        SDL_RenderClear(renderer);

        // Kart çizim sistemi
        for (int i = 0; i < TOTAL_CARDS; i++)
        {
            rects[i].w = (int)currentWidths[i];

            rects[i].x =
                origX[i] +
                (CARD_MAX_SIZE - rects[i].w) / 2;

            // Açık kart renkleri
            if (revealed[i])
            {
                int colorIndex = cards[i];

                SDL_SetRenderDrawColor(
                    renderer,
                    cardColors[colorIndex][0],
                    cardColors[colorIndex][1],
                    cardColors[colorIndex][2],
                    255
                );
            }

            // Kapalı kart rengi
            else
            {
                SDL_SetRenderDrawColor(
                    renderer,
                    200,
                    200,
                    200,
                    255
                );
            }

            SDL_RenderFillRect(renderer, &rects[i]);
        }

        // Üst bilgi kutusu
        SDL_Rect ui = { 10, 10, 220, 60 };

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);

        SDL_RenderFillRect(renderer, &ui);

        // Oyun bitiş ekranı
        if (gameOver)
        {
            SDL_Rect over = { 150, 150, 340, 180 };

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 230);

            SDL_RenderFillRect(renderer, &over);

            printf("\nGAME OVER\n");
            printf("Press R to Restart\n");
        }

        SDL_RenderPresent(renderer);

        // Konsol bilgi sistemi
        if (bestMoves == 9999)
        {
            printf(
                "Time: %d sec | Moves: %d | Best: ---\r",
                elapsedTime,
                moves
            );
        }
        else
        {
            if (isNewRecord)
            {
                printf(
                    "Time: %d sec | Moves: %d | NEW RECORD | Best: %d\r",
                    elapsedTime,
                    moves,
                    bestMoves
                );
            }
            else
            {
                printf(
                    "Time: %d sec | Moves: %d | Best: %d moves\r",
                    elapsedTime,
                    moves,
                    bestMoves
                );
            }
        }
    }

    // Gereksiz dosya oluşmasını engeller
    if (bestMoves == 9999)
    {
        remove("highscore.txt");
    }

    SDL_DestroyRenderer(renderer);

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}