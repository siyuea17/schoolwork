#include "GameWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QtMath>
#include <QMessageBox>

// ============================================================================
// 构造函数
// ============================================================================
GameWidget::GameWidget(QWidget* parent)
    : QWidget(parent)
    , m_hasSelection(false)
    , m_selectedRow(-1)
    , m_selectedCol(-1)
    , m_isAnimating(false)
    , m_showingHint(false)
    , m_hintRow1(-1), m_hintCol1(-1)
    , m_hintRow2(-1), m_hintCol2(-1)
    , m_hintFlashCount(0)
{
    setMouseTracking(false);
    setMinimumSize(500, 420);

    // 从qrc资源加载方块图片
    loadTileImages();

    // 提示闪烁计时器
    m_hintTimer = new QTimer(this);
    m_hintTimer->setInterval(200);
    connect(m_hintTimer, &QTimer::timeout, this, [this]() {
        m_hintFlashCount++;
        if (m_hintFlashCount >= 12)
            clearHintTimer();
        update();
    });

    // 空闲自动提示计时器
    m_idleTimer = new QTimer(this);
    m_idleTimer->setInterval(IDLE_HINT_DELAY);
    m_idleTimer->setSingleShot(true);
    connect(m_idleTimer, &QTimer::timeout, this, [this]() {
        if (!m_isAnimating && !m_board.isWin() && !m_showingHint)
            showHint();
    });
}

GameWidget::~GameWidget()
{
}

// ============================================================================
// 从qrc加载20张方块图片到缓存
// ============================================================================
void GameWidget::loadTileImages()
{
    m_tilePixmaps.clear();
    m_tilePixmaps.reserve(GameBoard::TILE_TYPES);
    for (int i = 0; i < GameBoard::TILE_TYPES; ++i)
    {
        QString path = QString(":/tiles/images/tile_%1.png")
                           .arg(i + 1, 2, 10, QChar('0'));
        QPixmap pix(path);
        if (!pix.isNull())
            m_tilePixmaps.append(pix);
    }
}

// ============================================================================
// 开始新游戏
// ============================================================================
void GameWidget::startNewGame()
{
    clearHintTimer();
    m_board.reset();
    m_hasSelection = false;
    m_selectedRow = -1;
    m_selectedCol = -1;
    m_isAnimating = false;
    m_animPath = PathInfo();

    emit scoreChanged(0);
    emit tilesRemainingChanged(GameBoard::TOTAL_TILES);
    emit moveCountChanged(0);

    m_idleTimer->start();
    computeLayout();
    update();
}

// ============================================================================
// 显示提示
// ============================================================================
void GameWidget::showHint()
{
    if (m_isAnimating) return;
    if (m_board.isWin()) return;

    clearHintTimer();
    m_hasSelection = false;
    m_selectedRow = -1;
    m_selectedCol = -1;

    PathInfo hint = m_board.findHint();
    if (!hint.valid)
    {
        // 无可用配对，自动重排
        shuffleBoard();
        emit noMovesLeft();
        return;
    }

    m_hintRow1 = hint.corners.first().y();
    m_hintCol1 = hint.corners.first().x();
    m_hintRow2 = hint.corners.last().y();
    m_hintCol2 = hint.corners.last().x();
    m_showingHint = true;
    m_hintFlashCount = 0;
    m_hintTimer->start();
    m_idleTimer->stop();  // 提示已显示，停止空闲计时
    update();
}

// ============================================================================
// 重排方块（仅内部调用：当无可用移动时自动重排）
// ============================================================================
void GameWidget::shuffleBoard()
{
    if (m_isAnimating) return;

    clearHintTimer();
    m_hasSelection = false;
    m_selectedRow = -1;
    m_selectedCol = -1;

    m_board.shuffle();

    // 重排后重新开始空闲计时
    m_idleTimer->start();
    update();
}

// ============================================================================
// 清除提示
// ============================================================================
void GameWidget::clearHintTimer()
{
    m_hintTimer->stop();
    m_showingHint = false;
    m_hintRow1 = -1;
    m_hintCol1 = -1;
    m_hintRow2 = -1;
    m_hintCol2 = -1;
    m_hintFlashCount = 0;
    // 提示清除后重新开始空闲计时（如果游戏仍在进行）
    if (!m_board.isWin() && !m_isAnimating)
        m_idleTimer->start();
}

// ============================================================================
// 计算布局
// ============================================================================
void GameWidget::computeLayout()
{
    double availW = width() - 2.0 * MARGIN;
    double availH = height() - 2.0 * MARGIN;

    double tileW = availW / GameBoard::TOTAL_COLS;
    double tileH = availH / GameBoard::TOTAL_ROWS;
    m_tileSize = qMin(tileW, tileH);

    m_offsetX = (width() - m_tileSize * GameBoard::TOTAL_COLS) / 2.0;
    m_offsetY = (height() - m_tileSize * GameBoard::TOTAL_ROWS) / 2.0;
}

// ============================================================================
// 坐标转换
// ============================================================================
QRectF GameWidget::tileRect(int row, int col) const
{
    double x = m_offsetX + col * m_tileSize;
    double y = m_offsetY + row * m_tileSize;
    return QRectF(x, y, m_tileSize, m_tileSize);
}

QPointF GameWidget::tileCenter(int row, int col) const
{
    QRectF r = tileRect(row, col);
    return r.center();
}

// ============================================================================
// 点击检测
// ============================================================================
int GameWidget::hitTest(const QPoint& pos, int& outRow, int& outCol) const
{
    outCol = static_cast<int>((pos.x() - m_offsetX) / m_tileSize);
    outRow = static_cast<int>((pos.y() - m_offsetY) / m_tileSize);

    if (outRow < 0 || outRow >= GameBoard::TOTAL_ROWS ||
        outCol < 0 || outCol >= GameBoard::TOTAL_COLS)
        return -1;

    if (outRow < 1 || outRow > GameBoard::ROWS ||
        outCol < 1 || outCol > GameBoard::COLS)
        return -1;

    return 0;
}

// ============================================================================
// 重绘事件
// ============================================================================
void GameWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    drawBackground(painter);

    // 绘制所有方块
    for (int r = 1; r <= GameBoard::ROWS; ++r)
    {
        for (int c = 1; c <= GameBoard::COLS; ++c)
        {
            int type = m_board.getTile(r, c);
            if (type != GameBoard::EMPTY)
                drawTile(painter, r, c, type);
        }
    }

    // 选择高亮
    if (m_hasSelection && !m_isAnimating)
        drawSelection(painter, m_selectedRow, m_selectedCol);

    // 提示高亮
    if (m_showingHint && m_hintFlashCount % 2 == 0)
        drawHintHighlight(painter);

    // 连接路径动画
    if (m_isAnimating)
        drawConnectionPath(painter);
}

// ============================================================================
// 绘制背景
// ============================================================================
void GameWidget::drawBackground(QPainter& painter)
{
    painter.fillRect(rect(), QColor("#2C3E50"));

    QRectF boardRect(m_offsetX, m_offsetY,
                     m_tileSize * GameBoard::TOTAL_COLS,
                     m_tileSize * GameBoard::TOTAL_ROWS);
    painter.fillRect(boardRect, QColor("#34495E"));

    painter.setPen(QPen(QColor("#3D566E"), 0.5));
    for (int r = 0; r <= GameBoard::TOTAL_ROWS; ++r)
    {
        double y = m_offsetY + r * m_tileSize;
        painter.drawLine(QPointF(m_offsetX, y),
                         QPointF(m_offsetX + GameBoard::TOTAL_COLS * m_tileSize, y));
    }
    for (int c = 0; c <= GameBoard::TOTAL_COLS; ++c)
    {
        double x = m_offsetX + c * m_tileSize;
        painter.drawLine(QPointF(x, m_offsetY),
                         QPointF(x, m_offsetY + GameBoard::TOTAL_ROWS * m_tileSize));
    }
}

// ============================================================================
// 绘制单个方块（使用qrc中的PNG图片）
// ============================================================================
void GameWidget::drawTile(QPainter& painter, int row, int col, int type)
{
    int typeIndex = type - 1;
    if (typeIndex < 0 || typeIndex >= m_tilePixmaps.size()) return;

    QRectF rect = tileRect(row, col);
    double padding = m_tileSize * 0.08;
    QRectF inner = rect.adjusted(padding, padding, -padding, -padding);

    // 缩放绘制PNG图标
    const QPixmap& pix = m_tilePixmaps[typeIndex];
    painter.drawPixmap(inner.toRect(), pix);
}

// ============================================================================
// 绘制选择高亮
// ============================================================================
void GameWidget::drawSelection(QPainter& painter, int row, int col)
{
    QRectF rect = tileRect(row, col);
    double padding = m_tileSize * 0.04;
    QRectF inner = rect.adjusted(padding, padding, -padding, -padding);

    QPen pen(QColor("#FFD700"), m_tileSize * 0.07);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(inner, m_tileSize * 0.12, m_tileSize * 0.12);

    QPen outerPen(QColor(255, 215, 0, 100), m_tileSize * 0.12);
    painter.setPen(outerPen);
    painter.drawRoundedRect(inner, m_tileSize * 0.12, m_tileSize * 0.12);
}

// ============================================================================
// 绘制提示高亮
// ============================================================================
void GameWidget::drawHintHighlight(QPainter& painter)
{
    QPen pen(QColor("#00FF88"), m_tileSize * 0.08);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    for (int i = 0; i < 2; ++i)
    {
        int row = (i == 0) ? m_hintRow1 : m_hintRow2;
        int col = (i == 0) ? m_hintCol1 : m_hintCol2;
        QRectF rect = tileRect(row, col);
        double padding = m_tileSize * 0.04;
        QRectF inner = rect.adjusted(padding, padding, -padding, -padding);
        painter.drawRoundedRect(inner, m_tileSize * 0.12, m_tileSize * 0.12);
    }
}

// ============================================================================
// 绘制连接路径
// ============================================================================
void GameWidget::drawConnectionPath(QPainter& painter)
{
    if (m_animPath.corners.size() < 2) return;

    QVector<QPointF> pixelPoints;
    for (const QPoint& cp : m_animPath.corners)
        pixelPoints.append(tileCenter(cp.y(), cp.x()));

    // 外发光
    painter.setPen(QPen(QColor(255, 200, 50, 80), m_tileSize * 0.14,
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (int i = 0; i < pixelPoints.size() - 1; ++i)
        painter.drawLine(pixelPoints[i], pixelPoints[i + 1]);

    // 主连接线
    painter.setPen(QPen(m_animColor, m_tileSize * 0.06,
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (int i = 0; i < pixelPoints.size() - 1; ++i)
        painter.drawLine(pixelPoints[i], pixelPoints[i + 1]);

    // 拐点圆圈
    painter.setBrush(m_animColor);
    painter.setPen(Qt::NoPen);
    for (int i = 1; i < pixelPoints.size() - 1; ++i)
        painter.drawEllipse(pixelPoints[i], m_tileSize * 0.08, m_tileSize * 0.08);

    // 端点圆
    painter.setBrush(QColor(255, 255, 255));
    painter.drawEllipse(pixelPoints.first(), m_tileSize * 0.1, m_tileSize * 0.1);
    painter.drawEllipse(pixelPoints.last(), m_tileSize * 0.1, m_tileSize * 0.1);
}

// ============================================================================
// 鼠标点击事件
// ============================================================================
void GameWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_isAnimating) return;

    int row, col;
    if (hitTest(event->pos(), row, col) < 0)
    {
        m_hasSelection = false;
        m_selectedRow = -1;
        m_selectedCol = -1;
        clearHintTimer();
        update();
        return;
    }

    if (m_board.isEmpty(row, col))
    {
        m_hasSelection = false;
        m_selectedRow = -1;
        m_selectedCol = -1;
        clearHintTimer();
        update();
        return;
    }

    clearHintTimer();

    // 玩家有操作，重置空闲计时器
    m_idleTimer->start();

    if (!m_hasSelection)
    {
        m_hasSelection = true;
        m_selectedRow = row;
        m_selectedCol = col;
        update();
    }
    else
    {
        if (m_selectedRow == row && m_selectedCol == col)
        {
            m_hasSelection = false;
            m_selectedRow = -1;
            m_selectedCol = -1;
            update();
            return;
        }

        if (m_board.getTile(row, col) != m_board.getTile(m_selectedRow, m_selectedCol))
        {
            m_selectedRow = row;
            m_selectedCol = col;
            update();
            return;
        }

        tryMatch(row, col);
    }
}

// ============================================================================
// 尝试匹配
// ============================================================================
void GameWidget::tryMatch(int row, int col)
{
    PathInfo path = m_board.findPath(m_selectedRow, m_selectedCol, row, col);
    if (path.valid)
        executeMatch(path);
    else
    {
        m_selectedRow = row;
        m_selectedCol = col;
        update();
    }
}

// ============================================================================
// 执行匹配动画
// ============================================================================
void GameWidget::executeMatch(const PathInfo& path)
{
    m_isAnimating = true;
    m_animPath = path;
    m_animColor = QColor("#FF8C00");
    m_hasSelection = false;
    m_selectedRow = -1;
    m_selectedCol = -1;

    // 匹配成功，停止空闲计时（finishMatch 后会重新启动）
    m_idleTimer->stop();

    update();
    QTimer::singleShot(500, this, &GameWidget::finishMatch);
}

// ============================================================================
// 完成匹配
// ============================================================================
void GameWidget::finishMatch()
{
    if (!m_animPath.valid) return;

    int r1 = m_animPath.corners.first().y();
    int c1 = m_animPath.corners.first().x();
    int r2 = m_animPath.corners.last().y();
    int c2 = m_animPath.corners.last().x();

    m_board.removeTiles(r1, c1, r2, c2);
    m_board.addScore(10);

    m_isAnimating = false;
    m_animPath = PathInfo();

    emit scoreChanged(m_board.getScore());
    emit tilesRemainingChanged(m_board.getRemainingTiles());
    emit moveCountChanged(m_board.getMoves());

    update();
    checkGameState();
}

// ============================================================================
// 检查游戏状态
// ============================================================================
void GameWidget::checkGameState()
{
    if (m_board.isWin())
    {
        m_idleTimer->stop();
        emit gameWon();
        return;
    }

    if (!m_board.hasValidMoves())
    {
        // 自动重排，循环直到有可用的移动（通常一次就够）
        int maxAttempts = 10;
        while (!m_board.hasValidMoves() && maxAttempts-- > 0)
        {
            m_board.shuffle();
        }
        update();
        emit noMovesLeft();  // 通知状态栏显示"已自动重排"
    }

    // 匹配完成后重新开始空闲计时
    m_idleTimer->start();
}

// ============================================================================
// 窗口大小改变
// ============================================================================
void GameWidget::resizeEvent(QResizeEvent* /*event*/)
{
    computeLayout();
    update();
}
