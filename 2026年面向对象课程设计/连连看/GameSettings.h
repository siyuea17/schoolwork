#pragma once
// ============================================================================
// 文件：GameSettings.h
// 角色：游戏设置的"数据容器"——存储所有可调参数和游戏存档
//
// 存储位置：exe 同目录下的 settings.ini 和 save.dat 文件
//   不用注册表——不用担心搞乱系统，删了文件就还原所有设置。
//
// INI 格式简单易读，用文本编辑器就能打开看。
// save.dat 是 JSON 格式，存棋盘数据。
// ============================================================================

#include <QSettings>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDir>

// ============================================================================
// 难度枚举
// ============================================================================
enum class Difficulty {
    Easy = 0,      // 简单：10种图案×8个副本=80方块，类型少容易配对
    Normal = 1,    // 中等：20种图案×4个副本=80方块
    Hard = 2       // 困难：40种图案×2个副本=80方块，类型多难配对
};

// ============================================================================
// 将难度转为可读字符串
// ============================================================================
inline QString difficultyName(Difficulty d) {
    switch (d) {
        case Difficulty::Easy:   return QString::fromUtf8("简单");
        case Difficulty::Normal: return QString::fromUtf8("中等");
        case Difficulty::Hard:   return QString::fromUtf8("困难");
    }
    return QString::fromUtf8("中等");
}

// ============================================================================
// 每种难度对应的方块参数
// ============================================================================
struct DifficultyParams {
    int tileTypes;   // 图案种类数
    int copies;      // 每种图案的副本数
};

inline DifficultyParams paramsForDifficulty(Difficulty d) {
    switch (d) {
        case Difficulty::Easy:   return {10, 8};   // 10种×8个=80
        case Difficulty::Normal: return {20, 4};   // 20种×4个=80
        case Difficulty::Hard:   return {40, 2};   // 40种×2个=80
    }
    return {20, 4};
}

// ============================================================================
// 统一的配置文件路径
//
// 所有配置文件都放在 exe 同级目录，和程序一起移动不丢失。
// QCoreApplication::applicationDirPath() = exe 所在文件夹
// ============================================================================
inline QString settingsFilePath() {
    return QCoreApplication::applicationDirPath() + "/settings.ini";
}

inline QString saveFilePath() {
    return QCoreApplication::applicationDirPath() + "/save.dat";
}

// ============================================================================
// GameSettings —— 游戏设置结构体
//
// 存到 settings.ini（INI格式），用文本编辑器就能打开修改。
// ============================================================================
struct GameSettings {
    int volume = 70;          // 音量 0~100，默认70
    int iconScale = 100;      // 图标缩放百分比：80=小, 100=正常, 120=大
    Difficulty difficulty = Difficulty::Normal;  // 下把游戏难度
    int highScore = 0;        // 历史最高分

    // ---------- 持久化 ----------

    // 保存到 exe 同目录的 settings.ini
    void save() const {
        QSettings s(settingsFilePath(), QSettings::IniFormat);
        s.beginGroup("Settings");
        s.setValue("volume", volume);
        s.setValue("iconScale", iconScale);
        s.setValue("difficulty", static_cast<int>(difficulty));
        s.setValue("highScore", highScore);
        s.endGroup();
    }

    // 从 settings.ini 读取
    void load() {
        QSettings s(settingsFilePath(), QSettings::IniFormat);
        s.beginGroup("Settings");
        volume = s.value("volume", 70).toInt();
        iconScale = s.value("iconScale", 100).toInt();
        difficulty = static_cast<Difficulty>(
            s.value("difficulty", 1).toInt());
        highScore = s.value("highScore", 0).toInt();
        s.endGroup();
    }
};

// ============================================================================
// 游戏存档数据结构（用于"继续上一次"功能）
// ============================================================================
struct SavedGameState {
    bool hasSaved = false;    // 是否有存档
    int rows = 8;             // 棋盘行数
    int cols = 10;            // 棋盘列数
    int tileTypes = 20;       // 图案种类
    int copies = 4;           // 每类副本
    int score = 0;            // 分数
    int moves = 0;            // 步数
    int remainingTiles = 0;   // 剩余方块
    int comboCount = 0;       // 当前连击数
    int elapsedSeconds = 0;   // 已用时间
    Difficulty difficulty = Difficulty::Normal;

    // 棋盘数据序列化：把所有格子的类型按行拼接成一个整数列表
    QVector<int> gridData;
};

// ============================================================================
// 存档管理器 —— 负责存档的保存、加载、检查
//
// 存档格式：JSON，存到 exe 同目录的 save.dat
//   这样即使删了 exe 重新编译，只要输出目录不变，存档还在。
//
// 如果不想保留存档，直接删掉 save.dat 就行。
// ============================================================================
class SaveManager {
public:
    // 保存游戏状态到 save.dat（JSON格式）
    static void save(const SavedGameState& state) {
        QJsonObject obj;
        obj["hasSaved"] = true;
        obj["rows"] = state.rows;
        obj["cols"] = state.cols;
        obj["tileTypes"] = state.tileTypes;
        obj["copies"] = state.copies;
        obj["score"] = state.score;
        obj["moves"] = state.moves;
        obj["remainingTiles"] = state.remainingTiles;
        obj["comboCount"] = state.comboCount;
        obj["elapsedSeconds"] = state.elapsedSeconds;
        obj["difficulty"] = static_cast<int>(state.difficulty);

        QJsonArray gridArr;
        for (int v : state.gridData)
            gridArr.append(v);
        obj["gridData"] = gridArr;

        QJsonDocument doc(obj);
        QFile file(saveFilePath());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
        }
    }

    // 从 save.dat 加载游戏状态
    static SavedGameState load() {
        SavedGameState state;

        QFile file(saveFilePath());
        if (!file.open(QIODevice::ReadOnly))
            return state;

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!doc.isObject()) return state;

        QJsonObject obj = doc.object();
        state.hasSaved = obj["hasSaved"].toBool(false);
        if (!state.hasSaved) return state;

        state.rows = obj["rows"].toInt(8);
        state.cols = obj["cols"].toInt(10);
        state.tileTypes = obj["tileTypes"].toInt(20);
        state.copies = obj["copies"].toInt(4);
        state.score = obj["score"].toInt(0);
        state.moves = obj["moves"].toInt(0);
        state.remainingTiles = obj["remainingTiles"].toInt(0);
        state.comboCount = obj["comboCount"].toInt(0);
        state.elapsedSeconds = obj["elapsedSeconds"].toInt(0);
        state.difficulty = static_cast<Difficulty>(
            obj["difficulty"].toInt(1));

        QJsonArray gridArr = obj["gridData"].toArray();
        for (const QJsonValue& v : gridArr)
            state.gridData.append(v.toInt());

        return state;
    }

    // 删除存档文件
    static void clear() {
        QFile::remove(saveFilePath());
    }

    // 是否有存档
    static bool hasSavedGame() {
        return QFile::exists(saveFilePath());
    }
};
