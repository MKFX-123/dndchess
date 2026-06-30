#include "Player.h"
#include <iostream>
#include <algorithm>

Player::Player(std::string n, bool ai)
    : name(n), hp(50), gold(3), round(1), shopLevel(1), isAI(ai) {
    board.fill(nullptr);
    refreshShop();
}

Player::~Player() {
    for (auto p : bench) delete p;
    for (auto p : shop) delete p;
    for (auto p : board) if (p) delete p;
}

// ============================================================
// 经济系统
// ============================================================

void Player::gainGold() {
    int income = 3 + (round - 1);
    if (income > 10) income = 10;
    gold += income;
}

bool Player::spendGold(int amount) {
    if (gold < amount) return false;
    gold -= amount;
    return true;
}

// ============================================================
// 商店系统
// ============================================================

// 按商店等级决定刷出的棋子等级
int Player::rollPieceLevel() const {
    int roll = Dice(100).roll();
    switch (shopLevel) {
        case 1:
            return 1;
        case 2:
            if (roll <= 70) return 1;
            return 2;
        case 3:
            if (roll <= 50) return 1;
            if (roll <= 80) return 2;
            return 3;
        case 4:
            if (roll <= 35) return 1;
            if (roll <= 60) return 2;
            if (roll <= 80) return 3;
            return 4;
        case 5:
            if (roll <= 25) return 1;
            if (roll <= 45) return 2;
            if (roll <= 65) return 3;
            if (roll <= 85) return 4;
            return 5;
    }
    return 1;
}

// 随机生成一个基础职业棋子（不刷进阶职业）
ChessPiece* Player::generateRandomPiece(int maxLevel) {
    int pieceLevel = rollPieceLevel();
    if (pieceLevel > maxLevel) pieceLevel = maxLevel;

    int job = Dice(7).roll();
    std::string n = "棋子" + std::to_string(Dice(999).roll());

    // 基础属性按棋子等级缩放
    int scale = pieceLevel;
    ChessPiece* piece = nullptr;

    switch (job) {
        case 1: piece = new Fighter(n, Rarity::Common, 1, 16+scale, 12, 14+scale, 10, 10, 10); break;
        case 2: piece = new Barbarian(n, Rarity::Common, 1, 18+scale, 12, 16+scale, 8, 10, 10); break;
        case 3: piece = new Paladin(n, Rarity::Common, 1, 16+scale, 10, 14+scale, 10, 12, 14); break;
        case 4: piece = new Ranger(n, Rarity::Common, 1, 12, 16+scale, 12, 10, 14, 10); break;
        case 5: piece = new Bard(n, Rarity::Common, 1, 10, 14, 12, 12, 10, 16+scale); break;
        case 6: piece = new Wizard(n, Rarity::Common, 1, 8, 12, 10, 18+scale, 14, 12); break;
        case 7: piece = new Cleric(n, Rarity::Common, 1, 10, 12, 12, 12, 16+scale, 12); break;
    }

    // 设置棋子等级（通过升级）
    for (int i = 1; i < pieceLevel; i++) piece->levelUp();
    return piece;
}

void Player::refreshShop() {
    // 释放旧商店
    for (auto p : shop) delete p;
    shop.clear();

    // 生成新棋子，数量 = 商店等级 + 2
    int count = shopLevel + 2;
    for (int i = 0; i < count; i++) {
        shop.push_back(generateRandomPiece(shopLevel));
    }
}

bool Player::upgradeShop() {
    if (shopLevel >= 5) return false;
    int cost = shopLevel + 4;
    if (!spendGold(cost)) return false;
    shopLevel++;
    return true;
}

void Player::clearShop() {
    for (auto p : shop) delete p;
    shop.clear();
}

// ============================================================
// 棋子管理
// ============================================================

bool Player::buyPiece(int index) {
    if (index < 0 || index >= (int)shop.size()) return false;
    ChessPiece* piece = shop[index];
    if (!spendGold(piece->getGold())) return false;
    bench.push_back(piece);
    shop.erase(shop.begin() + index);
    return true;
}

bool Player::upgradePiece(int benchIndex) {
    if (benchIndex < 0 || benchIndex >= (int)bench.size()) return false;
    ChessPiece* piece = bench[benchIndex];
    if (piece->getLevel() >= 5) return false;
    int cost = piece->getGold() * piece->getLevel();
    if (!spendGold(cost)) return false;
    piece->levelUp();
    return true;
}

// 合并两个同等级同系别棋子为进阶职业
ChessPiece* Player::mergePiece(int idxA, int idxB) {
    if (idxA < 0 || idxA >= (int)bench.size()) return nullptr;
    if (idxB < 0 || idxB >= (int)bench.size()) return nullptr;
    if (idxA == idxB) return nullptr;

    ChessPiece* a = bench[idxA];
    ChessPiece* b = bench[idxB];

    // 等级必须相同
    if (a->getLevel() != b->getLevel()) return nullptr;

    // 判断系别（近战/远程/法术）通过攻击距离
    int rangeA = a->getRange();
    int rangeB = b->getRange();
    if (rangeA != rangeB) return nullptr;

    // 取较高属性 +2
    int s = std::max(a->getStr(), b->getStr()) + 2;
    int d = std::max(a->getDex(), b->getDex()) + 2;
    int c = std::max(a->getCon(), b->getCon()) + 2;
    int i = std::max(a->getInt(), b->getInt()) + 2;
    int w = std::max(a->getWis(), b->getWis()) + 2;
    int ch = std::max(a->getCha(), b->getCha()) + 2;
    int totalGold = a->getGold() + b->getGold();
    std::string n = a->getName() + "+" + b->getName();

    ChessPiece* merged = nullptr;

    // 近战系合并（距离 1）
    if (rangeA == 1) {
        bool isFighterA = dynamic_cast<Fighter*>(a) != nullptr;
        bool isBarbarianA = dynamic_cast<Barbarian*>(a) != nullptr;
        bool isPaladinA = dynamic_cast<Paladin*>(a) != nullptr;
        bool isFighterB = dynamic_cast<Fighter*>(b) != nullptr;
        bool isBarbarianB = dynamic_cast<Barbarian*>(b) != nullptr;
        bool isPaladinB = dynamic_cast<Paladin*>(b) != nullptr;

        if ((isBarbarianA && isFighterB) || (isFighterA && isBarbarianB)) {
            merged = new Berserker(n, Rarity::Rare, totalGold, s, d, c, i, w, ch);
        } else if ((isPaladinA && isBarbarianB) || (isBarbarianA && isPaladinB)) {
            merged = new Defender(n, Rarity::Rare, totalGold, s, d, c, i, w, ch);
        } else if ((isFighterA && isPaladinB) || (isPaladinA && isFighterB)) {
            merged = new PaladinKnight(n, Rarity::Rare, totalGold, s, d, c, i, w, ch);
        }
    }
    // 远程系合并（距离 3）
    else if (rangeA == 3) {
        bool isRangerA = dynamic_cast<Ranger*>(a) != nullptr;
        bool isBardA = dynamic_cast<Bard*>(a) != nullptr;
        bool isRangerB = dynamic_cast<Ranger*>(b) != nullptr;
        bool isBardB = dynamic_cast<Bard*>(b) != nullptr;

        if ((isRangerA && isBardB) || (isBardA && isRangerB)) {
            merged = new ArcaneArcher(n, Rarity::Rare, totalGold, s, d, c, i, w, ch);
        }
    }
    // 法术系合并（距离 4）
    else if (rangeA == 4) {
        bool isWizardA = dynamic_cast<Wizard*>(a) != nullptr;
        bool isClericA = dynamic_cast<Cleric*>(a) != nullptr;
        bool isWizardB = dynamic_cast<Wizard*>(b) != nullptr;
        bool isClericB = dynamic_cast<Cleric*>(b) != nullptr;

        if ((isWizardA && isClericB) || (isClericA && isWizardB)) {
            merged = new Oracle(n, Rarity::Rare, totalGold, s, d, c, i, w, ch);
        }
    }

    if (!merged) return nullptr;

    // 设置合并后等级（与原等级相同）
    while (merged->getLevel() < a->getLevel()) merged->levelUp();

    // 从备战席移除两个棋子
    int lo = std::min(idxA, idxB);
    int hi = std::max(idxA, idxB);
    delete bench[hi];
    bench.erase(bench.begin() + hi);
    delete bench[lo];
    bench.erase(bench.begin() + lo);

    // 加入备战席
    bench.push_back(merged);
    return merged;
}

void Player::arrangeBoard(int benchIndex, int boardPos) {
    if (benchIndex < 0 || benchIndex >= (int)bench.size()) return;
    if (boardPos < 0 || boardPos >= 7) return;

    // 若目标位置已有棋子，交换回备战席
    if (board[boardPos]) {
        bench.push_back(board[boardPos]);
    }
    board[boardPos] = bench[benchIndex];
    board[boardPos]->setPos(boardPos);
    bench.erase(bench.begin() + benchIndex);
}

void Player::removeFromBoard(int boardPos) {
    if (boardPos < 0 || boardPos >= 7) return;
    if (!board[boardPos]) return;
    bench.push_back(board[boardPos]);
    board[boardPos] = nullptr;
}

int Player::getBoardCount() const {
    int count = 0;
    for (auto p : board) if (p) count++;
    return count;
}

// ============================================================
// 显示
// ============================================================

void Player::display() const {
    std::cout << "===== " << name << " =====" << std::endl;
    std::cout << "生命: " << hp << "/50  金币: " << gold
              << "  回合: " << round
              << "  商店等级: " << shopLevel << std::endl;

    std::cout << "\n[商店] (刷新 1g, 升级 " << (shopLevel >= 5 ? "已满级" : std::to_string(shopLevel + 4) + "g") << ")" << std::endl;
    for (size_t i = 0; i < shop.size(); i++) {
        std::cout << "  " << i + 1 << ". ";
        shop[i]->display();
        std::cout << std::endl;
    }

    std::cout << "\n[备战席]" << std::endl;
    if (bench.empty()) {
        std::cout << "  (空)" << std::endl;
    } else {
        for (size_t i = 0; i < bench.size(); i++) {
            std::cout << "  " << i + 1 << ". ";
            bench[i]->display();
            std::cout << std::endl;
        }
    }

    std::cout << "\n[战场阵型]" << std::endl;
    for (int i = 0; i < 7; i++) {
        std::cout << "  [" << i << "] ";
        if (board[i]) {
            board[i]->display();
        } else {
            std::cout << "空";
        }
        std::cout << std::endl;
    }
}
