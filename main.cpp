
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <thread>
#include <algorithm>
#include "Windows.h"

#include "Base64.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))



using uint = unsigned int;
using uint32 = unsigned int;
using uint64 = unsigned long long;

using uchar = unsigned char;
using StringFilter = float (*)(uchar* in, int len);

struct Node;

struct Node
{
    uchar c = '\0';
    int next = -1;
    int down = -1;
    int count = 0;
};

uchar ToLower(uchar c) {
    if (c < 'a') {
        c += 'a' - 'A';
    }
    return c;
}

uchar ToUpper(uchar c) {
    if (c > 'Z') {
        c -= 'a' - 'A';
    }
    return c;
}

bool IsAlpha(uchar c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool IsLower(uchar c) {
    return (c >= 'a' && c <= 'z');
}

bool IsUpper(uchar c) {
    return (c >= 'A' && c <= 'Z');
}

bool IsDigit(uchar c) {
    return (c >= '0' && c <= '9');
}


const uchar allowed_chars[] = " .,-;:?!0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//const uchar allowed_chars[] = " .,!?0123456789àáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß";

class Test
{
    std::vector<float> bigramsF;
    std::vector<float> trigramsF;
    std::vector<int> charPointers[26];

    float kurnel3(uchar* in) {
        bool ok = true;
        bool isAlpha[3];
        bool isUpper[3];
        bool isLower[3];
        bool isSentEnd[3];
        bool isCommaType[3];
        bool isSpace[3];
        bool isDefis[3];
        bool isTrigram;
        uchar lowered[3];
        for (int i = 0; i < 3; ++i) {
            isLower[i] = IsLower(in[i]);
            isUpper[i] = IsUpper(in[i]);
            isAlpha[i] = IsAlpha(in[i]);
            isSentEnd[i] = in[i] == '.' || in[i] == '?' || in[i] == '!';
            isCommaType[i] = in[i] == ',' || in[i] == ';' || in[i] == ':';
            isSpace[i] = in[i] == ' ';
            isDefis[i] = in[i] == '-';
            lowered[i] = ToLower(in[i]);
        }




        isTrigram = isAlpha[0] && isAlpha[1] && isAlpha[2];
        float ret = 1.f;
        if (ok && isTrigram) {
            uint triIdx = ((lowered[0] - 'a') * 26 + (lowered[1] - 'a')) * 26 + (lowered[2] - 'a');
            ret = trigramsF[triIdx];
        }
        return ok ? ret : 0.f;
    }

    float kurnel2(uchar* in) {
        bool ok = true;
        bool isAlpha[2];
        bool isUpper[2];
        bool isLower[2];
        bool isSentEnd[2];
        bool isCommaType[2];
        bool isSpace[2];
        bool isDefis[2];
        bool isDigit[2];
        bool isBigram;
        uchar lowered[2];
        for (int i = 0; i < 2; ++i) {
            isLower[i] = IsLower(in[i]);
            isUpper[i] = IsUpper(in[i]);
            isAlpha[i] = IsAlpha(in[i]);
            isDigit[i] = IsDigit(in[i]);
            isSentEnd[i] = in[i] == '.' || in[i] == '?' || in[i] == '!';
            isCommaType[i] = in[i] == ',' || in[i] == ';' || in[i] == ':';
            isSpace[i] = in[i] == ' ';
            isDefis[i] = in[i] == '-';
            lowered[i] = ToLower(in[i]);
        }
        isBigram = isAlpha[0] && isAlpha[1];
        ok &= !isLower[0] || (isLower[0] && !isUpper[1]);
        ok &= !isCommaType[0] || (isCommaType[0] && isSpace[1]);
        ok &= !isSentEnd[0] || (isSentEnd[0] && isSpace[1]);
        ok &= !isDefis[0] || (isDefis[0] && isAlpha[1]);
        ok &= !isDefis[1] || (isDefis[1] && isAlpha[0]);
        ok &= !(isDigit[0] && isAlpha[1]) && !(isDigit[1] && isAlpha[0]);
        float ret = 1.f;
        if (ok && isBigram) {
            uint biIdx = (lowered[0] - 'a') * 26 + (lowered[1] - 'a');
            ret = bigramsF[biIdx];
        }
        return ok ? ret : 0.f;
    }

    int TraceNodes(uchar* in, int len, int nodePos, bool rev = false) {
        std::vector<Node>& _nodes = rev ? nodesRev : nodes;
        int i = 0;
        while (i < len) {
            uchar c = ToLower(in[rev ? -i : i]);
            if (_nodes[nodePos].down < 0) {
                break;
            }
            else {
                nodePos = _nodes[nodePos].down;
                while (_nodes[nodePos].c != c && _nodes[nodePos].next >= 0) {
                    nodePos = _nodes[nodePos].next;
                }
                if (_nodes[nodePos].c != c) {
                    break;
                }
            }
            ++i;
        }
        if (i == len) {
            int endPos = _nodes[nodePos].down;
            while (endPos >= 0) {
                if (_nodes[endPos].c == '\0') {
                    ++i;
                    break;
                }
                endPos = _nodes[endPos].next;
            }
        }
        return i;
    }

    bool IsWordPart(uchar* in, int len) {
        uchar strtCh = ToLower(in[0]);
        const auto& pointerList = charPointers[strtCh - 'a'];
        for (int nodePos : pointerList) {
            if (TraceNodes(in + 1, len - 1, nodePos) >= len - 1) {
                return true;
            }
        }
        return false;
    }

    bool IsWord(uchar* in, int len) {
        return TraceNodes(in, len, 0) == len + 1;
    }

    bool IsWordStart(uchar* in, int len) {
        return TraceNodes(in, len, 0) >= len;
    }

    bool IsWordEnd(uchar* in, int len) {
        return TraceNodes(in + len - 1, len, 0, true) >= len;
    }


    float MainFilter(uchar* in, int len) {
        if (len < 2) {
            return 0.f;
        }
        float biAvrg = 1.f;
        if (len >= 2) {
            float biSum = 0.f;
            for (int i = 0; i + 1 < len; ++i) {
                float ret = kurnel2(in + i);
                if (ret == 0.f) {
                    return 0.f;
                }
                biSum += ret;
            }
            biAvrg = biSum / (len - 1);
        }

        float triAvrg = 1.f;
        if (len >= 3) {
            float triSum = 0.f;
            for (int i = 0; i + 2 < len; ++i) {
                float ret = kurnel3(in + i);
                if (ret == 0.f) {
                    return 0.f;
                }
                triSum += ret;
            }
            triAvrg = triSum / (len - 2);
        }


        int wordParts = 0;
        int i = 0;
        while (i < len) {
            while (i < len && !IsAlpha(in[i])) {
                ++i;
            }
            if (!IsAlpha(in[i])) {
                break;
            }
            bool isWordStart = i != 0;
            int start = i;
            while (i < len && IsAlpha(in[i])) {
                ++i;
            }
            bool isWordEnd = i != len;
            int end = i;
            int len = end - start;
            if (len > 0) {
                if (isWordStart && isWordEnd) {
                    if (!IsWord(in + start, len))
                        return 0.f;
                }
                else if (isWordStart) {
                    if (!IsWordStart(in + start, len))
                        return 0.f;
                }
                else if (isWordEnd) {
                    if (!IsWordEnd(in + start, len))
                        return 0.f;
                }
                else {
                    if (!IsWordPart(in + start, len))
                        return 0.f;
                }
                wordParts += end - start;
            }
        }



        return biAvrg + triAvrg + wordParts;
    }


    uint64 masks[256];
    std::vector<Node> nodes;
    std::vector<Node> nodesRev;
    std::vector<uchar> code1;
    std::vector<uchar> code2;
    std::vector<std::pair<uchar, uchar>> table[256];
    std::vector<uint64> allowedChars;
    std::vector<uchar> helper;


    std::vector<uint> posibleCount1;
    std::vector<uint> posibleCount2;
    std::vector<uchar> text1, text2;

    struct WordRecord {
        std::vector<uchar> word;
        std::vector<uchar> mirror;
        uint len;
        float score;
    };

    int printRec(std::vector<WordRecord>* pWords, bool start, bool caps, int depthLeft, uchar* tmp, uchar* tmpMirror, int tmpPos, int nodePos, std::vector<uchar>& text, int pos) {
        if (depthLeft == 0 || nodePos < 0 || pos >= text.size()) {
            return 0;
        }
        int count = 0;
        while (nodePos >= 0) {
            if (nodes[nodePos].c == '\0') {
                float res = MainFilter(tmpMirror, tmpPos);
                if (res != 0) {
                    if (pWords) {
                        uint wordIdx = pWords->size();
                        pWords->push_back({});
                        WordRecord& word = (*pWords)[wordIdx];
                        word.len = tmpPos;
                        word.score = res;
                        word.word.resize(tmpPos);
                        word.mirror.resize(tmpPos);
                        memcpy(word.word.data(), tmp, tmpPos);
                        memcpy(word.mirror.data(), tmpMirror, tmpPos);
                    }
                    count++;
                }
            }
            if (!caps && ((allowedChars[pos] & masks[nodes[nodePos].c]) != 0) && (text[pos] == '*' || text[pos] == '%' || text[pos] == nodes[nodePos].c)) {
                tmp[tmpPos] = nodes[nodePos].c;
                tmpMirror[tmpPos] = nodes[nodePos].c ^ helper[pos];
                count += printRec(pWords, false, false, depthLeft - 1, tmp, tmpMirror, tmpPos + 1, nodes[nodePos].down, text, pos + 1);
            }
            uchar C = ToUpper(nodes[nodePos].c);
            if ((caps || start) && ((allowedChars[pos] & masks[C]) != 0) && (text[pos] == '*' || text[pos] == '%' || text[pos] == C)) {
                tmp[tmpPos] = C;
                tmpMirror[tmpPos] = C ^ helper[pos];
                count += printRec(pWords, false, caps, depthLeft - 1, tmp, tmpMirror, tmpPos + 1, nodes[nodePos].down, text, pos + 1);
            }
            nodePos = nodes[nodePos].next;
        }
        return count;
    }

    void recalcPosibleCount(uint start, uint end) {
        std::vector<uchar> tmp;
        std::vector<uchar> tmpMirror;
        tmp.resize(100);
        tmpMirror.resize(100);
        for (int i = start; i < end && i < posibleCount1.size(); ++i) {
            posibleCount1[i] = printRec(nullptr, true, false, 100, tmp.data(), tmpMirror.data(), 0, 1, text1, i);
            posibleCount1[i] += printRec(nullptr, true, true, 100, tmp.data(), tmpMirror.data(), 0, 1, text1, i);
            posibleCount2[i] = printRec(nullptr, true, true, 100, tmp.data(), tmpMirror.data(), 0, 1, text2, i);
            posibleCount2[i] += printRec(nullptr, true, false, 100, tmp.data(), tmpMirror.data(), 0, 1, text2, i);
        }

    }

    void printState(int windowStart, int windowEnd, int pos) {
        printf("pos: %6d\n", pos);
        for (int i = windowStart; i < windowEnd; ++i) {
            printf("%6d ", posibleCount1[i]);
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            printf("   %c   ", text1[i]);
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            uchar cross = helper[i];
            const auto& vars = table[cross];
            printf((cross == 0 ? "   #   " : "%6d "), int(vars.size()));
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            printf("   %c   ", text2[i]);
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            printf("%6d ", posibleCount2[i]);
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            printf(i == pos ? "   ^   " : "       ");
        }
        printf("\n");
        printf("\n");

        uchar cross = helper[pos];
        const auto& vars = table[cross];

        printf("cross = ");
        for (int j = 7; j > 0; --j) {
            printf("%d", (uchar)((cross) >> j) & 0x1);
        }
        printf("\n");
        printf("\n");


        for (int i = 0; i < vars.size(); ++i) {
            auto pair = vars[i];
            printf("%2d (%c,%c); %2d (%c,%c);\n", i, pair.first, pair.second, i + vars.size(), pair.second, pair.first);
        }
        printf("\n");
    }

    void init() {
        {
            FILE* in = fopen("./tree.bin", "rb");
            fseek(in, 0L, SEEK_END);
            long sz = ftell(in) / sizeof(Node);
            fseek(in, 0L, SEEK_SET);
            nodes.resize(sz);

            fread(nodes.data(), sizeof(Node), sz, in);
            fclose(in);
        }
        {
            FILE* in = fopen("./treeRev.bin", "rb");
            fseek(in, 0L, SEEK_END);
            long sz = ftell(in) / sizeof(Node);
            fseek(in, 0L, SEEK_SET);
            nodesRev.resize(sz);

            fread(nodesRev.data(), sizeof(Node), sz, in);
            fclose(in);
        }
        {
            FILE* in = fopen("./charPointers.bin", "rb");
            for (int i = 0; i < 26; ++i) {
                int size;
                fread(&size, sizeof(int), 1, in);
                charPointers[i].resize(size);
                fread(charPointers[i].data(), sizeof(int), size, in);
            }
            fclose(in);
        }
        {
            FILE* in = fopen("./bigrams.bin", "rb");
            fseek(in, 0L, SEEK_END);
            long sz = ftell(in) / sizeof(float);
            fseek(in, 0L, SEEK_SET);
            bigramsF.resize(sz);

            fread(bigramsF.data(), sizeof(float), sz, in);
            fclose(in);
        }
        {
            FILE* in = fopen("./trigrams.bin", "rb");
            fseek(in, 0L, SEEK_END);
            long sz = ftell(in) / sizeof(float);
            fseek(in, 0L, SEEK_SET);
            trigramsF.resize(sz);

            fread(trigramsF.data(), sizeof(float), sz, in);
            fclose(in);
        }
        {
            FILE* in = fopen("./input1.txt", "r");
            fseek(in, 0L, SEEK_END);
            long sz = ftell(in);
            fseek(in, 0L, SEEK_SET);
            std::vector<uchar> tmp;
            tmp.resize(sz);

            fread(tmp.data(), sizeof(uchar), sz, in);
            fclose(in);
            macaron::Base64::Decode(tmp, code1);
        }
        {
            FILE* in = fopen("./input2.txt", "r");
            fseek(in, 0L, SEEK_END);
            long sz = ftell(in);
            fseek(in, 0L, SEEK_SET);
            std::vector<uchar> tmp;
            tmp.resize(sz);

            fread(tmp.data(), sizeof(uchar), sz, in);
            fclose(in);
            macaron::Base64::Decode(tmp, code2);
        }

        for (int first = 0; first < sizeof(allowed_chars) - 1; ++first) {

            for (int second = first; second < sizeof(allowed_chars) - 1; ++second) {
                uchar val = allowed_chars[first] ^ allowed_chars[second];
                table[val].push_back({ allowed_chars[first], allowed_chars[second] });
            }
        }

        helper.resize(code1.size());
        for (int i = 0; i < code1.size(); ++i) {
            helper[i] = code1[i] ^ code2[i];
        }
        /*
            printf("\n");
            for (int i = 0; i < 256; ++i) {
                printf("%02X = %2d: ", i, (int)table[i].size());
                for (auto pair : table[i]) {
                    printf("(%c,%c)", pair.first, pair.second);
                }
                printf("\n");
            }
        */
        for (int i = 0; i < 256; ++i) masks[i] = 0;
        {
            masks['a'] = 0x0000000000000001;
            masks['b'] = 0x0000000000000002;
            masks['c'] = 0x0000000000000004;
            masks['d'] = 0x0000000000000008;
            masks['e'] = 0x0000000000000010;
            masks['f'] = 0x0000000000000020;
            masks['g'] = 0x0000000000000040;
            masks['h'] = 0x0000000000000080;
            masks['i'] = 0x0000000000000100;
            masks['j'] = 0x0000000000000200;
            masks['k'] = 0x0000000000000400;
            masks['l'] = 0x0000000000000800;
            masks['m'] = 0x0000000000001000;
            masks['n'] = 0x0000000000002000;
            masks['o'] = 0x0000000000004000;
            masks['p'] = 0x0000000000008000;
            masks['q'] = 0x0000000000010000;
            masks['r'] = 0x0000000000020000;
            masks['s'] = 0x0000000000040000;
            masks['t'] = 0x0000000000080000;
            masks['u'] = 0x0000000000100000;
            masks['v'] = 0x0000000000200000;
            masks['w'] = 0x0000000000400000;
            masks['x'] = 0x0000000000800000;
            masks['y'] = 0x0000000001000000;
            masks['z'] = 0x0000000002000000;
            masks[' '] = 0x0000000004000000;
            masks['-'] = 0x0000000008000000;
            masks[','] = 0x0000000010000000;
            masks['.'] = 0x0000000020000000;
            masks['A'] = 0x0000000100000000;
            masks['B'] = 0x0000000200000000;
            masks['C'] = 0x0000000400000000;
            masks['D'] = 0x0000000800000000;
            masks['E'] = 0x0000001000000000;
            masks['F'] = 0x0000002000000000;
            masks['G'] = 0x0000004000000000;
            masks['H'] = 0x0000008000000000;
            masks['I'] = 0x0000010000000000;
            masks['J'] = 0x0000020000000000;
            masks['K'] = 0x0000040000000000;
            masks['L'] = 0x0000080000000000;
            masks['M'] = 0x0000100000000000;
            masks['N'] = 0x0000200000000000;
            masks['O'] = 0x0000400000000000;
            masks['P'] = 0x0000800000000000;
            masks['Q'] = 0x0001000000000000;
            masks['R'] = 0x0002000000000000;
            masks['S'] = 0x0004000000000000;
            masks['T'] = 0x0008000000000000;
            masks['U'] = 0x0010000000000000;
            masks['V'] = 0x0020000000000000;
            masks['W'] = 0x0040000000000000;
            masks['X'] = 0x0080000000000000;
            masks['Y'] = 0x0100000000000000;
            masks['Z'] = 0x0200000000000000;
        }
        allowedChars.resize(code1.size());
        for (int i = 0; i < code1.size(); ++i) {
            allowedChars[i] = 0;
            uchar cross = helper[i];
            const auto& vars = table[cross];
            for (auto pair : vars) {
                allowedChars[i] |= masks[pair.first] | masks[pair.second];
            }
        }

        text1.resize(code1.size());
        text2.resize(code1.size());
        for (int i = 0; i < code1.size(); ++i) {
            if (helper[i] < 64) {
                text1[i] = text2[i] = '%';
            }
            else {
                text1[i] = text2[i] = '*';
            }
        }

        /*  // Load chackpoint
            uchar checkPoint[] = "Chapter 1 An Empty Road The Wheel of Time turns, and Ages come and pass, leaving memories that become legend. Legend fades to myth, and even myth is long forgotten when the Age that gave it birth comes again. In one Age, called the Third Age by some, an Age yet to come, an Age long past, a wind rose in the Mountains of Mist. The wind was not the beginning. There are neither beginnings nor endings to the turning of the Wheel of Time. But it was a beginning.";
            for (int i = 0; i < sizeof(checkPoint) - 1; ++i) {
                text1[i] = checkPoint[i];
                text2[i] = checkPoint[i] ^ helper[i];
            }
        */
        posibleCount1.resize(code1.size());
        posibleCount2.resize(code1.size());
    }

public:
    int test() {
        setlocale(LC_ALL, "Russian");

        init();

        bool needRefresh = true;
        bool needRecalc = true;
        int pos = 0;
        int windowSize = 25;
        int windowStart = 0;
        int windowEnd = windowSize;
        while (true) {
            if (needRefresh) {
                system("cls");
                int windowStartPrev = windowStart;
                windowStart = MAX(pos / windowSize * windowSize, 0);
                windowEnd = MIN(pos / windowSize * windowSize + windowSize, helper.size() - 1);
                if (needRecalc || windowStart != windowStartPrev) {
                    recalcPosibleCount(windowStart, windowEnd);
                    needRecalc = false;
                }
                printState(windowStart, windowEnd, pos);
                needRefresh = false;
            }

            uchar cross = helper[pos];
            const auto& vars = table[cross];

            int res;

            bool isControl = false;
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                isControl = true;
            }
            bool isCapsLock = false;
            if (GetKeyState(VK_CAPITAL) & 0x0001)
            {
                isCapsLock = true;
            }
            if (GetKeyState(VK_LEFT) & 0x8000)
            {
                pos -= isControl ? windowSize : 1;
                needRefresh = true;
            }
            if (GetKeyState(VK_RIGHT) & 0x8000)
            {
                pos += isControl ? windowSize : 1;
                needRefresh = true;
            }
            if (GetKeyState(VK_ADD) & 0x8000)
            {
                {
                    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF) {}
                }
                int ret = scanf("%d", &res);
                if (ret > 0) {
                    if (res < vars.size()) {
                        text1[pos] = vars[res].first;
                        text2[pos] = vars[res].second;
                    }
                    else if (res - vars.size() < vars.size()) {
                        text2[pos] = vars[res - vars.size()].first;
                        text1[pos] = vars[res - vars.size()].second;
                    }
                    needRecalc = true;
                }
                needRefresh = true;
            }
            if (GetKeyState(VK_SPACE) & 0x8000)
            {
                std::vector<uchar> tmp1, tmp2;
                tmp1.resize(100);
                tmp2.resize(100);

                std::vector<WordRecord> words;
                int count = printRec(&words, true, isCapsLock, 100, tmp1.data(), tmp2.data(), 0, 1, isControl ? text2 : text1, pos);
                printf("%d\n", count);

                std::sort(words.begin(), words.end(), [](const WordRecord& a, const WordRecord& b) {return (a.score + a.len) > (b.score + b.len); });

                for (uint i = 0; i < words.size(); ++i) {
                    printf("%d: %f\n", i, words[i].score);
                    printf("%.*s\n", words[i].len, words[i].word.data());
                    printf("%.*s\n", words[i].len, words[i].mirror.data());
                    printf("\n");
                }
                int i = -1;
                {
                    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF) {}
                }
                int ret = scanf("%d", &i);
                if (ret > 0) {
                    auto& word1 = isControl ? words[i].mirror : words[i].word;
                    auto& word2 = isControl ? words[i].word : words[i].mirror;
                    for (uint j = 0; j < words[i].len; ++j) {
                        text1[pos + j] = word1[j];
                        text2[pos + j] = word2[j];
                    }
                    needRecalc = true;
                }
                needRefresh = true;
            }

            pos = MAX(MIN(pos, helper.size() - 1), 0);
            std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
        }

        return 0;
    }
};



int select(const char* filename) {

    FILE* in = fopen(filename, "rb");
    fseek(in, 0L, SEEK_END);
    long sz = ftell(in);
    fseek(in, 0L, SEEK_SET);
    std::vector<uchar> text;
    text.resize(sz);
    fread(text.data(), sizeof(uchar), sz, in);
    fclose(in);

    std::list<std::pair<uchar*, uint>> words;

    std::vector<uint> bigrams;
    uint bigramsTotal = 0;
    bigrams.resize(26 * 26);
    std::vector<uint> trigrams;
    uint trigramsTotal = 0;
    trigrams.resize(26 * 26 * 26);

    int alphaCounter = 0;
    for (uint i = 0; i < sz; ++i) {
        if (IsAlpha(text[i])) {
            alphaCounter++;
            if (alphaCounter >= 2) {
                uint biIdx = (ToLower(text[i - 1]) - 'a') * 26 + (ToLower(text[i]) - 'a');
                bigrams[biIdx]++;
                bigramsTotal++;
            }
            if (alphaCounter >= 3) {
                uint triIdx = ((ToLower(text[i - 2]) - 'a') * 26 + (ToLower(text[i - 1]) - 'a')) * 26 + (ToLower(text[i]) - 'a');
                trigrams[triIdx]++;
                trigramsTotal++;
            }
        }
        else {
            if (alphaCounter > 0) {
                words.push_back({ &text[i - alphaCounter], alphaCounter });
            }
            alphaCounter = 0;
        }
    }

    {
        FILE* out = fopen("./bigrams.bin", "wb");
        std::vector<float> bigramsF;
        float avrg = bigramsTotal / (float)(bigrams.size());
        bigramsF.resize(bigrams.size());
        for (uint biIdx = 0; biIdx < bigrams.size(); ++biIdx) {
            bigramsF[biIdx] = bigrams[biIdx] / avrg;
        }
        fwrite(bigramsF.data(), sizeof(float), bigramsF.size(), out);

        fflush(out);
        fclose(out);
    }
    {
        FILE* out = fopen("./trigrams.bin", "wb");
        std::vector<float> trigramsF;
        float avrg = trigramsTotal / (float)(trigrams.size());
        trigramsF.resize(trigrams.size());
        for (uint triIdx = 0; triIdx < trigrams.size(); ++triIdx) {
            trigramsF[triIdx] = trigrams[triIdx] / avrg;
        }
        fwrite(trigramsF.data(), sizeof(float), trigramsF.size(), out);

        fflush(out);
        fclose(out);
    }
    {
        FILE* out = fopen("./out.txt", "w");

        for (auto& word : words) {
            fprintf(out, "%.*s\n", word.second, word.first);
        }

        fflush(out);
        fclose(out);
    }

    return 0;
}

int buildTree() {
    std::vector<uchar> textOrd;
    std::list<std::pair<uchar*, uint>> words;
    {
        FILE* in = fopen("./out.txt", "rb");
        fseek(in, 0L, SEEK_END);
        long sz = ftell(in);
        fseek(in, 0L, SEEK_SET);
        textOrd.resize(sz);
        int read = fread(textOrd.data(), sizeof(uchar), sz, in);
        fflush(in);
        fclose(in);
    }

    int wordStart = 0;
    while (wordStart < textOrd.size()) {
        bool ok = true;
        int i = wordStart;
        while (i < textOrd.size() && textOrd[i] != '\r')
        {
            ++i;
        }
        int wordLen = i - wordStart;
        words.push_back({ &textOrd[wordStart], wordLen });
        wordStart = i + 2;
    }

    std::vector<Node> nodes;
    nodes.push_back({});

    std::vector<int> charPointers[26];

    for (auto& word : words) {
        int pos = 0;
        for (int i = 0; i < word.second; ++i) {
            nodes[pos].count++;
            uchar c = ToLower(word.first[i]);

            if (nodes[pos].down < 0) {
                int newPos = nodes.size();
                nodes.push_back({ c, -1, -1, 0 });
                nodes[pos].down = newPos;
                charPointers[c - 'a'].push_back(newPos);
                pos = newPos;
            }
            else {
                pos = nodes[pos].down;
                while (nodes[pos].c != c && nodes[pos].next >= 0) {
                    pos = nodes[pos].next;
                }
                if (nodes[pos].c != c) {
                    int newPos = nodes.size();
                    nodes.push_back({ c, -1, -1, 0 });
                    nodes[pos].next = newPos;
                    charPointers[c - 'a'].push_back(newPos);
                    pos = newPos;
                }
            }
        }
        int endPos = nodes[pos].down;
        while (endPos >= 0) {
            if (nodes[endPos].c == '\0') {
                nodes[endPos].count++;
                break;
            }
            endPos = nodes[endPos].next;
        }
        if (endPos < 0) {
            int newPos = nodes.size();
            nodes.push_back({ '\0', -1, -1, 1 });
            nodes[pos].down = newPos;
        }
    }



    FILE* out = fopen("./tree.bin", "wb");
    fwrite(nodes.data(), sizeof(Node), nodes.size(), out);
    fflush(out);
    fclose(out);

    out = fopen("./charPointers.bin", "wb");

    for (int i = 0; i < 26; ++i) {
        int size = charPointers[i].size();
        fwrite(&size, sizeof(int), 1, out);
        fwrite(charPointers[i].data(), sizeof(int), size, out);
    }
    fflush(out);
    fclose(out);

    nodes.clear();
    nodes.push_back({});

    for (auto& word : words) {
        int pos = 0;
        for (int i = word.second - 1; i >= 0; --i) {
            nodes[pos].count++;
            uchar c = ToLower(word.first[i]);

            if (nodes[pos].down < 0) {
                int newPos = nodes.size();
                nodes.push_back({ c, -1, -1, 0 });
                nodes[pos].down = newPos;
                pos = newPos;
            }
            else {
                pos = nodes[pos].down;
                while (nodes[pos].c != c && nodes[pos].next >= 0) {
                    pos = nodes[pos].next;
                }
                if (nodes[pos].c != c) {
                    int newPos = nodes.size();
                    nodes.push_back({ c, -1, -1, 0 });
                    nodes[pos].next = newPos;
                    pos = newPos;
                }
            }
        }
        int endPos = nodes[pos].down;
        while (endPos >= 0) {
            if (nodes[endPos].c == '\0') {
                nodes[endPos].count++;
                break;
            }
            endPos = nodes[endPos].next;
        }
        if (endPos < 0) {
            int newPos = nodes.size();
            nodes.push_back({ '\0', -1, -1, 1 });
            nodes[pos].down = newPos;
        }
    }



    out = fopen("./treeRev.bin", "wb");
    fwrite(nodes.data(), sizeof(Node), nodes.size(), out);
    fflush(out);
    fclose(out);


    return 0;
}

int main() {

    select("./big.txt");
    buildTree();
    Test t;
    t.test();

    return 0;
}