#!/c/Users/siyuea/AppData/Local/Python/pythoncore-3.14-64/python.exe
"""Generate StarUML .mdj class diagram for LianLianKan."""

import json, uuid, os

def id():
    return "AAAAAA" + uuid.uuid4().hex[:16].upper()

def ref(i):
    return {"$ref": i}

# ===== ID pool =====
PID = id()
MODEL_ID = id()
PKG_QT_ID = id()
PKG_APP_ID = id()
PKG_DATA_ID = id()

# Qt base classes
Q_WIDGET = id()
Q_MAINWINDOW = id()
Q_DIALOG = id()

# App classes
GB_ID = id()
GW_ID = id()
MW_ID = id()
SW_ID = id()
SD_ID = id()

# Data types
DIFF_ID = id()
DP_ID = id()
GS_ID = id()
SGS_ID = id()
PI_ID = id()
SM_ID = id()

# ComboEffect nested struct
CE_ID = id()

# Enum literals
EASY_ID = id()
NORMAL_ID = id()
HARD_ID = id()

elems = []

def add(e):
    elems.append(e)
    return e["_id"]

def mkobj(_type, _id, name, _parent, **kw):
    o = {"_type": _type, "_id": _id, "name": name, "_parent": ref(_parent)}
    o.update(kw)
    return o

def mkattr(_id, _parent, name, visibility="private", atype="", isStatic=False):
    o = mkobj("UMLAttribute", _id, name, _parent, visibility=visibility, type=atype, isStatic=isStatic)
    return o

def mkop(_id, _parent, name, visibility="public", rettype="", isStatic=False, params=None):
    o = mkobj("UMLOperation", _id, name, _parent, visibility=visibility, isStatic=isStatic)
    plist = []
    if params:
        for pname, ptype, pdir in params:
            pid = id()
            plist.append(mkobj("UMLParameter", pid, pname or "", _id, type=ptype, direction=pdir))
    o["parameters"] = plist
    if rettype:
        rid = id()
        plist.append(mkobj("UMLParameter", rid, "", _id, type=rettype, direction="return"))
    return o

def mkgen(_id, _parent, source, target):
    return add(mkobj("UMLGeneralization", _id, "", _parent, source=ref(source), target=ref(target)))

def mkassoc(_id, _parent, name, end1_ref, end1_agg, end1_nav, end1_mult,
            end2_ref, end2_agg, end2_nav, end2_mult):
    eid1 = id()
    eid2 = id()
    end1 = mkobj("UMLAssociationEnd", eid1, name, _id,
                  reference=ref(end1_ref), aggregation=end1_agg,
                  navigable=end1_nav, multiplicity=end1_mult)
    end2 = mkobj("UMLAssociationEnd", eid2, name, _id,
                  reference=ref(end2_ref), aggregation=end2_agg,
                  navigable=end2_nav, multiplicity=end2_mult)
    return add(mkobj("UMLAssociation", _id, name, _parent, end1=end1, end2=end2))

def mkdep(_id, _parent, source, target):
    return add(mkobj("UMLDependency", _id, "", _parent, source=ref(source), target=ref(target)))

# ===== Project =====
add(mkobj("Project", PID, "连连看 UML", "", ownedElements=[]))

# ===== Model =====
add(mkobj("UMLModel", MODEL_ID, "Model", PID, ownedElements=[]))

# ===== Packages =====
add(mkobj("UMLPackage", PKG_QT_ID, "Qt Framework", MODEL_ID, ownedElements=[]))
add(mkobj("UMLPackage", PKG_APP_ID, "Application", MODEL_ID, ownedElements=[]))
add(mkobj("UMLPackage", PKG_DATA_ID, "Data Types", MODEL_ID, ownedElements=[]))

# ===== Qt parents =====
add(mkobj("UMLClass", Q_WIDGET, "QWidget", PKG_QT_ID, visibility="public", isAbstract="true"))
add(mkobj("UMLClass", Q_MAINWINDOW, "QMainWindow", PKG_QT_ID, visibility="public"))
add(mkobj("UMLClass", Q_DIALOG, "QDialog", PKG_QT_ID, visibility="public"))

# ===== Difficulty enum =====
def add_enum_literal(parent_id, lid, name):
    return add(mkobj("UMLEnumerationLiteral", lid, name, parent_id))

add(mkobj("UMLEnumeration", DIFF_ID, "Difficulty", PKG_DATA_ID,
          visibility="public", ownedElements=[ref(EASY_ID), ref(NORMAL_ID), ref(HARD_ID)]))
add_enum_literal(DIFF_ID, EASY_ID, "Easy = 0")
add_enum_literal(DIFF_ID, NORMAL_ID, "Normal = 1")
add_enum_literal(DIFF_ID, HARD_ID, "Hard = 2")

# ===== structs =====
def mkstruct(iid, parent, name, attrs, ops=None):
    cid = add(mkobj("UMLClass", iid, name, parent, visibility="public", stereotype="struct"))
    for aname, avis, atyp, astatic in attrs:
        aid = id()
        add(mkattr(aid, iid, aname, avis, atyp, astatic))
    if ops:
        for oname, ovis, oret, ostatic, oparams in ops:
            oid = id()
            add(mkop(oid, iid, oname, ovis, oret, ostatic, oparams))
    return cid

mkstruct(DP_ID, PKG_DATA_ID, "DifficultyParams", [
    ("rows", "public", "int", False),
    ("cols", "public", "int", False),
    ("tileTypes", "public", "int", False),
    ("copies", "public", "int", False),
], [
    ("totalTiles", "public", "int", False, []),
])

mkstruct(GS_ID, PKG_DATA_ID, "GameSettings", [
    ("volume", "public", "int", False),
    ("iconScale", "public", "int", False),
    ("difficulty", "public", "Difficulty", False),
    ("highScore", "public", "int", False),
], [
    ("save", "public", "void", False, []),
    ("load", "public", "void", False, []),
])

mkstruct(SGS_ID, PKG_DATA_ID, "SavedGameState", [
    ("hasSaved", "public", "bool", False),
    ("rows", "public", "int", False),
    ("cols", "public", "int", False),
    ("tileTypes", "public", "int", False),
    ("copies", "public", "int", False),
    ("score", "public", "int", False),
    ("moves", "public", "int", False),
    ("remainingTiles", "public", "int", False),
    ("comboCount", "public", "int", False),
    ("elapsedSeconds", "public", "int", False),
    ("difficulty", "public", "Difficulty", False),
    ("gridData", "public", "QVector<int>", False),
])

mkstruct(PI_ID, PKG_DATA_ID, "PathInfo", [
    ("valid", "public", "bool", False),
    ("corners", "public", "QVector<QPoint>", False),
])

# ===== GameBoard =====
add(mkobj("UMLClass", GB_ID, "GameBoard", PKG_APP_ID, visibility="public"))
for aname, avis, atyp, astatic in [
    ("DEFAULT_ROWS", "public", "int", True),
    ("DEFAULT_COLS", "public", "int", True),
    ("DEFAULT_TILE_TYPES", "public", "int", True),
    ("DEFAULT_COPIES", "public", "int", True),
    ("EMPTY", "public", "int", True),
    ("COMBO_WINDOW_MS", "public", "int", True),
    ("m_rows", "private", "int", False),
    ("m_cols", "private", "int", False),
    ("m_tileTypes", "private", "int", False),
    ("m_copiesPerType", "private", "int", False),
    ("m_grid", "private", "QVector<QVector<int>>", False),
    ("m_score", "private", "int", False),
    ("m_remainingTiles", "private", "int", False),
    ("m_moves", "private", "int", False),
    ("m_comboCount", "private", "int", False),
    ("m_lastMatchTimeMs", "private", "qint64", False),
    ("m_rng", "private", "std::mt19937", False),
]:
    aid = id()
    add(mkattr(aid, GB_ID, aname, avis, atyp, astatic))

for oname, ovis, oret, ostatic, oparams in [
    ("GameBoard", "public", "", False, [("rows", "int", "in"), ("cols", "int", "in"), ("tileTypes", "int", "in"), ("copiesPerType", "int", "in")]),
    ("initBoard", "public", "void", False, []),
    ("reset", "public", "void", False, []),
    ("findPath", "public", "PathInfo", False, [("r1", "int", "in"), ("c1", "int", "in"), ("r2", "int", "in"), ("c2", "int", "in")]),
    ("hasValidMoves", "public", "bool", False, []),
    ("findHint", "public", "PathInfo", False, []),
    ("isWin", "public", "bool", False, []),
    ("removeTiles", "public", "void", False, [("r1", "int", "in"), ("c1", "int", "in"), ("r2", "int", "in"), ("c2", "int", "in")]),
    ("shuffle", "public", "void", False, []),
    ("calculateComboScore", "public", "int", False, []),
    ("resetCombo", "public", "void", False, []),
    ("getComboCount", "public", "int", False, []),
    ("getTile", "public", "int", False, [("r", "int", "in"), ("c", "int", "in")]),
    ("setTile", "public", "void", False, [("r", "int", "in"), ("c", "int", "in"), ("type", "int", "in")]),
    ("isEmpty", "public", "bool", False, [("r", "int", "in"), ("c", "int", "in")]),
    ("getScore", "public", "int", False, []),
    ("getRemainingTiles", "public", "int", False, []),
    ("getMoves", "public", "int", False, []),
    ("addScore", "public", "void", False, [("points", "int", "in")]),
    ("rows", "public", "int", False, []),
    ("cols", "public", "int", False, []),
    ("totalRows", "public", "int", False, []),
    ("totalCols", "public", "int", False, []),
    ("tileTypes", "public", "int", False, []),
    ("copiesPerType", "public", "int", False, []),
    ("totalTiles", "public", "int", False, []),
    ("serializeGrid", "public", "QVector<int>", False, []),
    ("deserializeGrid", "public", "void", False, [("data", "QVector<int>", "in")]),
    ("setScore", "public", "void", False, [("s", "int", "in")]),
    ("setMoves", "public", "void", False, [("m", "int", "in")]),
    ("setRemainingTiles", "public", "void", False, [("r", "int", "in")]),
    ("setComboCount", "public", "void", False, [("c", "int", "in")]),
    ("isRowClear", "private", "bool", False, [("r", "int", "in"), ("c1", "int", "in"), ("c2", "int", "in")]),
    ("isColClear", "private", "bool", False, [("r1", "int", "in"), ("r2", "int", "in"), ("c", "int", "in")]),
    ("tryDirectLink", "private", "PathInfo", False, [("r1", "int", "in"), ("c1", "int", "in"), ("r2", "int", "in"), ("c2", "int", "in")]),
    ("tryOneTurnLink", "private", "PathInfo", False, [("r1", "int", "in"), ("c1", "int", "in"), ("r2", "int", "in"), ("c2", "int", "in")]),
    ("tryTwoTurnLink", "private", "PathInfo", False, [("r1", "int", "in"), ("c1", "int", "in"), ("r2", "int", "in"), ("c2", "int", "in")]),
]:
    oid = id()
    add(mkop(oid, GB_ID, oname, ovis, oret, ostatic, oparams))

# ===== ComboEffect nested struct (owned by GameWidget) =====
add(mkobj("UMLClass", CE_ID, "ComboEffect", "", visibility="private", stereotype="struct"))
add(mkattr(id(), CE_ID, "comboCount", "private", "int"))
add(mkattr(id(), CE_ID, "remainingFrames", "private", "int"))
add(mkattr(id(), CE_ID, "startPos", "private", "QPointF"))

# ===== GameWidget =====
add(mkobj("UMLClass", GW_ID, "GameWidget", PKG_APP_ID, visibility="public",
          ownedElements=[ref(CE_ID)]))
for aname, avis, atyp, astatic in [
    ("m_board", "private", "GameBoard", False),
    ("m_tilePixmaps", "private", "QVector<QPixmap>", False),
    ("m_iconScale", "private", "int", False),
    ("m_tileSize", "private", "double", False),
    ("m_offsetX", "private", "double", False),
    ("m_offsetY", "private", "double", False),
    ("m_hasSelection", "private", "bool", False),
    ("m_selectedRow", "private", "int", False),
    ("m_selectedCol", "private", "int", False),
    ("m_isAnimating", "private", "bool", False),
    ("m_animPath", "private", "PathInfo", False),
    ("m_animColor", "private", "QColor", False),
    ("m_showingHint", "private", "bool", False),
    ("m_hintRow1", "private", "int", False),
    ("m_hintCol1", "private", "int", False),
    ("m_hintRow2", "private", "int", False),
    ("m_hintCol2", "private", "int", False),
    ("m_hintTimer", "private", "QTimer*", False),
    ("m_hintFlashCount", "private", "int", False),
    ("m_idleTimer", "private", "QTimer*", False),
    ("m_isShuffling", "private", "bool", False),
    ("m_showingShuffleMsg", "private", "bool", False),
    ("m_shuffleMsgFrames", "private", "int", False),
    ("m_comboEffects", "private", "QVector<ComboEffect>", False),
    ("m_comboTimer", "private", "QTimer*", False),
    ("m_isPaused", "private", "bool", False),
    ("MARGIN", "private", "int", True),
    ("COMBO_FLOAT_FRAMES", "private", "int", True),
    ("IDLE_HINT_DELAY", "private", "int", True),
]:
    add(mkattr(id(), GW_ID, aname, avis, atyp, astatic))

gw_ops = [
    ("GameWidget", "public", "", False, [("rows", "int", "in"), ("cols", "int", "in"), ("tileTypes", "int", "in"), ("copiesPerType", "int", "in"), ("iconScale", "int", "in"), ("parent", "QWidget*", "in")]),
    ("~GameWidget", "public", "", False, []),
    ("startNewGame", "public", "void", False, []),
    ("showHint", "public", "void", False, []),
    ("setIconScale", "public", "void", False, [("percent", "int", "in")]),
    ("iconScale", "public", "int", False, []),
    ("setPaused", "public", "void", False, [("paused", "bool", "in")]),
    ("isPaused", "public", "bool", False, []),
    ("getScore", "public", "int", False, []),
    ("getMoves", "public", "int", False, []),
    ("getComboCount", "public", "int", False, []),
    ("getRemainingTiles", "public", "int", False, []),
    ("boardRows", "public", "int", False, []),
    ("boardCols", "public", "int", False, []),
    ("boardTileTypes", "public", "int", False, []),
    ("boardCopiesPerType", "public", "int", False, []),
    ("serializeBoard", "public", "QVector<int>", False, []),
    ("deserializeBoard", "public", "void", False, [("data", "QVector<int>", "in")]),
    ("setBoardScore", "public", "void", False, [("s", "int", "in")]),
    ("setBoardMoves", "public", "void", False, [("m", "int", "in")]),
    ("setBoardRemainingTiles", "public", "void", False, [("r", "int", "in")]),
    ("setBoardComboCount", "public", "void", False, [("c", "int", "in")]),
    ("computeLayout", "public", "void", False, []),
    ("clearHintTimer", "public", "void", False, []),
    ("paintEvent", "protected", "void", False, [("event", "QPaintEvent*", "in")]),
    ("mousePressEvent", "protected", "void", False, [("event", "QMouseEvent*", "in")]),
    ("resizeEvent", "protected", "void", False, [("event", "QResizeEvent*", "in")]),
    ("drawBackground", "private", "void", False, [("painter", "QPainter&", "in")]),
    ("drawTile", "private", "void", False, [("painter", "QPainter&", "in"), ("row", "int", "in"), ("col", "int", "in"), ("type", "int", "in")]),
    ("drawSelection", "private", "void", False, [("painter", "QPainter&", "in"), ("row", "int", "in"), ("col", "int", "in")]),
    ("drawConnectionPath", "private", "void", False, [("painter", "QPainter&", "in")]),
    ("drawHintHighlight", "private", "void", False, [("painter", "QPainter&", "in")]),
    ("drawShuffleMessage", "private", "void", False, [("painter", "QPainter&", "in")]),
    ("drawComboEffects", "private", "void", False, [("painter", "QPainter&", "in")]),
    ("drawPauseOverlay", "private", "void", False, [("painter", "QPainter&", "in")]),
    ("tileRect", "private", "QRectF", False, [("row", "int", "in"), ("col", "int", "in")]),
    ("tileCenter", "private", "QPointF", False, [("row", "int", "in"), ("col", "int", "in")]),
    ("hitTest", "private", "int", False, [("pos", "QPoint&", "in"), ("outRow", "int&", "inout"), ("outCol", "int&", "inout")]),
    ("tryMatch", "private", "void", False, [("row", "int", "in"), ("col", "int", "in")]),
    ("executeMatch", "private", "void", False, [("path", "PathInfo", "in")]),
    ("finishMatch", "private", "void", False, []),
    ("checkGameState", "private", "void", False, []),
    ("shuffleBoard", "private", "void", False, []),
    ("loadTileImages", "private", "void", False, []),
    ("scoreChanged", "public", "", False, [("newScore", "int", "in")]),
    ("tilesRemainingChanged", "public", "", False, [("remaining", "int", "in")]),
    ("moveCountChanged", "public", "", False, [("moves", "int", "in")]),
    ("gameWon", "public", "", False, []),
    ("noMovesLeft", "public", "", False, []),
    ("comboCountChanged", "public", "", False, [("comboCount", "int", "in")]),
]
for oname, ovis, oret, ostatic, oparams in gw_ops:
    add(mkop(id(), GW_ID, oname, ovis, oret, ostatic, oparams))

# ===== MainWindow =====
add(mkobj("UMLClass", MW_ID, "MainWindow", PKG_APP_ID, visibility="public"))
for aname, avis, atyp, astatic in [
    ("ui", "private", "Ui::MainWindowClass", False),
    ("m_stack", "private", "QStackedWidget*", False),
    ("m_startWidget", "private", "StartWidget*", False),
    ("m_gameWidget", "private", "GameWidget*", False),
    ("m_scoreLabel", "private", "QLabel*", False),
    ("m_timerLabel", "private", "QLabel*", False),
    ("m_remainingLabel", "private", "QLabel*", False),
    ("m_movesLabel", "private", "QLabel*", False),
    ("m_comboLabel", "private", "QLabel*", False),
    ("m_gameTimer", "private", "QTimer*", False),
    ("m_elapsedSeconds", "private", "int", False),
    ("m_bgMusic", "private", "QMediaPlayer*", False),
    ("m_audioOutput", "private", "QAudioOutput*", False),
    ("m_matchSound", "private", "QMediaPlayer*", False),
    ("m_winSound", "private", "QMediaPlayer*", False),
    ("m_hintSound", "private", "QMediaPlayer*", False),
    ("m_settings", "private", "GameSettings", False),
    ("m_isRestoring", "private", "bool", False),
    ("m_isPaused", "private", "bool", False),
]:
    add(mkattr(id(), MW_ID, aname, avis, atyp, astatic))

for oname, ovis, oret, ostatic, oparams in [
    ("MainWindow", "public", "", False, [("parent", "QWidget*", "in")]),
    ("~MainWindow", "public", "", False, []),
    ("closeEvent", "protected", "void", False, [("event", "QCloseEvent*", "in")]),
    ("onContinueGame", "private", "void", False, []),
    ("onNewGame", "private", "void", False, []),
    ("onOpenSettings", "private", "void", False, []),
    ("onReturnToMenu", "private", "void", False, []),
    ("onHint", "private", "void", False, []),
    ("onPause", "private", "void", False, []),
    ("onTimerTick", "private", "void", False, []),
    ("onGameWon", "private", "void", False, []),
    ("onNoMovesLeft", "private", "void", False, []),
    ("onComboChanged", "private", "void", False, [("comboCount", "int", "in")]),
    ("createStatusBar", "private", "void", False, []),
    ("connectGameSignals", "private", "void", False, []),
    ("initMusic", "private", "void", False, []),
    ("initSoundEffects", "private", "void", False, []),
    ("findAudioFile", "private", "QString", False, [("filename", "QString", "in")]),
    ("applySettings", "private", "void", False, []),
    ("saveGameState", "private", "void", False, []),
    ("clearSavedGame", "private", "void", False, []),
    ("pauseGame", "private", "void", False, []),
    ("resumeGame", "private", "void", False, []),
]:
    add(mkop(id(), MW_ID, oname, ovis, oret, ostatic, oparams))

# ===== StartWidget =====
add(mkobj("UMLClass", SW_ID, "StartWidget", PKG_APP_ID, visibility="public"))
for aname, avis, atyp, astatic in [
    ("m_titleLabel", "private", "QLabel*", False),
    ("m_highScoreLabel", "private", "QLabel*", False),
    ("m_continueBtn", "private", "QPushButton*", False),
    ("m_newGameBtn", "private", "QPushButton*", False),
    ("m_settingsBtn", "private", "QPushButton*", False),
]:
    add(mkattr(id(), SW_ID, aname, avis, atyp, astatic))

for oname, ovis, oret, ostatic, oparams in [
    ("StartWidget", "public", "", False, [("parent", "QWidget*", "in")]),
    ("refreshContinueButton", "public", "void", False, []),
    ("continueGame", "public", "", False, []),
    ("newGame", "public", "", False, []),
    ("openSettings", "public", "", False, []),
]:
    add(mkop(id(), SW_ID, oname, ovis, oret, ostatic, oparams))

# ===== SettingsDialog =====
add(mkobj("UMLClass", SD_ID, "SettingsDialog", PKG_APP_ID, visibility="public"))
for aname, avis, atyp, astatic in [
    ("m_volumeSlider", "private", "QSlider*", False),
    ("m_volumeLabel", "private", "QLabel*", False),
    ("m_iconSizeCombo", "private", "QComboBox*", False),
    ("m_difficultyCombo", "private", "QComboBox*", False),
    ("m_difficultyHint", "private", "QLabel*", False),
]:
    add(mkattr(id(), SD_ID, aname, avis, atyp, astatic))

for oname, ovis, oret, ostatic, oparams in [
    ("SettingsDialog", "public", "", False, [("current", "GameSettings", "in"), ("parent", "QWidget*", "in")]),
    ("getSettings", "public", "GameSettings", False, []),
    ("onVolumeChanged", "private", "void", False, [("value", "int", "in")]),
]:
    add(mkop(id(), SD_ID, oname, ovis, oret, ostatic, oparams))

# ===== SaveManager =====
add(mkobj("UMLClass", SM_ID, "SaveManager", PKG_DATA_ID, visibility="public", isAbstract="true"))
for oname, ovis, oret, ostatic, oparams in [
    ("save", "public", "void", True, [("state", "SavedGameState", "in")]),
    ("load", "public", "SavedGameState", True, []),
    ("clear", "public", "void", True, []),
    ("hasSavedGame", "public", "bool", True, []),
]:
    add(mkop(id(), SM_ID, oname, ovis, oret, ostatic, oparams))

# ===== Relationships =====
# Inheritance: child -> parent
mkgen(id(), MODEL_ID, GW_ID, Q_WIDGET)
mkgen(id(), MODEL_ID, SW_ID, Q_WIDGET)
mkgen(id(), MODEL_ID, MW_ID, Q_MAINWINDOW)
mkgen(id(), MODEL_ID, SD_ID, Q_DIALOG)

# Composition: whole -> part  (composite on end1 pointing to part)
mkassoc(id(), MODEL_ID, "", GB_ID, "composite", False, "1", GW_ID, "none", True, "1")
mkassoc(id(), MODEL_ID, "", GW_ID, "composite", False, "1", MW_ID, "none", True, "1")
mkassoc(id(), MODEL_ID, "", SW_ID, "composite", False, "1", MW_ID, "none", True, "1")

# Dependency: client -> supplier
mkdep(id(), MODEL_ID, GB_ID, PI_ID)
mkdep(id(), MODEL_ID, MW_ID, GS_ID)
mkdep(id(), MODEL_ID, SD_ID, GS_ID)
mkdep(id(), MODEL_ID, SM_ID, SGS_ID)
mkdep(id(), MODEL_ID, SGS_ID, DIFF_ID)
mkdep(id(), MODEL_ID, GS_ID, DIFF_ID)

# ===== Assemble project =====
project = elems[0]
project["ownedElements"] = [ref(MODEL_ID)]

# Build parent-child ownedElements
parent_map = {}
for e in elems[1:]:  # skip project
    prent_ref = e.get("_parent", {}).get("$ref", "")
    if prent_ref not in parent_map:
        parent_map[prent_ref] = []
    parent_map[prent_ref].append(e["_id"])

# Recurse: assign ownedElements from parent_map
for e in elems:
    eid = e["_id"]
    if eid in parent_map:
        cur = e.get("ownedElements", [])
        # Remove any existing refs already there (like enum literals)
        existing_ids = {x.get("$ref", "") for x in cur}
        for child_id in parent_map[eid]:
            if child_id not in existing_ids:
                cur.append(ref(child_id))
        e["ownedElements"] = cur

out_path = os.path.join(os.path.dirname(__file__) or ".", "连连看类图.mdj")
with open(out_path, "w", encoding="utf-8") as f:
    json.dump(project, f, ensure_ascii=False, indent=2)
print(f"MDJ saved: {out_path}")
