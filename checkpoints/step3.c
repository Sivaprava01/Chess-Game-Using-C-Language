#undef main
#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640
#define TILE_SIZE 80

// ------------------ STRUCT DEFINITIONS ------------------
typedef struct {
    char type;   // 'P', 'R', 'N', 'B', 'Q', 'K'
    char color;  // 'w' or 'b'
    int hasMoved;
} Piece;

typedef struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    Piece movedPiece;
    Piece capturedPiece;
    struct Move* next;
} Move;

// Stack for move history
typedef struct StackNode {
    Move move;
    struct StackNode* next;
} StackNode;

// Linked list for captured pieces
typedef struct CapturedNode {
    Piece piece;
    struct CapturedNode* next;
} CapturedNode;

// Queue for move suggestions (optional, minimal)
typedef struct QueueNode {
    Move move;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
} MoveQueue;

// ------------------ GLOBALS ------------------
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

char currentTurn = 'w';
StackNode* moveStack = NULL;
CapturedNode* capturedHead = NULL;
MoveQueue suggestionQueue = {NULL, NULL};

// ------------------ UTILS ------------------
void pushMove(Move move) {
    StackNode* node = malloc(sizeof(StackNode));
    node->move = move;
    node->next = moveStack;
    moveStack = node;
}

void addCapturedPiece(Piece piece) {
    CapturedNode* node = malloc(sizeof(CapturedNode));
    node->piece = piece;
    node->next = capturedHead;
    capturedHead = node;
}

void enqueueMove(Move move) {
    QueueNode* node = malloc(sizeof(QueueNode));
    node->move = move;
    node->next = NULL;
    if (suggestionQueue.rear) suggestionQueue.rear->next = node;
    else suggestionQueue.front = node;
    suggestionQueue.rear = node;
}

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
    if (dest.type != 0 && dest.color == piece.color) return 0;

    int rowDiff = toRow - fromRow;
    int colDiff = toCol - fromCol;
    int absRow = abs(rowDiff);
    int absCol = abs(colDiff);

    switch (piece.type) {
        case 'P': {
            int dir = (piece.color == 'w') ? -1 : 1;
            if (fromCol == toCol && dest.type == 0) {
                if (toRow == fromRow + dir) return 1;
                if (!piece.hasMoved && toRow == fromRow + 2 * dir &&
                    board[fromRow + dir][fromCol].type == 0) return 1;
            }
            if (absCol == 1 && toRow == fromRow + dir && dest.type != 0 && dest.color != piece.color)
                return 1;
            return 0;
        }
        case 'R': {
            if (fromRow == toRow) {
                int step = (toCol > fromCol) ? 1 : -1;
                for (int c = fromCol + step; c != toCol; c += step)
                    if (board[fromRow][c].type != 0) return 0;
                return 1;
            }
            if (fromCol == toCol) {
                int step = (toRow > fromRow) ? 1 : -1;
                for (int r = fromRow + step; r != toRow; r += step)
                    if (board[r][fromCol].type != 0) return 0;
                return 1;
            }
            return 0;
        }
        case 'B': {
            if (absRow == absCol) {
                int rowStep = (rowDiff > 0) ? 1 : -1;
                int colStep = (colDiff > 0) ? 1 : -1;
                for (int i = 1; i < absRow; i++) {
                    if (board[fromRow + i * rowStep][fromCol + i * colStep].type != 0) return 0;
                }
                return 1;
            }
            return 0;
        }
        case 'Q': {
            if (fromRow == toRow || fromCol == toCol) {
                return isMoveValid((Piece){'R', piece.color, piece.hasMoved}, fromRow, fromCol, toRow, toCol);
            }
            if (absRow == absCol) {
                return isMoveValid((Piece){'B', piece.color, piece.hasMoved}, fromRow, fromCol, toRow, toCol);
            }
            return 0;
        }
        case 'N': {
            return (absRow == 2 && absCol == 1) || (absRow == 1 && absCol == 2);
        }
        case 'K': {
            return absRow <= 1 && absCol <= 1;
        }
    }

    return 0;
}

int isKingInCheck(char color) {
    // Find the king of the specified color
    int kingRow = -1, kingCol = -1;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j].type == 'K' && board[i][j].color == color) {
                kingRow = i;
                kingCol = j;
                break;
            }
        }
    }

    // Check if any opponent piece can attack the king
    char opponentColor = (color == 'w') ? 'b' : 'w';
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j].color == opponentColor) {
                if (isMoveValid(board[i][j], i, j, kingRow, kingCol)) {
                    return 1; // King is in check
                }
            }
        }
    }
    return 0; // King is not in check
}




int isCheckmate(char color) {
    if (!isKingInCheck(color)) return 0;

    for (int fromRow = 0; fromRow < 8; ++fromRow) {
        for (int fromCol = 0; fromCol < 8; ++fromCol) {
            if (board[fromRow][fromCol].type != 0 && board[fromRow][fromCol].color == color) {
                for (int toRow = 0; toRow < 8; ++toRow) {
                    for (int toCol = 0; toCol < 8; ++toCol) {
                        if (isMoveValid(board[fromRow][fromCol], fromRow, fromCol, toRow, toCol)) {
                            // Save original pieces
                            Piece moved = board[fromRow][fromCol];
                            Piece captured = board[toRow][toCol];

                            // Simulate move
                            board[toRow][toCol] = moved;
                            board[fromRow][fromCol] = (Piece){0, 0, 0};

                            int stillInCheck = isKingInCheck(color);

                            // Undo move
                            board[fromRow][fromCol] = moved;
                            board[toRow][toCol] = captured;

                            if (!stillInCheck) return 0; // Escape from check exists
                        }
                    }
                }
            }
        }
    }

    return 1; // No valid move to escape check
}


int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *window = SDL_CreateWindow("SDL Chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    drawBoard(renderer);

    int running = 1;
    SDL_Event e;
    int selectedRow = -1, selectedCol = -1;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;

            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int col = e.button.x / TILE_SIZE;
                int row = e.button.y / TILE_SIZE;

                if (selectedRow == -1)
                {
                    if (board[row][col].type != 0 && board[row][col].color == currentTurn)
                    {
                        selectedRow = row;
                        selectedCol = col;
                    }
                }
                else
                {
                    Piece selectedPiece = board[selectedRow][selectedCol];
                    if (isMoveValid(selectedPiece, selectedRow, selectedCol, row, col))
                    {
                        Move move = {selectedRow, selectedCol, row, col, selectedPiece, board[row][col]};
                        
                        // Simulate the move
                        board[row][col] = selectedPiece;
                        board[row][col].hasMoved = 1;
                        board[selectedRow][selectedCol] = (Piece){0, 0, 0};

                        // Check if move leaves king in check
                        if (isKingInCheck(currentTurn))
                        {
                            // Undo move if invalid due to check
                            board[selectedRow][selectedCol] = selectedPiece;
                            board[row][col] = move.capturedPiece;
                        }
                        else
                        {
                            // Valid move, apply changes
                            if (move.capturedPiece.type != 0)
                                addCapturedPiece(move.capturedPiece);
                            pushMove(move);
                            currentTurn = (currentTurn == 'w') ? 'b' : 'w';

                            // Check for checkmate
                            if (isCheckmate(currentTurn))
                            {
                                printf("Checkmate! %s wins.\n", (currentTurn == 'w') ? "Black" : "White");
                                running = 0; // Stop the game
                            }
                        }

                        selectedRow = -1;
                        selectedCol = -1;
                        drawBoard(renderer);
                    }
                    else
                    {
                        // Invalid move, deselect
                        selectedRow = -1;
                        selectedCol = -1;
                    }
                }
            }
        }

        SDL_Delay(16); // Add a small delay to reduce CPU usage
    }

    // Cleanup (must be inside main)
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}


