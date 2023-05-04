#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    string outpath = "../data/output.txt", stdpath = "../data/std_output.txt";
    if (argc > 2) {
        outpath = argv[1];
        stdpath = argv[2];
    }
    ifstream fout(outpath);
    ifstream fstd(stdpath);
    if (!fout || !fstd) {
        cerr << "Error opening file!" << endl;
        return 1;
    }
    string out, std;
    int char_tot = 0, sent_tot = 0;
    int char_right = 0, sent_right = 0;
    while ((fout >> out) && (fstd >> std)) {
        sent_tot++;
        for (int i=0; i+1<out.size() && i+1<std.size(); i+=2) {
            if (out[i]==std[i] && out[i+1]==std[i+1]) char_right++;
            char_tot++;
        }
        if (out == std) sent_right++;
    }
    cout << sent_tot << " tokens compared." << endl;
    cout << "Characters correct: (" << char_right << "/" << char_tot << "), percentage: " << 1.0*char_right/char_tot << endl;
    cout << "Sentences correct: (" << sent_right << "/" << sent_tot << "), percentage: " << 1.0*sent_right/sent_tot << endl;
    return 0;
}