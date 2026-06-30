#include "AIPlayer.h"
#include <iostream>
#include <algorithm>

AIPlayer::AIPlayer(std::string n) : Player(n, true) {}

// AI 自动执行一回合准备阶段
void AIPlayer::autoPlay() {
    std::cout << "\n[" << name << " AI 决策中...]" << std::endl;

    int refreshCount = 0;

    tryMerge();
    tryUpgradeShop();
    tryBuy();
    tryUpgradePiece();
    tryRefresh(refreshCount);
    autoArrange();
}

// 1. 合并检查：遍历备战席，有可合并的就合
void AIPlayer::tryMerge() {
    bool merged = true;
    while (merged) {
        merged = false;
        for (size_t i = 0; i < bench.size() && !merged; i++) {
            for (size_t j = i + 1; j < bench.size() && !merged; j++) {
                ChessPiece* a = bench[i];
                ChessPiece* b = bench[j];

                // 同等级 + 同系别（攻击距离相同）
                if (a->getLevel() != b->getLevel()) continue;
                if (a->getRange() != b->getRange()) continue;
                if (a->getSymbol() == b->getSymbol()) continue;  // 同职业跳过

                ChessPiece* result = mergePiece((int)i, (int)j);
                if (result) {
                    std::cout << "  [AI] 合并: " << result->getName() << std::endl;
                    merged = true;
                }
            }
        }
    }
}

// 2. 升级商店：金币富裕时升级
void AIPlayer::tryUpgradeShop() {
    if (shopLevel >= 5) return;
    int cost = shopLevel + 4;
    if (gold >= cost + 2) {  // 留 2g 余量
        upgradeShop();
        std::cout << "  [AI] 升级商店到 " << shopLevel << " 级" << std::endl;
    }
}

// 3. 购买棋子：贪心选性价比最高的
void AIPlayer::tryBuy() {
    bool bought = true;
    while (bought && bench.size() < 7) {
        bought = false;
        int bestIdx = -1;
        double bestRatio = 0;

        for (size_t i = 0; i < shop.size(); i++) {
            if (gold < shop[i]->getGold()) continue;
            double ratio = (double)shop[i]->getPower() / shop[i]->getGold();
            if (ratio > bestRatio) {
                bestRatio = ratio;
                bestIdx = (int)i;
            }
        }

        if (bestIdx >= 0) {
            std::cout << "  [AI] 购买: ";
            shop[bestIdx]->display();
            std::cout << std::endl;
            buyPiece(bestIdx);
            bought = true;
        }
    }
}

// 4. 升级棋子：选战力最低的先升（提升空间最大）
void AIPlayer::tryUpgradePiece() {
    bool upgraded = true;
    while (upgraded) {
        upgraded = false;
        int weakestIdx = -1;
        int weakestPower = 99999;

        for (size_t i = 0; i < bench.size(); i++) {
            ChessPiece* p = bench[i];
            if (p->getLevel() >= 5) continue;
            int cost = p->getGold() * p->getLevel();
            if (gold < cost) continue;
            if (p->getPower() < weakestPower) {
                weakestPower = p->getPower();
                weakestIdx = (int)i;
            }
        }

        if (weakestIdx >= 0) {
            std::cout << "  [AI] 升级: ";
            bench[weakestIdx]->display();
            std::cout << std::endl;
            upgradePiece(weakestIdx);
            upgraded = true;
        }
    }
}

// 5. 刷新商店：备战席有空位且金币富裕时刷新
void AIPlayer::tryRefresh(int& refreshCount) {
    while (refreshCount < 2 && bench.size() < 7 && gold >= 3) {
        refreshShop();
        refreshCount++;
        std::cout << "  [AI] 刷新商店" << std::endl;
        tryBuy();
    }
}

// 6-7. 排列阵型：嘲讽左/潜行右/输出中，补满战场
void AIPlayer::autoArrange() {
    // 先把战场上的棋子撤回备战席
    for (int i = 0; i < 7; i++) {
        if (board[i]) {
            removeFromBoard(i);
        }
    }

    // 分类棋子
    std::vector<ChessPiece*> taunts;    // 嘲讽
    std::vector<ChessPiece*> stealths;  // 潜行
    std::vector<ChessPiece*> others;    // 其他

    for (auto* p : bench) {
        if (p->isTaunt()) taunts.push_back(p);
        else if (p->isStealth()) stealths.push_back(p);
        else others.push_back(p);
    }

    // 其他棋子按战力降序（战力高的放中间）
    std::sort(others.begin(), others.end(), [](ChessPiece* a, ChessPiece* b) {
        return a->getPower() > b->getPower();
    });

    // 按顺序填入战场：嘲讽 0-2，其他 3-4，潜行 5-6
    int pos = 0;
    for (auto* p : taunts) {
        if (pos < 3) {
            auto it = std::find(bench.begin(), bench.end(), p);
            if (it != bench.end()) {
                arrangeBoard((int)(it - bench.begin()), pos);
                pos++;
            }
        }
    }

    pos = 3;
    for (auto* p : others) {
        if (pos < 5) {
            auto it = std::find(bench.begin(), bench.end(), p);
            if (it != bench.end()) {
                arrangeBoard((int)(it - bench.begin()), pos);
                pos++;
            }
        }
    }

    pos = 5;
    for (auto* p : stealths) {
        if (pos < 7) {
            auto it = std::find(bench.begin(), bench.end(), p);
            if (it != bench.end()) {
                arrangeBoard((int)(it - bench.begin()), pos);
                pos++;
            }
        }
    }

    // 若战场还有空位，把备战席剩余棋子按战力降序补上
    for (int i = 0; i < 7 && !bench.empty(); i++) {
        if (!board[i]) {
            std::sort(bench.begin(), bench.end(), [](ChessPiece* a, ChessPiece* b) {
                return a->getPower() > b->getPower();
            });
            arrangeBoard(0, i);
        }
    }

    std::cout << "  [AI] 阵型排列完成 (上场 " << getBoardCount() << " 个棋子)" << std::endl;
}
