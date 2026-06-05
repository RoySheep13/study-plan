#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>
#include <cstdlib>

using namespace std;

// 科目結構
struct Subject {
    string name;
    int level;  // 熟練度 1(不熟) ~ 5(很穩)
    int goal;   // 預計總章節數
    int done;   // 已完成章節數
};

// 全域變數
vector<Subject> subs;
string examName = "第二次段考";
string examDate = "2026-11-05";
int dailyBlocks = 4;        // 每天讀書節數 (一節約50分鐘)
string todayStr = "2026-06-05";
bool goBackTriggered = false; // 是否按下 b 回上一題

// 日期相差天數計算
int getDiffDays(string d1, string d2) {
    int y1, m1, day1, y2, m2, day2;
    sscanf(d1.c_str(), "%d-%d-%d", &y1, &m1, &day1);
    sscanf(d2.c_str(), "%d-%d-%d", &y2, &m2, &day2);

    tm t1 = {0, 0, 12, day1, m1 - 1, y1 - 1900};
    tm t2 = {0, 0, 12, day2, m2 - 1, y2 - 1900};

    time_t time1 = mktime(&t1);
    time_t time2 = mktime(&t2);

    return (int)floor(difftime(time2, time1) / 86400.0);
}

// 存檔與讀檔
void saveData() {
    ofstream out("plan_data.txt");
    out << examName << "\n" << examDate << "\n" << dailyBlocks << "\n";
    out << subs.size() << "\n";
    for (size_t i = 0; i < subs.size(); i++) {
        out << subs[i].name << " " << subs[i].level << " " 
            << subs[i].goal << " " << subs[i].done << "\n";
    }
    out.close();
}

void loadData() {
    ifstream in("plan_data.txt");
    if (!in.is_open()) {
        subs = {{"國文", 3, 5, 0}, {"英文", 2, 8, 0}, {"數學", 1, 10, 0}, {"自然", 3, 6, 0}, {"社會", 4, 4, 0}};
        return;
    }
    in >> examName >> examDate >> dailyBlocks;
    int count;
    in >> count;
    subs.clear();
    for (int i = 0; i < count; i++) {
        Subject s;
        in >> s.name >> s.level >> s.goal >> s.done;
        subs.push_back(s);
    }
    in.close();
}

// 核心演算法：累積額度 (Credit) 配節機制與進度分配
void generateAndPrintPlan() {
    int daysLeft = getDiffDays(todayStr, examDate);
    cout << "\n========================================\n";
    cout << "  距離 [" << examName << "] (" << examDate << ") 還有 " << daysLeft << " 天\n";
    cout << "========================================\n";

    if (daysLeft <= 0) {
        cout << "考試日期已過或設定錯誤！\n";
        return;
    }

    // 1. 計算各科權重 (熟練度越低，權重越高)
    double totalWeight = 0;
    vector<double> weights(subs.size());
    for (size_t i = 0; i < subs.size(); i++) {
        weights[i] = (6 - subs[i].level);
        totalWeight += weights[i];
    }

    // 2. 累積額度 (Credit) 演算法分配每日節數
    vector<double> credit(subs.size(), 0.0);
    vector<int> allocatedBlocks(subs.size(), 0);

    for (size_t i = 0; i < subs.size(); i++) {
        credit[i] += dailyBlocks * (weights[i] / totalWeight);
    }

    for (int b = 0; b < dailyBlocks; b++) {
        int maxIdx = 0;
        for (size_t i = 1; i < subs.size(); i++) {
            if (credit[i] > credit[maxIdx]) maxIdx = i;
        }
        allocatedBlocks[maxIdx]++;
        credit[maxIdx] -= 1.0; // 扣除已分配額度
    }

    // 3. 輸出今日建議計畫 (包含新進度與錯題複習時間)
    cout << "\n【今日推薦排程】(每日安排 " << dailyBlocks << " 節，1節=50分鐘)\n";
    for (size_t i = 0; i < subs.size(); i++) {
        if (allocatedBlocks[i] == 0) continue;

        int remain = subs[i].goal - subs[i].done;
        cout << " - " << subs[i].name << "：" << allocatedBlocks[i] << " 節";

        if (remain > 0) {
            int newUnit = subs[i].done + 1;
            cout << " (進度：第 " << newUnit << " 章，剩餘 " << remain << " 章未讀)";
        } else {
            cout << " (進度：完成預定章節，進行總複習與錯題整理)";
        }
        cout << "\n";
    }
}

// 讀取輸入與「按 b 回上一題」控制邏輯
string getInputLine(string prompt) {
    string input;
    cout << prompt;
    cin >> input;
    if (input == "b" || input == "B") {
        goBackTriggered = true;
    } else {
        goBackTriggered = false;
    }
    return input;
}

// 設定嚮導：支援逐步引導與按 b 返回上一題
void setupWizard() {
    cout << "\n--- 修改讀書計畫設定 (輸入 b 可回到上一題) ---\n";
    int step = 0;

    while (step < 3 + (int)subs.size()) {
        goBackTriggered = false;

        if (step == 0) {
            string val = getInputLine("1. 考試名稱 (目前: " + examName + "): ");
            if (goBackTriggered) { cout << "[已是最早項目]\n"; continue; }
            if (val != "") examName = val;
        } 
        else if (step == 1) {
            string val = getInputLine("2. 考試日期 YYYY-MM-DD (目前: " + examDate + "): ");
            if (goBackTriggered) { step--; continue; }
            if (val != "") examDate = val;
        } 
        else if (step == 2) {
            string val = getInputLine("3. 每日讀書節數 (目前: " + to_string(dailyBlocks) + " 節): ");
            if (goBackTriggered) { step--; continue; }
            if (val != "") dailyBlocks = atoi(val.c_str());
        } 
        else {
            int subIdx = step - 3;
            cout << "\n[" << subs[subIdx].name << "] 熟練度 1(不熟)~5(很穩) (目前 " << subs[subIdx].level << "): ";
            string val = getInputLine("");
            if (goBackTriggered) { step--; continue; }
            if (val != "") subs[subIdx].level = atoi(val.c_str());
        }

        step++;
    }

    saveData();
    cout << "\n[成功] 設定已更新並儲存至 plan_data.txt！\n";
}

int main() {
#ifdef _WIN32
    system("chcp 65001 > nul"); // 避免 CMD 中文亂碼
#endif

    loadData();
    int choice;

    while (true) {
        generateAndPrintPlan();
        cout << "\n----------------------------------------\n";
        cout << "今日日期：" << todayStr << "\n";
        cout << "1. 重新設定考試與各科進度 (支援 b 回上一題)\n";
        cout << "2. 修改今天日期\n";
        cout << "0. 儲存並離開\n";
        cout << "請選擇操作 (0-2)：";

        if (!(cin >> choice)) break;

        if (choice == 0) break;
        if (choice == 1) setupWizard();
        if (choice == 2) {
            cout << "請輸入新的今天日期 (YYYY-MM-DD)：";
            cin >> todayStr;
        }
    }

    cout << "\n加油！祝考試順利！\n";
    return 0;
}