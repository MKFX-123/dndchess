#include "Game.h"
#include "../tui.h"
#include "../battlelog.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

// 辅助：用于"按回车继续"的占位变量
namespace { std::string& input_dummy() { static std::string s; return s; } }

Game::Game() : player(nullptr), ai(nullptr), gameOver(false), battleWinner(0), battleDamage(0) {}

Game::~Game() {
    delete player;
    delete ai;
}

// ============================================================
// 主循环
// ============================================================

void Game::run() {
    while (!gameOver) {
        showMainMenu();
    }
}

void Game::showMainMenu() {
    clearScreen();
    printLine(50);
    printTitle("  自走棋对战系统 v1.0");
    printLine(50);
    std::cout << "\n  " << yellow("1.") << " 开始新游戏\n" << std::endl;
    std::cout << "  " << yellow("2.") << " 读取存档\n" << std::endl;
    std::cout << "  " << yellow("3.") << " 退出游戏\n" << std::endl;
    std::cout << "  请选择: ";

    std::string input;
    std::getline(std::cin, input);

    if (input == "1") startGame();
    else if (input == "2") loadGame();
    else if (input == "3") gameOver = true;
}

void Game::startGame() {
    clearScreen();
    printLine(40);
    printTitle("  新游戏");
    printLine(40);

    std::cout << "\n  请输入玩家名: ";
    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) name = "玩家";

    delete player;
    delete ai;
    player = new Player(name);
    ai = new AIPlayer("AI对手");

    std::cout << "\n  " << green("游戏开始！") << std::endl;
    std::cout << "  你有 50 点生命，击败 AI 即可获胜。\n" << std::endl;
    std::cout << "  按回车继续...";
    std::cin.ignore();  // 清除输入缓冲区的换行符

    // 主游戏循环
    while (!gameOver) {
        preparePhase();
        if (gameOver) break;
        battlePhase();
        settlePhase();
    }
}

// ============================================================
// 准备阶段（命令交互）
// ============================================================

void Game::preparePhase() {
    player->gainGold();
    ai->gainGold();
    ai->autoPlay();
    ai->nextRound();

    std::string cmd;
    while (true) {
        renderUI();
        std::cout << "\n  > ";
        std::getline(std::cin, cmd);

        if (cmd == "next" || cmd == "n") {
            if (player->getBoardCount() == 0) {
                std::cout << "  " << red("战场上至少需要一个棋子！") << std::endl;
                std::cout << "  按回车继续...";
                std::getline(std::cin, input_dummy());
                continue;
            }
            break;
        }
        if (cmd == "quit" || cmd == "q") {
            gameOver = true;
            return;
        }
        if (!handleCommand(cmd)) {
            std::cout << "  " << red("未知命令，输入 help 查看帮助") << std::endl;
            std::cout << "  按回车继续...";
            std::getline(std::cin, input_dummy());
        }
    }
    player->nextRound();
}

bool Game::handleCommand(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string op;
    iss >> op;

    if (op == "help" || op == "h") {
        showHelp();
        return true;
    }
    if (op == "shop") { showShop(); return true; }
    if (op == "bench") { showBench(); return true; }
    if (op == "board") { showBoard(); return true; }
    if (op == "status") { showStatus(); return true; }
    if (op == "save") { saveGame(); return true; }
    if (op == "log") {
        BattleLog::enabled() = !BattleLog::enabled();
        std::cout << "  " << (BattleLog::enabled() ? green("战斗日志已开启") : yellow("战斗日志已关闭")) << std::endl;
        std::cout << "  按回车继续...";
        std::getline(std::cin, input_dummy());
        return true;
    }

    if (op == "buy") {
        int n;
        if (!(iss >> n)) { std::cout << "  " << red("用法: buy <编号>") << std::endl; return true; }
        if (player->buyPiece(n - 1)) {
            std::cout << "  " << green("购买成功！") << std::endl;
        } else {
            std::cout << "  " << red("购买失败（金币不足或编号无效）") << std::endl;
        }
        std::cout << "  按回车继续...";
        std::getline(std::cin, input_dummy());
        return true;
    }
    if (op == "refresh") {
        if (player->spendGold(1)) {
            player->refreshShop();
            std::cout << "  " << green("商店已刷新") << std::endl;
        } else {
            std::cout << "  " << red("金币不足") << std::endl;
        }
        std::cout << "  按回车继续...";
        std::getline(std::cin, input_dummy());
        return true;
    }
    if (op == "upgrade-shop" || op == "us") {
        if (player->upgradeShop()) {
            std::cout << "  " << green("商店升级到 ") << player->getShopLevel() << green(" 级") << std::endl;
        } else {
            std::cout << "  " << red("金币不足或已满级") << std::endl;
        }
        std::cout << "  按回车继续...";
        std::getline(std::cin, input_dummy());
        return true;
    }
    if (op == "upgrade" || op == "up") {
        int n;
        if (!(iss >> n)) { std::cout << "  " << red("用法: upgrade <备战席编号>") << std::endl; return true; }
        if (player->upgradePiece(n - 1)) {
            std::cout << "  " << green("升级成功！") << std::endl;
        } else {
            std::cout << "  " << red("升级失败（金币不足或编号无效）") << std::endl;
        }
        std::cout << "  按回车继续...";
        std::getline(std::cin, input_dummy());
        return true;
    }
    if (op == "place") {
        int b, p;
        if (!(iss >> b >> p)) { std::cout << "  " << red("用法: place <备战席编号> <战场位置0-6>") << std::endl; return true; }
        player->arrangeBoard(b - 1, p);
        std::cout << "  " << green("已放置") << std::endl;
        std::cout << "  按回车继续...";
        std::getline(std::cin, input_dummy());
        return true;
    }
    if (op == "remove") {
        int p;
        if (!(iss >> p)) { std::cout << "  " << red("用法: remove <战场位置0-6>") << std::endl; return true; }
        player->removeFromBoard(p);
        std::cout << "  " << green("已收回") << std::endl;
        std::cout << "  按回车继续...";
        std::getline(std::cin, input_dummy());
        return true;
    }
    if (op == "merge") {
        int a, b;
        if (!(iss >> a >> b)) { std::cout << "  " << red("用法: merge <编号A> <编号B>") << std::endl; return true; }
        ChessPiece* result = player->mergePiece(a - 1, b - 1);
        if (result) {
            std::cout << "  " << green("合并成功！获得: ") << result->getName() << std::endl;
        } else {
            std::cout << "  " << red("合并失败（需同等级同系别不同职业）") << std::endl;
        }
        std::cout << "  按回车继续...";
        std::getline(std::cin, input_dummy());
        return true;
    }

    return false;
}

// ============================================================
// TUI 渲染
// ============================================================

void Game::renderUI() {
    clearScreen();
    printLine(50);
    printTitle("  自走棋对战系统  回合 " + std::to_string(player->getRound()));
    printLine(50);

    // 玩家状态栏
    std::cout << "\n  " << bold(player->getName())
              << "  生命: " << hpColor(player->getHP(), 50)
              << "  金币: " << yellow(std::to_string(player->getGold()) + "g")
              << "  商店: " << std::to_string(player->getShopLevel()) << "级"
              << "\n" << std::endl;

    // 左侧：商店 + 备战席 + 战场  |  右侧：命令列表
    // 商店
    std::cout << "  " << cyan("【商店】") << " (刷新1g, 升级"
              << (player->getShopLevel() >= 5 ? "已满级" : std::to_string(player->getShopLevel() + 4) + "g")
              << ")" << std::endl;
    auto& shop = player->getShop();
    for (size_t i = 0; i < shop.size(); i++) {
        std::cout << "    " << yellow(std::to_string(i + 1) + ".") << " ";
        shop[i]->display();
        std::cout << std::endl;
    }

    // 备战席
    std::cout << "\n  " << cyan("【备战席】") << std::endl;
    auto& bench = player->getBench();
    if (bench.empty()) {
        std::cout << "    (空)" << std::endl;
    } else {
        for (size_t i = 0; i < bench.size(); i++) {
            std::cout << "    " << yellow(std::to_string(i + 1) + ".") << " ";
            bench[i]->display();
            std::cout << std::endl;
        }
    }

    // 战场
    std::cout << "\n  " << cyan("【战场阵型】") << std::endl;
    auto& board = player->getBoard();
    std::cout << "  +---+---+---+---+---+---+---+" << std::endl;
    std::cout << "  |";
    for (int i = 0; i < 7; i++) {
        if (board[i]) {
            std::string sym = board[i]->getSymbol();
            if (board[i]->isTaunt()) std::cout << meleeColor(" " + sym + " ");
            else if (board[i]->isStealth()) std::cout << rangedColor(" " + sym + " ");
            else std::cout << " " << sym << " ";
        } else {
            std::cout << " . ";
        }
        std::cout << "|";
    }
    std::cout << std::endl;
    std::cout << "  +---+---+---+---+---+---+---+" << std::endl;
    for (int i = 0; i < 7; i++) {
        if (board[i]) {
            std::cout << "    [" << i << "] ";
            board[i]->display();
            std::cout << std::endl;
        }
    }

    // 右侧命令列表
    std::cout << "\n  " << magenta("━━━ 命令列表 ━━━") << std::endl;
    std::cout << "  " << yellow("shop")     << "    查看商店" << std::endl;
    std::cout << "  " << yellow("buy <n>")  << "  购买商店第n个" << std::endl;
    std::cout << "  " << yellow("refresh")  << "  刷新商店(1g)" << std::endl;
    std::cout << "  " << yellow("us")       << "      升级商店" << std::endl;
    std::cout << "  " << yellow("bench")    << "   查看备战席" << std::endl;
    std::cout << "  " << yellow("board")    << "   查看战场" << std::endl;
    std::cout << "  " << yellow("place <b> <p>") << " 备战b放战场p" << std::endl;
    std::cout << "  " << yellow("remove <p>") << "  收回战场p" << std::endl;
    std::cout << "  " << yellow("up <n>")   << "    升级备战n" << std::endl;
    std::cout << "  " << yellow("merge <a> <b>") << " 合并a和b" << std::endl;
    std::cout << "  " << yellow("status")   << "   查看状态" << std::endl;
    std::cout << "  " << yellow("save")     << "    存档" << std::endl;
    std::cout << "  " << yellow("log")      << "     切换战斗日志" << std::endl;
    std::cout << "  " << yellow("next")     << "    开始战斗" << std::endl;
    std::cout << "  " << yellow("help")     << "    帮助" << std::endl;
    std::cout << "  " << yellow("quit")     << "    退出" << std::endl;
}

void Game::showShop() {
    auto& shop = player->getShop();
    std::cout << "\n  " << cyan("【商店】") << std::endl;
    for (size_t i = 0; i < shop.size(); i++) {
        std::cout << "  " << yellow(std::to_string(i + 1) + ".") << " ";
        shop[i]->display();
        std::cout << std::endl;
    }
    std::cout << "  按回车继续...";
    std::getline(std::cin, input_dummy());
}

void Game::showBench() {
    auto& bench = player->getBench();
    std::cout << "\n  " << cyan("【备战席】") << std::endl;
    if (bench.empty()) {
        std::cout << "  (空)" << std::endl;
    } else {
        for (size_t i = 0; i < bench.size(); i++) {
            std::cout << "  " << yellow(std::to_string(i + 1) + ".") << " ";
            bench[i]->display();
            std::cout << std::endl;
        }
    }
    std::cout << "  按回车继续...";
    std::getline(std::cin, input_dummy());
}

void Game::showBoard() {
    auto& board = player->getBoard();
    std::cout << "\n  " << cyan("【战场阵型】") << std::endl;
    std::cout << "  +---+---+---+---+---+---+---+" << std::endl;
    std::cout << "  |";
    for (int i = 0; i < 7; i++) {
        if (board[i]) {
            std::cout << " " << board[i]->getSymbol() << " ";
        } else {
            std::cout << " . ";
        }
        std::cout << "|";
    }
    std::cout << std::endl;
    std::cout << "  +---+---+---+---+---+---+---+" << std::endl;
    for (int i = 0; i < 7; i++) {
        if (board[i]) {
            std::cout << "  [" << i << "] ";
            board[i]->display();
            std::cout << std::endl;
        }
    }
    std::cout << "  按回车继续...";
    std::getline(std::cin, input_dummy());
}

void Game::showStatus() {
    std::cout << "\n  " << cyan("【玩家状态】") << std::endl;
    player->display();
    std::cout << "\n  按回车继续...";
    std::getline(std::cin, input_dummy());
}

void Game::showHelp() {
    std::cout << "\n  " << magenta("━━━ 命令帮助 ━━━") << std::endl;
    std::cout << "  shop            查看商店" << std::endl;
    std::cout << "  buy <n>         购买商店第n个棋子" << std::endl;
    std::cout << "  refresh         刷新商店(1g)" << std::endl;
    std::cout << "  us              升级商店" << std::endl;
    std::cout << "  bench           查看备战席" << std::endl;
    std::cout << "  board           查看战场" << std::endl;
    std::cout << "  place <b> <p>   备战席b放战场p" << std::endl;
    std::cout << "  remove <p>      收回战场p" << std::endl;
    std::cout << "  up <n>          升级备战席n" << std::endl;
    std::cout << "  merge <a> <b>   合并备战席a和b" << std::endl;
    std::cout << "  status          查看状态" << std::endl;
    std::cout << "  save            存档" << std::endl;
    std::cout << "  log             切换战斗日志开关" << std::endl;
    std::cout << "  next            开始战斗" << std::endl;
    std::cout << "  quit            退出游戏" << std::endl;
    std::cout << "  按回车继续...";
    std::getline(std::cin, input_dummy());
}

// ============================================================
// 战斗阶段
// ============================================================

void Game::battlePhase() {
    Battlefield bf;
    bf.setup(*player, *ai);
    int winner = bf.battle();

    // 战斗结果临时存储，结算阶段用
    battleWinner = winner;
    if (winner == 1) {
        battleDamage = bf.getSurvivorLevelSum(1);
    } else {
        battleDamage = bf.getSurvivorLevelSum(2);
    }

    std::cout << "\n  按回车继续...";
    std::getline(std::cin, input_dummy());
}

// ============================================================
// 结算阶段
// ============================================================

void Game::settlePhase() {
    clearScreen();
    printLine(40);
    printTitle("  回合结算");
    printLine(40);

    if (battleWinner == 1) {
        std::cout << "\n  " << green("胜利！") << std::endl;
        std::cout << "  AI 受到 " << battleDamage << " 点伤害" << std::endl;
        ai->takeDamage(battleDamage);
        // 胜利奖励在下一回合 gainGold 时体现，这里显示提示
        std::cout << "  " << yellow("下回合额外获得 1g 胜利奖励") << std::endl;
    } else {
        std::cout << "\n  " << red("失败！") << std::endl;
        std::cout << "  你受到 " << battleDamage << " 点伤害" << std::endl;
        player->takeDamage(battleDamage);
    }

    std::cout << "\n  " << bold(player->getName())
              << "  生命: " << hpColor(player->getHP(), 50) << std::endl;
    std::cout << "  " << bold(ai->getName())
              << "  生命: " << hpColor(ai->getHP(), 50) << std::endl;

    // 判定游戏结束
    if (player->isDead()) {
        std::cout << "\n  " << red("游戏失败！你的生命归零了。") << std::endl;
        gameOver = true;
    } else if (ai->isDead()) {
        std::cout << "\n  " << green("游戏胜利！你击败了 AI！") << std::endl;
        gameOver = true;
    }

    std::cout << "\n  按回车继续...";
    std::getline(std::cin, input_dummy());
}

// ============================================================
// 存档/读档
// ============================================================

void Game::saveGame() {
    std::ofstream ofs("savegame.txt");
    if (!ofs) {
        std::cout << "  " << red("存档失败！") << std::endl;
        return;
    }
    ofs << player->getName() << " "
        << player->getHP() << " "
        << player->getGold() << " "
        << player->getRound() << " "
        << player->getShopLevel() << std::endl;
    ofs.close();
    std::cout << "  " << green("存档成功！") << std::endl;
    std::cout << "  按回车继续...";
    std::getline(std::cin, input_dummy());
}

void Game::loadGame() {
    std::ifstream ifs("savegame.txt");
    if (!ifs) {
        std::cout << "  " << red("没有找到存档！") << std::endl;
        std::cout << "  按回车继续...";
        std::getline(std::cin, input_dummy());
        return;
    }
    std::string name;
    int hp, gold, round, shopLevel;
    ifs >> name >> hp >> gold >> round >> shopLevel;
    ifs.close();

    delete player;
    delete ai;
    player = new Player(name);
    ai = new AIPlayer("AI对手");

    // 恢复状态（简化：只恢复基本属性）
    while (player->getRound() < round) player->nextRound();
    while (player->getShopLevel() < shopLevel) player->upgradeShop();

    std::cout << "  " << green("读档成功！") << std::endl;
    std::cout << "  按回车继续...";
    std::getline(std::cin, input_dummy());

    while (!gameOver) {
        preparePhase();
        if (gameOver) break;
        battlePhase();
        settlePhase();
    }
}
