#include <SDL/SDL.h>
#include <stdbool.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>

#define WIDTH 640
#define HEIGHT 480
#define CELL_SIZE (HEIGHT / 3)

// Enum for the game state
typedef enum { EMPTY, PLAYER_X, PLAYER_O } Cell;

// Function prototypes
void draw_board(SDL_Surface *screen, Cell board[3][3]);
bool check_winner(Cell board[3][3], Cell player);
bool is_board_full(Cell board[3][3]);

int cursor_row=0, cursor_col=0;
Cell board[3][3];
Cell current_player;
char winner;

void setPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
    // Make sure the coordinates are within the surface bounds
    if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
        // Lock surface if necessary
        if (SDL_MUSTLOCK(surface)) {
            SDL_LockSurface(surface);
        }

        // Get a pointer to the pixel data
        Uint32 *pixels = (Uint32 *)surface->pixels;
        // Calculate the pixel position and set the color
        pixels[(y * surface->w) + x] = color;

        // Unlock surface if necessary
        if (SDL_MUSTLOCK(surface)) {
            SDL_UnlockSurface(surface);
        }
    }
}

void drawCursor(SDL_Surface *surface, int row, int col) {
    int x_start = col * CELL_SIZE;
    int y_start = row * CELL_SIZE;

    Uint32 green = SDL_MapRGB(surface->format, 0, 255, 0);  // Green color for the cursor

    // Top border
    SDL_Rect top = { x_start, y_start, CELL_SIZE, 5 };
    SDL_FillRect(surface, &top, green);

    // Bottom border
    SDL_Rect bottom = { x_start, y_start + CELL_SIZE - 5, CELL_SIZE, 5 };
    SDL_FillRect(surface, &bottom, green);

    // Left border
    SDL_Rect left = { x_start, y_start, 5, CELL_SIZE };
    SDL_FillRect(surface, &left, green);

    // Right border
    SDL_Rect right = { x_start + CELL_SIZE - 5, y_start, 5, CELL_SIZE };
    SDL_FillRect(surface, &right, green);
}

void drawCircle(SDL_Surface *surface, int32_t centreX, int32_t centreY, int32_t radius, Uint32 color) {
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y) {
        // Each of the following renders an octant of the circle
        setPixel(surface, centreX + x, centreY - y, color);
        setPixel(surface, centreX + x, centreY + y, color);
        setPixel(surface, centreX - x, centreY - y, color);
        setPixel(surface, centreX - x, centreY + y, color);
        setPixel(surface, centreX + y, centreY - x, color);
        setPixel(surface, centreX + y, centreY + x, color);
        setPixel(surface, centreX - y, centreY - x, color);
        setPixel(surface, centreX - y, centreY + x, color);

        if (error <= 0) {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0) {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void drawX(SDL_Surface *surface, int row, int col) {
    int x_start = col * CELL_SIZE;
    int y_start = row * CELL_SIZE;
    int y_end = y_start + CELL_SIZE;
    Uint32 red = SDL_MapRGB(surface->format, 255, 0, 0); // Red color for X

    // Draw two diagonal lines to form an X
    for (int i = 10; i < CELL_SIZE - 10; i++) {
        // Top-left to bottom-right diagonal
        setPixel(surface, x_start + i, y_start + i, red);

        // Bottom-left to top-right diagonal
        setPixel(surface, x_start + i, y_end - i - 1, red);
    }
}

void draw_X_at(SDL_Surface *surface, int x, int y, int size) {
    int x_start = x; // Use provided x position
    int y_start = y; // Use provided y position
    int y_end = y_start + size; // Calculate the end y position
    Uint32 red = SDL_MapRGB(surface->format, 255, 0, 0); // Red color for X

    // Draw two diagonal lines to form an X
    for (int i = 0; i < size; i++) {
        // Top-left to bottom-right diagonal
        setPixel(surface, x_start + i, y_start + i, red);

        // Bottom-left to top-right diagonal
        setPixel(surface, x_start + i, y_end - i - 1, red);
    }
}

void drawW(SDL_Surface *screen, int x, int y) {
    // Set color for the W (black)
    Uint32 black = SDL_MapRGB(screen->format, 0, 0, 0);
    
    // Define the points for the W shape
    int height = 30; // Height of the W

    for (int i = 0; i <= height; i++) {
        setPixel(screen, x + i * 5 / height, y + i, black);
        setPixel(screen, x + 5 + i * 5 / height, y + height - i, black);
        setPixel(screen, x + 10 + i * 5 / height, y + i, black);
        setPixel(screen, x + 15 + i * 5 / height, y + height - i, black);
    }
}

void drawN(SDL_Surface *screen, int x, int y) {
    // Set color for the N (black)
    Uint32 black = SDL_MapRGB(screen->format, 0, 0, 0);
    
    // Define the points for the N shape
    int height = 30; // Height of the N
    int width = 20;  // Width of the N

    // Draw the lines to form the N
    // Left vertical line
    for (int i = 0; i < height; i++) {
        setPixel(screen, x, y + i, black);
    }
    
    // Diagonal line
    for (int i = 0; i <= height; i++) {
        setPixel(screen, x + i * width / height, y + i, black); // Draw diagonal
    }
    
    // Right vertical line
    for (int i = 0; i < height; i++) {
        setPixel(screen, x + width, y + i, black);
    }
}

void drawT(SDL_Surface *screen, int x, int y) {
    // Set color for the T (black)
    Uint32 black = SDL_MapRGB(screen->format, 0, 0, 0);
    
    // Define the points for the T shape
    int width = 20;  // Width of the T
    int height = 30; // Height of the T

    // Draw the horizontal line (top of the T)
    SDL_Rect line1 = { x, y, width, 5 }; // Top horizontal line
    SDL_FillRect(screen, &line1, black);

    // Draw the vertical line (stem of the T)
    SDL_Rect line2 = { x + width / 2, y, 5, height }; // Vertical line
    SDL_FillRect(screen, &line2, black);
}

void drawI(SDL_Surface *screen, int x, int y) {
    drawT(screen, x, y);
    SDL_Rect line = { x, y + 30, 20, 5 };
    SDL_FillRect(screen, &line, SDL_MapRGB(screen->format, 0, 0, 0)); // Horizontal line for I
}

void drawE(SDL_Surface *screen, int x, int y) {
    // Define the dimensions for the letter E
    int width = 20;  // Width of the E
    int height = 30; // Height of the E

    // Draw the vertical line (stem of the E)
    SDL_Rect line1 = { x, y, 5, height }; // Vertical line
    SDL_FillRect(screen, &line1, SDL_MapRGB(screen->format, 0, 0, 0)); 

    // Draw the three horizontal lines (top, middle, bottom of the E)
    SDL_Rect line2 = { x, y, width, 5 }; // Top horizontal line
    SDL_FillRect(screen, &line2, SDL_MapRGB(screen->format, 0, 0, 0));

    SDL_Rect line3 = { x, y + height / 2, width, 5 }; // Middle horizontal line
    SDL_FillRect(screen, &line3, SDL_MapRGB(screen->format, 0, 0, 0));

    SDL_Rect line4 = { x, y + height, width, 5 }; // Bottom horizontal line
    SDL_FillRect(screen, &line4, SDL_MapRGB(screen->format, 0, 0, 0));
}

void initializeBoard() {
   current_player = PLAYER_X;  // Start with player X
   winner=0;
   for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
         board[i][j]=EMPTY;
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);  // Initialize video subsystem
    SDL_Surface *screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE);
    if (!screen) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }
    SDL_ShowCursor(SDL_DISABLE);

    bool moved, movedBefore=false;
    initializeBoard();
    while (true) {
        maple_device_t *controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
        if (controller) {
            cont_state_t *state = (cont_state_t *)maple_dev_status(controller);
            moved=false;
            if (state) {
                if (state->buttons & CONT_DPAD_UP) {
                    moved=true;
                    if(!movedBefore)
                        cursor_row = (cursor_row > 0) ? cursor_row - 1 : 2;
                }
                else if (state->buttons & CONT_DPAD_DOWN) {
                    moved=true;
                    if(!movedBefore)
                        cursor_row = (cursor_row < 2) ? cursor_row + 1 : 0;
                }
                if (state->buttons & CONT_DPAD_LEFT) {
                    moved=true;
                    if(!movedBefore)
                        cursor_col = (cursor_col > 0) ? cursor_col - 1 : 2;
                }
                else if (state->buttons & CONT_DPAD_RIGHT) {
                    moved=true;
                    if(!movedBefore)
                        cursor_col = (cursor_col < 2) ? cursor_col + 1 : 0;
                }
                movedBefore=moved;

                if (state->buttons & CONT_A && !winner) {
                    if (board[cursor_row][cursor_col] == EMPTY) {
                        board[cursor_row][cursor_col] = current_player;

                        // Check for a winner or draw
                        if (check_winner(board, current_player)) {
                            winner = current_player==PLAYER_X?'X':'O';
                        } else if (is_board_full(board)) {
                            winner = 'D';
                        }

                        // Switch players
                        current_player = (current_player == PLAYER_X) ? PLAYER_O : PLAYER_X;
                    }
                }
                if(state->buttons & CONT_START && winner) {
                    initializeBoard();
                }
            }
        }

        // Draw the board and the cursor
        draw_board(screen, board);
        drawCursor(screen, cursor_row, cursor_col);
        SDL_Flip(screen);
    }

    SDL_Quit();
    return 0;
}

// Function to draw the game board
void draw_board(SDL_Surface *screen, Cell board[3][3]) {
    // Clear the screen
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));

    // Draw the grid
    for (int i = 1; i < 3; i++) {
        SDL_Rect line = { i * CELL_SIZE, 0, 5, HEIGHT };
        SDL_FillRect(screen, &line, SDL_MapRGB(screen->format, 0, 0, 0)); // Vertical lines
        line = (SDL_Rect){ 0, i * CELL_SIZE, HEIGHT, 5 };
        SDL_FillRect(screen, &line, SDL_MapRGB(screen->format, 0, 0, 0)); // Horizontal lines
    }

    drawCursor(screen, cursor_row, cursor_col);
    // Draw the X and O marks
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (board[y][x] == PLAYER_X) {
               drawX(screen, y, x);
            } else if (board[y][x] == PLAYER_O) {
               int centreX = x * CELL_SIZE + CELL_SIZE / 2;
               int centreY = y * CELL_SIZE + CELL_SIZE / 2;
               int radius = (CELL_SIZE - 20) / 2; // Adjust the radius to fit in the cell

               // Call drawCircle to draw 'O' in the correct spot
               Uint32 blue = SDL_MapRGB(screen->format, 0, 0, 255); // Blue color for O
               drawCircle(screen, centreX, centreY, radius, blue);
            }
        }
    }
    if(winner) {
        if(winner=='D') {
            drawT(screen, 490, 0);
            drawI(screen, 520, 0);
            drawE(screen, 550, 0);
            return;
        }
        if(winner=='X')
            draw_X_at(screen, 480, 0, 30);
        else
            drawCircle(screen, 495, 15, 15, SDL_MapRGB(screen->format, 0, 0, 255));
        drawW(screen, 540, 0);
        drawCircle(screen, 585, 15, 15, SDL_MapRGB(screen->format, 0, 0, 255));
        drawN(screen, 610, 0);
    }
}

// Function to check if a player has won
bool check_winner(Cell board[3][3], Cell player) {
    for (int i = 0; i < 3; i++) {
        // Check rows and columns
        if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
            (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
            return true;
        }
    }
    // Check diagonals
    return (board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
           (board[0][2] == player && board[1][1] == player && board[2][0] == player);
}

// Function to check if the board is full
bool is_board_full(Cell board[3][3]) {
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (board[y][x] == EMPTY) {
                return false;
            }
        }
    }
    return true;
}
