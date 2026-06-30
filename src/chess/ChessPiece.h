#ifndef CHESSPIECE_H
#define CHESSPIECE_H

#include <string>
#include <iostream>
#include "dice.h"

enum class Rarity { Common, Uncommon, Rare, Legendary, Mythic };

// ============================================================
// 抽象基类 ChessPiece
// ============================================================
class ChessPiece {
protected:
    std::string name;
    Rarity rarity;
    int gold;

    int str, dex, con, intel, wis, cha;
    int hp, hpMax, ac;
    int armorBonus;
    int range;
    int level;
    int pos;

    bool taunt;
    bool stealth;
    bool fumbled;
    bool alive;

public:
    ChessPiece(std::string n, Rarity r, int g,
               int s, int d, int c, int i, int w, int ch);
    virtual ~ChessPiece() = default;

    // 纯虚函数
    virtual void attack(ChessPiece& target) = 0;
    virtual void useSkill(ChessPiece& target) = 0;

    // 虚函数
    virtual void takeDamage(int dmg);
    virtual void display() const;

    // getter
    bool isAlive() const { return alive; }
    bool isTaunt() const { return taunt; }
    bool isStealth() const { return stealth; }
    bool isFumbled() const { return fumbled; }
    int getAC() const { return ac; }
    int getHP() const { return hp; }
    int getHPMax() const { return hpMax; }
    int getRange() const { return range; }
    int getLevel() const { return level; }
    int getPos() const { return pos; }
    int getGold() const { return gold; }
    int getPower() const;
    std::string getName() const { return name; }
    std::string getSymbol() const;
    Rarity getRarity() const { return rarity; }
    int getStr() const { return str; }
    int getDex() const { return dex; }
    int getCon() const { return con; }
    int getInt() const { return intel; }
    int getWis() const { return wis; }
    int getCha() const { return cha; }
    int getArmorBonus() const { return armorBonus; }

    // setter
    void setPos(int p) { pos = p; }
    void clearFumble() { fumbled = false; }
    void breakStealth() { stealth = false; }
    void levelUp();
    void healFull() { hp = hpMax; alive = true; fumbled = false; }

    // 运算符重载
    bool operator<(const ChessPiece& other) const { return getPower() < other.getPower(); }
    bool operator==(const ChessPiece& other) const { return getSymbol() == other.getSymbol(); }
    friend std::ostream& operator<<(std::ostream& os, const ChessPiece& p);
    friend ChessPiece* operator+(ChessPiece& a, ChessPiece& b);

protected:
    void recalcStats();
};

std::string rarityToString(Rarity r);

// ============================================================
// 中间基类：虚继承 ChessPiece
// ============================================================
class MeleePiece : virtual public ChessPiece {
protected:
    int meleeBonus;
public:
    MeleePiece() : ChessPiece("", Rarity::Common, 1, 10,10,10,10,10,10), meleeBonus(2) { range = 1; }
    MeleePiece(std::string n, Rarity r, int g,
               int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch), meleeBonus(2) { range = 1; }
    int getMeleeBonus() const { return meleeBonus; }
};

class RangedPiece : virtual public ChessPiece {
protected:
    int rangedBonus;
public:
    RangedPiece() : ChessPiece("", Rarity::Common, 1, 10,10,10,10,10,10), rangedBonus(2) { range = 3; }
    RangedPiece(std::string n, Rarity r, int g,
                int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch), rangedBonus(2) { range = 3; }
    int getRangedBonus() const { return rangedBonus; }
};

class SpellcasterPiece : virtual public ChessPiece {
protected:
    int focusBonus;
public:
    SpellcasterPiece() : ChessPiece("", Rarity::Common, 1, 10,10,10,10,10,10), focusBonus(2) { range = 4; }
    SpellcasterPiece(std::string n, Rarity r, int g,
                     int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch), focusBonus(2) { range = 4; }
    int getFocusBonus() const { return focusBonus; }
};

// ============================================================
// 7 基础职业
// ============================================================

// 战士：二次风（30% 再攻击一次）
class Fighter : virtual public MeleePiece {
public:
    Fighter(std::string n, Rarity r, int g,
            int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          MeleePiece(n, r, g, s, d, c, i, w, ch) {}
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
};

// 野蛮人：狂暴（HP<30% 伤害翻倍），嘲讽
class Barbarian : virtual public MeleePiece {
public:
    Barbarian(std::string n, Rarity r, int g,
              int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          MeleePiece(n, r, g, s, d, c, i, w, ch) { taunt = true; armorBonus = 1; recalcStats(); }
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
};

// 圣武士：固定减伤 3 点，嘲讽
class Paladin : virtual public MeleePiece {
public:
    Paladin(std::string n, Rarity r, int g,
            int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          MeleePiece(n, r, g, s, d, c, i, w, ch) { taunt = true; armorBonus = 3; recalcStats(); }
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
    void takeDamage(int dmg) override;
};

// 游侠：猎人印记，潜行
class Ranger : virtual public RangedPiece {
public:
    Ranger(std::string n, Rarity r, int g,
           int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          RangedPiece(n, r, g, s, d, c, i, w, ch) { stealth = true; }
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
};

// 吟游诗人：鼓舞（友军攻击力 +2）
class Bard : virtual public RangedPiece {
public:
    Bard(std::string n, Rarity r, int g,
         int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          RangedPiece(n, r, g, s, d, c, i, w, ch) {}
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
};

// 法师：火球术（溅射相邻敌人 50%）
class Wizard : virtual public SpellcasterPiece {
public:
    Wizard(std::string n, Rarity r, int g,
           int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          SpellcasterPiece(n, r, g, s, d, c, i, w, ch) {}
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
};

// 牧师：治疗术（攻击后治疗最低血友军）
class Cleric : virtual public SpellcasterPiece {
public:
    Cleric(std::string n, Rarity r, int g,
           int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          SpellcasterPiece(n, r, g, s, d, c, i, w, ch) {}
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
};

// ============================================================
// 5 进阶职业（多重继承）
// ============================================================

// 狂战士：Barbarian + Fighter
class Berserker : public Barbarian, public Fighter {
public:
    Berserker(std::string n, Rarity r, int g,
              int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          Barbarian(n, r, g, s, d, c, i, w, ch),
          Fighter(n, r, g, s, d, c, i, w, ch) {}
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
};

// 神卫士：Paladin + Barbarian
class Defender : public Paladin, public Barbarian {
public:
    Defender(std::string n, Rarity r, int g,
             int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          Paladin(n, r, g, s, d, c, i, w, ch),
          Barbarian(n, r, g, s, d, c, i, w, ch) {}
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
    void takeDamage(int dmg) override;
};

// 圣骑士：Fighter + Paladin
class PaladinKnight : public Fighter, public Paladin {
public:
    PaladinKnight(std::string n, Rarity r, int g,
                  int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          Fighter(n, r, g, s, d, c, i, w, ch),
          Paladin(n, r, g, s, d, c, i, w, ch) {}
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
    void takeDamage(int dmg) override;
};

// 魔射手：Ranger + Bard
class ArcaneArcher : public Ranger, public Bard {
public:
    ArcaneArcher(std::string n, Rarity r, int g,
                 int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          Ranger(n, r, g, s, d, c, i, w, ch),
          Bard(n, r, g, s, d, c, i, w, ch) {}
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
};

// 神谕者：Wizard + Cleric
class Oracle : public Wizard, public Cleric {
public:
    Oracle(std::string n, Rarity r, int g,
           int s, int d, int c, int i, int w, int ch)
        : ChessPiece(n, r, g, s, d, c, i, w, ch),
          Wizard(n, r, g, s, d, c, i, w, ch),
          Cleric(n, r, g, s, d, c, i, w, ch) {}
    void attack(ChessPiece& target) override;
    void useSkill(ChessPiece& target) override;
};

#endif
