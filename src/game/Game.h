#ifndef GAME_H
#define GAME_H

#include <string>
#include "player/Player.h"
#include "player/AIPlayer.h"
#include "battlefield/Battlefield.h"

class Game {
    Player* player;
    AIPlayer* ai;
    bool gameOver;
    int battleWinner;   // 1=玩家胜, 2=AI胜
    int battleDamage;   // 败方扣血量

public:
    Game();
    ~Game();

    void run();           // 主循环

private:
    void showMainMenu();  // 主菜单
    void startGame();     // 开始新游戏
    void loadGame();      // 读档
    void saveGame();      // 存档
    void preparePhase();  // 准备阶段（玩家命令交互）
    void battlePhase();   // 战斗阶段
    void settlePhase();   // 结算阶段
    void showHelp();      // 显示帮助

    // 命令处理
    bool handleCommand(const std::string& cmd);

    // TUI 显示
    void renderUI();      // 渲染主界面（左侧状态+棋盘，右侧命令列表）
    void showShop();
    void showBench();
    void showBoard();
    void showStatus();
};

#endif
