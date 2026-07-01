#ifndef TUI_H
#define TUI_H

#include <iostream>
#include <string>

// ANSI 颜色码工具

// 前景色
inline std::string red(const std::string& s)     { return "\033[31m" + s + "\033[0m"; }
inline std::string green(const std::string& s)   { return "\033[32m" + s + "\033[0m"; }
inline std::string yellow(const std::string& s)  { return "\033[33m" + s + "\033[0m"; }
inline std::string blue(const std::string& s)    { return "\033[34m" + s + "\033[0m"; }
inline std::string magenta(const std::string& s) { return "\033[35m" + s + "\033[0m"; }
inline std::string cyan(const std::string& s)    { return "\033[36m" + s + "\033[0m"; }
inline std::string white(const std::string& s)   { return "\033[37m" + s + "\033[0m"; }

// 加粗
inline std::string bold(const std::string& s)    { return "\033[1m" + s + "\033[0m"; }

// 按职业系别着色
inline std::string meleeColor(const std::string& s)    { return "\033[31m" + s + "\033[0m"; }  // 近战-红
inline std::string rangedColor(const std::string& s)   { return "\033[32m" + s + "\033[0m"; }  // 远程-绿
inline std::string casterColor(const std::string& s)   { return "\033[34m" + s + "\033[0m"; }  // 法术-蓝
inline std::string advancedColor(const std::string& s) { return "\033[1;35m" + s + "\033[0m"; } // 进阶-紫粗

// HP 颜色（低血量变红）
inline std::string hpColor(int hp, int hpMax) {
    std::string s = std::to_string(hp) + "/" + std::to_string(hpMax);
    if (hp * 100 < hpMax * 30) return "\033[31m" + s + "\033[0m";  // <30% 红
    if (hp * 100 < hpMax * 60) return "\033[33m" + s + "\033[0m";  // <60% 黄
    return "\033[32m" + s + "\033[0m";                              // 绿
}

// 清屏
inline void clearScreen() { std::cout << "\033[2J\033[H"; }

// 分隔线
inline void printLine(int width = 50) {
    std::cout << "\033[36m" << std::string(width, '=') << "\033[0m" << std::endl;
}

// 标题
inline void printTitle(const std::string& title) {
    std::cout << "\033[1;36m" << title << "\033[0m" << std::endl;
}

#endif
