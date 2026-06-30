#ifndef BATTLEFIELD_H
#define BATTLEFIELD_H

#include <array>
#include "chess/ChessPiece.h"
#include "player/Player.h"

// 战场类：7×2 棋盘，酒馆战棋式自动战斗
class Battlefield {
    std::array<ChessPiece*, 7> attacker;  // 攻击方（玩家）
    std::array<ChessPiece*, 7> defender;  // 防守方（AI）
    int round;
    int winner;                           // 0=未结束, 1=攻击方胜, 2=防守方胜

public:
    Battlefield();
    ~Battlefield();

    // 设置双方阵型（从 Player 的 board 拷贝指针）
    void setup(Player& atk, Player& def);

    // 选目标：潜行免疫 → 嘲讽优先 → 随机
    ChessPiece* pickTarget(std::array<ChessPiece*, 7>& enemies);

    // 一个棋子执行行动（选目标 + 攻击 + 破潜）
    void actOne(ChessPiece* piece, std::array<ChessPiece*, 7>& enemies);

    // 一方全部行动（从左到右）
    void turnSide(std::array<ChessPiece*, 7>& active, std::array<ChessPiece*, 7>& passive);

    // 回合结束结算（牧师治疗）
    void endTurn(std::array<ChessPiece*, 7>& side);

    // 判断某方是否全灭
    bool allDead(std::array<ChessPiece*, 7>& side);

    // 战斗主循环，返回胜方编号
    int battle();

    // 显示战场
    void display() const;

    // 获取败方存活棋子等级之和（用于扣血）
    int getSurvivorLevelSum(int side);
};

#endif
