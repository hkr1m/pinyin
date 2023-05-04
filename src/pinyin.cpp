#include <cassert>
#include <cstring>
#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <array>
using namespace std;

const int MAX_CHAR = 7000;
const int MAX_PINYIN = 450;
const int MAX_WORD = 10000000;
const double INF = 1E18;
string alphabet; // 汉字列表
string pinyin_list[MAX_PINYIN]; // 拼音列表，备用
int char_id[128][128]; // gbk 编码下汉字在 alphabet 中位置
unordered_map<string, int> pinyin_id; // 拼音在 pinyin_list 中位置
vector<int> char_of[MAX_PINYIN]; // 对应某个拼音的所有汉字的 char_id
vector<int> pinyin_of[MAX_CHAR]; // 对应某个汉字的所有拼音的 pinyin_id
int char_cnt, pinyin_cnt; // 汉字总数 & 拼音总数

template<const int N> struct ArrayHasher { // 长度为 N 的 std::array 的哈希方法
    size_t operator()(const array<int, N>& arr) const {
        size_t h = 0;
        for (auto e : arr)
            h ^= std::hash<int>{}(e)  + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

template<typename T> void prepend(T& arr, int x) { // 在 std::array 前端插入 x
    for (int i=arr.size()-1; i>=1; i--) arr[i] = arr[i-1];
    arr.front() = x;
}

template<typename T> void append(T& arr, int x) { // 在 std::array 后端插入 x
    for (int i=0; i+1<arr.size(); i++) arr[i] = arr[i+1];
    arr.back() = x;
}

void loadAlphabet() { // 载入汉字列表
    ifstream fin("../src/alphabet.txt");
    fin >> alphabet;
    fin.close();
    char_cnt = 0;
    for (int i=0; 2*i+1 < alphabet.size(); i++) {
        char_id[-alphabet[2*i]][-alphabet[2*i+1]] = i+1;
        char_cnt++;
    }
}

template<typename T> string chidToStr(const T& arr) { // 转换 char_id 列表到汉字串
    string res;
    for (int ch: arr) if (ch)
        res += alphabet.substr(2*ch-2, 2);
    return res;
}

void loadPinyin() { // 载入汉字与拼音的对应关系
    ifstream fin("../src/pinyin.txt");
    string cur_line, str;
    pinyin_cnt = 0;
    while (getline(fin, cur_line)) {
        stringstream ss(cur_line);
        ss >> str;
        pinyin_id[str] = ++pinyin_cnt;
        pinyin_list[pinyin_cnt] = str;
        while (ss >> str) {
            int id = char_id[-str[0]][-str[1]];
            char_of[pinyin_cnt].push_back(id);
            pinyin_of[id].push_back(pinyin_cnt);
        }
    }
    fin.close();
}

class BinaryGrammerModel { // 二元语法模型
    const double lambda = 0.99999999; // 平滑权重
    double (*weight)[MAX_CHAR];

    double calcWeight(int w, int v) { // 给定相邻两汉字出现 w 次，其中第一个汉字出现 v 次时的 weight 参数
        double prob = lambda * w / v + (1-lambda) * w;
        if (prob <= 0) return 1E9;
        return -log(prob);
    }

public:
    ~BinaryGrammerModel() {
        delete []weight;
    }

    void build(string path, int lim = MAX_WORD) { // 通过词频表建立 weight 参数，词频表包含每相邻两个汉字的出现次数
        weight = new double[MAX_CHAR][MAX_CHAR];
        int (*cnt)[MAX_CHAR] = new int[MAX_CHAR][MAX_CHAR]();
        ifstream fin(path);
        string str;
        int val;
        while (fin >> str >> val && lim--) {
            if (str.size()<2 || str[0]>=0 || str[1]>=0) continue;
            int ch1 = char_id[-str[0]][-str[1]];
            if (!ch1) continue;
            cnt[0][ch1] += val;
            cnt[0][0] += val;
            if (str.size()>=4 && str[2]<0 && str[3]<0) {
                int ch2 = char_id[-str[2]][-str[3]];
                if (ch2) cnt[ch1][ch2] += val;
                }
        }
        fin.close();
        for (int i=0; i<=char_cnt; i++)
            for (int j=1; j<=char_cnt; j++)
                weight[i][j] = calcWeight(cnt[i][j], cnt[0][i]);
        delete []cnt;
    }

    void directLoadWeights(string path) { // 直接从文件载入 weight 参数，备用
        weight = new double[MAX_CHAR][MAX_CHAR];
        ifstream fin(path);
        for (int i=0; i<=char_cnt; i++)
            for (int j=1; j<=char_cnt; j++)
                cin >> weight[i][j];
    }

    vector<int> run(vector<int> pinyin) { // viterbi 算法计算 weight 参数分层图最短路
        vector<double> f[pinyin.size()]; // f[i][j] 表示第 i 层以汉字 j 结束的结点的最小耗散
        vector<int> p[pinyin.size()]; // p[i][j] 表示第 i 层以汉字 j 结束的结点在最短路上的前驱结点
        f[0].resize(char_of[pinyin[0]].size());
        for (int i=0; i<f[0].size(); i++)
            f[0][i] = weight[0][char_of[pinyin[0]][i]];
        for (int i=1; i<pinyin.size(); i++) {
            f[i] = vector<double>(char_of[pinyin[i]].size(), INF);
            p[i].resize(f[i].size());
            for (int j=0; j<f[i-1].size(); j++)
                for (int k=0; k<f[i].size(); k++) {
                    double dist = f[i-1][j] + weight[char_of[pinyin[i-1]][j]][char_of[pinyin[i]][k]];
                    if (dist < f[i][k]) {
                        f[i][k] = dist;
                        p[i][k] = j;
                    }
                }
        }
        int lst = pinyin.size() - 1;
        // 从最近的终止结点回溯求最短路
        int k = min_element(f[lst].begin(), f[lst].end()) - f[lst].begin();
        vector<int> sentence;
        for (int i=lst; i>=0; i--) {
            sentence.push_back(char_of[pinyin[i]][k]);
            if (i) k = p[i][k];
        }
        reverse(sentence.begin(), sentence.end());
        return sentence;
    }
};

template<const int N> class MultipleGrammerModel { // N 元语法模型
    const double mu[8] = {0, 0, 0.99999999, 0.81, 0.25, 0.032, 0.004, 0.0005}; // 平滑参数
    const double EPS = 1E-9; // epsilon 优化参数
    // 用 std::array 存储汉字串 char_id 列表
    unordered_map<array<int, N>, double, ArrayHasher<N>> wgt;
    // wgt(w(1)...w(i-1)w(i)) = P(w(i)|w(1)...w(i-1))

    double getWeight(const array<int, N>& word) { // 计算加权耗散值
        array<int, N> suf{};
        suf.back() = word.back();
        double sum = wgt.count(suf) ? wgt[suf] : 0;
        for (int i=2; i<=N; i++) {
            suf[N-i] = word[N-i];
            sum *= 1 - mu[i];
            if (wgt.count(suf)) sum += mu[i] * wgt[suf];
        }
        if (sum <= EPS) return -1;
        return -log(sum);
    }
    
public:
    void build(string path, int lim = MAX_WORD) { // 通过词频表建立 weight 参数，词频表包含每相邻 N 个汉字的出现次数
        wgt.clear();
        unordered_map<array<int, N>, int, ArrayHasher<N>> cnt;
        ifstream fin(path);
        string str;
        int val;
        while (fin >> str >> val && lim--) {
            bool flag = 1;
            array<int, N> word{};
            cnt[word] += val;
            for (int i=0; i+1<str.size(); i+=2) {
                if (str[i]>=0 || str[i+1]>=0) break;
                int ch = char_id[-str[i]][-str[i+1]];
                if (!ch) break;
                append(word, ch);
                cnt[word] += val;
            }
        }
        fin.close();
        for (auto e: cnt) {
            auto pre = e.first;
            prepend(pre, 0);
            wgt[e.first] = 1.0 * e.second / cnt[pre];
        }
    }

    vector<int> run(vector<int> pinyin) { // viterbi 算法计算分层图最短路
        unordered_map<array<int, N-1>, double, ArrayHasher<N-1>> f[pinyin.size()];
        // f[i][w(i-n+2)...w(i)] 表示第 i 层以串 w(i-n+2)...w(i) 结束的结点的最小耗散
        unordered_map<array<int, N-1>, int, ArrayHasher<N-1>> p[pinyin.size()];
        // p[i][w(i-n+2)...w(i)] 表示第 i 层以串 w(i-n+2)...w(i) 结束的结点在最短路上的前驱结点

        array<int, N> word{};
        array<int, N-1> pre{};
        for (int ch: char_of[pinyin[0]]) {
            word.back() = pre.back() = ch;
            double w = getWeight(word);
            if (w > -0.5) f[0][pre] = w;
        }
        for (int i=1; i<pinyin.size(); i++) {
            for (int ch: char_of[pinyin[i]]) {
                for (pair<array<int, N-1>, double> e: f[i-1]) {
                    copy(e.first.begin(), e.first.end(), word.begin());
                    word.back() = ch;
                    double w = getWeight(word);
                    if (w < -0.5) continue;
                    append(e.first, ch);
                    if (!f[i].count(e.first) || e.second + w < f[i][e.first]) {
                        f[i][e.first] = e.second + w;
                        p[i][e.first] = word.front();
                    }
                }
            }
            if (f[i].empty()) {
                double mn = INF;
                for (auto e: f[i-1])
                    if (e.second < mn) {
                        mn = e.second;
                        pre = e.first;
                    }
                int fi = pre.front();
                append(pre, char_of[pinyin[i]][0]);
                f[i][pre] = mn + 1E9;
                p[i][pre] = fi;
            }
        }
        int lst = pinyin.size() - 1;
        double mn = INF;
        for (auto e: f[lst])
            if (e.second < mn) {
                mn = e.second;
                pre = e.first;
            }
        vector<int> sentence;
        for (int i=lst; i>=0; i--) {
            sentence.push_back(pre.back());
            if (i) prepend(pre, p[i][pre]);
        }
        reverse(sentence.begin(), sentence.end());
        return sentence;
    }
};

int main(int argc, char* argv[]) {
    string inpath = "../data/input.txt", outpath = "../data/output.txt";
    if (argc > 2) {
        inpath = argv[1];
        outpath = argv[2];
    }
    ifstream fin(inpath);
    ofstream fout(outpath);
    if (!fin || !fout) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    clock_t start, finish;
    cout << "Loading Source..." << endl;
    loadAlphabet();
    loadPinyin();

    cout << "Building Model... " << flush;
    start = clock();
    MultipleGrammerModel<3> model;
    model.build("../src/frequency.txt");
    finish = clock();
    cout << "Time = " << (double)(finish - start) / CLOCKS_PER_SEC << "s" << endl;

    cout << "Proceeding..." << endl;
    start = clock();
    string curline, str;
    while (getline(fin, curline)) {
        stringstream ss(curline);
        vector<int> pinyin;
        while (ss >> str)
            pinyin.push_back(pinyin_id[str]);
        fout << chidToStr(model.run(pinyin)) << endl;
    }
    fin.close();
    fout.close();
    finish = clock();
    cout << "Done. Proceeding time = " << (double)(finish - start) / CLOCKS_PER_SEC << "s" << endl;
    cout << "Output has been written in " << outpath << endl;
    return 0;
}
