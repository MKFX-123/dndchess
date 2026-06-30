#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>
#include <array>
#include "chess/ChessPiece.h"

class Player {
protected:
    std::string name;
    int hp;
    int gold;
    int round;
    int shopLevel;
    bool isAI;

    std::vector<ChessPiece*> bench;          // 备战席
    std::array<ChessPiece*, 7> board;        // 战场阵型（7 个位置）
    std::vector<ChessPiece*> shop;           // 当前商店展示的棋子

public:
    Player(std::string n, bool ai = false);
    virtual ~Player();

    // 经济
    void gainGold();
    bool spendGold(int amount);
    int getGold() const { return gold; }

    // 生命
    void takeDamage(int amount) { hp -= amount; if (hp < 0) hp = 0; }
    int getHP() const { return hp; }
    bool isDead() const { return hp <= 0; }

    // 回合
    int getRound() const { return round; }
    void nextRound() { round++; }

    // 商店
    void refreshShop();
    bool upgradeShop();
    int getShopLevel() const { return shopLevel; }
    const std::vector<ChessPiece*>& getShop() const { return shop; }
    void clearShop();

    // 棋子管理
    bool buyPiece(int index);
    bool upgradePiece(int benchIndex);
    ChessPiece* mergePiece(int idxA, int idxB);
    void arrangeBoard(int benchIndex, int boardPos);
    void removeFromBoard(int boardPos);

    // 查询
    const std::vector<ChessPiece*>& getBench() const { return bench; }
    std::array<ChessPiece*, 7>& getBoard() { return board; }
    std::string getName() const { return name; }
    bool isAIPlayer() const { return isAI; }
    int getBoardCount() const;

    // 显示
    virtual void display() const;

protected:
    ChessPiece* generateRandomPiece(int maxLevel);
    int rollPieceLevel() const;
};

#endif
