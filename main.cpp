
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



template<typename T>
static void ReadFullFile(const char* fileName, std::vector<T>& data)
{
    FILE* in = fopen(fileName, "rb");
    fseek(in, 0L, SEEK_END);
    long sz = ftell(in) / sizeof(T);
    fseek(in, 0L, SEEK_SET);

    data.resize(sz);

    fread(data.data(), sizeof(T), sz, in);
    fclose(in);
}

template<typename T>
static void WriteFullFile(const char* fileName, const std::vector<T>& data)
{
    FILE* out = fopen(fileName, "wb");
    fwrite(data.data(), sizeof(T), data.size(), out);
    fflush(out);
    fclose(out);
}

template<typename T>
struct triplet
{
    T val[3];
    triplet() {
        val[0] = 0;
        val[1] = 0;
        val[2] = 0;
    };
    triplet(T v0, T v1, T v2) {
        val[0] = v0;
        val[1] = v1;
        val[2] = v2;
    };

};

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


const uchar allowed_chars[] = " .,-;:?!\'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
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


    float MainFilter(uchar* in, int len, bool& filtered) {
        filtered = false;
        if (len < 2) {
            filtered = true;
            return 0.f;
        }
        float biAvrg = 1.f;
        if (len >= 2) {
            float biSum = 0.f;
            for (int i = 0; i + 1 < len; ++i) {
                float ret = kurnel2(in + i);
                if (ret == 0.f) {
                    filtered = true;
                    return 0.f;
                }
                biSum += ret;
            }
            biAvrg = biSum;
        }

        float triAvrg = 1.f;
        if (len >= 3) {
            float triSum = 0.f;
            for (int i = 0; i + 2 < len; ++i) {
                float ret = kurnel3(in + i);
                if (ret == 0.f) {
                    filtered = true;
                    return 0.f;
                }
                triSum += ret;
            }
            triAvrg = triSum;
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
                float m = 1;

                if (isWordStart && isWordEnd) {
                    if (IsWord(in + start, len)) {
                        m = 2;
                    }
                    else {
                        m = -1;
                    }
                }
                else if (isWordStart) {
                    if (!IsWordStart(in + start, len)) {
                        m = 1;
                    }
                    else {
                        m = -1;
                    }
                }
                else if (isWordEnd) {
                    if (!IsWordEnd(in + start, len)) {
                        m = 1;
                    }
                    else {
                        m = -1;
                    }
                }
                else {
                    if (!IsWordPart(in + start, len)) {
                        m = 1;
                    }
                    else {
                        m = -1;
                    }
                }
                wordParts += end - start;
            }
        }



        return biAvrg + triAvrg + wordParts;
    }


    uint64 masks[256];
    std::vector<Node> nodes;
    std::vector<Node> nodesRev;
    std::vector<uchar> codes[3];
    std::vector<std::pair<uchar, uchar>> table[256];
    std::vector < std::vector<triplet<uchar>>> tableBig;
    std::vector<uint64> allowedChars[3];
    std::vector<uchar> helpers[3];
    std::vector<triplet<uint>> typeMasks;


    std::vector<uint> posibleCounts[3];
    std::vector<uchar> texts[3];

    struct WordRecord {
        std::vector<uchar> words[3];
        uint len;
        float score;
    };

    int printRec(std::vector<WordRecord>* pWords, bool start, bool caps, int depthLeft, std::vector<uchar>* tmps, int tmpPos, int nodePos, int textIdx, int pos) {
        if (depthLeft == 0 || nodePos < 0 || pos >= texts[textIdx].size()) {
            return 0;
        }
        int count = 0;
        while (nodePos >= 0) {
            if (nodes[nodePos].c == '\0') {
                float res = 0;
                bool filtered = false;
                for (int i = 0; i < 3 && !filtered; ++i) {
                    res += MainFilter(tmps[(textIdx + i) % 3].data(), tmpPos, filtered);
                }
                if (!filtered) {
                    if (pWords) {
                        uint wordIdx = pWords->size();
                        pWords->push_back({});
                        WordRecord& word = (*pWords)[wordIdx];
                        word.len = tmpPos;
                        word.score = res;
                        for (int i = 0; i < 3; ++i) {
                            word.words[i].resize(tmpPos);
                            memcpy(word.words[i].data(), tmps[i].data(), tmpPos);
                        }
                    }
                    count++;
                }
            }
            if (!caps && ((allowedChars[textIdx][pos] & masks[nodes[nodePos].c]) != 0) && (texts[textIdx][pos] == '*' || texts[textIdx][pos] == '%' || texts[textIdx][pos] == nodes[nodePos].c)) {
                tmps[textIdx][tmpPos] = nodes[nodePos].c;
                tmps[(textIdx + 1) % 3][tmpPos] = nodes[nodePos].c ^ helpers[textIdx][pos];
                tmps[(textIdx + 2) % 3][tmpPos] = nodes[nodePos].c ^ helpers[(textIdx + 2) % 3][pos];
                count += printRec(pWords, false, false, depthLeft - 1, tmps, tmpPos + 1, nodes[nodePos].down, textIdx, pos + 1);
            }
            uchar C = ToUpper(nodes[nodePos].c);
            if ((caps || start) && ((allowedChars[textIdx][pos] & masks[C]) != 0) && (texts[textIdx][pos] == '*' || texts[textIdx][pos] == '%' || texts[textIdx][pos] == C)) {
                tmps[textIdx][tmpPos] = C;
                tmps[(textIdx + 1) % 3][tmpPos] = C ^ helpers[textIdx][pos];
                tmps[(textIdx + 2) % 3][tmpPos] = C ^ helpers[(textIdx + 2) % 3][pos];
                count += printRec(pWords, false, caps, depthLeft - 1, tmps, tmpPos + 1, nodes[nodePos].down, textIdx, pos + 1);
            }
            nodePos = nodes[nodePos].next;
        }
        return count;
    }

    void recalcPosibleCount(uint start, uint end) {
        std::vector<uchar> tmps[3];
        for (int textIdx = 0; textIdx < 3; ++textIdx) {
            tmps[textIdx].resize(100);
        }
        for (int textIdx = 0; textIdx < 3; ++textIdx) {
            for (int i = start; i < end && i < posibleCounts[textIdx].size(); ++i) {
                posibleCounts[textIdx][i] = printRec(nullptr, true, false, 100, tmps, 0, 1, textIdx, i);
                posibleCounts[textIdx][i] += printRec(nullptr, true, true, 100, tmps, 0, 1, textIdx, i);
            }
        }

    }

    void printState(int windowStart, int windowEnd, int pos) {
        printf("pos: %6d\n", pos);
        for (int textIdx = 0; textIdx < 3; ++textIdx) {
            for (int i = windowStart; i < windowEnd; ++i) {
                printf("%4d ", posibleCounts[textIdx][i]);
            }
            printf("\n");
            for (int i = windowStart; i < windowEnd; ++i) {
                if (texts[textIdx][i] != '*') {
                    printf("  %c  ", texts[textIdx][i]);
                }
                else {
                    printf("[");
                    uint cross = helpers[0][i] + 256 * (helpers[1][i] + 256 * helpers[2][i]);
                    uint mask = typeMasks[cross].val[textIdx];
                    printf("%c", (mask & 0x1) > 0 ? 'L' : ' ');
                    printf("%c", (mask & 0x2) > 0 ? 'U' : ' ');
                    printf("%c", (mask & 0x4) > 0 ? 'S' : ' ');
                    printf("]");
                }

            }
            printf("\n");
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            uint cross = helpers[0][i] + 256 * (helpers[1][i] + 256 * helpers[2][i]);
            const auto& vars = tableBig[cross];
            printf((cross == 0 ? "  #  " : "%4d "), int(vars.size()));
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            printf(i == pos ? "  ^  " : "     ");
        }
        printf("\n");
        printf("\n");

        uint cross = helpers[0][pos] + 256 * (helpers[1][pos] + 256 * helpers[2][pos]);
        const auto& vars = tableBig[cross];

        printf("cross = ");
        for (int j = 23; j >= 0; --j) {
            printf("%d", (uchar)((cross) >> j) & 0x1);
        }
        printf("\n");
        printf("\n");

        const uint maxLen = 20;

        for (int i = 0; i < vars.size() && i < maxLen; ++i) {
            for (int j = i; j < vars.size(); j += maxLen) {
                auto triplet = vars[j];
                printf("%2d (%c,%c,%c); ", j, triplet.val[0], triplet.val[1], triplet.val[2]);
            }
            printf("\n");
        }
        printf("\n");
    }

    void loadCheckPoint(int i) {
        char checkPointName[] = "./checkPoint0.txt";

        sprintf(checkPointName, "./checkPoint%d.txt", i);

        std::vector<uchar> checkPoint;
        {
            FILE* in = fopen(checkPointName, "rb");
            fseek(in, 0L, SEEK_END);
            long sz = ftell(in);
            fseek(in, 0L, SEEK_SET);
            checkPoint.resize(sz);

            fread(checkPoint.data(), sizeof(uchar), sz, in);
            fclose(in);
        }
        int sel = 0;
        // Load chackpoint
        for (int i = 0; i < checkPoint.size() && i < codes[0].size(); ++i) {
            if (checkPoint[i] != '*') {
                texts[sel][i] = checkPoint[i];
                texts[(sel + 1) % 3][i] = checkPoint[i] ^ helpers[sel][i];
                texts[(sel + 2) % 3][i] = checkPoint[i] ^ helpers[(sel + 2) % 3][i];
            }
            else {
                texts[sel][i] = '*';
                texts[(sel + 1) % 3][i] = '*';
                texts[(sel + 2) % 3][i] = '*';
            }
        }
    }

    void init() {
        ReadFullFile("./tree.bin", nodes);
        ReadFullFile("./treeRev.bin", nodesRev);
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
        ReadFullFile("./bigrams.bin", bigramsF);
        ReadFullFile("./trigrams.bin", trigramsF);
        {
            std::vector<uchar> tmp;
            ReadFullFile("./input1.txt", tmp);
            macaron::Base64::Decode(tmp, codes[0]);
        }
        {
            std::vector<uchar> tmp;
            ReadFullFile("./input2.txt", tmp);
            macaron::Base64::Decode(tmp, codes[1]);
        }
        {
            std::vector<uchar> tmp;
            ReadFullFile("./input3.txt", tmp);
            macaron::Base64::Decode(tmp, codes[2]);
        }

        for (int first = 0; first < sizeof(allowed_chars) - 1; ++first) {

            for (int second = 0; second < sizeof(allowed_chars) - 1; ++second) {
                uchar val = allowed_chars[first] ^ allowed_chars[second];
                table[val].push_back({ allowed_chars[first], allowed_chars[second] });
            }
        }
        tableBig.resize(256 * 256 * 256);
        for (int first = 0; first < sizeof(allowed_chars) - 1; ++first) {

            for (int second = 0; second < sizeof(allowed_chars) - 1; ++second) {
                for (int third = 0; third < sizeof(allowed_chars) - 1; ++third) {
                    uint val12 = allowed_chars[first] ^ allowed_chars[second];
                    uint val23 = allowed_chars[second] ^ allowed_chars[third];
                    uint val31 = allowed_chars[third] ^ allowed_chars[first];
                    uint cross = val12 + 256 * (val23 + 256 * val31);
                    tableBig[cross].push_back({ allowed_chars[first], allowed_chars[second], allowed_chars[third] });
                }
            }
        }

        for (int textIdx = 0; textIdx < 3; ++textIdx) {
            helpers[textIdx].resize(codes[textIdx].size());
            for (int i = 0; i < codes[textIdx].size(); ++i) {
                helpers[textIdx][i] = codes[textIdx][i] ^ codes[(textIdx + 1) % 3][i];
            }
        }

        uchar maskingChars[] = " .,-;:?!\'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        static_assert(sizeof(maskingChars) - 1 < 64, "");
        for (int i = 0; i < 256; ++i) masks[i] = 0;
        for (int i = 0; i < sizeof(maskingChars) - 1; ++i)
        {
            masks[maskingChars[i]] = 0x0000000000000001LL << i;
        }
        for (int textIdx = 0; textIdx < 3; ++textIdx) {
            allowedChars[textIdx].resize(codes[textIdx].size());
        }
        for (int i = 0; i < codes[0].size(); ++i) {
            for (int textIdx = 0; textIdx < 3; ++textIdx) {
                allowedChars[textIdx][i] = 0;
            }
            uint cross = helpers[0][i] + 256 * (helpers[1][i] + 256 * helpers[2][i]);
            const auto& vars = tableBig[cross];
            for (auto triplet : vars) {
                allowedChars[0][i] |= masks[triplet.val[0]];
                allowedChars[1][i] |= masks[triplet.val[1]];
                allowedChars[2][i] |= masks[triplet.val[2]];
            }
        }

        for (int textIdx = 0; textIdx < 3; ++textIdx) {
            texts[textIdx].resize(codes[textIdx].size());
            for (int i = 0; i < codes[textIdx].size(); ++i) {
                texts[textIdx][i] = '*';
            }
        }

        for (int textIdx = 0; textIdx < 3; ++textIdx) {
            posibleCounts[textIdx].resize(codes[textIdx].size());
        }

        typeMasks.resize(tableBig.size());
        for (int i = 0; i < 256 * 256 * 256; ++i) {
            triplet<uint> &typeMask = typeMasks[i];

            for (auto triplet : tableBig[i]) {
                for (int textIdx = 0; textIdx < 3; ++textIdx) {
                    if (IsLower(triplet.val[textIdx])) {
                        typeMask.val[textIdx] |= 0x1;
                    }
                    else if (IsUpper(triplet.val[textIdx])) {
                        typeMask.val[textIdx] |= 0x2;
                    }
                    else {
                        typeMask.val[textIdx] |= 0x4;
                    }
                }
            }
        }
        for (int textIdx = 0; textIdx < 3; ++textIdx) {
            posibleCounts[textIdx].resize(codes[textIdx].size());
        }
    }

public:
    int test() {

        init();

        loadCheckPoint(0);
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
                windowEnd = MIN(pos / windowSize * windowSize + windowSize, codes[0].size() - 1);
                if (needRecalc || windowStart != windowStartPrev) {
                    recalcPosibleCount(windowStart, windowEnd);
                    needRecalc = false;
                }
                printState(windowStart, windowEnd, pos);
                needRefresh = false;
            }

            uint cross = helpers[0][pos] + 256 * (helpers[1][pos] + 256 * helpers[2][pos]);
            const auto& vars = tableBig[cross];

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
                printf("Which triplet?\n");
                int ret = scanf("%d", &res);
                if (ret > 0) {
                    if (res < vars.size()) {
                        for (int textIdx = 0; textIdx < 3; ++textIdx) {
                            texts[textIdx][pos] = vars[res].val[textIdx];
                        }
                        needRecalc = true;
                    }
                }
                needRefresh = true;
            }
            if (GetKeyState(VK_SUBTRACT) & 0x8000)
            {
                {
                    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF) {}
                }
                for (int textIdx = 0; textIdx < 3; ++textIdx) {
                    texts[textIdx][pos] = '*';
                }
                needRecalc = true;
                needRefresh = true;
            }
            if (GetKeyState(VK_F5) & 0x8000)
            {
                WriteFullFile("./checkPoint0.txt", texts[0]);

                loadCheckPoint(0);

                WriteFullFile("./checkPoint1.txt", texts[1]);
                WriteFullFile("./checkPoint2.txt", texts[2]);

                {
                    std::vector<uchar> key;
                    key.resize(codes[0].size());
                    for (int i = 0; i < codes[0].size(); ++i) {
                        key[i] = codes[0][i] ^ texts[0][i];
                    }
                    key = macaron::Base64::Encode(key);
                    WriteFullFile("./key.txt", key);
                }
                needRecalc = true;
                needRefresh = true;
            }
            if (GetKeyState(VK_F6) & 0x8000)
            {
                uchar ch = '*';
                for (int i = pos; i < texts[0].size(); ++i) {
                    uchar tmp = texts[0][i];
                    texts[0][i] = ch;
                    ch = tmp;
                }
                WriteFullFile("./checkPoint0.txt", texts[0]);

                loadCheckPoint(0);

                WriteFullFile("./checkPoint1.txt", texts[1]);
                WriteFullFile("./checkPoint2.txt", texts[2]);
                needRecalc = true;
                needRefresh = true;
            }
            if (GetKeyState(VK_F7) & 0x8000)
            {
                for (int i = pos; i + 1 < texts[0].size(); ++i) {
                    texts[0][i] = texts[0][i + 1];
                }
                WriteFullFile("./checkPoint0.txt", texts[0]);

                loadCheckPoint(0);

                WriteFullFile("./checkPoint1.txt", texts[1]);
                WriteFullFile("./checkPoint2.txt", texts[2]);
                needRecalc = true;
                needRefresh = true;
            }
            if (GetKeyState(VK_SPACE) & 0x8000)
            {
                std::vector<uchar> tmps[3];
                for (int textIdx = 0; textIdx < 3; ++textIdx) {
                    tmps[textIdx].resize(100);
                }

                int mainTextIdx = -1;
                int count = 0;
                std::vector<WordRecord> wordsList;
                {
                    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF) {}
                }
                printf("Which text?\n");
                int ret = scanf("%d", &mainTextIdx);
                if (ret > 0) {
                    count = printRec(&wordsList, true, isCapsLock, 100, tmps, 0, 1, mainTextIdx, pos);
                }
                printf("%d\n", count);

                std::sort(wordsList.begin(), wordsList.end(), [](const WordRecord& a, const WordRecord& b) {return (a.score + a.len) > (b.score + b.len); });

                for (uint i = 0; i < wordsList.size(); ++i) {
                    printf("%d: %f\n", i, wordsList[i].score);
                    for (int textIdx = 0; textIdx < 3; ++textIdx) {
                        tmps[textIdx].resize(100);
                        printf("%.*s\n", wordsList[i].len, wordsList[i].words[textIdx].data());
                    }
                    printf("\n");
                }
                int i = -1;
                {
                    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF) {}
                }
                printf("Which word?\n");
                ret = scanf("%d", &i);
                if (ret > 0) {
                    auto& word = wordsList[i];
                    for (int textIdx = 0; textIdx < 3; ++textIdx) {
                        for (int j = 0; j < word.len; ++j) {
                            texts[textIdx][pos + j] = word.words[textIdx][j];
                        }
                    }
                    needRecalc = true;
                }
                needRefresh = true;
            }

            pos = MAX((pos + codes[0].size()) % codes[0].size(), 0);
            std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
        }

        return 0;
    }
};



int select(const char* filename) {

    std::vector<uchar> text;
    ReadFullFile(filename, text);
    {
        FILE* in = fopen("./checkPoint0.txt", "rb");
        fseek(in, 0L, SEEK_END);
        long sz = ftell(in);
        fseek(in, 0L, SEEK_SET);
        long offset = text.size();
        text.resize(offset + sz);
        fread(text.data() + offset, sizeof(uchar), sz, in);
        fclose(in);
    }
    {
        FILE* in = fopen("./checkPoint1.txt", "rb");
        fseek(in, 0L, SEEK_END);
        long sz = ftell(in);
        fseek(in, 0L, SEEK_SET);
        long offset = text.size();
        text.resize(offset + sz);
        fread(text.data() + offset, sizeof(uchar), sz, in);
        fclose(in);
    }
    {
        FILE* in = fopen("./checkPoint2.txt", "rb");
        fseek(in, 0L, SEEK_END);
        long sz = ftell(in);
        fseek(in, 0L, SEEK_SET);
        long offset = text.size();
        text.resize(offset + sz);
        fread(text.data() + offset, sizeof(uchar), sz, in);
        fclose(in);
    }

    std::list<std::pair<uchar*, uint>> words;

    std::vector<uint> bigrams;
    uint bigramsTotal = 0;
    bigrams.resize(26 * 26);
    std::vector<uint> trigrams;
    uint trigramsTotal = 0;
    trigrams.resize(26 * 26 * 26);

    int alphaCounter = 0;
    for (uint i = 0; i < text.size(); ++i) {
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
        std::vector<float> bigramsF;
        float avrg = bigramsTotal / (float)(bigrams.size());
        bigramsF.resize(bigrams.size());
        for (uint biIdx = 0; biIdx < bigrams.size(); ++biIdx) {
            bigramsF[biIdx] = bigrams[biIdx] / avrg;
        }
        WriteFullFile("./bigrams.bin", bigramsF);
    }
    {
        std::vector<float> trigramsF;
        float avrg = trigramsTotal / (float)(trigrams.size());
        trigramsF.resize(trigrams.size());
        for (uint triIdx = 0; triIdx < trigrams.size(); ++triIdx) {
            trigramsF[triIdx] = trigrams[triIdx] / avrg;
        }
        WriteFullFile("./trigrams.bin", trigramsF);
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
    ReadFullFile("./out.txt", textOrd);

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
        while (endPos >= 0 && nodes[endPos].next >= 0) {
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
        else if (nodes[endPos].c != '\0') {
            int newPos = nodes.size();
            nodes.push_back({ '\0', -1, -1, 1 });
            nodes[endPos].next = newPos;
        }
    }
    WriteFullFile("./tree.bin", nodes);
    {
        FILE* out = fopen("./charPointers.bin", "wb");

        for (int i = 0; i < 26; ++i) {
            int size = charPointers[i].size();
            fwrite(&size, sizeof(int), 1, out);
            fwrite(charPointers[i].data(), sizeof(int), size, out);
        }
        fflush(out);
        fclose(out);
    }
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
    WriteFullFile("./treeRev.bin", nodes);
    return 0;
}

int main() {

    select("./big.txt");
    buildTree();
    Test t;
    t.test();

    return 0;
}