#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

// 气球类
class Balloon {
public:
    int id;          // 气球编号，对应人的ID
    bool found;      // 是否已被找到
    int foundBy;     // 被谁找到的（-1表示未找到）

    Balloon(int i) : id(i), found(false), foundBy(-1) {}
};

// 人类
class Person {
public:
    int id;          // 人员ID
    bool hasBalloon; // 是否有自己的气球
    int balloonsHeld[2]; // 当前拿着的气球ID（-1表示空手）
    bool hasEntered; // 是否进过仓库
    int partner;     // 配对伙伴ID

    Person(int i) : id(i), hasBalloon(false), hasEntered(false), partner(-1) {
        balloonsHeld[0] = balloonsHeld[1] = -1;
    }
};

// 仓库类
class Warehouse {
private:
    vector<Balloon> balloons;
    vector<Person> people;
    int rounds;  // 总轮次数

public:
    Warehouse(int total = 50) : rounds(0) {
        // 初始化50个人和50个气球
        for (int i = 0; i < total; i++) {
            people.push_back(Person(i));
            balloons.push_back(Balloon(i));
        }
    }

    // 算法1：随机配对策略
    void randomPairingStrategy() {
        cout << "=== 随机配对策略 ===" << endl;

        // 第一步：随机配对
        vector<int> unpaired;
        for (int i = 0; i < 50; i++) unpaired.push_back(i);

        random_shuffle(unpaired.begin(), unpaired.end());

        for (int i = 0; i < 25; i++) {
            int p1 = unpaired[i * 2];
            int p2 = unpaired[i * 2 + 1];
            people[p1].partner = p2;
            people[p2].partner = p1;
        }

        // 第二步：按轮次进入仓库
        int remaining = 50;
        while (remaining > 0) {
            rounds++;
            cout << "\n第 " << rounds << " 轮：" << endl;

            // 选择5组配对（每组派1人进入）
            vector<int> entrants;
            int count = 0;

            for (int i = 0; i < 50 && count < 5; i++) {
                if (!people[i].hasBalloon && !people[i].hasEntered) {
                    // 如果配对的另一个人也没有气球，优先让这个人进
                    if (!people[people[i].partner].hasBalloon) {
                        entrants.push_back(i);
                        people[i].hasEntered = true;
                        count++;
                    }
                }
            }

            // 如果不够5人，补足
            for (int i = 0; i < 50 && count < 5; i++) {
                if (!people[i].hasBalloon && !people[i].hasEntered &&
                    find(entrants.begin(), entrants.end(), i) == entrants.end()) {
                    entrants.push_back(i);
                    people[i].hasEntered = true;
                    count++;
                }
            }

            // 模拟进入仓库寻找气球
            cout << "进入仓库的人员: ";
            for (int id : entrants) cout << id << " ";
            cout << endl;

            int foundThisRound = 0;
            for (int personId : entrants) {
                Person& p = people[personId];

                // 寻找自己的气球
                if (!p.hasBalloon && !balloons[personId].found) {
                    p.hasBalloon = true;
                    p.balloonsHeld[0] = personId;
                    balloons[personId].found = true;
                    balloons[personId].foundBy = personId;
                    remaining--;
                    foundThisRound++;
                }

                // 寻找配对伙伴的气球（如果还有空位）
                if (p.balloonsHeld[1] == -1 && !people[p.partner].hasBalloon &&
                    !balloons[p.partner].found) {
                    p.balloonsHeld[1] = p.partner;
                    balloons[p.partner].found = true;
                    balloons[p.partner].foundBy = personId;
                    people[p.partner].hasBalloon = true;
                    remaining--;
                    foundThisRound++;
                }
            }

            cout << "本轮找到气球数: " << foundThisRound << endl;
            cout << "剩余未找到人数: " << remaining << endl;

            if (remaining == 0) break;
        }

        cout << "\n总轮次: " << rounds << endl;
    }

    // 算法2：分组轮换策略（更高效）
    void groupRotationStrategy() {
        cout << "\n=== 分组轮换策略 ===" << endl;

        // 重置状态
        reset();

        // 将50人分成10组，每组5人
        vector<vector<int>> groups(10);
        for (int i = 0; i < 50; i++) {
            groups[i / 5].push_back(i);
        }

        // 每轮进入一个组，但每个组员负责自己和一个特定组员的气球
        for (int round = 0; round < 10; round++) {
            rounds = round + 1;

            // 当前轮次进入的组
            int currentGroup = round % 10;

            cout << "\n第 " << rounds << " 轮 - 第 " << currentGroup << " 组进入" << endl;
            cout << "组员: ";
            for (int id : groups[currentGroup]) cout << id << " ";
            cout << endl;

            // 每个组员的目标：自己的气球 + 下一组对应位置的气球
            int foundThisRound = 0;
            for (int i = 0; i < 5; i++) {
                int personId = groups[currentGroup][i];
                Person& p = people[personId];

                // 目标1：自己的气球
                if (!p.hasBalloon && !balloons[personId].found) {
                    p.hasBalloon = true;
                    p.balloonsHeld[0] = personId;
                    balloons[personId].found = true;
                    remaining--;
                    foundThisRound++;
                }

                // 目标2：下一组对应位置的气球
                int nextGroup = (currentGroup + 1) % 10;
                int targetId = groups[nextGroup][i];

                if (p.balloonsHeld[1] == -1 && !people[targetId].hasBalloon &&
                    !balloons[targetId].found) {
                    p.balloonsHeld[1] = targetId;
                    balloons[targetId].found = true;
                    balloons[targetId].foundBy = personId;
                    people[targetId].hasBalloon = true;
                    remaining--;
                    foundThisRound++;
                }
            }

            cout << "本轮找到气球数: " << foundThisRound << endl;
            cout << "剩余未找到人数: " << remaining << endl;

            if (remaining == 0) break;
        }

        cout << "\n总轮次: " << rounds << endl;
    }

    // 算法3：智能调度算法（最优）
    void optimalScheduling() {
        cout << "\n=== 智能调度算法 ===" << endl;
        reset();

        // 状态跟踪
        vector<bool> hasBalloon(50, false);
        vector<bool> balloonFound(50, false);

        int round = 0;
        int remaining = 50;

        while (remaining > 0) {
            round++;
            cout << "\n第 " << round << " 轮：" << endl;

            // 选择5个还没有气球的人
            vector<int> entrants;
            for (int i = 0; i < 50 && entrants.size() < 5; i++) {
                if (!hasBalloon[i]) {
                    entrants.push_back(i);
                }
            }

            cout << "进入人员: ";
            for (int id : entrants) cout << id << " ";
            cout << endl;

            // 每个人先找自己的气球
            for (int personId : entrants) {
                if (!balloonFound[personId]) {
                    balloonFound[personId] = true;
                    hasBalloon[personId] = true;
                    remaining--;
                }
            }

            // 如果还有空手的人，帮别人找
            for (int personId : entrants) {
                // 这个人已经拿了一个气球（自己的），还有一个空位
                // 帮谁找？找还没有被标记为已找到的气球
                for (int targetId = 0; targetId < 50; targetId++) {
                    if (!balloonFound[targetId] && targetId != personId) {
                        balloonFound[targetId] = true;
                        hasBalloon[targetId] = true;
                        remaining--;
                        break;
                    }
                }
            }

            cout << "本轮解决人数: " << (50 - remaining) << endl;
            cout << "剩余人数: " << remaining << endl;
        }

        cout << "\n总轮次: " << round << endl;
    }

    // 显示结果
    void showResults() {
        cout << "\n=== 最终结果 ===" << endl;
        for (int i = 0; i < 50; i++) {
            cout << "人" << i << ": ";
            if (people[i].hasBalloon) {
                cout << "找到自己的气球";
                if (balloons[i].foundBy != i) {
                    cout << " (由" << balloons[i].foundBy << "帮忙找到)";
                }
            }
            else {
                cout << "未找到气球";
            }
            cout << endl;
        }
    }

private:
    void reset() {
        rounds = 0;
        remaining = 50;
        for (int i = 0; i < 50; i++) {
            people[i].hasBalloon = false;
            people[i].hasEntered = false;
            people[i].balloonsHeld[0] = people[i].balloonsHeld[1] = -1;
            balloons[i].found = false;
            balloons[i].foundBy = -1;
        }
    }

    int remaining;
};

// 简化版算法（用于理解）
void simplifiedAlgorithm() {
    cout << "=== 简化版最优算法 ===" << endl;

    /* 策略：
       1. 将50人编号为0-49
       2. 每5人一组，共10组：G0-G9
       3. 进行5轮，每轮进一组（5人），每人拿2个气球
       4. 第i轮（i=0..4），第Gi组进入，每人负责：
          - 自己的气球
          - 第G(i+5)组中对应位置人的气球
       5. 这样，5轮后所有人都能找到气球
    */

    cout << "\n分组方案：" << endl;
    for (int i = 0; i < 10; i++) {
        cout << "组" << i << ": ";
        for (int j = 0; j < 5; j++) {
            cout << (i * 5 + j) << " ";
        }
        cout << endl;
    }

    cout << "\n轮次安排：" << endl;
    for (int round = 0; round < 5; round++) {
        cout << "\n第" << round + 1 << "轮：" << endl;
        cout << "  进入组: " << round << endl;
        cout << "  每个人负责:" << endl;

        for (int pos = 0; pos < 5; pos++) {
            int personId = round * 5 + pos;
            int targetId = (round + 5) * 5 + pos;  // 对应位置的人
            cout << "    人" << personId << ": 自己的气球 + 人" << targetId << "的气球" << endl;
        }
    }

    cout << "\n分析：" << endl;
    cout << "- 每轮5人进入，每人拿2个气球 → 每轮带出10个气球" << endl;
    cout << "- 5轮共带出50个气球，刚好完成" << endl;
    cout << "- 每个人只进入一次仓库" << endl;
    cout << "- 完美利用协作，达到理论最优解" << endl;
}

int main() {
    // 运行简化版算法说明
    simplifiedAlgorithm();

    cout << "\n\n" << string(50, '=') << "\n\n";

    // 运行模拟
    Warehouse warehouse;

    cout << "模拟运行三种策略：" << endl;
    warehouse.randomPairingStrategy();

    cout << "\n" << string(40, '-') << endl;

    warehouse.groupRotationStrategy();

    cout << "\n" << string(40, '-') << endl;

    warehouse.optimalScheduling();

    // warehouse.showResults();

    return 0;
}