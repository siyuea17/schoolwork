// GameWidget.cpp — 游戏画布实现，包含绘制、鼠标交互和动画逻辑

#include "GameWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QtMath>
#include <QMessageBox>

// ---- 构造与析构 ----

GameWidget::GameWidget(int rows, int cols, int tileTypes, int copiesPerType,
                       int iconScale, QWidget* parent)
    : QWidget(parent)
    , m_board(rows, cols, tileTypes, copiesPerType)
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
    , m_isPaused(false)              // 一开始没有暂停
{
    setMouseTracking(false);
    setMinimumSize(500, 420);

    // 启动时一次性加载所有方块图片到内存，避免 paintEvent 中反复读磁盘
    loadTileImages();

    // 提示闪烁定时器：每 200ms 切换绿色高亮的显示状态，闪烁 12 次后自动停止
    m_hintTimer = new QTimer(this);
    m_hintTimer->setInterval(200);

    connect(m_hintTimer, &QTimer::timeout, this, [this]() {
        m_hintFlashCount++;
        if (m_hintFlashCount >= 12)
            clearHintTimer();
        update();
    });

    // 连击特效定时器：50ms 间隔驱动浮动文字动画和重排提示倒计时
    m_comboTimer = new QTimer(this);
    m_comboTimer->setInterval(50);
    connect(m_comboTimer, &QTimer::timeout, this, [this]() {
        for (int i = m_comboEffects.size() - 1; i >= 0; --i)
        {
            m_comboEffects[i].remainingFrames--;
            if (m_comboEffects[i].remainingFrames <= 0)
                m_comboEffects.removeAt(i);
        }

        if (m_showingShuffleMsg)
        {
            m_shuffleMsgFrames--;
            if (m_shuffleMsgFrames <= 0)
            {
                m_showingShuffleMsg = false;
                m_isShuffling = false;
            }
        }

        update();

        if (m_comboEffects.isEmpty() && !m_showingShuffleMsg)
            m_comboTimer->stop();
    });

    // 空闲自动提示定时器：10 秒无操作自动显示提示（单次触发）
    m_idleTimer = new QTimer(this);
    m_idleTimer->setInterval(IDLE_HINT_DELAY);
    m_idleTimer->setSingleShot(true);

    connect(m_idleTimer, &QTimer::timeout, this, [this]() {
        if (!m_isAnimating &&
            !m_board.isWin() &&
            !m_showingHint)
            showHint();
    });
}

GameWidget::~GameWidget()
{
}

// ---- 图片加载 ----

void GameWidget::loadTileImages()
{
    m_tilePixmaps.clear();
    m_tilePixmaps.reserve(m_board.tileTypes());

    int needed = m_board.tileTypes();

    for (int i = 0; i < needed; ++i)
    {
        QString path = QString(":/tiles/images/tile_%1.png")
                           .arg(i + 1, 2, 10, QChar('0'));
        QPixmap pix(path);
        if (!pix.isNull())
        {
            m_tilePixmaps.append(pix);
        }
        else
        {
            // 加载失败时生成纯色备用图，色调均匀分布在色环上
            QPixmap fallback(64, 64);
            fallback.fill(QColor::fromHsv(i * 360 / needed, 200, 255));
            m_tilePixmaps.append(fallback);
        }
    }
}

// ---- 布局与坐标变换 ----

void GameWidget::setIconScale(int percent)
{
    m_iconScale = percent;
    computeLayout();
    update();
}

// setPaused() —— 切换暂停/恢复状态
// 被 MainWindow 调用（暂停按钮 / 关闭确认窗口 / 新游戏确认窗口）
void GameWidget::setPaused(bool paused)
{
    if (m_isPaused == paused)                                // 如果状态没有变化
        return;                                              // 就什么也不做

    m_isPaused = paused;                                     // 更新暂停标志

    if (m_isPaused)                                          // 如果变为暂停状态
    {
        m_idleTimer->stop();                                 // 停止空闲计时（暂停时不需要自动提示）
        clearHintTimer();                                    // 清除正在显示的提示
    }
    else                                                     // 否则（变为恢复状态）
    {
        // 如果游戏还在进行（没通关、没动画中），重新开始空闲计时
        if (!m_board.isWin() && !m_isAnimating)              // 如果没通关 且 没在动画中
            m_idleTimer->start();                            // 就重新开始 10 秒空闲计时
    }

    update();                                                // 请求重绘（显示/隐藏遮罩）
}

// 自适应布局：根据窗口大小、棋盘尺寸、缩放比例计算方块像素大小
void GameWidget::computeLayout()
{
    double scale = m_iconScale / 100.0;
    double availW = width()  - 2.0 * MARGIN;
    double availH = height() - 2.0 * MARGIN;

    double baseTileW = availW / m_board.totalCols();
    double baseTileH = availH / m_board.totalRows();
    double baseTile = qMin(baseTileW, baseTileH);

    m_tileSize = baseTile * scale;

    double boardW = m_tileSize * m_board.totalCols();
    double boardH = m_tileSize * m_board.totalRows();
    if (boardW > availW || boardH > availH)
    {
        m_tileSize = qMin(availW / m_board.totalCols(),
                          availH / m_board.totalRows());
    }

    constexpr double MIN_TILE = 24.0;
    if (m_tileSize < MIN_TILE)
        m_tileSize = MIN_TILE;

    m_offsetX = (width()  - m_tileSize * m_board.totalCols()) / 2.0;
    m_offsetY = (height() - m_tileSize * m_board.totalRows()) / 2.0;
}

// 逻辑坐标 (row, col) → 屏幕像素矩形
QRectF GameWidget::tileRect(int row, int col) const
{
    double x = m_offsetX + col * m_tileSize;
    double y = m_offsetY + row * m_tileSize;
    return QRectF(x, y, m_tileSize, m_tileSize);
}

// 格子中心点像素坐标（用于连线端点与拐角）
QPointF GameWidget::tileCenter(int row, int col) const
{
    QRectF r = tileRect(row, col);
    return r.center();
}

// 鼠标像素坐标 → 棋盘逻辑坐标。返回 0=有效, -1=无效位置
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

// ---- 游戏流程控制 ----

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
    m_isPaused = false;              // 清除暂停状态（新游戏不应该暂停）

    emit scoreChanged(0);
    emit tilesRemainingChanged(m_board.totalTiles());
    emit moveCountChanged(0);
    emit comboCountChanged(0);

    m_idleTimer->start();
    computeLayout();
    update();
}

// 显示提示：先查找可消除配对，找不到则自动重排
void GameWidget::showHint()
{
    if (m_isAnimating || m_isShuffling)
        return;
    if (m_board.isWin())
        return;
    if (m_isPaused)                               // 如果游戏已暂停
        return;                                   // 就不显示提示

    clearHintTimer();
    m_hasSelection = false;
    m_selectedRow = -1;
    m_selectedCol = -1;

    PathInfo hint = m_board.findHint();

    if (!hint.valid)
    {
        shuffleBoard();
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

// 自动重排：先显示提示文字约 0.8 秒，再执行洗牌
void GameWidget::shuffleBoard()
{
    if (m_isAnimating) return;

    clearHintTimer();
    m_hasSelection = false;
    m_selectedRow = -1;
    m_selectedCol = -1;

    m_isShuffling = true;
    m_showingShuffleMsg = true;
    m_shuffleMsgFrames = 16;
    m_idleTimer->stop();

    if (!m_comboTimer->isActive())
        m_comboTimer->start();

    emit noMovesLeft();
    update();

    QTimer::singleShot(800, this, [this]() {
        m_board.shuffle();
        m_showingShuffleMsg = false;
        m_isShuffling = false;
        m_idleTimer->start();
        update();
        checkGameState();
    });
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

    if (!m_board.isWin() && !m_isAnimating
        && !m_isPaused)                                  // 且没有暂停
        m_idleTimer->start();
}

void GameWidget::deserializeBoard(const QVector<int>& data)
{
    m_board.deserializeGrid(data);
    loadTileImages();
    computeLayout();
}

// ---- 绘制（paintEvent 按图层顺序依次绘制） ----

void GameWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 图层1：背景与网格
    drawBackground(painter);

    // 图层2：所有方块
    for (int r = 1; r <= m_board.rows(); ++r)
    {
        for (int c = 1; c <= m_board.cols(); ++c)
        {
            int type = m_board.getTile(r, c);
            if (type != GameBoard::EMPTY)
                drawTile(painter, r, c, type);
        }
    }

    // 图层3：选中高亮（金色）
    if (m_hasSelection && !m_isAnimating)
        drawSelection(painter, m_selectedRow, m_selectedCol);

    // 图层4：提示高亮（绿色，偶数帧显示 → 闪烁效果）
    if (m_showingHint && m_hintFlashCount % 2 == 0)
        drawHintHighlight(painter);

    // 图层5：消除连接线动画
    if (m_isAnimating)
        drawConnectionPath(painter);

    // 图层6：连击浮动文字
    drawComboEffects(painter);

    // 图层7：重排提示文字
    if (m_showingShuffleMsg)
        drawShuffleMessage(painter);

    // 图层8：如果暂停了，画半透明遮罩 + "已暂停"文字
    if (m_isPaused)
        drawPauseOverlay(painter);
}

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

void GameWidget::drawTile(QPainter& painter, int row, int col, int type)
{
    int typeIndex = type - 1;
    if (typeIndex < 0 || typeIndex >= m_tilePixmaps.size())
        return;

    QRectF rect = tileRect(row, col);
    double padding = m_tileSize * 0.08;
    QRectF inner = rect.adjusted(padding, padding, -padding, -padding);

    const QPixmap& pix = m_tilePixmaps[typeIndex];
    painter.drawPixmap(inner.toRect(), pix);
}

// 双层金色边框制造发光光晕效果
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

// 绘制连接线：光晕层 + 主体线 + 拐点圆圈 + 端点白色圆
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

    // 主体线
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

// 连击文字向上飘动，透明度渐入→全显→渐出
void GameWidget::drawComboEffects(QPainter& painter)
{
    for (const ComboEffect& fx : m_comboEffects)
    {
        if (fx.comboCount < 2) continue;

        float progress = 1.0f - fx.remainingFrames / float(COMBO_FLOAT_FRAMES);

        float alpha;
        if (progress < 0.2f)
            alpha = progress / 0.2f;
        else if (progress > 0.7f)
            alpha = (1.0f - progress) / 0.3f;
        else
            alpha = 1.0f;

        float yOffset = -progress * m_tileSize * 2.0f;
        QPointF pos(fx.startPos.x(), fx.startPos.y() + yOffset);

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

        QPen outlinePen(QColor(0, 0, 0, static_cast<int>(180 * alpha)));
        painter.setPen(outlinePen);
        QRectF textRect(pos.x() - 120, pos.y() - 20, 240, 40);
        painter.drawText(textRect.adjusted(-1, -1, -1, -1), Qt::AlignCenter, text);
        painter.drawText(textRect.adjusted( 1, -1,  1, -1), Qt::AlignCenter, text);
        painter.drawText(textRect.adjusted(-1,  1, -1,  1), Qt::AlignCenter, text);
        painter.drawText(textRect.adjusted( 1,  1,  1,  1), Qt::AlignCenter, text);

        QPen textPen(color);
        painter.setPen(textPen);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

void GameWidget::drawShuffleMessage(QPainter& painter)
{
    double x = m_offsetX + (m_board.totalCols() * m_tileSize) / 2.0;
    double y = m_offsetY + (m_board.totalRows() * m_tileSize) / 2.0;

    QFont font;
    font.setPointSize(qMax(18, static_cast<int>(m_tileSize * 0.5)));
    font.setBold(true);
    painter.setFont(font);

    QString text = QString::fromUtf8("无可用移动，正在自动重排...");

    QRectF textRect(x - 250, y - 25, 500, 50);
    painter.fillRect(textRect, QColor(0, 0, 0, 160));

    QPen outlinePen(QColor(0, 0, 0));
    painter.setPen(outlinePen);
    for (int dx = -1; dx <= 1; dx += 2)
        for (int dy = -1; dy <= 1; dy += 2)
            painter.drawText(textRect.adjusted(dx, dy, dx, dy),
                             Qt::AlignCenter, text);

    painter.setPen(QPen(QColor("#FFD700")));
    painter.drawText(textRect, Qt::AlignCenter, text);
}

// drawPauseOverlay() —— 画暂停遮罩（半透明黑底 + "已暂停"大字）
// 暂停时整个棋盘上方覆盖一层半透明黑色 + 正中央显示醒目的"已暂停"文字
void GameWidget::drawPauseOverlay(QPainter& painter)
{
    // ---- 计算棋盘区域的像素坐标 ----
    double boardW = m_tileSize * m_board.totalCols();       // 棋盘总宽度（像素）
    double boardH = m_tileSize * m_board.totalRows();       // 棋盘总高度（像素）
    QRectF boardRect(m_offsetX, m_offsetY, boardW, boardH); // 棋盘矩形区域

    // ---- 第1层：覆盖整个棋盘的半透明深色遮罩 ----
    painter.fillRect(boardRect, QColor(0, 0, 0, 130));

    // ---- 第2层：棋盘正中央的大号"已暂停"文字 ----
    double cx = m_offsetX + boardW / 2.0;                  // 棋盘水平中心
    double cy = m_offsetY + boardH / 2.0;                  // 棋盘垂直中心

    QFont font;
    font.setPointSize(qMax(28, static_cast<int>(m_tileSize * 0.8)));
    font.setBold(true);
    painter.setFont(font);

    // 画黑色描边
    QPen outlinePen(QColor(0, 0, 0));
    painter.setPen(outlinePen);
    QRectF textRect(cx - 200, cy - 40, 400, 80);
    for (int dx = -2; dx <= 2; dx += 4)
        for (int dy = -2; dy <= 2; dy += 4)
            painter.drawText(textRect.adjusted(dx, dy, dx, dy),
                             Qt::AlignCenter, QString::fromUtf8("已暂停"));

    // 画主体白色文字
    painter.setPen(QPen(QColor("#EEEEEE")));
    painter.drawText(textRect, Qt::AlignCenter, QString::fromUtf8("已暂停"));
}

// ---- 鼠标交互 ----

void GameWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_isAnimating || m_isShuffling || m_isPaused) return;

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

    // 玩家有操作，重置 10 秒空闲计时
    m_idleTimer->start();

    if (!m_hasSelection)
    {
        // 第一次选中
        m_hasSelection = true;
        m_selectedRow = row;
        m_selectedCol = col;
        update();
    }
    else
    {
        // 已有选中，处理第二个点击
        if (m_selectedRow == row && m_selectedCol == col)
        {
            // 点击同一方块 → 取消选中
            m_hasSelection = false;
            m_selectedRow = -1;
            m_selectedCol = -1;
            update();
            return;
        }

        if (m_board.getTile(row, col) !=
            m_board.getTile(m_selectedRow, m_selectedCol))
        {
            // 图案不同 → 切换选中
            m_selectedRow = row;
            m_selectedCol = col;
            update();
            return;
        }

        // 图案相同 → 尝试配对消除
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

// 开始消除动画：显示连接线 500ms，然后真正消除
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

void GameWidget::finishMatch()
{
    if (!m_animPath.valid) return;

    int r1 = m_animPath.corners.first().y();
    int c1 = m_animPath.corners.first().x();
    int r2 = m_animPath.corners.last().y();
    int c2 = m_animPath.corners.last().x();

    int comboScore = m_board.calculateComboScore();
    int comboCount = m_board.getComboCount();

    m_board.removeTiles(r1, c1, r2, c2);
    m_board.addScore(comboScore);

    if (comboCount >= 1)
    {
        QPointF p1 = tileCenter(r1, c1);
        QPointF p2 = tileCenter(r2, c2);
        QPointF midPos((p1.x() + p2.x()) / 2.0,
                       qMin(p1.y(), p2.y()));

        ComboEffect fx;
        fx.comboCount = comboCount + 1;
        fx.remainingFrames = COMBO_FLOAT_FRAMES;
        fx.startPos = midPos;
        m_comboEffects.append(fx);

        if (!m_comboTimer->isActive())
            m_comboTimer->start();
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

// 消除后检查：通关？死局自动重排？正常继续？
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

// ---- 窗口大小变化 ----

void GameWidget::resizeEvent(QResizeEvent*)
{
    computeLayout();
    update();
}
