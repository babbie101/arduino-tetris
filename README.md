# Arduino ST7735 Tetris

Step-by-step modular implementation of Tetris built in C++ for Arduino using an ST7735 1.8" SPI TFT Display.

## Hardware & Connection Points
- **Display:** 1.8" ST7735 SPI TFT (`INITR_BLACKTAB`, Rotation 3)
- **CS:** Pin A3
- **RST:** Pin A5
- **DC:** Pin A4
- **MOSI:** Pin A6
- **SCLK:** Pin A7

## Current Features
- Logical 10x14 cell board grid mapped to pixel space.
- Non-blocking tick timer using `millis()` for gravity movement.
- Basic O-piece rendering and boundary collision checking (`canMoveTo`).
- 2D memory array (`board[14][10]`) initialized for tracking locked blocks.
