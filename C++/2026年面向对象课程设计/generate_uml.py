#!/c/Users/siyuea/AppData/Local/Python/pythoncore-3.14-64/python.exe
"""UML class diagram — every member listed separately."""

import graphviz, os, html

os.environ["PATH"] += os.pathsep + r"C:\Users\siyuea\AppData\Local\Temp\graphviz\Graphviz-15.1.0-win64\bin"

g = graphviz.Digraph(name="LLK", format="png", engine="dot")
g.attr(
    label="LianLianKan — Class Diagram",
    labelloc="t", fontsize="24", fontname="Arial",
    rankdir="TB", dpi="200", bgcolor="#FAFBFC",
    pad="0.15", margin="0.2",
    nodesep="0.08", ranksep="0.3",
    splines="polyline", newrank="true",
)
g.attr("node", fontname="Microsoft YaHei", fontsize="12", margin="0.04,0.02")
g.attr("edge", fontname="Microsoft YaHei", fontsize="10")

E = html.escape

def cls(name, fields, methods):
    label = f'<<TABLE border="0" cellborder="1" cellspacing="0" cellpadding="3">'
    label += f'<TR><TD bgcolor="#4A90D9"><FONT color="white" point-size="13"><B>{E(name)}</B></FONT></TD></TR>'
    for v, txt in fields:
        label += f'<TR><TD align="left"><I>{E(v)}</I> {E(txt)}</TD></TR>'
    if methods:
        label += f'<TR><TD align="left" sides="T">'
        label += '<BR align="left"/>'.join(f'{E(v)} <B>{E(sig)}</B>' for v, sig in methods)
        label += '</TD></TR>'
    label += '</TABLE>>'
    g.node(name, label, shape="none", margin="0,0")

def st(name, fields, methods=None):
    label = f'<<TABLE border="0" cellborder="1" cellspacing="0" cellpadding="2">'
    label += f'<TR><TD bgcolor="#E8D5B7"><B>&lt;&lt;struct&gt;&gt; {E(name)}</B></TD></TR>'
    for v, txt in fields:
        label += f'<TR><TD align="left">{E(v)} {E(txt)}</TD></TR>'
    if methods:
        label += f'<TR><TD align="left" sides="T">'
        label += '<BR align="left"/>'.join(f'{E(v)} <B>{E(sig)}</B>' for v, sig in methods)
        label += '</TD></TR>'
    label += '</TABLE>>'
    g.node(name, label, shape="none", margin="0,0")

def en(name, vals):
    label = f'<<TABLE border="0" cellborder="1" cellspacing="0" cellpadding="2">'
    label += f'<TR><TD bgcolor="#B7D7A8"><B>&lt;&lt;enum&gt;&gt; {E(name)}</B></TD></TR>'
    for v in vals:
        label += f'<TR><TD align="left">{E(v)}</TD></TR>'
    label += '</TABLE>>'
    g.node(name, label, shape="none", margin="0,0")

# ====== Difficulty ======
en("Difficulty", ["Easy = 0", "Normal = 1", "Hard = 2"])

# ====== structs ======
st("DifficultyParams", [
    ("+", "rows : int"),
    ("+", "cols : int"),
    ("+", "tileTypes : int"),
    ("+", "copies : int"),
], [
    ("+", "totalTiles() : int"),
])
st("GameSettings", [
    ("+", "volume : int"),
    ("+", "iconScale : int"),
    ("+", "difficulty : Difficulty"),
    ("+", "highScore : int"),
], [
    ("+", "save() : void"),
    ("+", "load() : void"),
])
st("SavedGameState", [
    ("+", "hasSaved : bool"),
    ("+", "rows : int"),
    ("+", "cols : int"),
    ("+", "tileTypes : int"),
    ("+", "copies : int"),
    ("+", "score : int"),
    ("+", "moves : int"),
    ("+", "remainingTiles : int"),
    ("+", "comboCount : int"),
    ("+", "elapsedSeconds : int"),
    ("+", "difficulty : Difficulty"),
    ("+", "gridData : QVector&lt;int&gt;"),
])
st("PathInfo", [
    ("+", "valid : bool"),
    ("+", "corners : QVector&lt;QPoint&gt;"),
])

# ====== GameBoard ======
cls("GameBoard", [
    # -- static constexpr --
    ("+", "DEFAULT_ROWS = 8 : static constexpr int"),
    ("+", "DEFAULT_COLS = 10 : static constexpr int"),
    ("+", "DEFAULT_TILE_TYPES = 20 : static constexpr int"),
    ("+", "DEFAULT_COPIES = 4 : static constexpr int"),
    ("+", "EMPTY = 0 : static constexpr int"),
    ("+", "COMBO_WINDOW_MS = 5000 : static constexpr int"),
    # -- data --
    ("-", "m_rows : int"),
    ("-", "m_cols : int"),
    ("-", "m_tileTypes : int"),
    ("-", "m_copiesPerType : int"),
    ("-", "m_grid : QVector&lt;QVector&lt;int&gt;&gt;"),
    ("-", "m_score : int"),
    ("-", "m_remainingTiles : int"),
    ("-", "m_moves : int"),
    ("-", "m_comboCount : int"),
    ("-", "m_lastMatchTimeMs : qint64"),
    ("-", "m_rng : std::mt19937"),
], [
    # -- public --
    ("+", "GameBoard(rows, cols, tileTypes, copiesPerType)"),
    ("+", "initBoard() : void"),
    ("+", "reset() : void"),
    ("+", "findPath(r1, c1, r2, c2) : PathInfo"),
    ("+", "hasValidMoves() : bool"),
    ("+", "findHint() : PathInfo"),
    ("+", "isWin() : bool"),
    ("+", "removeTiles(r1, c1, r2, c2) : void"),
    ("+", "shuffle() : void"),
    ("+", "calculateComboScore() : int"),
    ("+", "resetCombo() : void"),
    ("+", "getComboCount() : int"),
    ("+", "getTile(r, c) : int"),
    ("+", "setTile(r, c, type) : void"),
    ("+", "isEmpty(r, c) : bool"),
    ("+", "getScore() : int"),
    ("+", "getRemainingTiles() : int"),
    ("+", "getMoves() : int"),
    ("+", "addScore(points) : void"),
    ("+", "rows() : int"),
    ("+", "cols() : int"),
    ("+", "totalRows() : int"),
    ("+", "totalCols() : int"),
    ("+", "tileTypes() : int"),
    ("+", "copiesPerType() : int"),
    ("+", "totalTiles() : int"),
    ("+", "serializeGrid() : QVector&lt;int&gt;"),
    ("+", "deserializeGrid(data) : void"),
    ("+", "setScore(s) : void"),
    ("+", "setMoves(m) : void"),
    ("+", "setRemainingTiles(r) : void"),
    ("+", "setComboCount(c) : void"),
    # -- private --
    ("-", "isRowClear(r, c1, c2) : bool"),
    ("-", "isColClear(r1, r2, c) : bool"),
    ("-", "tryDirectLink(r1, c1, r2, c2) : PathInfo"),
    ("-", "tryOneTurnLink(r1, c1, r2, c2) : PathInfo"),
    ("-", "tryTwoTurnLink(r1, c1, r2, c2) : PathInfo"),
])

# ====== GameWidget ======
cls("GameWidget", [
    # -- data --
    ("-", "m_board : GameBoard"),
    ("-", "m_tilePixmaps : QVector&lt;QPixmap&gt;"),
    ("-", "m_iconScale : int"),
    ("-", "m_tileSize : double"),
    ("-", "m_offsetX : double"),
    ("-", "m_offsetY : double"),
    ("-", "m_hasSelection : bool"),
    ("-", "m_selectedRow : int"),
    ("-", "m_selectedCol : int"),
    ("-", "m_isAnimating : bool"),
    ("-", "m_animPath : PathInfo"),
    ("-", "m_animColor : QColor"),
    ("-", "m_showingHint : bool"),
    ("-", "m_hintRow1 : int"),
    ("-", "m_hintCol1 : int"),
    ("-", "m_hintRow2 : int"),
    ("-", "m_hintCol2 : int"),
    ("-", "m_hintTimer : QTimer*"),
    ("-", "m_hintFlashCount : int"),
    ("-", "m_idleTimer : QTimer*"),
    ("-", "m_isShuffling : bool"),
    ("-", "m_showingShuffleMsg : bool"),
    ("-", "m_shuffleMsgFrames : int"),
    ("-", "m_comboEffects : QVector&lt;ComboEffect&gt;"),
    ("-", "m_comboTimer : QTimer*"),
    ("-", "m_isPaused : bool"),
    ("-", "MARGIN = 15 : static constexpr int"),
    ("-", "COMBO_FLOAT_FRAMES = 30 : static constexpr int"),
    ("-", "IDLE_HINT_DELAY = 10000 : static constexpr int"),
    # -- nested private struct --
    ("-", "&lt;&lt;struct&gt;&gt; ComboEffect"),
    ("-", "  comboCount : int"),
    ("-", "  remainingFrames : int"),
    ("-", "  startPos : QPointF"),
], [
    # -- public --
    ("+", "GameWidget(rows, cols, tileTypes, copiesPerType, iconScale, parent)"),
    ("+", "~GameWidget()"),
    ("+", "startNewGame() : void"),
    ("+", "showHint() : void"),
    ("+", "setIconScale(percent) : void"),
    ("+", "iconScale() : int"),
    ("+", "setPaused(paused) : void"),
    ("+", "isPaused() : bool"),
    ("+", "getScore() : int"),
    ("+", "getMoves() : int"),
    ("+", "getComboCount() : int"),
    ("+", "getRemainingTiles() : int"),
    ("+", "boardRows() : int"),
    ("+", "boardCols() : int"),
    ("+", "boardTileTypes() : int"),
    ("+", "boardCopiesPerType() : int"),
    ("+", "serializeBoard() : QVector&lt;int&gt;"),
    ("+", "deserializeBoard(data) : void"),
    ("+", "setBoardScore(s) : void"),
    ("+", "setBoardMoves(m) : void"),
    ("+", "setBoardRemainingTiles(r) : void"),
    ("+", "setBoardComboCount(c) : void"),
    ("+", "computeLayout() : void"),
    ("+", "clearHintTimer() : void"),
    # -- protected --
    ("#", "paintEvent(event) : void"),
    ("#", "mousePressEvent(event) : void"),
    ("#", "resizeEvent(event) : void"),
    # -- private --
    ("-", "drawBackground(painter) : void"),
    ("-", "drawTile(painter, row, col, type) : void"),
    ("-", "drawSelection(painter, row, col) : void"),
    ("-", "drawConnectionPath(painter) : void"),
    ("-", "drawHintHighlight(painter) : void"),
    ("-", "drawShuffleMessage(painter) : void"),
    ("-", "drawComboEffects(painter) : void"),
    ("-", "drawPauseOverlay(painter) : void"),
    ("-", "tileRect(row, col) : QRectF"),
    ("-", "tileCenter(row, col) : QPointF"),
    ("-", "hitTest(pos, &amp;outRow, &amp;outCol) : int"),
    ("-", "tryMatch(row, col) : void"),
    ("-", "executeMatch(path) : void"),
    ("-", "finishMatch() : void"),
    ("-", "checkGameState() : void"),
    ("-", "shuffleBoard() : void"),
    ("-", "loadTileImages() : void"),
    # -- signals --
    ("~", "scoreChanged(newScore)"),
    ("~", "tilesRemainingChanged(remaining)"),
    ("~", "moveCountChanged(moves)"),
    ("~", "gameWon()"),
    ("~", "noMovesLeft()"),
    ("~", "comboCountChanged(comboCount)"),
])

# ====== MainWindow ======
cls("MainWindow", [
    ("-", "ui : Ui::MainWindowClass"),
    ("-", "m_stack : QStackedWidget*"),
    ("-", "m_startWidget : StartWidget*"),
    ("-", "m_gameWidget : GameWidget*"),
    ("-", "m_scoreLabel : QLabel*"),
    ("-", "m_timerLabel : QLabel*"),
    ("-", "m_remainingLabel : QLabel*"),
    ("-", "m_movesLabel : QLabel*"),
    ("-", "m_comboLabel : QLabel*"),
    ("-", "m_gameTimer : QTimer*"),
    ("-", "m_elapsedSeconds : int"),
    ("-", "m_bgMusic : QMediaPlayer*"),
    ("-", "m_audioOutput : QAudioOutput*"),
    ("-", "m_matchSound : QMediaPlayer*"),
    ("-", "m_winSound : QMediaPlayer*"),
    ("-", "m_hintSound : QMediaPlayer*"),
    ("-", "m_settings : GameSettings"),
    ("-", "m_isRestoring : bool"),
    ("-", "m_isPaused : bool"),
], [
    ("+", "MainWindow(parent)"),
    ("+", "~MainWindow()"),
    ("#", "closeEvent(event) : void"),
    ("-", "onContinueGame() : void"),
    ("-", "onNewGame() : void"),
    ("-", "onOpenSettings() : void"),
    ("-", "onReturnToMenu() : void"),
    ("-", "onHint() : void"),
    ("-", "onPause() : void"),
    ("-", "onTimerTick() : void"),
    ("-", "onGameWon() : void"),
    ("-", "onNoMovesLeft() : void"),
    ("-", "onComboChanged(comboCount) : void"),
    ("-", "createStatusBar() : void"),
    ("-", "connectGameSignals() : void"),
    ("-", "initMusic() : void"),
    ("-", "initSoundEffects() : void"),
    ("-", "findAudioFile(filename) : QString"),
    ("-", "applySettings() : void"),
    ("-", "saveGameState() : void"),
    ("-", "clearSavedGame() : void"),
    ("-", "pauseGame() : void"),
    ("-", "resumeGame() : void"),
])

# ====== StartWidget ======
cls("StartWidget", [
    ("-", "m_titleLabel : QLabel*"),
    ("-", "m_highScoreLabel : QLabel*"),
    ("-", "m_continueBtn : QPushButton*"),
    ("-", "m_newGameBtn : QPushButton*"),
    ("-", "m_settingsBtn : QPushButton*"),
], [
    ("+", "StartWidget(parent)"),
    ("+", "refreshContinueButton() : void"),
    ("~", "continueGame()"),
    ("~", "newGame()"),
    ("~", "openSettings()"),
])

# ====== SettingsDialog ======
cls("SettingsDialog", [
    ("-", "m_volumeSlider : QSlider*"),
    ("-", "m_volumeLabel : QLabel*"),
    ("-", "m_iconSizeCombo : QComboBox*"),
    ("-", "m_difficultyCombo : QComboBox*"),
    ("-", "m_difficultyHint : QLabel*"),
], [
    ("+", "SettingsDialog(current, parent)"),
    ("+", "getSettings() : GameSettings"),
    ("-", "onVolumeChanged(value) : void"),
])

# ====== SaveManager ======
cls("SaveManager", [], [
    ("+", "save(state) : static void"),
    ("+", "load() : static SavedGameState"),
    ("+", "clear() : static void"),
    ("+", "hasSavedGame() : static bool"),
])

# ====== Qt parents ======
g.node("QWidget", shape="box", style="filled", fillcolor="#F0F0F0", fontsize="7",
       label="&#171;Qt&#187;\nQWidget", margin="0.06,0.02")
g.node("QMainWindow", shape="box", style="filled", fillcolor="#F0F0F0", fontsize="7",
       label="&#171;Qt&#187;\nQMainWindow", margin="0.06,0.02")
g.node("QDialog", shape="box", style="filled", fillcolor="#F0F0F0", fontsize="7",
       label="&#171;Qt&#187;\nQDialog", margin="0.06,0.02")

# ====== ranks (TB: source=top, sink=bottom) ======
with g.subgraph() as s:
    s.attr(rank="min")
    s.node("QWidget"); s.node("QMainWindow"); s.node("QDialog")

with g.subgraph() as s:
    s.attr(rank="same")
    s.node("GameBoard"); s.node("GameWidget"); s.node("MainWindow")
    s.node("StartWidget"); s.node("SettingsDialog")

with g.subgraph() as s:
    s.attr(rank="max")
    s.node("PathInfo"); s.node("DifficultyParams"); s.node("GameSettings")
    s.node("SavedGameState"); s.node("Difficulty"); s.node("SaveManager")

# ====== edges ======
# inheritance
for child, parent in [("GameWidget", "QWidget"), ("StartWidget", "QWidget"),
                       ("MainWindow", "QMainWindow"), ("SettingsDialog", "QDialog")]:
    g.edge(parent, child, arrowhead="empty", style="solid",
           penwidth="1.2", color="#555", arrowsize="0.5")

# composition
for whole, part in [("GameWidget", "GameBoard"),
                    ("MainWindow", "GameWidget"),
                    ("MainWindow", "StartWidget")]:
    g.edge(whole, part, arrowhead="diamond", style="solid",
           penwidth="1.0", color="#666", arrowsize="0.5")

# dependency
deps = [
    ("GameBoard", "PathInfo"),
    ("MainWindow", "GameSettings"),
    ("SettingsDialog", "GameSettings"),
    ("SaveManager", "SavedGameState"),
    ("SavedGameState", "Difficulty"),
    ("GameSettings", "Difficulty"),
]
for src, dst in deps:
    g.edge(src, dst, arrowhead="open", style="dashed",
           penwidth="0.7", color="#999", arrowsize="0.4")

out = os.path.join(os.path.dirname(__file__) or ".", "连连看类图")
g.render(out, cleanup=True)
print("OK")
