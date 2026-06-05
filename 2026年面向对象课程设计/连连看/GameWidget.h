#pragma once
// ============================================================================
// 文件：GameWidget.h
// 角色：连连看的"画布"——负责显示棋盘、处理点击、播放动画
//
// 本次更新：
//   1. 支持动态棋盘尺寸（通过构造函数参数）
//   2. 支持图标缩放（settings.iconScale 控制）
//   3. 连击分显示——消除时显示"COMBO x3!"等特效文字
//   4. 新增 comboCountChanged 信号
// ============================================================================

#include <QWidget>      // QWidget —— 所有界面控件的基类，GameWidget 继承它成为游戏画布
#include <QTimer>        // QTimer —— 定时器，用于提示闪烁、空闲自动提示、连击动画
#include <QVector>       // QVector —— Qt 动态数组，用于存储图片缓存和连击特效列表
#include <QPoint>        // QPoint —— 二维整数坐标，用于方格行列定位
#include <QColor>        // QColor —— 颜色类，用于绘制高亮、连线、特效等
#include <QPixmap>       // QPixmap —— 位图图片类，用于加载和显示方块图案
#include "GameBoard.h"   // GameBoard —— 棋盘逻辑（模型层），GameWidget 拥有一个 GameBoard 实例

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    // ======================== 构造与析构 ========================
    // rows/cols — 游戏区域行/列数
    // tileTypes — 图案种类数
    // iconScale — 图标缩放百分比（80=小, 100=正常, 120=大）
    explicit GameWidget(int rows = GameBoard::DEFAULT_ROWS,
                        int cols = GameBoard::DEFAULT_COLS,
                        int tileTypes = GameBoard::DEFAULT_TILE_TYPES,
                        int copiesPerType = GameBoard::DEFAULT_COPIES,
                        int iconScale = 100,
                        QWidget* parent = nullptr);

    ~GameWidget();

    // ======================== 游戏操作 ========================

    void startNewGame();
    void showHint();

    // ======================== 设置 ========================

    void setIconScale(int percent);
    int iconScale() const { return m_iconScale; }

    // ======================== 状态获取 ========================

    int getScore() const { return m_board.getScore(); }
    int getMoves() const { return m_board.getMoves(); }
    int getComboCount() const { return m_board.getComboCount(); }
    int getRemainingTiles() const { return m_board.getRemainingTiles(); }

    // 棋盘参数（供 MainWindow 存档用）
    int boardRows() const { return m_board.rows(); }
    int boardCols() const { return m_board.cols(); }
    int boardTileTypes() const { return m_board.tileTypes(); }
    int boardCopiesPerType() const { return m_board.copiesPerType(); }

    // 导出/导入棋盘数据（存档用）
    QVector<int> serializeBoard() const { return m_board.serializeGrid(); }
    void deserializeBoard(const QVector<int>& data);
    void setBoardScore(int s) { m_board.setScore(s); }
    void setBoardMoves(int m) { m_board.setMoves(m); }
    void setBoardRemainingTiles(int r) { m_board.setRemainingTiles(r); }
    void setBoardComboCount(int c) { m_board.setComboCount(c); }

    // 公开的布局方法和提示清除（MainWindow 需要调用）
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
    // ======================== 布局计算 ========================

    // ======================== 绘制函数 ========================
    void drawBackground(QPainter& painter);
    void drawTile(QPainter& painter, int row, int col, int type);
    void drawSelection(QPainter& painter, int row, int col);
    void drawConnectionPath(QPainter& painter);
    void drawHintHighlight(QPainter& painter);
    void drawShuffleMessage(QPainter& painter);  // 绘制重排提示
    void drawComboEffects(QPainter& painter);   // 绘制浮动连击文字

    // ======================== 坐标转换 ========================
    QRectF tileRect(int row, int col) const;
    QPointF tileCenter(int row, int col) const;
    int hitTest(const QPoint& pos, int& outRow, int& outCol) const;

    // ======================== 匹配逻辑 ========================
    void tryMatch(int row, int col);
    void executeMatch(const PathInfo& path);
    void finishMatch();
    void checkGameState();
    void shuffleBoard();

    // ======================== 资源加载 ========================
    void loadTileImages();

    // 当 tileTypes > 已有图片数时，通过色相旋转生成额外图案
    QPixmap generateHueRotatedTile(const QPixmap& source, int hueShift);

    // ======================== 提示控制 ========================

private:
    // ---- 棋盘数据 ----
    GameBoard m_board;

    // ---- 方块图片 ----
    QVector<QPixmap> m_tilePixmaps;

    // ---- 设置 ----
    int m_iconScale;     // 图标缩放百分比

    // ---- 布局参数 ----
    static constexpr int MARGIN = 15;
    double m_tileSize;
    double m_offsetX;
    double m_offsetY;

    // ---- 选择状态 ----
    bool m_hasSelection;
    int m_selectedRow;
    int m_selectedCol;

    // ---- 动画状态 ----
    bool m_isAnimating;
    PathInfo m_animPath;
    QColor m_animColor;

    // ---- 提示状态 ----
    bool m_showingHint;
    int m_hintRow1, m_hintCol1;
    int m_hintRow2, m_hintCol2;
    QTimer* m_hintTimer;
    int m_hintFlashCount;

    // ---- 空闲自动提示 ----
    QTimer* m_idleTimer;
    static constexpr int IDLE_HINT_DELAY = 10000;

    // ---- 自动重排 ----
    bool m_isShuffling;            // 是否正在自动重排（锁定操作）
    bool m_showingShuffleMsg;      // 是否显示重排提示文字
    int m_shuffleMsgFrames;        // 重排提示剩余帧数
    struct ComboEffect {
        int comboCount = 0;
        int remainingFrames = 0;
        QPointF startPos;       // 浮动文字起始位置（屏幕坐标）
    };
    QVector<ComboEffect> m_comboEffects;   // 可同时多个连击特效叠加
    static constexpr int COMBO_FLOAT_FRAMES = 30;  // 浮动持续约1.5秒(30帧×50ms)
    QTimer* m_comboTimer;                      // 独立计时器驱动连击动画
};
