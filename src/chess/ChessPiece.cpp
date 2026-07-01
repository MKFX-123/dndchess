#include "ChessPiece.h"
#include "../battlelog.h"

// ============================================================
// ChessPiece 基类实现
// ============================================================

ChessPiece::ChessPiece(std::string n, int g,
                       int s, int d, int c, int i, int w, int ch)
    : name(n), gold(g),
      str(s), dex(d), con(c), intel(i), wis(w), cha(ch),
      armorBonus(0), range(0), level(1), pos(-1),
      taunt(false), stealth(false), fumbled(false), alive(true) {
    recalcStats();
    hp = hpMax;
}

void ChessPiece::recalcStats() {
    hpMax = 10 + con * 2;
    ac = 10 + getMod(dex) + armorBonus;
    for (int k = 1; k < level; k++) {
        hpMax = hpMax * 3 / 2;
    }
    if (hp > hpMax) hp = hpMax;
}

int ChessPiece::getPower() const {
    return str + dex + con + intel + wis + cha + hpMax + ac + level * 10;
}

std::string ChessPiece::getSymbol() const {
    if (name.empty()) return "?";
    return name.substr(0, 1);
}

void ChessPiece::takeDamage(int dmg) {
    hp -= dmg;
    if (hp <= 0) { hp = 0; alive = false; }
    if (stealth) stealth = false;
}

void ChessPiece::display() const {
    std::cout << "[" << getSymbol() << "] " << name
              << " Lv" << level
              << " HP:" << hp << "/" << hpMax
              << " AC:" << ac
              << (alive ? "" : " [阵亡]")
              << (fumbled ? " [失误]" : "")
              << (taunt ? " [嘲讽]" : "")
              << (stealth ? " [潜行]" : "");
}

void ChessPiece::levelUp() {
    level++;
    str += 2;
    con += 1;
    recalcStats();
}

std::ostream& operator<<(std::ostream& os, const ChessPiece& p) {
    os << "[" << p.getSymbol() << "] " << p.name
       << " HP:" << p.hp << "/" << p.hpMax;
    return os;
}

// ============================================================
// 7 基础职业实现
// ============================================================

// 战士：d8 + STR，30% 二次风
void Fighter::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(str);
    int hit = roll + mod + meleeBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, meleeBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(8).roll();
        int dmg = (dmgRoll + mod + meleeBonus) * (roll == 20 ? 2 : 1);
        BattleLog::damage(8, dmgRoll, mod, meleeBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, meleeBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Fighter::useSkill(ChessPiece& target) {
    if (Dice(100).roll() <= (level >= 3 ? 50 : 30)) { BattleLog::skill("二次风"); attack(target); }
}

// 野蛮人：d8 + STR，HP<30% 狂暴翻倍
void Barbarian::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(str);
    int hit = roll + mod + meleeBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, meleeBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(8).roll();
        int dmg = (dmgRoll + mod + meleeBonus) * (roll == 20 ? 2 : 1);
        bool raged = (hp * 100 < hpMax * 30);
        if (raged) { dmg *= 2; BattleLog::skill("狂暴(伤害翻倍)"); }
        BattleLog::damage(8, dmgRoll, mod, meleeBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, meleeBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Barbarian::useSkill(ChessPiece& target) {}

// 圣武士：d8 + STR，固定减伤 3
void Paladin::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(str);
    int hit = roll + mod + meleeBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, meleeBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(8).roll();
        int dmg = (dmgRoll + mod + meleeBonus) * (roll == 20 ? 2 : 1);
        BattleLog::damage(8, dmgRoll, mod, meleeBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, meleeBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Paladin::useSkill(ChessPiece& target) {}
void Paladin::takeDamage(int dmg) {
    int reduced = dmg - 3;
    if (reduced < 1) reduced = 1;
    ChessPiece::takeDamage(reduced);
}

// 游侠：d6 + DEX，猎人印记
void Ranger::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(dex);
    int hit = roll + mod + rangedBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, rangedBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(6).roll();
        int dmg = (dmgRoll + mod + rangedBonus) * (roll == 20 ? 2 : 1);
        BattleLog::damage(6, dmgRoll, mod, rangedBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, rangedBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Ranger::useSkill(ChessPiece& target) {}

// 吟游诗人：d6 + CHA，鼓舞
void Bard::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(cha);
    int hit = roll + mod + rangedBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, rangedBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(6).roll();
        int dmg = (dmgRoll + mod + rangedBonus) * (roll == 20 ? 2 : 1);
        BattleLog::damage(6, dmgRoll, mod, rangedBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, rangedBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Bard::useSkill(ChessPiece& target) {}

// 法师：d10 + INT，火球术溅射
void Wizard::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(intel);
    int hit = roll + mod + focusBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, focusBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(10).roll();
        int dmg = (dmgRoll + mod + focusBonus) * (roll == 20 ? 2 : 1);
        BattleLog::damage(10, dmgRoll, mod, focusBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, focusBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Wizard::useSkill(ChessPiece& target) {}

// 牧师：d8 + WIS，治疗术
void Cleric::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(wis);
    int hit = roll + mod + focusBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, focusBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(8).roll();
        int dmg = (dmgRoll + mod + focusBonus) * (roll == 20 ? 2 : 1);
        BattleLog::damage(8, dmgRoll, mod, focusBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, focusBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Cleric::useSkill(ChessPiece& target) {}

// ============================================================
// 5 进阶职业实现（多重继承）
// ============================================================

// 狂战士：野蛮人狂暴 + 战士二次风
void Berserker::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(str);
    int hit = roll + mod + meleeBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, meleeBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(8).roll();
        int dmg = (dmgRoll + mod + meleeBonus) * (roll == 20 ? 2 : 1);
        if (hp * 100 < hpMax * 30) { dmg *= 2; BattleLog::skill("狂暴(伤害翻倍)"); }
        BattleLog::damage(8, dmgRoll, mod, meleeBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, meleeBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Berserker::useSkill(ChessPiece& target) {
    if (Dice(100).roll() <= (level >= 3 ? 50 : 30)) { BattleLog::skill("二次风"); attack(target); }
}

// 神卫士：圣武士减伤 + 野蛮人狂暴
void Defender::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(str);
    int hit = roll + mod + meleeBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, meleeBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(8).roll();
        int dmg = (dmgRoll + mod + meleeBonus) * (roll == 20 ? 2 : 1);
        if (hp * 100 < hpMax * 30) { dmg *= 2; BattleLog::skill("狂暴(伤害翻倍)"); }
        BattleLog::damage(8, dmgRoll, mod, meleeBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, meleeBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Defender::useSkill(ChessPiece& target) {}
void Defender::takeDamage(int dmg) {
    int reduced = dmg - 3;
    if (reduced < 1) reduced = 1;
    ChessPiece::takeDamage(reduced);
}

// 圣骑士：战士输出 + 圣武士减伤
void PaladinKnight::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(str);
    int hit = roll + mod + meleeBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, meleeBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(8).roll();
        int dmg = (dmgRoll + mod + meleeBonus) * (roll == 20 ? 2 : 1);
        BattleLog::damage(8, dmgRoll, mod, meleeBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, meleeBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void PaladinKnight::useSkill(ChessPiece& target) {
    if (Dice(100).roll() <= (level >= 3 ? 50 : 30)) { BattleLog::skill("二次风"); attack(target); }
}
void PaladinKnight::takeDamage(int dmg) {
    int reduced = dmg - 3;
    if (reduced < 1) reduced = 1;
    ChessPiece::takeDamage(reduced);
}

// 魔射手：游侠射击 + 吟游诗人鼓舞
void ArcaneArcher::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(dex);
    int hit = roll + mod + rangedBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, rangedBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(6).roll();
        int dmg = (dmgRoll + mod + rangedBonus) * (roll == 20 ? 2 : 1);
        BattleLog::damage(6, dmgRoll, mod, rangedBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, rangedBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void ArcaneArcher::useSkill(ChessPiece& target) {}

// 神谕者：法师火球术 + 牧师治疗
void Oracle::attack(ChessPiece& target) {
    if (fumbled) { fumbled = false; return; }
    int roll = Dice(20).roll();
    int mod = getMod(intel);
    int hit = roll + mod + focusBonus;
    BattleLog::attack(name, target.getName());
    if (roll == 1) { fumbled = true; BattleLog::fumble(roll); BattleLog::end(); return; }
    if (roll == 20 || hit >= target.getAC()) {
        BattleLog::hitRoll(roll, mod, focusBonus, hit, target.getAC(), roll == 20);
        int dmgRoll = Dice(10).roll();
        int dmg = (dmgRoll + mod + focusBonus) * (roll == 20 ? 2 : 1);
        BattleLog::damage(10, dmgRoll, mod, focusBonus, dmg, roll == 20);
        target.takeDamage(dmg);
        BattleLog::result(target.getName(), dmg, target.getHP());
        BattleLog::end();
        useSkill(target);
    } else {
        BattleLog::miss(roll, mod, focusBonus, hit, target.getAC());
        BattleLog::end();
    }
}
void Oracle::useSkill(ChessPiece& target) {}
