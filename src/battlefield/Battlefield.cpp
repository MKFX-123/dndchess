#include "Battlefield.h"
#include <iostream>
#include <vector>
#include <algorithm>

Battlefield::Battlefield() : round(0), winner(0) {
    attacker.fill(nullptr);
    defender.fill(nullptr);
}

Battlefield::~Battlefield() {}

// 从 Player 的 board 拷贝指针到战场
void Battlefield::setup(Player& atk, Player& def) {
    auto& atkBoard = atk.getBoard();
    auto& defBoard = def.getBoard();
    for (int i = 0; i < 7; i++) {
        attacker[i] = atkBoard[i];
        defender[i] = defBoard[i];
    }
    round = 0;
    winner = 0;
}

// 选目标：潜行免疫 → 嘲讽优先 → 随机
ChessPiece* Battlefield::pickTarget(std::array<ChessPiece*, 7>& enemies) {
    std::vector<ChessPiece*> candidates;

    // 先收集嘲讽棋子（未潜行、存活）
    for (auto* e : enemies) {
        if (e && e->isAlive() && !e->isStealth() && e->isTaunt()) {
            candidates.push_back(e);
        }
    }
    // 无嘲讽则收集所有存活且未潜行的
    if (candidates.empty()) {
        for (auto* e : enemies) {
            if (e && e->isAlive() && !e->isStealth()) {
                candidates.push_back(e);
            }
        }
    }
    if (candidates.empty()) return nullptr;

    // 随机选一个
    int idx = Dice((int)candidates.size()).roll() - 1;
    return candidates[idx];
}

// 一个棋子执行行动
void Battlefield::actOne(ChessPiece* piece, std::array<ChessPiece*, 7>& enemies) {
    if (!piece || !piece->isAlive()) return;

    ChessPiece* target = pickTarget(enemies);
    if (!target) return;

    piece->attack(*target);

    // 主动攻击后破潜
    if (piece->isStealth()) piece->breakStealth();
}

// 一方全部行动（从左到右）
void Battlefield::turnSide(std::array<ChessPiece*, 7>& active, std::array<ChessPiece*, 7>& passive) {
    for (int i = 0; i < 7; i++) {
        actOne(active[i], passive);
    }
}

// 回合结束结算：牧师治疗最低血友军
void Battlefield::endTurn(std::array<ChessPiece*, 7>& side) {
    ChessPiece* lowest = nullptr;
    int lowestHP = 99999;
    for (auto* p : side) {
        if (p && p->isAlive() && p->getHP() < p->getHPMax()) {
            if (p->getHP() < lowestHP) {
                lowestHP = p->getHP();
                lowest = p;
            }
        }
    }
    // 牧师存在时触发治疗
    bool hasCleric = false;
    for (auto* p : side) {
        if (p && p->isAlive() && dynamic_cast<Cleric*>(p) != nullptr) {
            hasCleric = true;
            break;
        }
    }
    if (hasCleric && lowest) {
        int heal = Dice(8).roll() + 2;
        int newHP = lowest->getHP() + heal;
        if (newHP > lowest->getHPMax()) newHP = lowest->getHPMax();
        std::cout << "  [牧师] 治疗 " << lowest->getName()
                  << " +" << (newHP - lowest->getHP()) << " HP" << std::endl;
    }
}

bool Battlefield::allDead(std::array<ChessPiece*, 7>& side) {
    for (auto* p : side) {
        if (p && p->isAlive()) return false;
    }
    return true;
}

// 战斗主循环
int Battlefield::battle() {
    std::cout << "\n========== 战斗开始 ==========" << std::endl;
    display();

    while (winner == 0) {
        round++;
        std::cout << "\n--- 第 " << round << " 回合 ---" << std::endl;

        // 攻击方行动
        std::cout << "[攻击方行动]" << std::endl;
        turnSide(attacker, defender);

        if (allDead(defender)) { winner = 1; break; }

        // 防守方行动
        std::cout << "[防守方行动]" << std::endl;
        turnSide(defender, attacker);

        if (allDead(attacker)) { winner = 2; break; }

        // 回合结束结算
        endTurn(attacker);
        endTurn(defender);

        // 防止无限循环
        if (round >= 50) {
            // 按存活棋子数判定
            int atkAlive = 0, defAlive = 0;
            for (auto* p : attacker) if (p && p->isAlive()) atkAlive++;
            for (auto* p : defender) if (p && p->isAlive()) defAlive++;
            winner = (atkAlive >= defAlive) ? 1 : 2;
            break;
        }
    }

    std::cout << "\n========== 战斗结束 ==========" << std::endl;
    std::cout << (winner == 1 ? "攻击方胜利!" : "防守方胜利!") << std::endl;
    display();

    // 战斗结束：存活棋子回满血，清除负面状态
    for (auto* p : attacker) if (p && p->isAlive()) p->healFull();
    for (auto* p : defender) if (p && p->isAlive()) p->healFull();

    return winner;
}

// 显示战场
void Battlefield::display() const {
    std::cout << "\n  +---+---+---+---+---+---+---+" << std::endl;
    std::cout << "  |";
    for (int i = 0; i < 7; i++) {
        if (defender[i] && defender[i]->isAlive()) {
            std::cout << " " << defender[i]->getSymbol() << " |";
        } else {
            std::cout << " . |";
        }
    }
    std::cout << "  <- 防守方" << std::endl;
    std::cout << "  +---+---+---+---+---+---+---+" << std::endl;
    std::cout << "  |";
    for (int i = 0; i < 7; i++) {
        if (attacker[i] && attacker[i]->isAlive()) {
            std::cout << " " << attacker[i]->getSymbol() << " |";
        } else {
            std::cout << " . |";
        }
    }
    std::cout << "  <- 攻击方" << std::endl;
    std::cout << "  +---+---+---+---+---+---+---+" << std::endl;

    // 详细信息
    std::cout << "\n[攻击方]";
    for (int i = 0; i < 7; i++) {
        if (attacker[i] && attacker[i]->isAlive()) {
            std::cout << "\n  [" << i << "] ";
            attacker[i]->display();
        }
    }
    std::cout << "\n[防守方]";
    for (int i = 0; i < 7; i++) {
        if (defender[i] && defender[i]->isAlive()) {
            std::cout << "\n  [" << i << "] ";
            defender[i]->display();
        }
    }
    std::cout << std::endl;
}

// 获取某方存活棋子等级之和（败方扣血用）
int Battlefield::getSurvivorLevelSum(int side) {
    int sum = 0;
    auto& arr = (side == 1) ? attacker : defender;
    for (auto* p : arr) {
        if (p && p->isAlive()) sum += p->getLevel();
    }
    return sum;
}
