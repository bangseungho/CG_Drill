#define _CRT_SECURE_NO_WARNINGS
#define MAX 1000
#include <iostream>
using namespace std;

void ReverseWord(char str[]);
void PrintInfo(char str[]);
void AddWord(char str[]);

int main()
{
    int word_cnt = 0;
    int num_cnt = 0;
    int cap_cnt = 0;
    char input = { };

    FILE* fp = fopen("data.txt", "r");
    char str[MAX] = { 0, };

    fread(str, 1, MAX, fp);
    
    fclose(fp);

    while (true)
    {
        cin >> input;

        if (input == 'q')
            break;

        switch (input)
        {
        case 'd':
            ReverseWord(str);
            break;
        case 'e':
            AddWord(str);
            break;
        case 'f':
            break;
        case 'g':
            break;
        case 'h':
            break;
        }

    }

    return 0;
}

void ReverseWord(char str[])
{
    char temp[MAX] = {};
    
    int len = strlen(str);

    for (int i = 0; i<= len; i++)
    {
        temp[i] = str[strlen(str) - i - 1];
    }

    for (int i = 0; i <= len; i++)
    {
        str[i] = temp[i];
    }

    str[len] = '\0';

    PrintInfo(str);
}

void AddWord(char str[])
{
    int cnt = 0;

    for (int i = 0; i <= strlen(str); i++)
    {
        if (cnt % 4 == 0)
        {
            str[i] = '@';
        }
        cnt++;
    }

    PrintInfo(str);
}

void PrintInfo(char str[])
{
    int len = strlen(str);

    for (int i = 0; i <= len; i++)
    {
        cout << str[i];
    }

    cout << endl;
}

