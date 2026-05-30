#pragma once

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPoint>
#include <QColor>
#include <QPixmap>
#include "GameBoard.h"

// 游戏区域控件：显示棋盘、处理点击、播放消除动画
class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget* parent = nullptr);
    ~GameWidget();

    // 游戏操作
    void startNewGame();
    void showHint();
    void shuffleBoard();

    // 状态获取
    int getScore() const { return m_board.getScore(); }
    int getMoves() const { return m_board.getMoves(); }

signals:
    void scoreChanged(int newScore);
    void tilesRemainingChanged(int remaining);
    void moveCountChanged(int moves);
    void gameWon();
    void noMovesLeft();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // 布局计算
    void computeLayout();

    // 绘制
    void drawBackground(QPainter& painter);
    void drawTile(QPainter& painter, int row, int col, int type);
    void drawSelection(QPainter& painter, int row, int col);
    void drawConnectionPath(QPainter& painter);
    void drawHintHighlight(QPainter& painter);

    // 坐标转换
    QRectF tileRect(int row, int col) const;
    QPointF tileCenter(int row, int col) const;
    int hitTest(const QPoint& pos, int& outRow, int& outCol) const;

    // 匹配逻辑
    void tryMatch(int row, int col);
    void executeMatch(const PathInfo& path);
    void finishMatch();
    void checkGameState();

    // 加载资源
    void loadTileImages();

    // 动画
    void clearHintTimer();

private:
    GameBoard m_board;

    // 方块图片缓存
    QVector<QPixmap> m_tilePixmaps;                  // 从.qrc加载的20张方块图片

    // 布局参数
    static constexpr int MARGIN = 15;
    double m_tileSize;
    double m_offsetX;
    double m_offsetY;

    // 选择状态
    bool m_hasSelection;
    int m_selectedRow;
    int m_selectedCol;

    // 动画状态
    bool m_isAnimating;
    PathInfo m_animPath;
    QColor m_animColor;

    // 提示状态
    bool m_showingHint;
    int m_hintRow1, m_hintCol1;
    int m_hintRow2, m_hintCol2;
    QTimer* m_hintTimer;
    int m_hintFlashCount;
};
