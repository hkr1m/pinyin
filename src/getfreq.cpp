#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <vector>
using namespace std;

const int MAX_LINE = 100000;
const int MAX_WORD = 9000000;
const int LEN = 3;
char curline[MAX_LINE];
string alphabet;
int char_id[128][128];
int char_cnt;
unordered_map<string, int> cnt;
vector<pair<string, int>> words;

void loadAlphabet() {
    ifstream fin("../src/alphabet.txt");
    fin >> alphabet;
    fin.close();
    char_cnt = 0;
    for (int i=0; 2*i+1 < alphabet.size(); i++) {
        char_id[-alphabet[2*i]][-alphabet[2*i+1]] = i+1;
        char_cnt++;
    }
}

void loadSource(string path) {
    cout << "Loading from " << path << " ... " << flush;
    ifstream fin(path);
    if (!fin) {
        cout << "Failed" << endl;
        return;
    }
    while (fin.getline(curline, MAX_LINE, '\n')) {
        char *p = curline;
        while (*p && *p != '\n') {
            if (*p < 0 && *(p+1) < 0) {
                if (char_id[-(*p)][-*(p+1)]) {
                    string word;
                    for (int i=0; i<LEN; i++) {
                        if (*(p+2*i) >= 0 || *(p+2*i+1) >= 0 || !char_id[-*(p+2*i)][-*(p+2*i+1)]) break;
                        word.push_back(*(p+2*i));
                        word.push_back(*(p+2*i+1));
                    }
                    cnt[word] += 1;
                }
                p += 2;
            } else p++;
        }
    }
    cout << "Completed" << endl;
}

bool cmp(const pair<string, int>& lhs, const pair<string, int>& rhs) {
    return lhs.second > rhs.second;
}

int main() {
    loadAlphabet();
    loadSource("../src/sina_news_gbk/2016-02.txt");
    loadSource("../src/sina_news_gbk/2016-04.txt");
    loadSource("../src/sina_news_gbk/2016-05.txt");
    loadSource("../src/sina_news_gbk/2016-06.txt");
    loadSource("../src/sina_news_gbk/2016-07.txt");
    loadSource("../src/sina_news_gbk/2016-08.txt");
    loadSource("../src/sina_news_gbk/2016-09.txt");
    loadSource("../src/sina_news_gbk/2016-10.txt");
    loadSource("../src/sina_news_gbk/2016-11.txt");
    ofstream fout("../src/frequency.txt");
    cout << "Proceeding..." << endl;
    for (auto word: cnt) words.push_back(word);
    sort(words.begin(), words.end(), cmp);
    
    int lim = MAX_WORD;
    for (auto word: words) {
        if (!(lim--)) break;
        fout << word.first << " " << word.second << endl;
    }
    fout.close();
    cout << "Done." << endl;
    return 0;
}