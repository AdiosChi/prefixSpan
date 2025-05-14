#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <unordered_map>
#include <queue>
#include <fstream>
#include <sstream>
#include <algorithm>
#include<map>
#include<set>
using namespace std;


class Entry {



public:
    string key;
    int value;
    // 構造函數
    Entry(const string& key, int value)
        : key(key), value(value) {}

    // Getter 方法
    string getKey() const {
        return key;
    }

    int getValue() const {
        return value;
    }

    // 排序
    bool operator<(const Entry& other) const {
        return value > other.value;
    }

};


class Sequence {
private:
    int startIdx;
    vector<string> firstItem;
    vector<vector<string>> seq;

public:
    Sequence(int idx) : startIdx(idx) {}
    Sequence(int idx, const vector<string>& firstItem, const vector<vector<string>>& seq)
        : startIdx(idx), firstItem(firstItem), seq(seq) {}


    vector<vector<string>> getSeq() const {
        return seq;
    }

    vector<string> getFirstItem() const {
        return firstItem;
    }

    int getStartIdx() const {
        return startIdx;
    }

    void setSeq(const vector<vector<string>>& newSeq) {
        seq = newSeq;
    }

    void setFirstItem(const vector<string>& newFirstItem) {
        firstItem = newFirstItem;
    }

    int size() const {
        return seq.size();
    }

    vector<string> get(int i) const {
        if (i == startIdx && !firstItem.empty()) {
            return firstItem;
        }
        return seq[i];
    }
    std::string toString() const {
        std::string str;
        for (int i = startIdx; i < seq.size(); i++) {
            const auto& itemset = get(i);
            str += "[" + join(itemset, ",") + "]";
        }
        return str;
    }


    static std::string join(const std::vector<std::string>& v, const std::string& delimiter) {
        std::ostringstream oss;
        for (size_t i = 0; i < v.size(); ++i) {
            if (i != 0) oss << delimiter;
            oss << v[i];
        }
        return oss.str();
    }
};


class PrefixSpan {
private:
    string inputPath;
    int numSequences;
    double minSup;
    double threshold;
    map<string, int> freqSeqPatterns;

public:
    PrefixSpan(const string& inputPath, double minSup) : inputPath(inputPath), minSup(minSup) {
        cout << "Threshold of Support: " + to_string(minSup) << endl;
    }

    map<string, int> findPatterns(vector<Sequence>& sequences) {
        freqSeqPatterns.clear();

        map<string, int> suffixItemMap = ISitemsetCount(sequences, vector<string>());
        for (const auto& pair : suffixItemMap) {
            const string& suffixItem = pair.first;
            if (pair.second >= threshold) {
                vector<vector<string>> prefixSeq;
                prefixSeq.push_back(vector<string>{suffixItem});
                freqSeqPatterns[getPrefixString(prefixSeq)] = pair.second;

                findProjSeq(sequences, prefixSeq);
            }
        }

        return freqSeqPatterns;
    }

private:
    //複製序列
    vector<vector<string>> copyStringList(const vector<vector<string>>& seq) {
        vector<vector<string>> newList;
        for (const auto& itemset : seq) {
            newList.push_back(itemset);
        }
        return newList;
    }

    static vector<string> addStringArr(const vector<string>& a, const string& e) {
        vector<string> newArr(a.size() + 1);
        bool added = false;
        for (size_t i = 0; i <= a.size(); i++) {
            if (added) {
                newArr[i] = a[i - 1];
            }
            else if (i == a.size() || e < a[i]) {
                newArr[i] = e;
                added = true;
            }
            else {
                newArr[i] = a[i];
            }
        }
        return newArr;
    }

    //I和S擴展
    vector<vector<string>> ISextension(const vector<vector<string>>& prefixSeq, const string& suffixItem) {
        vector<vector<string>> newPrefixSeq = copyStringList(prefixSeq);
        if (suffixItem.find("_") != string::npos) { //I擴展
            vector<string> itemset = newPrefixSeq.back();//取得最後一個itemset
            vector<string> lastPrefix = addStringArr(itemset, suffixItem.substr(1));//在把itemset加_之後的
            newPrefixSeq.back() = lastPrefix;
        }
        else {
            newPrefixSeq.push_back(vector<string>{suffixItem}); //S擴展
        }
        return newPrefixSeq;
    }
    //把前面的擴展完的set存下來
    string getPrefixString(const vector<vector<string>>& prefixSeq) {
        string prefix;
        for (const auto& itemset : prefixSeq) {
            prefix += "[";
            for (const auto& item : itemset) {
                prefix += item + ",";
            }
            if (!itemset.empty()) prefix.pop_back();
            prefix += "]";
        }
        return prefix;
    }
    //遞迴搜尋投影序列
    void findProjSeq(vector<Sequence>& sequences, vector<vector<string>>& prefixSeq) {
        vector<Sequence> projectedSeq = projSeq(sequences, prefixSeq);
        map<string, int> suffixItemMap = ISitemsetCount(projectedSeq, prefixSeq.back());

        for (const auto& pair : suffixItemMap) {
            const string& suffixItem = pair.first;
            if (pair.second >= threshold) {
                vector<vector<string>> newprefixSeq = ISextension(prefixSeq, suffixItem); //跑I和S擴展

                //把新的prefixSeq存到freqSeqPatterns
                if (freqSeqPatterns.find(getPrefixString(newprefixSeq)) != freqSeqPatterns.end()) {
                    continue;
                }
                freqSeqPatterns[getPrefixString(newprefixSeq)] = pair.second;

                findProjSeq(projectedSeq, newprefixSeq);
            }
        }
    }

    vector<Sequence> projSeq(const vector<Sequence>& sequences, const vector<vector<string>>& prefixSeq) {
        vector<Sequence> projectedSequences;

        for (const Sequence& seq : sequences) { //跑整個序列
            for (int i = seq.getStartIdx(); i < seq.size(); i++) { //讀同一時間的
                vector<string> itemset = seq.get(i);
                vector<string> lastPrefix = prefixSeq.back();

                bool existPrefixItem = true;
                if (lastPrefix.size() == 1 && find(itemset.begin(), itemset.end(), "_") != itemset.end()) {//確認這個item是不是最後一個
                    existPrefixItem = false;
                }
                else if (find(itemset.begin(), itemset.end(), lastPrefix.back()) == itemset.end()) {
                    existPrefixItem = false;
                }
                else {
                    for (const string& prefixItem : lastPrefix) { //prefixItem且_是itemset最後一個的話
                        if (find(itemset.begin(), itemset.end(), prefixItem) == itemset.end() &&
                            find(itemset.begin(), itemset.end(), "_") == itemset.end()) {
                            existPrefixItem = false;
                            break;
                        }
                        itemset.erase(remove(itemset.begin(), itemset.end(), prefixItem), itemset.end()); //不是就把目前的prefixItem刪掉
                    }
                }
                //如果還有有找到PrefixItem
                if (existPrefixItem) {
                    for (size_t j = 0; j < prefixSeq.size() - 1; j++) {
                        for (const string& prefixItem : prefixSeq[j]) {
                            itemset.erase(remove(itemset.begin(), itemset.end(), prefixItem), itemset.end()); //把投影前的序列刪掉
                        }
                    }

                    itemset.erase(remove(itemset.begin(), itemset.end(), "_"), itemset.end());//把_刪掉
                    //如果itemset還有剩餘的item 把_加回來並從存到投影序列
                    if (!itemset.empty()) {
                        itemset.push_back("_");
                        projectedSequences.emplace_back(i, itemset, seq.getSeq());
                        break;
                    }
                    //否則i+1還在序列範圍內就把i+1後的序列都存到投影序列
                    else if ((i + 1) < seq.size()) {
                        projectedSequences.emplace_back(i + 1, vector<string>(), seq.getSeq());
                        break;
                    }
                }
            }
        }

        return projectedSequences;
    }
    //計算pattern出現次數
    map<string, int> countItem(const string& item, map<string, int>& map) {
        auto it = map.find(item);
        if (it == map.end()) {
            map[item] = 1;
        }
        else {
            it->second++;
        }
        return map;
    }

    map<string, int> ISitemsetCount(const vector<Sequence>& sequences, const vector<string>& lastPrefix) {
        map<string, int> suffixItems;
        for (const Sequence& seq : sequences) { //編歷所有sid
            set<string> addedItems; //用來記錄被countItem處理過的

            for (int i = seq.getStartIdx(); i < seq.size(); i++) {
                vector<string> itemset = seq.get(i);
                //先檢查有沒有_ 有的話刪掉，不然就檢查有沒有處理過
                bool existPrefixSymbol = (find(itemset.begin(), itemset.end(), "_") != itemset.end());
                if (existPrefixSymbol) {
                    itemset.erase(remove(itemset.begin(), itemset.end(), "_"), itemset.end());
                }
                else {
                    for (const string& item : itemset) {
                        if (addedItems.find(item) == addedItems.end()) {
                            countItem(item, suffixItems);
                            addedItems.insert(item);
                        }
                    }
                }
                //檢查Prefix有沒有已經被處理過了，如果有且還存在itemset裡就把他刪掉
                bool existPrefixItems = !lastPrefix.empty();
                if (existPrefixItems) {
                    for (const string& prefixItem : lastPrefix) {
                        if (find(itemset.begin(), itemset.end(), prefixItem) == itemset.end()) {
                            existPrefixItems = false;
                            break;
                        }
                        itemset.erase(remove(itemset.begin(), itemset.end(), prefixItem), itemset.end());
                    }
                }
                //判斷同個itemset有沒有處理過
                if (existPrefixSymbol || existPrefixItems) {
                    for (const string& item : itemset) {
                        if (addedItems.find("_" + item) == addedItems.end()) {
                            countItem("_" + item, suffixItems);
                            addedItems.insert("_" + item);
                        }
                    }
                }
            }
        }

        return suffixItems;
    }

public:

    vector<Sequence> LoadData() {
        vector<Sequence> sequences;
        map<string, priority_queue<Entry>> sequenceData;
        ifstream data_in(inputPath);
        string line;
        //資料讀取近來
        while (getline(data_in, line)) {
            istringstream iss(line);
            string sid;
            int tid;
            string item;
            iss >> sid >> tid >> item;

            sequenceData[sid].emplace(item, tid);
        }
        data_in.close();

        for (const auto& pair : sequenceData) {
            const string& sid = pair.first;
            const priority_queue<Entry>& rd = pair.second;
            vector<string> itemset;
            vector<vector<string>> sequence;
            int tid = -1;
            auto rd_copy = rd;
            //依序取讀到資料
            while (!rd_copy.empty()) {
                const Entry& entry = rd_copy.top();
                if (tid == -1 || entry.value == tid) {//判斷是不是同個時間購買
                    tid = entry.value;
                    itemset.push_back(entry.key);
                }
                else {
                    sequence.push_back(itemset);//如果不同就更新時間並加到下一個itemset
                    itemset.clear();
                    tid = entry.value;
                    itemset.push_back(entry.key);
                }
                rd_copy.pop();
                if (rd_copy.empty()) {
                    sequence.push_back(itemset);
                }
            }

            sequences.emplace_back(0, vector<string>(), sequence); //把class Sequence 要的資料存到sequences
        }

        numSequences = sequences.size();//算有幾個序列
        threshold = minSup * numSequences;
        cout << "有幾個SID :" + to_string(numSequences) << endl;
        return sequences;
    }
};

int main() {

    string filePath = "test.txt";
    double minSupport = 0;
    PrefixSpan prefixSpan(filePath, minSupport);

    vector<Sequence> sequences = prefixSpan.LoadData();
    map<string, int> patterns = prefixSpan.findPatterns(sequences);

    for (const auto& pattern : patterns) {
        cout << "Pattern: " << pattern.first << "  Support Count: " << pattern.second << endl;
    }

    return 0;
}
