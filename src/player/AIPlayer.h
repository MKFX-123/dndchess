#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "Player.h"

// AI 玩家：继承 Player，用贪心算法自动决策
class AIPlayer : public Player {
public:
    AIPlayer(std::string n);

    // AI 自动执行一回合的准备阶段
    void autoPlay();

private:
    // 贪心决策各步骤
    void tryMerge();          // 1. 合并检查
    void tryUpgradeShop();    // 2. 升级商店
    void tryBuy();            // 3. 购买棋子
    void tryUpgradePiece();   // 4. 升级棋子
    void tryRefresh(int& refreshCount);  // 5. 刷新商店
    void autoArrange();       // 6-7. 排列阵型 + 补满战场
};

#endif
