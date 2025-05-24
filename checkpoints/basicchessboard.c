#undef main
#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640
#define TILE_SIZE 80

const char *board[8][8] = {
    { "chess-rook-icon.png", "chess-knight-icon.png", "chess-bishop-icon.png", "chess-queen-icon.png",
      "chess-king-icon.png", "chess-bishop-icon.png", "chess-knight-icon.png", "chess-rook-icon.png" },
    { "chess-pawn-icon.png", "chess-pawn-icon.png", "chess-pawn-icon.png", "chess-pawn-icon.png",
      "chess-pawn-icon.png", "chess-pawn-icon.png", "chess-pawn-icon.png", "chess-pawn-icon.png" },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { "wChess-Pawn-icon.png", "wChess-Pawn-icon.png", "wChess-Pawn-icon.png", "wChess-Pawn-icon.png",
      "wChess-Pawn-icon.png", "wChess-Pawn-icon.png", "wChess-Pawn-icon.png", "wChess-Pawn-icon.png" },
    { "wChess-Rook-icon.png", "wChess-Knight-icon.png", "wBishop-icon.png", "wChess-Queen-icon.png",
      "wChess-King-icon.png", "wBishop-icon.png", "wChess-Knight-icon.png", "wChess-Rook-icon.png" }
};

SDL_Texture* loadTexture(SDL_Renderer *renderer, const char *filePath) {
    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "images/%s", filePath);
    SDL_Surface *surface = IMG_Load(fullPath);
    if (!surface) {
        printf("Failed to load image %s: %s\n", filePath, IMG_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *window = SDL_CreateWindow("SDL Chessboard", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White background
    SDL_RenderClear(renderer);

    SDL_Color light = {200, 200, 200, 255};
    SDL_Color dark = {100, 100, 100, 255};

    // Draw board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            SDL_Rect tile = { col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            SDL_Color color = (row + col) % 2 == 0 ? light : dark;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &tile);

            if (board[row][col]) {
                SDL_Texture *piece = loadTexture(renderer, board[row][col]);
                if (piece) {
                    SDL_RenderCopy(renderer, piece, NULL, &tile);
                    SDL_DestroyTexture(piece); // free after rendering
                }
            }
        }
    }

    SDL_RenderPresent(renderer);

    // Wait before closing
    SDL_Event e;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
