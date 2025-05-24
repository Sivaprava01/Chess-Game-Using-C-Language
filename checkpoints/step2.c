#undef main
#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640
#define TILE_SIZE 80

typedef struct {
    char type;   // 'P', 'R', 'N', 'B', 'Q', 'K'
    char color;  // 'w' or 'b'
    int hasMoved; // 0 = not moved, 1 = already moved
} Piece;

Piece board[8][8] = {
    {{'R','b',0}, {'N','b',0}, {'B','b',0}, {'Q','b',0}, {'K','b',0}, {'B','b',0}, {'N','b',0}, {'R','b',0}},
    {{'P','b',0}, {'P','b',0}, {'P','b',0}, {'P','b',0}, {'P','b',0}, {'P','b',0}, {'P','b',0}, {'P','b',0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{'P','w',0}, {'P','w',0}, {'P','w',0}, {'P','w',0}, {'P','w',0}, {'P','w',0}, {'P','w',0}, {'P','w',0}},
    {{'R','w',0}, {'N','w',0}, {'B','w',0}, {'Q','w',0}, {'K','w',0}, {'B','w',0}, {'N','w',0}, {'R','w',0}}
};

char currentTurn = 'w';  // White always starts

const char* getImageFile(char type, char color) {
    if (color == 'w') {
        switch (type) {
            case 'P': return "wChess-Pawn-icon.png";
            case 'R': return "wChess-Rook-icon.png";
            case 'N': return "wChess-Knight-icon.png";
            case 'B': return "wBishop-icon.png";
            case 'Q': return "wChess-Queen-icon.png";
            case 'K': return "wChess-King-icon.png";
        }
    } else {
        switch (type) {
            case 'P': return "chess-pawn-icon.png";
            case 'R': return "chess-rook-icon.png";
            case 'N': return "chess-knight-icon.png";
            case 'B': return "chess-bishop-icon.png";
            case 'Q': return "chess-queen-icon.png";
            case 'K': return "chess-king-icon.png";
        }
    }
    return NULL;
}

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

void drawBoard(SDL_Renderer *renderer) {
    SDL_Color light = {200, 200, 200, 255};
    SDL_Color dark = {100, 100, 100, 255};

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            SDL_Rect tile = { col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            SDL_Color color = (row + col) % 2 == 0 ? light : dark;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &tile);

            Piece p = board[row][col];
            if (p.type != 0) {
                const char *imgFile = getImageFile(p.type, p.color);
                SDL_Texture *piece = loadTexture(renderer, imgFile);
                if (piece) {
                    SDL_RenderCopy(renderer, piece, NULL, &tile);
                    SDL_DestroyTexture(piece);
                }
            }
        }
    }

    SDL_RenderPresent(renderer);
}

int isValidMove(int r1, int c1, int r2, int c2) {
    return (r2 >= 0 && r2 < 8 && c2 >= 0 && c2 < 8);
}

int isMoveValid(Piece piece, int fromRow, int fromCol, int toRow, int toCol) {
    if (!isValidMove(fromRow, fromCol, toRow, toCol)) return 0;

    Piece dest = board[toRow][toCol];
    if (dest.type != 0 && dest.color == piece.color) return 0; // Can't capture own piece

    int rowDiff = toRow - fromRow;
    int colDiff = toCol - fromCol;

    switch (piece.type) {
        case 'P': {
            int dir = (piece.color == 'w') ? -1 : 1;
            if (fromCol == toCol) {
                if (rowDiff == dir && dest.type == 0) return 1;
                if (!piece.hasMoved && rowDiff == 2 * dir &&
                    board[fromRow + dir][fromCol].type == 0 && dest.type == 0) return 1;
            } else if (abs(colDiff) == 1 && rowDiff == dir && dest.type != 0 && dest.color != piece.color) {
                return 1; // Diagonal capture
            }
            break;
        }
        case 'R': {
            if (fromRow == toRow) {
                int step = (colDiff > 0) ? 1 : -1;
                for (int c = fromCol + step; c != toCol; c += step)
                    if (board[fromRow][c].type != 0) return 0;
                return 1;
            }
            if (fromCol == toCol) {
                int step = (rowDiff > 0) ? 1 : -1;
                for (int r = fromRow + step; r != toRow; r += step)
                    if (board[r][fromCol].type != 0) return 0;
                return 1;
            }
            break;
        }
        case 'N': {
            if ((abs(rowDiff) == 2 && abs(colDiff) == 1) || (abs(rowDiff) == 1 && abs(colDiff) == 2)) {
                return 1;
            }
            break;
        }
        case 'B': {
            if (abs(rowDiff) == abs(colDiff)) {
                int rStep = (rowDiff > 0) ? 1 : -1;
                int cStep = (colDiff > 0) ? 1 : -1;
                int r = fromRow + rStep, c = fromCol + cStep;
                while (r != toRow && c != toCol) {
                    if (board[r][c].type != 0) return 0;
                    r += rStep;
                    c += cStep;
                }
                return 1;
            }
            break;
        }
        case 'Q': {
            // Queen = Rook + Bishop
            if (fromRow == toRow || fromCol == toCol) {
                Piece temp = piece;
                temp.type = 'R';
                return isMoveValid(temp, fromRow, fromCol, toRow, toCol);
            }
            if (abs(rowDiff) == abs(colDiff)) {
                Piece temp = piece;
                temp.type = 'B';
                return isMoveValid(temp, fromRow, fromCol, toRow, toCol);
            }
            break;
        }
        case 'K': {
            if (abs(rowDiff) <= 1 && abs(colDiff) <= 1) {
                return 1;
            }
            // Castling is Step 3
            break;
        }
    }

    return 0;
}


int main() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *window = SDL_CreateWindow("SDL Chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    drawBoard(renderer);

    int running = 1;
    SDL_Event e;
    int selectedRow = -1, selectedCol = -1;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int col = e.button.x / TILE_SIZE;
                int row = e.button.y / TILE_SIZE;

                if (selectedRow == -1) {
                    // Select only if it's that player's turn
                    if (board[row][col].type != 0 && board[row][col].color == currentTurn) {
                        selectedRow = row;
                        selectedCol = col;
                    }
                } else {
                    Piece selectedPiece = board[selectedRow][selectedCol];
                    if (isMoveValid(selectedPiece, selectedRow, selectedCol, row, col)) {
                        board[row][col] = selectedPiece;
                        board[row][col].hasMoved = 1;
                        board[selectedRow][selectedCol] = (Piece){0, 0, 0};
                        currentTurn = (currentTurn == 'w') ? 'b' : 'w';
                        drawBoard(renderer);
                    }
                    selectedRow = selectedCol = -1;
                }
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
