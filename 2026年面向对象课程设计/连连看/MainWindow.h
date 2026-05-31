#pragma once

#include <QtWidgets/QMainWindow>
#include <QLabel>
#include <QTimer>
#include "ui_MainWindow.h"

class GameWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onNewGame();
    void onHint();
    void onTimerTick();
    void onGameWon();
    void onNoMovesLeft();

private:
    Ui::MainWindowClass ui;

    GameWidget* m_gameWidget;

    QLabel* m_scoreLabel;
    QLabel* m_timerLabel;
    QLabel* m_remainingLabel;
    QLabel* m_movesLabel;

    QTimer* m_gameTimer;
    int m_elapsedSeconds;
};
