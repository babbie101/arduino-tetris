#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// ===== SCREEN (ST7735 1.8" SPI TFT) =====
#define TFT_CS    A3
#define TFT_RST   A5
#define TFT_DC    A4
#define TFT_MOSI  A6
#define TFT_SCLK  A7

// Joystick Pins
#define JOY_F 5
#define JOY_B 6
#define JOY_L 7
#define JOY_R 8
#define JOY_M 9

const int CELL = 8;
const int COLS = 10;
const int ROWS = 14;
const int BOARD_X = 10;
const int BOARD_Y = 8;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// ===== SHAPE OFFSET DEFINITIONS =====
int O_PIECE[4][2] = { {0,0}, {1,0}, {0,1}, {1,1} };
int I_PIECE[4][2] = { {0,0}, {0,1}, {0,2}, {0,3} };

int (*ALL_PIECES[2])[2] = { O_PIECE, I_PIECE };
uint16_t PIECE_COLORS[2] = { ST77XX_YELLOW, ST77XX_CYAN };

// Active piece state
int currentPiece[4][2];
uint16_t currentPieceColor = ST77XX_YELLOW;
int pieceCol = 4;
int pieceRow = -1;

// Timing & Board State
unsigned long lastDropTime = 0;
int dropInterval = 500;
int board[ROWS][COLS] = {0};

void drawCell(int col, int row, uint16_t color)
{
  if (row < 0) return;
  int pixelX = BOARD_X + col * CELL;
  int pixelY = BOARD_Y + row * CELL;
  tft.fillRect(pixelX, pixelY, CELL, CELL, color);
}

void drawBoardBorder()
{
  tft.drawRect(BOARD_X, BOARD_Y, COLS * CELL, ROWS * CELL, ST77XX_BLUE);
}

void redrawFullBoard()
{
  tft.fillScreen(ST77XX_BLACK);
  drawBoardBorder();

  for (int r = 0; r < ROWS; r++)
  {
    for (int c = 0; c < COLS; c++)
    {
      if (board[r][c] == 1)
      {
        drawCell(c, r, ST77XX_WHITE);
      }
    }
  }
}

void drawPiece(int col, int row, int shape[4][2], uint16_t color)
{
  for (int i = 0; i < 4; i++)
  {
    drawCell(col + shape[i][0], row + shape[i][1], color);
  }
}

void lockPiece(int col, int row, int shape[4][2])
{
  for (int i = 0; i < 4; i++)
  {
    int c = col + shape[i][0];
    int r = row + shape[i][1];
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
    {
      board[r][c] = 1;
    }
  }
}

bool canMoveTo(int newCol, int newRow, int shape[4][2])
{
  for (int i = 0; i < 4; i++)
  {
    int targetCol = newCol + shape[i][0];
    int targetRow = newRow + shape[i][1];

    if (targetRow >= ROWS) return false;   // Floor
    if (targetCol < 0) return false;      // Left wall
    if (targetCol >= COLS) return false;  // Right wall

    if (targetRow >= 0 && board[targetRow][targetCol] == 1) return false;
  }
  return true;
}

void rotatePieceClockwise(int src[4][2], int dest[4][2])
{
  for (int i = 0; i < 4; i++)
  {
    int x = src[i][0];
    int y = src[i][1];
    dest[i][0] = -y;
    dest[i][1] =  x;
  }
}

void checkAndClearLines()
{
  for (int r = ROWS - 1; r >= 0; r--)
  {
    bool fullRow = true;

    for (int c = 0; c < COLS; c++)
    {
      if (board[r][c] == 0)
      {
        fullRow = false;
        break;
      }
    }

    if (fullRow)
    {
      // Shift rows above down by 1
      for (int y = r; y > 0; y--)
      {
        for (int c = 0; c < COLS; c++)
        {
          board[y][c] = board[y - 1][c];
        }
      }

      // Empty top row
      for (int c = 0; c < COLS; c++)
      {
        board[0][c] = 0;
      }

      r++; // Re-check this row index
      redrawFullBoard();
    }
  }
}

void spawnNewPiece()
{
  int randomIndex = random(0, 2);
  currentPieceColor = PIECE_COLORS[randomIndex];
  
  for (int i = 0; i < 4; i++) {
    currentPiece[i][0] = ALL_PIECES[randomIndex][i][0];
    currentPiece[i][1] = ALL_PIECES[randomIndex][i][1];
  }

  pieceCol = 4;
  pieceRow = -1;
}

void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(A0));

  pinMode(JOY_F, INPUT_PULLUP);
  pinMode(JOY_L, INPUT_PULLUP);
  pinMode(JOY_R, INPUT_PULLUP);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  drawBoardBorder();
  spawnNewPiece();
  drawPiece(pieceCol, pieceRow, currentPiece, currentPieceColor);
}

void loop()
{
  // 1. Read Forward Joystick (Rotate)
  if (digitalRead(JOY_F) == LOW)
  {
    int tempPiece[4][2];
    rotatePieceClockwise(currentPiece, tempPiece);

    if (canMoveTo(pieceCol, pieceRow, tempPiece))
    {
      drawPiece(pieceCol, pieceRow, currentPiece, ST77XX_BLACK);
      
      for (int i = 0; i < 4; i++) {
        currentPiece[i][0] = tempPiece[i][0];
        currentPiece[i][1] = tempPiece[i][1];
      }

      drawPiece(pieceCol, pieceRow, currentPiece, currentPieceColor);
      drawBoardBorder();
      delay(200);
    }
  }

  // 2. Read Left Input
  if (digitalRead(JOY_L) == LOW)
  {
    if (canMoveTo(pieceCol - 1, pieceRow, currentPiece))
    {
      drawPiece(pieceCol, pieceRow, currentPiece, ST77XX_BLACK);
      pieceCol = pieceCol - 1;
      drawPiece(pieceCol, pieceRow, currentPiece, currentPieceColor);
      drawBoardBorder();
      delay(150);
    }
  }

  // 3. Read Right Input
  if (digitalRead(JOY_R) == LOW)
  {
    if (canMoveTo(pieceCol + 1, pieceRow, currentPiece))
    {
      drawPiece(pieceCol, pieceRow, currentPiece, ST77XX_BLACK);
      pieceCol = pieceCol + 1;
      drawPiece(pieceCol, pieceRow, currentPiece, currentPieceColor);
      drawBoardBorder();
      delay(150);
    }
  }

  // 4. Gravity Tick
  unsigned long currentTime = millis();
  if (currentTime - lastDropTime >= dropInterval)
  {
    lastDropTime = currentTime;

    if (canMoveTo(pieceCol, pieceRow + 1, currentPiece))
    {
      drawPiece(pieceCol, pieceRow, currentPiece, ST77XX_BLACK);
      pieceRow = pieceRow + 1;
      drawPiece(pieceCol, pieceRow, currentPiece, currentPieceColor);
      drawBoardBorder();
    }
    else
    {
      lockPiece(pieceCol, pieceRow, currentPiece);
      checkAndClearLines(); // Scan & clear full rows
      spawnNewPiece();

      if (!canMoveTo(pieceCol, pieceRow, currentPiece))
      {
        for (int r = 0; r < ROWS; r++)
          for (int c = 0; c < COLS; c++)
            board[r][c] = 0;

        tft.fillScreen(ST77XX_BLACK);
        drawBoardBorder();
      }

      drawPiece(pieceCol, pieceRow, currentPiece, currentPieceColor);
      drawBoardBorder();
    }
  }
}