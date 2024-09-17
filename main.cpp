
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>

#include "Base64.h"

#define MAX(x, y) (((x) > (y)) ? (x) : y)
#define MIN(x, y) (((x) < (y)) ? (x) : y)

void printRec(int pos, int depthLeft, unsigned char* tmp1, unsigned char* tmp2, int tmpPos, unsigned char* tmpData1, unsigned char* tmpData2, unsigned char* helper, std::vector<std::pair<unsigned char, unsigned char>>* table) {
    if (depthLeft == 0) {
        printf("%.*s\n%.*s\n\n", tmpPos, tmp1, tmpPos, tmp2);
        return;
    }
    if (tmpData1[pos] != '*') {
        tmp1[tmpPos] = tmpData1[pos];
        tmp2[tmpPos] = tmpData2[pos];
        printRec(pos + 1, depthLeft - 1, tmp1, tmp2, tmpPos + 1, tmpData1, tmpData2, helper, table);
        return;
    }
    unsigned char cross = helper[pos];
    const auto& vars = table[cross];
    for (int i = 0; i < vars.size(); ++i) {
        auto pair = vars[i];
        tmp1[tmpPos] = pair.first;
        tmp2[tmpPos] = pair.second;
        printRec(pos + 1, depthLeft - 1, tmp1, tmp2, tmpPos + 1, tmpData1, tmpData2, helper, table);
        tmp1[tmpPos] = pair.second;
        tmp2[tmpPos] = pair.first;
        printRec(pos + 1, depthLeft - 1, tmp1, tmp2, tmpPos + 1, tmpData1, tmpData2, helper, table);
    }
}

const unsigned char allowed_chars[] = " .,!-:;()?0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//const unsigned char allowed_chars[] = " .,!?0123456789àáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß";

int test() {
    setlocale(LC_ALL, "Russian");

    FILE* in = fopen("./input1.txt", "r");
    fseek(in, 0L, SEEK_END);
    long sz = ftell(in);
    fseek(in, 0L, SEEK_SET);
    std::vector<unsigned char> text1;
    text1.resize(sz);

    fread(text1.data(), sizeof(unsigned char), sz, in);
    fclose(in);
    std::vector<unsigned char> text1Decoded;
    macaron::Base64::Decode(text1, text1Decoded);

    in = fopen("./input2.txt", "r");
    fseek(in, 0L, SEEK_END);
    sz = ftell(in);
    fseek(in, 0L, SEEK_SET);
    std::vector<unsigned char> text2;
    text2.resize(sz);

    fread(text2.data(), sizeof(unsigned char), sz, in);
    fclose(in);
    std::vector<unsigned char> text2Decoded;
    macaron::Base64::Decode(text2, text2Decoded);
    
    for (int i = 0; i < text1Decoded.size(); ++i) {
        for (int j = 7; j > 0; --j) {
            //printf("%d", (unsigned char)((str1Decoded[i]^str2Decoded[i]) >> j) & 0x1);
        }
        //printf(" ");
    }


    std::vector<std::pair<unsigned char, unsigned char>> table[256];
    for (int first = 0; first < sizeof(allowed_chars) - 1; ++first) {
        
        for (int second = first; second < sizeof(allowed_chars) - 1; ++second) {
            unsigned char val = (unsigned char)allowed_chars[first] ^ (unsigned char)allowed_chars[second];
            table[val].push_back({allowed_chars[first], allowed_chars[second]});
        }
    }

    std::vector<unsigned char> helper;
    helper.resize(text1Decoded.size());
    for (int i = 0; i < text1Decoded.size(); ++i) {
        helper[i] = (unsigned char)(text1Decoded[i] ^ text2Decoded[i]);
    }

    printf("\n");
    for (int i = 0; i < 256; ++i) {
        printf("%02X = %2d: ", i, (int)table[i].size());
        for (auto pair : table[i]) {
            printf("(%c,%c)", pair.first, pair.second);
        }
        printf("\n");
    }


    for (int i = 0; i < text1Decoded.size(); ++i) {
        //printf("%02X - %02X - %02X\n", (unsigned char)(str1Decoded[i]), (unsigned char)(str2Decoded[i]), (unsigned char)(str1Decoded[i] ^ str2Decoded[i]));
    }

    std::vector<unsigned char> tmpData1, tmpData2;
    tmpData1.resize(text1Decoded.size());
    tmpData2.resize(text1Decoded.size());
    for (int i = 0; i < text1Decoded.size(); ++i) {
        tmpData1[i] = tmpData2[i] = '*';
    }

    getchar();

    int pos = 0;
    int windowSize = 50;
    while (true) {
        system("cls");

        int windowStart = MAX(pos / windowSize * windowSize, 0);
        int windowEnd = MIN(pos / windowSize * windowSize + windowSize, helper.size() - 1);

        for (int i = windowStart; i < windowEnd; ++i) {
            unsigned char cross = helper[i];
            const auto& vars = table[cross];
            printf((cross == 0 ? " # " : "%2d "), int(vars.size()));
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            printf(" %c ", tmpData1[i]);
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            printf(" %c ", tmpData2[i]);
        }
        printf("\n");
        for (int i = windowStart; i < windowEnd; ++i) {
            printf(i == pos ? " ^ " : "   ");
        }
        printf("\n");
        printf("\n");

        unsigned char cross = helper[pos];
        const auto& vars = table[cross];

        printf("cross = ");
        for (int j = 7; j > 0; --j) {
            printf("%d", (unsigned char)((cross) >> j) & 0x1);
        }
        printf("\n");
        printf("\n");

        for (int i = 0; i < vars.size(); ++i) {
            auto pair = vars[i];
            printf("%2d (%c,%c); %2d (%c,%c);\n", i, pair.first, pair.second, i + vars.size(), pair.second, pair.first);
        }
        printf("\n");

        int res;
        int ret = getchar();
        switch (ret) {
        case '+':
            pos++;
            break;
        case '-':
            pos--;
            break;
        case '*':
            pos += windowSize;
            break;
        case '/':
            pos -= windowSize;
            break;
        case '=':
            scanf("%d", &res);
            if (res < vars.size()) {
                tmpData1[pos] = vars[res].first;
                tmpData2[pos] = vars[res].second;
            }
            else if (res - vars.size() < vars.size()) {
                tmpData2[pos] = vars[res - vars.size()].first;
                tmpData1[pos] = vars[res - vars.size()].second;
            }
            break;
        case 'c':
        {
            scanf("%d", &res);
            std::vector<unsigned char> tmp1, tmp2;
            tmp1.resize(res + 1);
            tmp2.resize(res + 1);
            tmp1[res] = tmp2[res] = '\0';
            printRec(pos, res, tmp1.data(), tmp2.data(), 0, tmpData1.data(), tmpData2.data(), helper.data(), table);
            getchar();
            getchar();
            break;
        }

        }
        pos = MAX(MIN(pos, helper.size() - 1), 0);
    }

    return 0;
}

int select() {

    FILE* in = fopen("./dict.txt", "r");
    fseek(in, 0L, SEEK_END);
    long sz = ftell(in);
    fseek(in, 0L, SEEK_SET);
    std::vector<unsigned char> text1;
    text1.resize(sz);
    fread(text1.data(), sizeof(unsigned char), sz, in);
    fclose(in);

    unsigned int table[256];
    for (int i = 0; i < 256; ++i) table[i] = 0;

    for (int i = 0; i < text1.size(); ++i) {
        table[text1[i]]++;
    }

    for (int i = 0; i < 256; ++i) {
        printf("%c: %d\n", (unsigned char)i, table[i]);
    }

    for (int i = 0; i < 256; ++i) table[i] = 0;

    table['a'] = 0x00000001;
    table['b'] = 0x00000002;
    table['c'] = 0x00000004;
    table['d'] = 0x00000008;
    table['e'] = 0x00000010;
    table['f'] = 0x00000020;
    table['g'] = 0x00000040;
    table['h'] = 0x00000080;
    table['i'] = 0x00000100;
    table['j'] = 0x00000200;
    table['k'] = 0x00000400;
    table['l'] = 0x00000800;
    table['m'] = 0x00001000;
    table['n'] = 0x00002000;
    table['o'] = 0x00004000;
    table['p'] = 0x00008000;
    table['q'] = 0x00010000;
    table['r'] = 0x00020000;
    table['s'] = 0x00040000;
    table['t'] = 0x00080000;
    table['u'] = 0x00100000;
    table['v'] = 0x00200000;
    table['w'] = 0x00400000;
    table['x'] = 0x00800000;
    table['y'] = 0x01000000;
    table['z'] = 0x02000000;
    table[' '] = 0x04000000;
    table['-'] = 0x08000000;
    table[','] = 0x10000000;
    table['.'] = 0x20000000;


    FILE* out = fopen("./out.txt", "w");
    int maxLen = 0;
    int wordStart = 0;
    while (wordStart < text1.size()) {
        bool ok = true;
        int i = wordStart;
        unsigned char prevC = '\n';
        int charRep = 1;
        while (i < text1.size() && text1[i] != '\n')
        {
            unsigned char c = text1[i];
            if (c == prevC) {
                ++charRep;
            }
            else {
                prevC = c;
                charRep = 1;
            }
            ok = ok && (table[text1[i]] != 0) && charRep <= 3;
            ++i;
        }
        int wordLen = i - wordStart;
        if (ok && wordLen >= 3 && wordLen <= 30) {
            maxLen = MAX(maxLen, wordLen);
            fprintf(out, "%.*s\n", wordLen, text1.data() + wordStart);
        }
        wordStart = i + 1;
    }

    fflush(out);
    fclose(out);

    return 0;
}

struct Node;

struct Node
{
    unsigned char c = '\0';
    unsigned int next = 0;
    unsigned int down = 0;
};

int buildTree(unsigned int posT, unsigned int posB, std::vector<unsigned char>& text, std::vector<Node>& buffer) {
    unsigned char c = text[posT];
    return 0;
}


int test2() {

    FILE* in = fopen("./out.txt", "rb");
    fseek(in, 0L, SEEK_END);
    long sz = ftell(in);
    fseek(in, 0L, SEEK_SET);
    std::vector<unsigned char> text;
    text.resize(sz);
    int read = fread(text.data(), sizeof(unsigned char), sz, in);
    fflush(in);
    fclose(in);

    unsigned int table[256];
    for (int i = 0; i < 256; ++i) table[i] = 0;

    table['a'] = 0;
    table['b'] = 1;
    table['c'] = 2;
    table['d'] = 3;
    table['e'] = 4;
    table['f'] = 5;
    table['g'] = 6;
    table['h'] = 7;
    table['i'] = 8;
    table['j'] = 9;
    table['k'] = 10;
    table['l'] = 11;
    table['m'] = 12;
    table['n'] = 13;
    table['o'] = 14;
    table['p'] = 15;
    table['q'] = 16;
    table['r'] = 17;
    table['s'] = 18;
    table['t'] = 19;
    table['u'] = 20;
    table['v'] = 21;
    table['w'] = 22;
    table['x'] = 23;
    table['y'] = 24;
    table['z'] = 25;
    table[' '] = 26;
    table['-'] = 27;
    table[','] = 28;
    table['.'] = 29;

    FILE* outE = fopen("./outErr.txt", "w");
    FILE* outG = fopen("./outG.txt", "w");

    int prevWordStart = -1;
    int prevWordLen = -1;

    int wordStart = 0;
    while (wordStart < text.size()) {
        bool ok = true;
        int i = wordStart;
        while (i < text.size() && text[i] != '\r')
        {
            ++i;
        }
        int wordLen = i - wordStart;
        if (prevWordStart != -1) {
            int cmp = 0;
            for (int j = 0; j < wordLen && j < prevWordLen; ++j) {
                unsigned char c = text[wordStart + j];
                unsigned char cPrev = text[prevWordStart + j];
                if (c < cPrev) {
                    cmp = -1;
                    break;
                }
                else if (c > cPrev) {
                    cmp = 1;
                    break;
                }
            }
            if (cmp == 0) {
                if (wordLen < prevWordLen) {
                    cmp = -1;
                }
                else {
                    cmp = 1;
                }
            }
            if (cmp < 0) {
                fprintf(outE, "%.*s\n", wordLen, text.data() + wordStart);
            }
            else {
                fprintf(outG, "%.*s\n", wordLen, text.data() + wordStart);
            }
        }
        prevWordStart = wordStart;
        prevWordLen = wordLen;
        wordStart = i + 2;
    }
;
    fclose(outE);
    fclose(outG);

    return 0;
}
int main() {
    test2();

    return 0;
}