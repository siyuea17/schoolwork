#pragma once
// GameSettings.h — 游戏设置与存档，存储到 settings.ini 和 save.dat

#include <QSettings>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDir>

// ---- 难度枚举 ----
enum class Difficulty {
    Easy = 0, Normal = 1, Hard = 2
};

inline QString difficultyName(Difficulty d) {
    switch (d) {
        case Difficulty::Easy:   return QString::fromUtf8("简单");
        case Difficulty::Normal: return QString::fromUtf8("中等");
        case Difficulty::Hard:   return QString::fromUtf8("困难");
    }
    return QString::fromUtf8("中等");
}

// ---- 每种难度对应的棋盘参数 ----
// rows*cols = tileTypes*copies，确保每种图案数量均等配对
struct DifficultyParams {
    int rows, cols, tileTypes, copies;
    int totalTiles() const { return rows * cols; }
};

inline DifficultyParams paramsForDifficulty(Difficulty d) {
    switch (d) {
        case Difficulty::Easy:   return {6, 8,   8,  6};
        case Difficulty::Normal: return {8, 10, 20,  4};
        case Difficulty::Hard:   return {12, 14, 28,  6};
    }
    return {8, 10, 20, 4};
}

// ---- 配置文件路径 ----
inline QString settingsFilePath() {
    return QCoreApplication::applicationDirPath() + "/settings.ini";
}
inline QString saveFilePath() {
    return QCoreApplication::applicationDirPath() + "/save.dat";
}

// ---- GameSettings —— 游戏设置（存到 settings.ini） ----
struct GameSettings {
    int volume = 70;
    int iconScale = 100;
    Difficulty difficulty = Difficulty::Normal;
    int highScore = 0;

    void save() const {
        QSettings s(settingsFilePath(), QSettings::IniFormat);
        s.beginGroup("Settings");
        s.setValue("volume", volume);
        s.setValue("iconScale", iconScale);
        s.setValue("difficulty", static_cast<int>(difficulty));
        s.setValue("highScore", highScore);
        s.endGroup();
    }

    void load() {
        QSettings s(settingsFilePath(), QSettings::IniFormat);
        s.beginGroup("Settings");
        volume = s.value("volume", 70).toInt();
        iconScale = s.value("iconScale", 100).toInt();
        difficulty = static_cast<Difficulty>(s.value("difficulty", 1).toInt());
        highScore = s.value("highScore", 0).toInt();
        s.endGroup();
    }
};

// ---- 游戏存档数据结构（"继续上一次"用） ----
struct SavedGameState {
    bool hasSaved = false;
    int rows = 8, cols = 10;
    int tileTypes = 20, copies = 4;
    int score = 0, moves = 0, remainingTiles = 0, comboCount = 0;
    int elapsedSeconds = 0;
    Difficulty difficulty = Difficulty::Normal;
    QVector<int> gridData;
};

// ---- 存档管理器（save.dat，JSON 格式） ----
class SaveManager {
public:
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
        state.difficulty = static_cast<Difficulty>(obj["difficulty"].toInt(1));

        QJsonArray gridArr = obj["gridData"].toArray();
        for (const QJsonValue& v : gridArr)
            state.gridData.append(v.toInt());

        return state;
    }

    static void clear() { QFile::remove(saveFilePath()); }
    static bool hasSavedGame() { return QFile::exists(saveFilePath()); }
};
