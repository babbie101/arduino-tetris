#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// ===== SCREEN (ST7735 1.8" SPI TFT) =====
#define TFT_CS    A3
#define TFT_RST   A5
#define TFT_DC    A4
#define TFT_MOSI  A6
#define TFT_SCLK  A7

const int CELL = 8;
const int COLS = 10;
const int ROWS = 14;
const int BOARD_X = 10;
const int BOARD_Y = 8;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Active piece coordinates
int pieceCol = 4;
int pieceRow = 1;

// Timing & Board State
unsigned long lastDropTime = 0;
int dropInterval = 500;
int board[ROWS][COLS] = {0}; // Grid map: 0 = empty, 1 = occupied

void drawCell(int col, int row, uint16_t color)
{
  int pixelX = BOARD_X + col * CELL;
  int pixelY = BOARD_Y + row * CELL;
  tft.fillRect(pixelX, pixelY, CELL, CELL, color);
}

void drawOPiece(int col, int row, uint16_t color)
{
  drawCell(col,     row,     color);
  drawCell(col + 1, row,     color);
  drawCell(col,     row + 1, color);
  drawCell(col + 1, row + 1, color);
}

bool canMoveTo(int newCol, int newRow)
{
  if (newRow + 2 > ROWS) return false;
  if (newCol < 0) return false;
  if (newCol + 2 > COLS) return false;
  return true;
}

void setup()
{
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  tft.drawRect(BOARD_X, BOARD_Y, COLS * CELL, ROWS * CELL, ST77XX_BLUE);
  drawOPiece(pieceCol, pieceRow, ST77XX_BLUE);
}

void loop()
{
  unsigned long currentTime = millis();

  if (currentTime - lastDropTime >= dropInterval)
  {
    lastDropTime = currentTime;

    if (canMoveTo(pieceCol, pieceRow + 1))
    {
      drawOPiece(pieceCol, pieceRow, ST77XX_BLACK);
      pieceRow = pieceRow + 1;
      drawOPiece(pieceCol, pieceRow, ST77XX_BLUE);
    }
  }
}