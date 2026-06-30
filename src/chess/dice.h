#ifndef DICE_H
#define DICE_H

#include <random>

// DnD 骰子：Dice(20).roll() 掷 d20；Dice(8) + 3 掷 d8 加 3
class Dice {
    int sides;
    static std::mt19937& gen() {
        static std::mt19937 g(std::random_device{}());
        return g;
    }
public:
    explicit Dice(int s) : sides(s) {}
    int roll() const {
        std::uniform_int_distribution<int> dist(1, sides);
        return dist(gen());
    }
    int operator+(int bonus) const { return roll() + bonus; }
    int operator-(int penalty) const { return roll() - penalty; }
};

// DnD 属性调整值：(属性 - 10) / 2
inline int getMod(int ability) { return (ability - 10) / 2; }

#endif
