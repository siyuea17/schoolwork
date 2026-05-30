# Link Game (连连看) — Code Documentation

## Project Overview

A classic tile-matching puzzle game built with **Qt 6.11.1** and **C++17**. The player clicks pairs of identical tiles that can be connected by a path with at most 2 turns. When all 80 tiles are eliminated, the player wins.

| Item | Value |
|---|---|
| Framework | Qt 6.11.1 (modules: core, gui, widgets) |
| Language | C++17 |
| Compiler | MSVC 2022 (v143) |
| Build System | Visual Studio 2022 + Qt VS Tools |
| Grid Size | 8 rows × 10 columns (80 tiles) |
| Tile Types | 20 types, 4 of each |

---

## File Structure

```
LinkGame/
├── main.cpp              # Application entry point
├── MainWindow.h          # Main window class declaration
├── MainWindow.cpp        # Main window: toolbar, status bar, timer, signal/slot wiring
├── MainWindow.ui         # Qt Designer UI file: toolbar actions, window layout
├── resources.qrc         # Qt resource file: registers all 20 tile PNG images
├── GameBoard.h           # Game logic class declaration
├── GameBoard.cpp         # Board initialization, path-finding algorithm, shuffle, hint
├── GameWidget.h          # Custom game widget class declaration (QWidget subclass)
├── GameWidget.cpp        # Tile rendering, mouse input, match animation
├── images/               # 20 PNG tile images (128×128 pixels each)
│   ├── tile_01.png
│   ├── ...
│   └── tile_20.png
├── LinkGame.vcxproj      # Visual Studio project file
└── LinkGame.vcxproj.filters
```

---

## Class Descriptions

### 1. `GameBoard` — Game Logic (Pure C++, no Qt meta-object)

**File:** `GameBoard.h` / `GameBoard.cpp`

This is the **model** layer. It manages all game state and rules without any UI dependency.

**Key Members:**

| Member | Type | Description |
|---|---|---|
| `m_grid[10][12]` | `int` | Internal board. Outer border (row 0, row 9, col 0, col 11) is always empty. Tiles occupy rows 1–8, cols 1–10. Value 0 = empty, 1–20 = tile type. |
| `m_score` | `int` | Current score (+10 per matched pair) |
| `m_remainingTiles` | `int` | Number of tiles still on the board |
| `m_moves` | `int` | Number of successful matches made |

**Key Methods:**

| Method | Description |
|---|---|
| `initBoard()` | Creates 80 tiles (4 per type), shuffles randomly, places on grid |
| `findPath(r1,c1,r2,c2)` | Core algorithm — tries to connect two tiles (see below) |
| `findHint()` | Scans entire board and returns the first valid pair |
| `hasValidMoves()` | Returns true if at least one matchable pair exists |
| `shuffle()` | Collects remaining tiles, shuffles them, places back |
| `removeTiles(r1,c1,r2,c2)` | Sets two positions to EMPTY, updates counters |

**Path-Finding Algorithm (`findPath`):**

The algorithm tries three connection types in order, returning the first valid one:

1. **Direct (0 turns):** Both tiles on the same row OR same column, and every cell between them is empty.
2. **1 turn (L-shape):** Tries two corner positions — (r1,c2) and (r2,c1). The corner cell must be empty, and both line segments must be clear.
3. **2 turns (Z/U-shape):**
   - Scans every row `r`: if cells (r,c1) and (r,c2) are both empty, checks if the 3-segment path through them is clear.
   - Scans every column `c`: if cells (r1,c) and (r2,c) are both empty, checks if the 3-segment path through them is clear.

The empty border (row 0, row 9, col 0, col 11) allows paths to wrap around the outside of the board.

**Return Value:** `PathInfo` struct containing:
```cpp
struct PathInfo {
    bool valid;              // true if a path was found
    QVector<QPoint> corners; // 2–4 corner points forming the path
};
```

---

### 2. `GameWidget` — Game Display & Interaction (QWidget subclass)

**File:** `GameWidget.h` / `GameWidget.cpp`

This is the **view+controller** layer. It owns a `GameBoard` instance and handles all rendering and user input.

**Key Members:**

| Member | Type | Description |
|---|---|---|
| `m_board` | `GameBoard` | The game logic instance |
| `m_tilePixmaps` | `QVector<QPixmap>` | 20 pre-loaded tile images from `resources.qrc` |
| `m_tileSize` | `double` | Computed pixel size of each square tile |
| `m_offsetX/Y` | `double` | Pixel offset to center the board |
| `m_hasSelection` | `bool` | Whether a tile is currently selected |
| `m_selectedRow/Col` | `int` | Grid position of selected tile |
| `m_isAnimating` | `bool` | Lock to prevent clicks during path animation |
| `m_animPath` | `PathInfo` | Current connection path being displayed |
| `m_hintTimer` | `QTimer*` | Timer for blinking hint highlights |

**Key Methods:**

| Method | Description |
|---|---|
| `paintEvent()` | Draws background grid → draws tiles from pixmaps → draws selection/hint highlights → draws connection path if animating |
| `mousePressEvent()` | Hit-tests grid cell → handles tile selection and match attempt logic |
| `loadTileImages()` | Loads all 20 PNGs from `:/tiles/images/tile_XX.png` into `m_tilePixmaps` |
| `computeLayout()` | Calculates tile size and offsets to center the board in the widget |
| `executeMatch(path)` | Locks input, draws path for 500ms via `QTimer::singleShot`, then calls `finishMatch()` |
| `finishMatch()` | Removes tiles from board, emits score/remaining/moves signals, checks win/no-moves |
| `showHint()` | Calls `m_board.findHint()`, blinks the pair for ~2.4 seconds |

**Signals Emitted:**
- `scoreChanged(int)` — after each successful match
- `tilesRemainingChanged(int)` — after tile removal
- `moveCountChanged(int)` — after each match
- `gameWon()` — when all 80 tiles are removed
- `noMovesLeft()` — when no valid pair remains

---

### 3. `MainWindow` — Application Window (QMainWindow subclass)

**File:** `MainWindow.h` / `MainWindow.cpp` / `MainWindow.ui`

This is the **top-level window**. The UI layout (toolbar, actions, status bar placeholder) is defined in `MainWindow.ui` via Qt Designer.

**`.ui` File Contents:**
- Window title: "Link Game", size: 880×700
- `mainToolBar` with three actions: **New Game**, **Hint**, **Shuffle**
- `centralWidget` placeholder (replaced by `GameWidget` in code)
- `statusBar` placeholder (labels added in code)

**Key Members:**

| Member | Type | Description |
|---|---|---|
| `m_gameWidget` | `GameWidget*` | The game board widget, set as central widget |
| `m_gameTimer` | `QTimer*` | 1-second interval for elapsed time |
| `m_elapsedSeconds` | `int` | Seconds since game started |
| `m_scoreLabel` | `QLabel*` | Status bar: "Score: 0" |
| `m_timerLabel` | `QLabel*` | Status bar: "Time: 0s" |
| `m_remainingLabel` | `QLabel*` | Status bar: "Remaining: 80" |
| `m_movesLabel` | `QLabel*` | Status bar: "Moves: 0" |

**Signal/Slot Wiring:**
```
Toolbar Actions (from .ui)          GameWidget Signals
    │                                     │
    ├─ newGameAction  ──→ onNewGame()     ├─ scoreChanged ──→ update status label
    ├─ hintAction     ──→ onHint()        ├─ tilesRemainingChanged ──→ update label
    └─ shuffleAction  ──→ onShuffle()     ├─ moveCountChanged ──→ update label
                                          ├─ gameWon ──→ show congratulations dialog
                                          └─ noMovesLeft ──→ show suggestion dialog
```

---

### 4. `main.cpp` — Entry Point

Standard Qt application bootstrap:

```cpp
QApplication app(argc, argv);
MainWindow window;
window.show();
return app.exec();
```

---

### 5. `resources.qrc` — Qt Resource File

Registers all 20 tile images so they are compiled into the executable. Images are accessed at runtime via paths like `:/tiles/images/tile_01.png`.

---

### 6. `images/` — Tile PNG Assets

20 pre-rendered PNG files (128×128 pixels each). Each tile has:
- A unique background color
- A white geometric shape (circle, square, triangle, star, heart, etc.)
- Gradient fill and highlight for a 3D appearance

The images are loaded once at startup by `GameWidget::loadTileImages()` and cached in `m_tilePixmaps`.

---

## Data Flow (Matching Two Tiles)

```
1. User clicks tile at (r1, c1)
   └─ mousePressEvent() → m_hasSelection = true, stores position

2. User clicks tile at (r2, c2) of same type
   └─ tryMatch(r2, c2) → m_board.findPath(r1, c1, r2, c2)

3. path.valid == true
   └─ executeMatch(path)
       ├─ m_isAnimating = true (blocks input)
       ├─ repaint() shows orange connection line
       └─ QTimer::singleShot(500ms) → finishMatch()

4. finishMatch()
   ├─ m_board.removeTiles() (grid updated, score +10)
   ├─ m_isAnimating = false
   ├─ Emit signals → status bar updates
   └─ checkGameState()
       ├─ isWin()  → emit gameWon()
       └─ !hasValidMoves() → emit noMovesLeft()
```

---

## Build Instructions

### Prerequisites
- Visual Studio 2022
- Qt 6.11.1 (MSVC 2022 x64)
- Qt VS Tools extension for Visual Studio

### Steps
1. Open `LinkGame.vcxproj` in Visual Studio 2022
2. Select **Debug | x64** configuration
3. Build → Build Solution (Ctrl+Shift+B)
4. Run → Start Debugging (F5)

### Command Line Build
```powershell
MSBuild.exe LinkGame.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
```
