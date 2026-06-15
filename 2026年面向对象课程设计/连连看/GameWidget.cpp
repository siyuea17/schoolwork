// ============================================================================
// 文件：GameWidget.cpp
// 角色：GameWidget 的实现——游戏的所有"看得见"的行为都在这里
// ============================================================================

#include "GameWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QtMath>
#include <QMessageBox>

// ============================================================================
// 构造函数 —— 支持动态棋盘尺寸和图标缩放
// ============================================================================
GameWidget::GameWidget(int rows, int cols, int tileTypes, int copiesPerType,
                       int iconScale, QWidget* parent)
    : QWidget(parent)
    , m_board(rows, cols, tileTypes, copiesPerType)  // 按参数构造棋盘
    , m_iconScale(iconScale)
    , m_hasSelection(false)
    , m_selectedRow(-1)
    , m_selectedCol(-1)
    , m_isAnimating(false)
    , m_showingHint(false)
    , m_hintRow1(-1), m_hintCol1(-1)
    , m_hintRow2(-1), m_hintCol2(-1)
    , m_isShuffling(false)
    , m_showingShuffleMsg(false)
    , m_shuffleMsgFrames(0)
    , m_comboEffects()
{
    setMouseTracking(false);
    setMinimumSize(500, 420);

    // ===== 加载方块图片 =====
    loadTileImages();

    // ===== 提示闪烁定时器（周期性） =====
    m_hintTimer = new QTimer(this);
    m_hintTimer->setInterval(200);

    connect(m_hintTimer, &QTimer::timeout, this, [this]() {
        m_hintFlashCount++;
        if (m_hintFlashCount >= 12)
            clearHintTimer();

        update();
    });

    // ===== 连击特效计时器（独立于提示计时器） =====
    m_comboTimer = new QTimer(this);
    m_comboTimer->setInterval(50);   // 50ms 一帧，约 20fps
    connect(m_comboTimer, &QTimer::timeout, this, [this]() {
        // 更新连击特效
        for (int i = m_comboEffects.size() - 1; i >= 0; --i) {
            m_comboEffects[i].remainingFrames--;
            if (m_comboEffects[i].remainingFrames <= 0)
                m_comboEffects.removeAt(i);
        }
        // 更新重排提示帧
        if (m_showingShuffleMsg) {
            m_shuffleMsgFrames--;
            if (m_shuffleMsgFrames <= 0) {
                m_showingShuffleMsg = false;
                m_isShuffling = false;
            }
        }
        update();
        if (m_comboEffects.isEmpty() && !m_showingShuffleMsg)
            m_comboTimer->stop();
    });

    // ===== 空闲自动提示定时器（单发） =====
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
// loadTileImages() —— 从 Qt 资源系统加载方块图片
//
// 优先加载 qrc 里的 tile_01~40.png（编译到 exe 里）。
// 如果 qrc 只注册了20张（旧版资源），则超出部分用色相旋转生成。
// ============================================================================
void GameWidget::loadTileImages()
{
    m_tilePixmaps.clear();
    m_tilePixmaps.reserve(m_board.tileTypes());

    // ---- 逐张加载，尽量多加载 ----
    int loadedCount = 0;
    for (int i = 0; i < 40; ++i)
    {
        QString path = QString(":/tiles/images/tile_%1.png")
                           .arg(i + 1, 2, 10, QChar('0'));
        QPixmap pix(path);
        if (!pix.isNull())
        {
            m_tilePixmaps.append(pix);
            loadedCount++;
        }
        else
            break;
    }

    // ---- 加载不足时生成占位图 ----
    if (loadedCount == 0)
    {
        for (int i = 0; i < m_board.tileTypes(); ++i)
        {
            QPixmap pix(64, 64);
            pix.fill(QColor::fromHsv(i * 360 / m_board.tileTypes(), 200, 255));
            m_tilePixmaps.append(pix);
        }
        return;
    }

    // 如果已加载的张数不够 tileTypes，用色相旋转补
    int needed = m_board.tileTypes() - loadedCount;
    for (int i = 0; i < needed; ++i)
    {
        int srcIdx = i % loadedCount;
        int hueShift = (i + 1) * 360 / (needed + 1);
        QPixmap variant = generateHueRotatedTile(m_tilePixmaps[srcIdx], hueShift);
        m_tilePixmaps.append(variant);
    }
}

// ============================================================================
// generateHueRotatedTile() —— 通过色相旋转生成额外的方块图案
//
// 用于困难模式（tileTypes > 20）时生成额外图案。
// 做法：把源图片的每个像素的色相在 HSV 空间里旋转 hueShift 度。
// 这样生成的图案"形状相同但颜色不同"，玩家可以区分。
// ============================================================================
QPixmap GameWidget::generateHueRotatedTile(const QPixmap& source, int hueShift)
{
    QImage img = source.toImage().convertToFormat(QImage::Format_ARGB32);

    // 色相旋转：只改非透明像素的色相
    for (int y = 0; y < img.height(); ++y)
    {
        for (int x = 0; x < img.width(); ++x)
        {
            QColor c = img.pixelColor(x, y);
            if (c.alpha() < 30)
                continue;  // 透明像素不处理，保持透明

            int h = c.hue();
            int s = c.saturation();
            int v = c.value();
            h = (h + hueShift) % 360;
            img.setPixelColor(x, y, QColor::fromHsv(h, s, v, c.alpha()));
        }
    }

    return QPixmap::fromImage(img);
}

// ============================================================================
// setIconScale() —— 设置图标缩放并重绘
// ============================================================================
void GameWidget::setIconScale(int percent)
{
    m_iconScale = percent;
    computeLayout();
    update();
}

// ============================================================================
// startNewGame() —— 开始新游戏
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
    m_comboEffects.clear();

    emit scoreChanged(0);
    emit tilesRemainingChanged(m_board.totalTiles());
    emit moveCountChanged(0);
    emit comboCountChanged(0);

    m_idleTimer->start();
    computeLayout();
    update();
}

// ============================================================================
// showHint() —— 显示提示功能
// ============================================================================
void GameWidget::showHint()
{
    if (m_isAnimating || m_isShuffling) return;
    if (m_board.isWin()) return;

    clearHintTimer();
    m_hasSelection = false;
    m_selectedRow = -1;
    m_selectedCol = -1;

    PathInfo hint = m_board.findHint();

    if (!hint.valid)
    {
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

    m_idleTimer->stop();

    update();
}

void GameWidget::shuffleBoard()
{
    if (m_isAnimating) return;

    clearHintTimer();
    m_hasSelection = false;
    m_selectedRow = -1;
    m_selectedCol = -1;

    // ===== 锁定操作，显示重排提示 =====
    m_isShuffling = true;
    m_showingShuffleMsg = true;
    m_shuffleMsgFrames = 16;  // 显示约0.8秒(16帧×50ms)
    m_idleTimer->stop();
    if (!m_comboTimer->isActive())
        m_comboTimer->start();
    emit noMovesLeft();
    update();

    // 延迟执行实际重排，给玩家看见提示文字的时间
    QTimer::singleShot(800, this, [this]() {
        m_board.shuffle();
        m_showingShuffleMsg = false;
        m_isShuffling = false;
        m_idleTimer->start();
        update();
    });
}

// ============================================================================
// drawShuffleMessage() —— 绘制重排提示文字
// ============================================================================
void GameWidget::drawShuffleMessage(QPainter& painter)
{
    double x = m_offsetX + (m_board.totalCols() * m_tileSize) / 2.0;
    double y = m_offsetY + (m_board.totalRows() * m_tileSize) / 2.0;

    QFont font;
    font.setPointSize(qMax(18, static_cast<int>(m_tileSize * 0.5)));
    font.setBold(true);
    painter.setFont(font);

    QString text = QString::fromUtf8("无可用移动，正在自动重排...");
    QColor color("#FFD700");

    // 文字框背景
    QRectF textRect(x - 250, y - 25, 500, 50);
    painter.fillRect(textRect, QColor(0, 0, 0, 160));

    // 描边
    QPen outlinePen(QColor(0, 0, 0));
    painter.setPen(outlinePen);
    for (int dx = -1; dx <= 1; dx += 2)
        for (int dy = -1; dy <= 1; dy += 2)
            painter.drawText(textRect.adjusted(dx, dy, dx, dy), Qt::AlignCenter, text);

    // 主体
    painter.setPen(QPen(color));
    painter.drawText(textRect, Qt::AlignCenter, text);
}

void GameWidget::clearHintTimer()
{
    m_hintTimer->stop();
    m_showingHint = false;
    m_hintRow1 = -1;
    m_hintCol1 = -1;
    m_hintRow2 = -1;
    m_hintCol2 = -1;
    m_hintFlashCount = 0;

    if (!m_board.isWin() && !m_isAnimating)
        m_idleTimer->start();
}

// ============================================================================
// deserializeBoard() —— 从存档数据恢复棋盘
// ============================================================================
void GameWidget::deserializeBoard(const QVector<int>& data)
{
    m_board.deserializeGrid(data);
    // 恢复后重新加载图片（可能尺寸变了）
    loadTileImages();
    computeLayout();
}

// ============================================================================
// computeLayout() —— 计算方块的像素布局
//
// 自适应算法：
//   1. 根据可用空间和棋盘尺寸计算基础 tile 大小
//   2. 应用 iconScale 缩放比例
//   3. 如果缩放后超出可用空间，自动缩回（保证不溢出）
//   4. 确保 tile 不小于 24px（太小无法点击），不小于可用空间约束
//
// 这样无论棋盘多大（Easy 6×8 → Hard 12×14）、图标缩放多少
// （80% → 120%），都能正常显示。
// ============================================================================
void GameWidget::computeLayout()
{
    double scale = m_iconScale / 100.0;
    double availW = width() - 2.0 * MARGIN;
    double availH = height() - 2.0 * MARGIN;

    // 第1步：计算基础 tile 大小（不缩放，刚好填满可用空间）
    double baseTileW = availW / m_board.totalCols();
    double baseTileH = availH / m_board.totalRows();
    double baseTile = qMin(baseTileW, baseTileH);

    // 第2步：应用缩放
    m_tileSize = baseTile * scale;

    // 第3步：缩放后如果超标，回退到刚好填满
    double boardW = m_tileSize * m_board.totalCols();
    double boardH = m_tileSize * m_board.totalRows();
    if (boardW > availW || boardH > availH) {
        // 取宽度和高度约束中更紧的那个
        m_tileSize = qMin(availW / m_board.totalCols(),
                          availH / m_board.totalRows());
    }

    // 第4步：安全下限——确保 tile 不会太小
    constexpr double MIN_TILE = 24.0;
    if (m_tileSize < MIN_TILE)
        m_tileSize = MIN_TILE;

    // 计算棋盘居中偏移
    m_offsetX = (width() - m_tileSize * m_board.totalCols()) / 2.0;
    m_offsetY = (height() - m_tileSize * m_board.totalRows()) / 2.0;
}

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

int GameWidget::hitTest(const QPoint& pos, int& outRow, int& outCol) const
{
    outCol = static_cast<int>((pos.x() - m_offsetX) / m_tileSize);
    outRow = static_cast<int>((pos.y() - m_offsetY) / m_tileSize);

    if (outRow < 0 || outRow >= m_board.totalRows() ||
        outCol < 0 || outCol >= m_board.totalCols())
        return -1;

    if (outRow < 1 || outRow > m_board.rows() ||
        outCol < 1 || outCol > m_board.cols())
        return -1;

    return 0;
}

// ============================================================================
// paintEvent() —— 绘制事件处理函数（整个游戏渲染的入口！）
//
// 绘制顺序：
//   1. 背景   2. 方块   3. 选中高亮   4. 提示高亮
//   5. 连接动画   6. 连击特效文字
// ============================================================================
void GameWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // ---- 第1层：背景 ----
    drawBackground(painter);

    // ---- 第2层：所有方块 ----
    for (int r = 1; r <= m_board.rows(); ++r)
    {
        for (int c = 1; c <= m_board.cols(); ++c)
        {
            int type = m_board.getTile(r, c);
            if (type != GameBoard::EMPTY)
                drawTile(painter, r, c, type);
        }
    }

    // ---- 第3层：选中高亮 ----
    if (m_hasSelection && !m_isAnimating)
        drawSelection(painter, m_selectedRow, m_selectedCol);

    // ---- 第4层：提示高亮 ----
    if (m_showingHint && m_hintFlashCount % 2 == 0)
        drawHintHighlight(painter);

    // ---- 第5层：连接路径动画 ----
    if (m_isAnimating)
        drawConnectionPath(painter);

    // ---- 第6层：连击特效 ----
    drawComboEffects(painter);

    // ---- 第7层：重排提示 ----
    if (m_showingShuffleMsg)
        drawShuffleMessage(painter);
}

// ============================================================================
// drawBackground() —— 绘制棋盘背景和网格线
// ============================================================================
void GameWidget::drawBackground(QPainter& painter)
{
    painter.fillRect(rect(), QColor("#2C3E50"));

    QRectF boardRect(m_offsetX, m_offsetY,
                     m_tileSize * m_board.totalCols(),
                     m_tileSize * m_board.totalRows());
    painter.fillRect(boardRect, QColor("#34495E"));

    painter.setPen(QPen(QColor("#3D566E"), 0.5));

    for (int r = 0; r <= m_board.totalRows(); ++r)
    {
        double y = m_offsetY + r * m_tileSize;
        painter.drawLine(QPointF(m_offsetX, y),
                         QPointF(m_offsetX + m_board.totalCols() * m_tileSize, y));
    }

    for (int c = 0; c <= m_board.totalCols(); ++c)
    {
        double x = m_offsetX + c * m_tileSize;
        painter.drawLine(QPointF(x, m_offsetY),
                         QPointF(x, m_offsetY + m_board.totalRows() * m_tileSize));
    }
}

// ============================================================================
// drawTile() —— 绘制单个方块
// ============================================================================
void GameWidget::drawTile(QPainter& painter, int row, int col, int type)
{
    int typeIndex = type - 1;
    if (typeIndex < 0 || typeIndex >= m_tilePixmaps.size()) return;

    QRectF rect = tileRect(row, col);

    double padding = m_tileSize * 0.08;
    QRectF inner = rect.adjusted(padding, padding, -padding, -padding);

    const QPixmap& pix = m_tilePixmaps[typeIndex];
    painter.drawPixmap(inner.toRect(), pix);
}

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

    // 端点白色圆
    painter.setBrush(QColor(255, 255, 255));
    painter.drawEllipse(pixelPoints.first(), m_tileSize * 0.1, m_tileSize * 0.1);
    painter.drawEllipse(pixelPoints.last(), m_tileSize * 0.1, m_tileSize * 0.1);
}

// ============================================================================
// drawComboEffects() —— 绘制浮动连击特效文字
//
// 效果：在消除的方块上方显示"COMBO x3!"，
//       缓慢上移，逐渐变淡消失。
//   连击1通常不显示
//   连击2-3：橙色
//   连击4-6：粉红色
//   连击7+：金色闪烁
// ============================================================================
void GameWidget::drawComboEffects(QPainter& painter)
{
    for (const ComboEffect& fx : m_comboEffects)
    {
        if (fx.comboCount < 2) continue;

        // 进度 0→1，0=刚出现，1=即将消失
        float progress = 1.0f - fx.remainingFrames / float(COMBO_FLOAT_FRAMES);
        // 透明度：前20%渐进，后30%渐出
        float alpha;
        if (progress < 0.2f)
            alpha = progress / 0.2f;
        else if (progress > 0.7f)
            alpha = (1.0f - progress) / 0.3f;
        else
            alpha = 1.0f;

        // 竖直偏移：向上浮动 2个 tileSize
        float yOffset = -progress * m_tileSize * 2.0f;

        QPointF pos(fx.startPos.x(), fx.startPos.y() + yOffset);

        // 根据连击等级选颜色
        QColor color;
        if (fx.comboCount >= 7)
            color = (fx.remainingFrames % 3 == 0) ? QColor("#FFD700") : QColor("#FF4444");
        else if (fx.comboCount >= 4)
            color = QColor("#FF69B4");
        else
            color = QColor("#FF8C00");

        color.setAlphaF(alpha);

        QString text = QString("COMBO x%1!").arg(fx.comboCount);

        QFont font;
        font.setPointSize(qMax(12, static_cast<int>(m_tileSize * 0.3)));
        font.setBold(true);
        painter.setFont(font);

        // 描边
        QPen outlinePen(QColor(0, 0, 0, static_cast<int>(180 * alpha)));
        painter.setPen(outlinePen);
        QRectF textRect(pos.x() - 120, pos.y() - 20, 240, 40);
        painter.drawText(textRect.adjusted(-1, -1, -1, -1), Qt::AlignCenter, text);
        painter.drawText(textRect.adjusted(1, -1, 1, -1), Qt::AlignCenter, text);
        painter.drawText(textRect.adjusted(-1, 1, -1, 1), Qt::AlignCenter, text);
        painter.drawText(textRect.adjusted(1, 1, 1, 1), Qt::AlignCenter, text);

        // 主体
        QPen textPen(color);
        painter.setPen(textPen);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

// ============================================================================
// mousePressEvent() —— 鼠标点击事件处理
// ============================================================================
void GameWidget::mousePressEvent(QMouseEvent* event)
{
    // 动画中或自动重排中不接受点击
    if (m_isAnimating || m_isShuffling) return;

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

void GameWidget::tryMatch(int row, int col)
{
    PathInfo path = m_board.findPath(m_selectedRow, m_selectedCol, row, col);

    if (path.valid)
    {
        executeMatch(path);
    }
    else
    {
        m_selectedRow = row;
        m_selectedCol = col;
        update();
    }
}

void GameWidget::executeMatch(const PathInfo& path)
{
    m_isAnimating = true;
    m_animPath = path;
    m_animColor = QColor("#FF8C00");
    m_hasSelection = false;
    m_selectedRow = -1;
    m_selectedCol = -1;

    m_idleTimer->stop();

    update();

    QTimer::singleShot(500, this, &GameWidget::finishMatch);
}

// ============================================================================
// finishMatch() —— 完成消除（含连击分计算）
//
// 改动：不再固定 +10 分，而是调用 calculateComboScore() 获得连击分。
//      消除后如果连击 ≥2，显示连击特效文字。
// ============================================================================
void GameWidget::finishMatch()
{
    if (!m_animPath.valid) return;

    int r1 = m_animPath.corners.first().y();
    int c1 = m_animPath.corners.first().x();
    int r2 = m_animPath.corners.last().y();
    int c2 = m_animPath.corners.last().x();

    // ===== 计算连击分 =====
    int comboScore = m_board.calculateComboScore();
    int comboCount = m_board.getComboCount();

    // ===== 消除方块 =====
    m_board.removeTiles(r1, c1, r2, c2);
    m_board.addScore(comboScore);

    // ===== 连击特效：在被消除的方块位置生成浮动文字 =====
    // 取两个消除方块的中点作为起始位置
    if (comboCount >= 1)
    {
        QPointF p1 = tileCenter(r1, c1);
        QPointF p2 = tileCenter(r2, c2);
        QPointF midPos((p1.x() + p2.x()) / 2.0,
                       qMin(p1.y(), p2.y()));  // 用上边那个方块的位置

        ComboEffect fx;
        fx.comboCount = comboCount + 1;  // comboCount 是消除前的值，+1 为本次
        fx.remainingFrames = COMBO_FLOAT_FRAMES;
        fx.startPos = midPos;
        m_comboEffects.append(fx);
        if (!m_comboTimer->isActive())
            m_comboTimer->start();       // 启动独立计时器
    }

    m_isAnimating = false;
    m_animPath = PathInfo();

    emit scoreChanged(m_board.getScore());
    emit tilesRemainingChanged(m_board.getRemainingTiles());
    emit moveCountChanged(m_board.getMoves());
    emit comboCountChanged(comboCount);

    update();
    checkGameState();
}

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
        int maxAttempts = 10;
        while (!m_board.hasValidMoves() && maxAttempts-- > 0)
        {
            m_board.shuffle();
        }
        update();
        emit noMovesLeft();
    }

    m_idleTimer->start();
}

void GameWidget::resizeEvent(QResizeEvent* /*event*/)
{
    computeLayout();
    update();
}
