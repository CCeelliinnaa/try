// mainwindow.cpp
#include "mainwindow.h"
#include <QPainter>
#include <QTimer>
#include <QMessageBox>
#include <QMouseEvent>
#include "ui_mainwindow.h"
#include <QPixmap>
//mainwindow 类是整个游戏的核心
//它负责管理游戏的整体逻辑，包括游戏的初始化、定时器事件处理、鼠标点击事件处理以及游戏界面的绘制等。

// 主窗口类的构造函数实现
mainwindow::mainwindow(QWidget *parent)
    : QWidget(parent), currentWave(0), totalWaves(10), difficulty(1), elapsedTime(0), isGameOver(false), showButtons(false), gameSpeed(1),
      selectedTower(nullptr), selectedObstacle(nullptr), isPaused(false), countdownValue(4),isVictory(false)
{
    ui = new Ui::mainwindow;
    ui->setupUi(this);
    this->setWindowTitle("Carrot_Defence保卫萝卜");  // 窗口标题设置
    this->setFixedSize(1200, 800); // 设置窗口固定大小为 1200x800
    // 加载初始界面背景图片
    // 在某个成员函数中（例如构造函数或初始化函数）
    bgmPlayer = new QMediaPlayer(this);
    QAudioOutput *audioOutput = new QAudioOutput(this);
    bgmPlayer->setAudioOutput(audioOutput);

    bgmPlayer->setSource(QUrl("qrc:/sounds/sounds/background_music.mp3"));  // Qt6 使用 setSource
    audioOutput->setVolume(0.01);  // 对应原 Qt5 中的 setVolume(1)（1% 音量）

    bgmPlayer->play();

    // 手动实现循环播放（Qt6 也可以使用 setLoops，但为了通用性保留手动方式）
    connect(bgmPlayer, &QMediaPlayer::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState state) {
                if (state == QMediaPlayer::StoppedState) {
                    bgmPlayer->play();
                }
            });
    // 加载游戏地图图片
    mapPixmap.load(":/images/images/Background2.png");
    setFixedSize(1200, 800);
    // 初始化定时器 ID，未点击开始游戏，计时器为无效状态
    waveTimerId = -1;
    // 创建萝卜对象
    radish = new Radish(QPoint(750, 600));
    // 加载游戏失败图片
    gameOverPixmap.load(":/images/images/lose2.png");
    // 加载游戏胜利贴图
    winPixmap.load(":/images/images/win2.png");
    winPixmap = winPixmap.scaled(1200,800);
    // 定义确定按钮点击区域（居中，覆盖贴图中的按钮区域）
    confirmButtonRect = QRect(479, 647, 243, 56);
    // 创建金钱管理对象，初始金钱为450
    money = new Money(450);
    // 加载金币图标
    coinPixmap.load(":/images/images/coin.png");

    // 加载各种按钮的正常状态图片
    QPixmap pauseNormalPixmap(":/images/images/pause_normal.png");
    QPixmap pauseHoverPixmap(":/images/images/pause_hover.png"); // 悬停图片加载
    QPixmap doubleSpeedNormalPixmap(":/images/images/double_speed_normal.png");
    QPixmap doubleSpeedHoverPixmap(":/images/images/double_speed_hover.png"); // 悬停图片加载
    QPixmap coffeeButtonPixmap(":/images/images/cannon_button.png");
    QPixmap musicButtonPixmap(":/images/images/poop_button.png");
    QPixmap aiButtonPixmap(":/images/images/star_button.png");
    QPixmap upgradeNormalGrayPixmap(":/images/images/upgrade_normal_gray.png");
    QPixmap upgradeNormalBluePixmap(":/images/images/upgrade_normal_blue.png");
    QPixmap upgradeHoverBluePixmap(":/images/images/upgrade_hover_blue.png");
    QPixmap removeNormalPixmap(":/images/images/remove_normal.png");
    QPixmap removeHoverPixmap(":/images/images/remove_hover.png");
    QPixmap crossPixmap(":/images/images/cross.png");

    // 保持纵横比对按键进行缩放
    QPixmap scaledPausePixmap = pauseNormalPixmap.scaled(60, 60, Qt::KeepAspectRatio);
    QPixmap scaledPauseHoverPixmap = pauseHoverPixmap.scaled(60, 60, Qt::KeepAspectRatio); // 缩放悬停图片
    QPixmap scaledDoubleSpeedPixmap = doubleSpeedNormalPixmap.scaled(120, 120, Qt::KeepAspectRatio);
    QPixmap scaledDoubleSpeedHoverPixmap = doubleSpeedHoverPixmap.scaled(120, 120, Qt::KeepAspectRatio); // 缩放悬停图片

    // 创建暂停和二倍速按钮
    buttons.append(new Button(QPoint(1000, 10), scaledPausePixmap, scaledPauseHoverPixmap, Button::PAUSE));
    buttons.append(new Button(QPoint(800, 10), scaledDoubleSpeedPixmap, scaledDoubleSpeedHoverPixmap, Button::DOUBLE_SPEED));

    // 初始化炮塔坑
    towerPits = TowerPit::createTowerPits();
    // 定义要放置障碍物的炮塔坑序号
    QList<int> targetIndices = {9, 15, 17,19,20,22,25, 33, 34,38};

    // 初始化障碍物
    QString obstaclePaths[10] = {
        ":/images/images/buildings_01.png",
        ":/images/images/obstacle2.png",
        ":/images/images/obstacle3.png",
        ":/images/images/obstacle4.png",
        ":/images/images/obstacle5.png",
        ":/images/images/obstacle6.png",
        ":/images/images/buildings_03.png",
        ":/images/images/bulidings_02.png",
        ":/images/images/obstacle9.png",
        ":/images/images/buildings_04.png"
    };
    // 定义不同障碍物的大小
        QList<QSize> obstacleSizes = {
            QSize(180, 180),//obstacle1
            QSize(100, 100),//obstacle2
            QSize(100, 100),//obstacle3
            QSize(100, 100),//obstacle4
            QSize(100, 100),//obstacle5
            QSize(100, 100),//obstacle6
            QSize(180, 180),//obstacle7
            QSize(180, 180),//obstacle8
            QSize(100, 100),//obstacle9
            QSize(180, 180)//obstacle10
        };

        int pathIndex = 0;
        for (int index : targetIndices) {
            for (TowerPit* pit : towerPits) {
                if (pit->getIndex() == index) {
                    QSize size = obstacleSizes[pathIndex];
                    Obstacle* obstacle = new Obstacle(pit->getPosition(), obstaclePaths[pathIndex], size.width(), size.height());
                    obstacles.append(obstacle);
                    pit->setObstacle(obstacle);
                    pathIndex++;
                    break;
                }
            }
        }

    startWindow = new StartWindow(this);  // 创建 StartWindow 对象
    startWindow->show();                  // 显示 StartWindow

    difficultyWindow = new DifficultyWindow(this); // 创建 DifficultyWindow 对象
    difficultyWindow->hide();

    connect(startWindow, &StartWindow::startGame, this, &mainwindow::onStartGame); // 连接开始游戏信号
    connect(difficultyWindow, &DifficultyWindow::difficultySelected, this, &mainwindow::onDifficultySelected); // 连接难度选择信号
    connect(startWindow, &StartWindow::showRules, this, &mainwindow::onShowRules); // 连接显示规则信号

    countdownLabel = new QLabel(this);    // 创建倒计时标签
    countdownLabel->setGeometry(500, 300, 200, 200); // 设置标签位置和大小
    countdownLabel->hide();               // 隐藏标签
    countdownLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    countdownTimer = new QTimer(this);    // 创建倒计时定时器
    connect(countdownTimer, &QTimer::timeout, this, &mainwindow::countdown); // 连接定时器超时信号
    // 加载倒计时贴图
    countdownPixmaps[0].load(":/images/images/3.png"); // 替换为实际的 3 的贴图路径
    countdownPixmaps[1].load(":/images/images/2.png"); // 替换为实际的 2 的贴图路径
    countdownPixmaps[2].load(":/images/images/1.png"); // 替换为实际的 1 的贴图路径
    countdownPixmaps[3].load(":/images/images/GO.png"); // 替换为实际的 GO 的贴图路径
    // 定义lose按钮的点击区域
    restartButtonRect = QRect(433, 459, 282, 66);
    returnButtonRect = QRect(434, 559, 280, 70);
}

// 主窗口类的析构函数实现，释放动态分配的资源
mainwindow::~mainwindow()
{
    for (Enemy* enemy : enemies) {
        delete enemy;
    }
    for (Tower* tower : towers) {
        delete tower;
    }
    for (Button* button : buttons) {
        delete button;
    }
    for (Obstacle* obstacle : obstacles) {
        delete obstacle;
    }
    for (TowerPit* pit : towerPits) {
        delete pit;
    }
    delete radish;
    delete money;
    delete ui;
    delete startWindow;
    delete countdownLabel;
    delete countdownTimer;
    delete bgmPlayer;
}

// 处理开始游戏信号的槽函数：显示难度选择
void mainwindow::onStartGame()
{
    startWindow->hide();
    startWindow->setAttribute(Qt::WA_TransparentForMouseEvents);
    difficultyWindow->show();
    difficultyWindow->raise();
}

// 处理难度选择信号的槽函数
void mainwindow::onDifficultySelected(int diff)
{
    difficulty = diff;
    switch (difficulty) {
    case 0: totalWaves = 8; break;   // 简单
    case 1: totalWaves = 10; break;  // 中等
    case 2: totalWaves = 12; break;  // 困难
    }
    difficultyWindow->hide();
    difficultyWindow->setAttribute(Qt::WA_TransparentForMouseEvents);
    countdownValue = 4; // 重置倒计时值
    countdownLabel->show();
    Tower::setGameSpeed(gameSpeed);
    countdownTimer->start(1000); // 每秒触发一次
}

// 处理显示规则信号的槽函数
void mainwindow::onShowRules()
{
    // 显示规则说明文本
    QMessageBox::information(this, "彩蛋", "恭喜你触发隐藏剧情：保送清华！后续内容敬请期待！");
}

// 倒计时槽函数
void mainwindow::countdown()
{
    if (countdownValue > 0) {
        countdownValue--;
        update(); // 触发 paintEvent 重新绘制界面
    } else {
        countdownTimer->stop();
        countdownLabel->hide();
        elapsedTime = 0; // 重置游戏时间
        currentWave = 0; // 重置波数
        waveTimerId = startTimer(100); // 开始游戏定时器
    }
}

// 绘制事件处理函数的实现，用于绘制游戏界面元素
void mainwindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    // 绘制背景图
    painter.drawPixmap(0, 0, backgroundPixmap.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    // 绘制游戏地图
    painter.drawPixmap(0, 0, mapPixmap.scaled(width(), height()));
    if (!startWindow->isVisible()) {
        // 只有在开始窗口隐藏后才绘制游戏元素
        // 绘制游戏地图
        painter.drawPixmap(0, 0, mapPixmap.scaled(width(), height()));
    }
    // 绘制所有敌人
    for (Enemy* enemy : enemies) {
        enemy->draw(&painter);
    }
    //绘制炮塔坑
    QPixmap pitPixmap(":/images/images/tower_pit.png"); // 假设的炮塔坑贴图路径
    for (TowerPit* pit : towerPits) {
        painter.drawPixmap(pit->getPosition(), pitPixmap.scaled(80,80,Qt::KeepAspectRatio));
    }
    // 绘制所有炮塔
    for (Tower* tower : towers) {
        tower->draw(&painter, enemies);
        // 绘制炮塔发射的子弹
        for (Bullet* bullet : tower->getBullets()) {
            bullet->draw(&painter);
        }
    }

    // 绘制萝卜
    radish->draw(&painter);

    // 绘制所有障碍物
    for (Obstacle* obstacle : obstacles) {
        obstacle->draw(&painter);
    }
    // 绘制倒计时贴图
    if (countdownValue > 0 && countdownValue <= 4) {
        QPixmap countdownPixmap = countdownPixmaps[4 - countdownValue];
        painter.drawPixmap((width() - countdownPixmap.width()) / 2, (height() - countdownPixmap.height()) / 2, countdownPixmap);
    }
    // 如果需要显示创建炮塔的按钮，则绘制它们
        if (showButtons) {
            QPixmap coffeeButtonPixmap(":/images/images/button_1.png");
            QPixmap musicButtonPixmap(":/images/images/button_4.png");
            QPixmap aiButtonPixmap(":/images/images/button_2.png");
            QPixmap teacherButtonPixmap(":/images/images/button_3.png");

            int buttonSize = 100;
            int gap = 10;
            int totalWidth = 4 * buttonSize + 3 * gap;
            int totalHeight = buttonSize;

            // 计算按钮起始位置，防止超出窗口边界
            int bx = clickPos.x();
            int by = clickPos.y();
            if (bx + totalWidth > width()) bx = width() - totalWidth;
            if (bx < 0) bx = 0;
            if (by + totalHeight > height()) by = by - totalHeight;
            if (by < 0) by = 0;

            // 绘制咖啡炮塔选择按钮
            painter.drawPixmap(bx, by, coffeeButtonPixmap.scaled(buttonSize, buttonSize));
            // 绘制音乐炮塔选择按钮
            painter.drawPixmap(bx + buttonSize + gap, by, musicButtonPixmap.scaled(buttonSize, buttonSize));
            // 绘制AI炮塔选择按钮
            painter.drawPixmap(bx + 2 * (buttonSize + gap), by, aiButtonPixmap.scaled(buttonSize, buttonSize));
            // 绘制老师炮塔选择按钮
            painter.drawPixmap(bx + 3 * (buttonSize + gap), by, teacherButtonPixmap.scaled(buttonSize, buttonSize));
        }

        // 如果有选中的炮塔，则绘制射程范围、升级和移除按钮
        if (selectedTower) {
            QPixmap upgradePixmap;
            // 根据金币数量选择升级按钮的图片
            if (money->canAfford(selectedTower->getUpgradeCost()) && selectedTower->get_level() == 1) {
                upgradePixmap = QPixmap(":/images/images/upgrade_normal_blue.png").scaled(50,50,Qt::KeepAspectRatio);
            }
            else {
                upgradePixmap = QPixmap(":/images/images/upgrade_normal_gray.png").scaled(50,50,Qt::KeepAspectRatio);
            }
            // 计算升级按钮的位置并绘制
            QPoint upgradePos(selectedTower->getPosition().x(), selectedTower->getPosition().y() - upgradePixmap.height() - 20);
            painter.drawPixmap(upgradePos, upgradePixmap);

            // 加载移除按钮的图片
            QPixmap removePixmap;
            removePixmap=QPixmap(":/images/images/remove_normal.png").scaled(50,50,Qt::KeepAspectRatio);
            // 计算移除按钮的位置并绘制
            QPoint removePos(selectedTower->getPosition().x(), selectedTower->getPosition().y() + 100);
            painter.drawPixmap(removePos, removePixmap);
        }
    // 绘制当前金钱数量
    QFont font;
    font.setFamily("Times New Roman"); // 设置字体家族为Times New Roman
    font.setPointSize(16);   // 设置字体大小为 16 磅
    font.setBold(true);      // 设置字体为粗体
    painter.setFont(font);   // 将字体应用到 QPainter
    painter.setPen(Qt::white);//设置字体颜色
    painter.drawText(130, 50, QString("Money: %1").arg(money->getAmount()));

    // 绘制所有按钮
    for (Button* button : buttons) {
        button->draw(&painter);
    }
    // 绘制波数信息
        QString waveText;
        if (isPaused) {
            waveText = "暂停中";
        } else {//%1 和 %2 是占位符，用于后续填充具体的值
            waveText = QString("%1/%2").arg(currentWave + 1).arg(totalWaves);
        }
        QFont waveFont;
        waveFont.setFamily("Ubuntu");
        waveFont.setPointSize(20);
        waveFont.setBold(true);
        painter.setFont(waveFont);
        QRect waveRect(0, 15, width(), 50);
        painter.drawText(waveRect, Qt::AlignHCenter | Qt::AlignVCenter, waveText);
        // 如果游戏胜利，绘制胜利贴图
        if (isVictory) {
            painter.drawPixmap(0, 0, width(), height(), winPixmap);
        }
        // 否则，如果游戏失败，绘制失败贴图
        else if (isGameOver) {
            painter.drawPixmap(0, 0, width(), height(), gameOverPixmap);
        }
}

// 定时器事件处理函数的实现，处理游戏的定时逻辑
void mainwindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == waveTimerId) {
        if (isGameOver || isPaused) {
            return;
        }

        for (int i = 0; i < gameSpeed; i++) {
            elapsedTime += 100;

            // 第一波：0-20秒
            if (currentWave == 0 && elapsedTime <= 20000) {
                if (elapsedTime % 3000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (elapsedTime % 5000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
            }
            // 进入第二波判断（>=30秒，间隔10秒）
            else if (currentWave == 0 && elapsedTime >= 30000) {
                currentWave = 1;
                elapsedTime = 30000;
            }

            // 第二波：30-50秒
            if (currentWave == 1 && elapsedTime - 30000 <= 20000) {
                int waveTime = elapsedTime - 30000;
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
            }
            // 进入第三波判断（>=60秒，间隔10秒）
            else if (currentWave == 1 && elapsedTime >= 60000) {
                currentWave = 2;
                elapsedTime = 60000;
            }

            // 第三波：60-80秒
            if (currentWave == 2 && elapsedTime - 60000 <= 20000) {
                int waveTime = elapsedTime - 60000;
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 进入第四波判断（>=90秒，间隔10秒）
            else if (currentWave == 2 && elapsedTime >= 90000) {
                currentWave = 3;
                elapsedTime = 90000;
            }

            // 第四波：90-110秒
            if (currentWave == 3 && elapsedTime - 90000 <= 20000) {
                int waveTime = elapsedTime - 90000;
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 进入第五波判断（>=120秒，间隔10秒）
            else if (currentWave == 3 && elapsedTime >= 120000) {
                currentWave = 4;
                elapsedTime = 120000;
            }

            // 第五波：120-150秒
            if (currentWave == 4 && elapsedTime - 120000 <= 30000) {
                int waveTime = elapsedTime - 120000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 6000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
            }
            // 进入第六波判断（>=160秒，间隔10秒）
            else if (currentWave == 4 && elapsedTime >= 160000) {
                currentWave = 5;
                elapsedTime = 160000;
            }

            // 第六波：160-180秒
            if (currentWave == 5 && elapsedTime - 160000 <= 20000) {
                int waveTime = elapsedTime - 160000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 进入第七波判断（>=190秒，间隔10秒）
            else if (currentWave == 5 && elapsedTime >= 190000) {
                currentWave = 6;
                elapsedTime = 190000;
            }

            // 第七波：190-210秒
            if (currentWave == 6 && elapsedTime - 190000 <= 20000) {
                int waveTime = elapsedTime - 190000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 进入第八波判断（>=220秒，间隔10秒）
            else if (currentWave == 6 && elapsedTime >= 220000) {
                currentWave = 7;
                elapsedTime = 220000;
            }

            // 第八波：220-240秒（中等/困难难度）
            if (difficulty != 0 && currentWave == 7 && elapsedTime - 220000 <= 20000) {
                int waveTime = elapsedTime - 220000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 简单难度：第8波即为Boss波：220-260秒
            if (difficulty == 0 && currentWave == 7 && elapsedTime - 220000 <= 40000) {
                int waveTime = elapsedTime - 220000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4500 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
                if (waveTime == 40000) {
                    generateMonster(Enemy::MonsterBoss);
                }
            }
            // 简单难度：等待Boss被消灭
            else if (difficulty == 0 && currentWave == 7 && elapsedTime >= 260000) {
                // 等待Boss被消灭
            }

            // 中等/困难难度：进入第九波判断（>=250秒，间隔10秒）
            else if (currentWave == 7 && elapsedTime >= 250000) {
                currentWave = 8;
                elapsedTime = 250000;
            }

            // 第九波：250-270秒
            if (currentWave == 8 && elapsedTime - 250000 <= 20000) {
                int waveTime = elapsedTime - 250000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 进入第十波判断（>=280秒，间隔10秒）
            else if (currentWave == 8 && elapsedTime >= 280000) {
                currentWave = 9;
                elapsedTime = 280000;
            }

            // 第十波：280-320秒（中等难度为Boss波，困难难度为普通波）
            if (currentWave == 9 && elapsedTime - 280000 <= 40000) {
                int waveTime = elapsedTime - 280000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4500 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
                // 中等难度：第十波最后生成Boss
                if (difficulty == 1 && waveTime == 40000) {
                    generateMonster(Enemy::MonsterBoss);
                }
                // 困难难度：第十波不生成Boss，继续后续波次
            }
            // 困难难度：进入第十一波判断（>=330秒，间隔10秒）
            else if (currentWave == 9 && difficulty == 2 && elapsedTime >= 330000) {
                currentWave = 10;
                elapsedTime = 330000;
            }
            // 中等难度：第十波后等待Boss消灭
            else if (currentWave == 9 && difficulty != 2 && elapsedTime >= 320000) {
                // 等待Boss被消灭
            }

            // 困难难度第十一波：330-370秒
            if (currentWave == 10 && elapsedTime - 330000 <= 40000) {
                int waveTime = elapsedTime - 330000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4500 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 困难难度：进入第十二波判断（>=380秒，间隔10秒）
            else if (currentWave == 10 && elapsedTime >= 380000) {
                currentWave = 11;
                elapsedTime = 380000;
            }

            // 困难难度第十二波：380-420秒（最终Boss波）
            if (currentWave == 11 && elapsedTime - 380000 <= 40000) {
                int waveTime = elapsedTime - 380000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4500 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
                // 最后生成Boss
                if (waveTime == 40000) {
                    generateMonster(Enemy::MonsterBoss);
                }
            }
            // 困难难度：第十二波后等待Boss消灭
            else if (currentWave == 11 && elapsedTime >= 420000) {
                // 等待Boss被消灭
            }

            // 移动所有敌人
            for (auto it = enemies.begin(); it != enemies.end(); ) {
                Enemy* enemy = *it;
                enemy->move();
                if (enemy->getHealth() <= 0 || isEnemyAtEnd(enemy)) {
                    // 检测是否消灭了Boss且萝卜存活
                    if (enemy->getType() == Enemy::MonsterBoss && enemy->getHealth() <= 0 && radish->getHealth() > 0) {
                        isVictory = true;//游戏胜利
                        killTimer(waveTimerId);
                        waveTimerId = -1;
                    }
                    if (enemy->getHealth() <= 0) {
                        // 根据不同类型的敌人增加不同数量的金钱
                        switch (enemy->getType()) {
                        case Enemy::Monster1:
                            money->earn(50);
                            break;
                        case Enemy::Monster2:
                            money->earn(80);
                            break;
                        case Enemy::Monster3:
                            money->earn(80);
                            break;
                        case Enemy::Monster4:
                            money->earn(80);
                            break;
                        case Enemy::Monster5:
                            money->earn(80);
                            break;
                        case Enemy::MonsterBoss:
                            money->earn(200);
                            break;
                        }
                    }
                    if (isEnemyAtEnd(enemy)) {
                        // 检查敌人是否与萝卜碰撞
                        checkEnemyCollideWithRadish();
                    }
                    delete enemy;
                    it = enemies.erase(it); // 移除死亡或到达终点的敌人
                } else {
                    ++it;
                }
            }

            // 炮塔攻击敌人
            for (Tower* tower : towers) {
                if (selectedObstacle && tower->isInRange(selectedObstacle->getPosition())) {
                    tower->attackObstacle(selectedObstacle);
                } else {
                    tower->attack(enemies);
                }
                tower->updateBullets();
            }

            // 障碍物被攻击处理
            for (auto it = obstacles.begin(); it != obstacles.end(); ) {
                Obstacle* obstacle = *it;
                if (obstacle->getHealth() <= 0) {
                    // 障碍物被摧毁处理
                    if (selectedObstacle == obstacle) {
                        selectedObstacle = nullptr;
                    }
                    for (TowerPit* pit : towerPits) {
                        if (pit->getObstacle() == obstacle) {
                            pit->setObstacle(nullptr);
                            break;
                        }
                    }
                    delete obstacle;
                    it = obstacles.erase(it);
                } else {
                    ++it;
                }
            }

            // 如果萝卜生命值为0，游戏失败
            if (radish->getHealth() <= 0) {
                isGameOver = true;
                killTimer(waveTimerId);  // 立即停止定时器
                waveTimerId = -1;
            }
        }
        // 更新界面
        update();
    }
}
// 鼠标按下事件处理函数的实现，处理鼠标点击操作
void mainwindow::mousePressEvent(QMouseEvent *event)
{
    if (isVictory) {
        QPoint clickPos = event->pos();
        if (confirmButtonRect.contains(clickPos)) {
            returnToStart(); // 返回开始界面
            return;
        }
        return; // 胜利界面下点击非按钮区域不做处理，防止穿透到底层游戏逻辑
    }
    if (isGameOver) {
            QPoint clickPos = event->pos();
            // 检查是否点击了重新开始按钮区域
            if (restartButtonRect.contains(clickPos)) {
                restartGame();
                return;
            }
            // 检查是否点击了返回按钮区域
            if (returnButtonRect.contains(clickPos)) {
                returnToStart();
                return;
            }
            return; // 失败界面下点击非按钮区域不做处理，防止穿透
        }

    QPoint clickPos = event->pos();

    // 检查是否点击了全局按钮（暂停、二倍速）
    for (Button* button : buttons) {
        if (button->isClicked(clickPos)) {
            handleButtonClick(button->getType());
            update(); // 强制更新界面，确保贴图刷新
            return;
        }
    }
    // ========== 优先级最高：弹出的操作按钮（升级/移除、创建炮台选项） ==========
    // 这些按钮绘制在炮塔/炮塔坑之上，因此点击检测必须优先于底层元素

    // 如果有选中的炮塔，优先检查是否点击了升级或移除按钮
    if (selectedTower) {
        QPixmap upgradePixmap;
        // 根据金币数量选择升级按钮的图片
        if (money->canAfford(selectedTower->getUpgradeCost())) {
            upgradePixmap = QPixmap(":/images/images/upgrade_normal_blue.png").scaled(50, 50, Qt::KeepAspectRatio);
        }
        else {
            upgradePixmap = QPixmap(":/images/images/upgrade_normal_gray.png").scaled(50, 50, Qt::KeepAspectRatio);
        }
        // 计算升级按钮的矩形区域（与paintEvent中的绘制位置保持一致）
        QPoint upgradePos(selectedTower->getPosition().x(), selectedTower->getPosition().y() - upgradePixmap.height() - 20);
        QRect upgradeRect(upgradePos, upgradePixmap.size());

        // 加载移除按钮的图片
        QPixmap removePixmap = QPixmap(":/images/images/remove_normal.png").scaled(50, 50, Qt::KeepAspectRatio);
        // 计算移除按钮的矩形区域（与paintEvent中的绘制位置保持一致）
        QPoint removePos(selectedTower->getPosition().x(), selectedTower->getPosition().y() + 100);
        QRect removeRect(removePos, removePixmap.size());

        // 如果点击了升级按钮且金币足够，进行升级操作并取消选中
        if (upgradeRect.contains(clickPos) && money->canAfford(selectedTower->getUpgradeCost())) {
            selectedTower->upgrade();
            money->spend(selectedTower->getUpgradeCost());
            selectedTower = nullptr;
            update();
            return;
        }

        // 如果点击了移除按钮，移除炮塔并取消选中
        if (removeRect.contains(clickPos)) {
            money->earn(selectedTower->getSellPrice());
            // 找到对应的炮塔坑，清除hasTower标记
            for (TowerPit* pit : towerPits) {
                if (pit->getRect().contains(selectedTower->getPosition())) {
                    pit->setHasTower(false);
                    break;
                }
            }
            auto it = std::find(towers.begin(), towers.end(), selectedTower);
            if (it != towers.end()) {
                delete *it;
                towers.erase(it);
            }
            selectedTower = nullptr; // 取消选中
            update();
            return;
        }
    }

    // 如果显示了创建炮台选项按钮，优先检查是否点击了某个选项按钮
    if (showButtons) {
        int buttonSize = 100;
        int gap = 10;
        int totalWidth = 4 * buttonSize + 3 * gap;
        int totalHeight = buttonSize;

        // 计算按钮起始位置，与绘制时一致
        int bx = this->clickPos.x();
        int by = this->clickPos.y();
        if (bx + totalWidth > width()) bx = width() - totalWidth;
        if (bx < 0) bx = 0;
        if (by + totalHeight > height()) by = by - totalHeight;
        if (by < 0) by = 0;

        // 定义四个选项按钮的矩形区域
        QRect coffeeButtonRect(bx, by, buttonSize, buttonSize);
        QRect musicButtonRect(bx + buttonSize + gap, by, buttonSize, buttonSize);
        QRect aiButtonRect(bx + 2 * (buttonSize + gap), by, buttonSize, buttonSize);
        QRect teacherButtonRect(bx + 3 * (buttonSize + gap), by, buttonSize, buttonSize);
        // 标记是否点击了有效按钮
        bool validButtonClicked = false;

        // 如果点击了咖啡炮塔选项按钮
        if (coffeeButtonRect.contains(clickPos)) {
            if (money->canAfford(Tower::getCost(Tower::COFFEE))) {
                towers.push_back(new Tower(Tower::COFFEE, this->clickPos));
                money->spend(Tower::getCost(Tower::COFFEE));
                validButtonClicked = true;
            }
        }
        // 如果点击了音乐炮塔选项按钮
        else if (musicButtonRect.contains(clickPos)) {
            if (money->canAfford(Tower::getCost(Tower::MUSIC))) {
                towers.push_back(new Tower(Tower::MUSIC, this->clickPos));
                money->spend(Tower::getCost(Tower::MUSIC));
                validButtonClicked = true;
            }
        }
        // 如果点击了AI炮塔选项按钮
        else if (aiButtonRect.contains(clickPos)) {
            if (money->canAfford(Tower::getCost(Tower::AI))) {
                towers.push_back(new Tower(Tower::AI, this->clickPos));
                money->spend(Tower::getCost(Tower::AI));
                validButtonClicked = true;
            }
        }
        // 如果点击了老师炮塔选项按钮
        else if (teacherButtonRect.contains(clickPos)) {
            if (money->canAfford(Tower::getCost(Tower::TEACHER))) {
                towers.push_back(new Tower(Tower::TEACHER, this->clickPos));
                money->spend(Tower::getCost(Tower::TEACHER));
                validButtonClicked = true;
            }
        }
        // 无论是否成功放置炮塔，点击按钮后都隐藏选项按钮
        showButtons = false;
        update(); // 刷新界面

        // 如果点击了有效按钮，直接返回，避免其他逻辑处理
        if (validButtonClicked) {
            // 标记对应的炮塔坑已有炮塔，防止重复创建
            for (TowerPit* pit : towerPits) {
                if (pit->getPosition() == this->clickPos) {
                    pit->setHasTower(true);
                    break;
                }
            }
            return;
        }
    }

    // ========== 普通优先级：底层元素点击检测 ==========

    // 检查是否点击了障碍物
    for (Obstacle* obstacle : obstacles) {
        QRect obstacleRect(obstacle->getPosition().toPoint(), obstacle->getPixmap().size());
        if (obstacleRect.contains(clickPos)) {
            if (selectedObstacle == obstacle) {
                // 再次点击已选中的障碍物，取消选中
                selectedObstacle->setSelected(false);
                selectedObstacle = nullptr;
            } else {
                if (selectedObstacle) {
                    selectedObstacle->setSelected(false);
                }
                selectedObstacle = obstacle;
                selectedObstacle->setSelected(true);
            }
            update();
            return;
        }
    }

    // 检查是否点击了炮塔坑
    for (TowerPit* pit : towerPits) {
        if (pit->getRect().contains(clickPos) && !pit->hasTower() && !pit->hasObstacle()) {
            this->clickPos = pit->getPosition();
            showButtons = true;
            update();
            return;
        }
    }

    // 检查是否点击了炮塔
    for (Tower* tower : towers) {
        // 使用固定80x80尺寸进行点击检测，与实际绘制尺寸一致，避免升级后因贴图尺寸变化导致检测区域错误
        QRect towerRect(tower->getPosition(), QSize(80, 80));
        if (towerRect.contains(clickPos)) {
            if (selectedTower == tower) {
                // 再次点击已选中的炮塔，取消选中
                selectedTower = nullptr;
            } else {
                selectedTower = tower;
            }
            update();
            return;
        }
    }

    // 点击了其他空白位置，取消所有选中状态并隐藏按钮
    if (selectedTower || selectedObstacle || showButtons) {
        if (selectedObstacle) {
            selectedObstacle->setSelected(false);
        }
        selectedTower = nullptr;
        selectedObstacle = nullptr;
        showButtons = false;
        update();
    }
}

// 生成指定类型的怪物
void mainwindow::generateMonster(int type)
{
    Enemy* enemy = new Enemy(static_cast<Enemy::Type>(type));
    // 根据波数增加怪物血量
    int healthBonus = 0;
    switch (currentWave) {
    case 0: case 1: healthBonus = 0; break;           // 第一二波不变
    case 2: healthBonus = 50; break;                   // 第三波+50%
    case 3: healthBonus = 100; break;                  // 第四波+100%
    case 4: healthBonus = 150; break;                  // 第五波+150%
    case 5: healthBonus = 180; break;                  // 第六波+180%
    case 6: healthBonus = 210; break;                  // 第七波+210%
    case 7: healthBonus = 250; break;                  // 第八波+250%
    case 8: healthBonus = 300; break;                  // 第九波+300%
    case 9: healthBonus = 300; break;                  // 第十波+300%
    case 10: healthBonus = 300; break;                 // 第十一波+300%
    case 11: healthBonus = 300; break;                 // 第十二波+300%
    }
    // 简单难度血量下降10%（Boss除外）
    if (difficulty == 0 && type != Enemy::MonsterBoss) {
        healthBonus = healthBonus * 60 / 100;
    }
    // 困难难度血量增加20%，11和12波额外增加10%
    if (difficulty == 2) {
        healthBonus = healthBonus * 120 / 100;
        if (currentWave >= 10) {
            healthBonus = healthBonus * 110 / 100;
        }
    }
    if (healthBonus > 0) {
        enemy->addHealth(enemy->getHealth() * healthBonus / 100);
    }
    enemies.append(enemy);
}

// 检查敌人是否到达终点
bool mainwindow::isEnemyAtEnd(Enemy* enemy)
{
    QPointF endPoint = enemy->getPathPoints().last();
    // 比较敌人当前的位置和终点位置是否相同，如果相同则说明敌人到达了终点
    return enemy->getPosition() == endPoint;
}

// 检查敌人是否与萝卜碰撞，并处理萝卜的生命值减少
void mainwindow::checkEnemyCollideWithRadish()
{
     // 遍历所有敌人，检查是否到达终点（萝卜位置）
    for (Enemy* enemy : enemies) {
        if (isEnemyAtEnd(enemy)) {
            switch (enemy->getType()) {
            case Enemy::Monster1:
                radish->takeDamage(0.5);
                break;
            case Enemy::Monster2:
            case Enemy::Monster3:
            case Enemy::Monster4:
            case Enemy::Monster5:
                radish->takeDamage(1);
                break;
            case Enemy::MonsterBoss:
                radish->takeDamage(10);
                break;
            }
        }
    }
}

// 创建指定类型的炮塔
void mainwindow::createTower(Tower::Type type)
{
    if (money->canAfford(Tower::getCost(type))) {
        towers.append(new Tower(type, clickPos));
        money->spend(Tower::getCost(type));
    }
}

// 处理按钮点击事件，根据按钮类型执行相应的操作
void mainwindow::handleButtonClick(Button::ButtonType type)
{
    switch (type) {
    case Button::PAUSE:
        if (waveTimerId != -1) {
            killTimer(waveTimerId);
            waveTimerId = -1;
            isPaused = true;
        } else {
            if (gameSpeed == 1) {
                waveTimerId = startTimer(100);
            } else {
                waveTimerId = startTimer(50);
            }
             isPaused = false;
        }
        update();
        break;
    case Button::DOUBLE_SPEED:
        gameSpeed = (gameSpeed == 1) ? 2 : 1;
        Tower::setGameSpeed(gameSpeed);
        if (waveTimerId != -1) {
            killTimer(waveTimerId);
            if (gameSpeed == 1) {
                waveTimerId = startTimer(100);
            } else {
                waveTimerId = startTimer(50);
            }
        }
        update();
        break;
    case Button::CLICK_OBSTACLE:
        break;
    case Button::TOWER_UPGRADE:
        break;
    case Button::TOWER_REMOVE:
        break;
    case Button::RESTART:
        break;
    case Button:: RETURN:
        break;
    }
}
// 重新开始游戏逻辑
void mainwindow::restartGame() {
    // 重置游戏状态
    isGameOver = false;
    isVictory = false;
    currentWave = 0;
    elapsedTime = 0;
    gameSpeed = 1;
    Tower::setGameSpeed(1);
    radish->resetHealth();
    money->reset(450);

    for (Button* btn : buttons) {
        if (btn->getType() == Button::DOUBLE_SPEED) {
            btn->setActive(false);
            break;
        }
    }

    // 清空敌人和炮塔
    for (Enemy* enemy : enemies) delete enemy;
    enemies.clear();
    for (Tower* tower : towers) delete tower;
    towers.clear();
    // 清空并重建障碍物
    for (Obstacle* obstacle : obstacles) delete obstacle;
    obstacles.clear();
    QList<int> targetIndices = {9, 15, 17,19,20,22,25, 33, 34,38};
    QString obstaclePaths[10] = {
        ":/images/images/buildings_01.png",
        ":/images/images/obstacle2.png",
        ":/images/images/obstacle3.png",
        ":/images/images/obstacle4.png",
        ":/images/images/obstacle5.png",
        ":/images/images/obstacle6.png",
        ":/images/images/buildings_03.png",
        ":/images/images/bulidings_02.png",
        ":/images/images/obstacle9.png",
        ":/images/images/buildings_04.png"
    };
    QList<QSize> obstacleSizes = {
        QSize(180, 180), QSize(100, 100), QSize(100, 100), QSize(100, 100),
        QSize(100, 100), QSize(100, 100), QSize(180, 180), QSize(180, 180),
        QSize(100, 100), QSize(180, 180)
    };
    int pathIndex = 0;
    for (int index : targetIndices) {
        for (TowerPit* pit : towerPits) {
            if (pit->getIndex() == index) {
                QSize size = obstacleSizes[pathIndex];
                Obstacle* obstacle = new Obstacle(pit->getPosition(), obstaclePaths[pathIndex], size.width(), size.height());
                obstacles.append(obstacle);
                pit->setObstacle(obstacle);
                pathIndex++;
                break;
            }
        }
    }
    // 重置所有炮塔坑的hasTower标记
    for (TowerPit* pit : towerPits) {
        pit->setHasTower(false);
    }

    // 重启定时器
    if (waveTimerId != -1) killTimer(waveTimerId);
    waveTimerId = startTimer(100);
    update();
}

// 返回开始界面逻辑
void mainwindow::returnToStart()
{
    // 重置所有游戏状态
    isGameOver = false;
    isVictory = false;
    currentWave = 0;
    elapsedTime = 0;
    gameSpeed = 1;
    Tower::setGameSpeed(1);

    for (Button* btn : buttons) {
        if (btn->getType() == Button::DOUBLE_SPEED) {
            btn->setActive(false);
            break;
        }
    }

    radish->resetHealth();
    money->reset(450);

    // 清空敌人和炮塔
    qDeleteAll(enemies);
    enemies.clear();
    qDeleteAll(towers);
    towers.clear();
    // 清空并重建障碍物
    qDeleteAll(obstacles);
    obstacles.clear();
    {
        QList<int> targetIndices = {9, 15, 17,19,20,22,25, 33, 34,38};
        QString obstaclePaths[10] = {
            ":/images/images/buildings_01.png",
            ":/images/images/obstacle2.png",
            ":/images/images/obstacle3.png",
            ":/images/images/obstacle4.png",
            ":/images/images/obstacle5.png",
            ":/images/images/obstacle6.png",
            ":/images/images/buildings_03.png",
            ":/images/images/bulidings_02.png",
            ":/images/images/obstacle9.png",
            ":/images/images/buildings_04.png"
        };
        QList<QSize> obstacleSizes = {
            QSize(180, 180), QSize(100, 100), QSize(100, 100), QSize(100, 100),
            QSize(100, 100), QSize(100, 100), QSize(180, 180), QSize(180, 180),
            QSize(100, 100), QSize(180, 180)
        };
        int pathIndex = 0;
        for (int index : targetIndices) {
            for (TowerPit* pit : towerPits) {
                if (pit->getIndex() == index) {
                    QSize size = obstacleSizes[pathIndex];
                    Obstacle* obstacle = new Obstacle(pit->getPosition(), obstaclePaths[pathIndex], size.width(), size.height());
                    obstacles.append(obstacle);
                    pit->setObstacle(obstacle);
                    pathIndex++;
                    break;
                }
            }
        }
    }
    // 重置所有炮塔坑的hasTower标记
    for (TowerPit* pit : towerPits) {
        pit->setHasTower(false);
    }

    // 重置UI选中状态
    selectedTower = nullptr;
    selectedObstacle = nullptr;
    showButtons = false;
    isPaused = false;
    difficulty = 1; // 重置难度为中等
    totalWaves = 10;

    // 停止所有定时器
    if (waveTimerId != -1) {
        killTimer(waveTimerId);
        waveTimerId = -1;
    }
    countdownTimer->stop();

    // 显示开始界面（不能调用this->hide()，否则Qt会连带隐藏子窗口startWindow）
    startWindow->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    startWindow->show();
    startWindow->raise();  // 确保startWindow在最上层
    difficultyWindow->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    difficultyWindow->hide();
    update();              // 强制重绘，清除胜利/失败贴图
}