#ifndef BATTLELOG_H
#define BATTLELOG_H

#include <iostream>
#include <string>

// 战斗日志系统（BG3 风格）
namespace BattleLog {
    inline bool& enabled() { static bool e = true; return e; }

    inline void attack(const std::string& attacker, const std::string& target) {
        if (!enabled()) return;
        std::cout << "  [" << attacker << "] 攻击 [" << target << "]";
    }

    inline void hitRoll(int d20, int mod, int bonus, int total, int ac, bool crit) {
        if (!enabled()) return;
        std::cout << "\n    命中检定: 1d20(" << d20 << ")";
        if (mod != 0) std::cout << (mod >= 0 ? " + " : " - ") << (mod >= 0 ? mod : -mod);
        if (bonus != 0) std::cout << " + " << bonus;
        std::cout << " = " << total << " vs AC " << ac;
        if (crit) std::cout << " -> 暴击!";
        else std::cout << " -> 命中!";
    }

    inline void miss(int d20, int mod, int bonus, int total, int ac) {
        if (!enabled()) return;
        std::cout << "\n    命中检定: 1d20(" << d20 << ")";
        if (mod != 0) std::cout << (mod >= 0 ? " + " : " - ") << (mod >= 0 ? mod : -mod);
        if (bonus != 0) std::cout << " + " << bonus;
        std::cout << " = " << total << " vs AC " << ac << " -> 未命中!";
    }

    inline void fumble(int d20) {
        if (!enabled()) return;
        std::cout << "\n    命中检定: 1d20(" << d20 << ") -> 大失败! 下回合无法行动";
    }

    inline void damage(int diceSides, int diceVal, int mod, int bonus, int total, bool crit) {
        if (!enabled()) return;
        std::cout << "\n    伤害: ";
        if (crit) std::cout << "2d" << diceSides << "(" << diceVal << "+" << diceVal << ")";
        else std::cout << "1d" << diceSides << "(" << diceVal << ")";
        if (mod != 0) std::cout << (mod >= 0 ? " + " : " - ") << (mod >= 0 ? mod : -mod);
        if (bonus != 0) std::cout << " + " << bonus;
        std::cout << " = " << total;
    }

    inline void result(const std::string& target, int dmg, int hpAfter) {
        if (!enabled()) return;
        std::cout << "\n    [" << target << "] 受到 " << dmg << " 点伤害 (HP: " << hpAfter << ")";
    }

    inline void reduced(const std::string& target, int original, int reduced, int hpAfter) {
        if (!enabled()) return;
        std::cout << "\n    [" << target << "] 减伤 " << (original - reduced) << " 点, 受到 " << reduced << " 点伤害 (HP: " << hpAfter << ")";
    }

    inline void skill(const std::string& skillName) {
        if (!enabled()) return;
        std::cout << "\n    [技能] " << skillName;
    }

    inline void end() {
        if (!enabled()) return;
        std::cout << std::endl;
    }
}

#endif
