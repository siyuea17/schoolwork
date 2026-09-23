#pragma once
// GameWidget.h — 游戏画布，负责显示棋盘、处理鼠标点击和动画

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPoint>
#include <QColor>
#include <QPixmap>
#include "GameBoard.h"

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    // ---- 构造与析构 ----
    explicit GameWidget(int rows = GameBoard::DEFAULT_ROWS,
                        int cols = GameBoard::DEFAULT_COLS,
                        int tileTypes = GameBoard::DEFAULT_TILE_TYPES,
                        int copiesPerType = GameBoard::DEFAULT_COPIES,
                        int iconScale = 100,
                        QWidget* parent = nullptr);
    ~GameWidget();

    // ---- 游戏操作 ----
    void startNewGame();
    void showHint();

    // ---- 设置 ----
    void setIconScale(int percent);
    int iconScale() const { return m_iconScale; }
    void setPaused(bool paused);              // 设置暂停/恢复状态
    bool isPaused() const { return m_isPaused; }  // 查询是否暂停

    // ---- 状态获取 ----
    int getScore() const { return m_board.getScore(); }
    int getMoves() const { return m_board.getMoves(); }
    int getComboCount() const { return m_board.getComboCount(); }
    int getRemainingTiles() const { return m_board.getRemainingTiles(); }
    int boardRows() const { return m_board.rows(); }
    int boardCols() const { return m_board.cols(); }
    int boardTileTypes() const { return m_board.tileTypes(); }
    int boardCopiesPerType() const { return m_board.copiesPerType(); }

    // ---- 存档 ----
    QVector<int> serializeBoard() const { return m_board.serializeGrid(); }
    void deserializeBoard(const QVector<int>& data);
    void setBoardScore(int s) { m_board.setScore(s); }
    void setBoardMoves(int m) { m_board.setMoves(m); }
    void setBoardRemainingTiles(int r) { m_board.setRemainingTiles(r); }
    void setBoardComboCount(int c) { m_board.setComboCount(c); }

    // ---- 布局与提示（MainWindow 需要调用） ----
    void computeLayout();
    void clearHintTimer();

signals:
    void scoreChanged(int newScore);
    void tilesRemainingChanged(int remaining);
    void moveCountChanged(int moves);
    void gameWon();
    void noMovesLeft();
    void comboCountChanged(int comboCount);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // ---- 绘制函数 ----
    void drawBackground(QPainter& painter);
    void drawTile(QPainter& painter, int row, int col, int type);
    void drawSelection(QPainter& painter, int row, int col);
    void drawConnectionPath(QPainter& painter);
    void drawHintHighlight(QPainter& painter);
    void drawShuffleMessage(QPainter& painter);
    void drawComboEffects(QPainter& painter);
    void drawPauseOverlay(QPainter& painter);   // 绘制暂停遮罩

    // ---- 坐标转换 ----
    QRectF tileRect(int row, int col) const;
    QPointF tileCenter(int row, int col) const;
    int hitTest(const QPoint& pos, int& outRow, int& outCol) const;

    // ---- 匹配逻辑 ----
    void tryMatch(int row, int col);
    void executeMatch(const PathInfo& path);
    void finishMatch();
    void checkGameState();
    void shuffleBoard();

    // ---- 资源加载 ----
    void loadTileImages();

private:
    // ---- 棋盘数据 ----
    GameBoard m_board;

    // ---- 方块图片 ----
    QVector<QPixmap> m_tilePixmaps;

    // ---- 设置 ----
    int m_iconScale;

    // ---- 布局参数 ----
    static constexpr int MARGIN = 15;
    double m_tileSize = 0;
    double m_offsetX = 0;
    double m_offsetY = 0;

    // ---- 选择状态 ----
    bool m_hasSelection = false;
    int m_selectedRow = 0;
    int m_selectedCol = 0;

    // ---- 动画状态 ----
    bool m_isAnimating = false;
    PathInfo m_animPath;
    QColor m_animColor;

    // ---- 提示状态 ----
    bool m_showingHint = false;
    int m_hintRow1 = 0, m_hintCol1 = 0;
    int m_hintRow2 = 0, m_hintCol2 = 0;
    QTimer* m_hintTimer = nullptr;
    int m_hintFlashCount = 0;

    // ---- 空闲自动提示 ----
    QTimer* m_idleTimer = nullptr;
    static constexpr int IDLE_HINT_DELAY = 10000;

    // ---- 自动重排 ----
    bool m_isShuffling = false;
    bool m_showingShuffleMsg = false;
    int m_shuffleMsgFrames = 0;

    // ---- 连击特效 ----
    struct ComboEffect {
        int comboCount = 0;
        int remainingFrames = 0;
        QPointF startPos;
    };
    QVector<ComboEffect> m_comboEffects;
    static constexpr int COMBO_FLOAT_FRAMES = 30;
    QTimer* m_comboTimer = nullptr;

    // ---- 暂停状态 ----
    bool m_isPaused = false;         // true = 游戏已暂停（绘制遮罩 + 忽略点击）
};
