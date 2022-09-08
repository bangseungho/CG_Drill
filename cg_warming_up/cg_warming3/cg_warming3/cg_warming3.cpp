#define _CRT_SECURE_NO_WARNINGS
#define MAX 1000
#include <iostream>
using namespace std;

void PrintInfo(char str[]);
void ReverseWord(char str[]);
void AddWord(char str[]);
void ReverseJump(char str[]);
void ChangeWord(char str[]);
void Samesame(char str[]);
static bool flag = false;

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

    PrintInfo(str);

    while (true)
    {
        
        cout << "input the command : ";
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
            ReverseJump(str);
            break;
        case 'g':
            ChangeWord(str);
            break;
        case 'h':
            Samesame(str);
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
    int j = 0;
    char temp[MAX]{};
    

    if (flag == false)
    {
        for (int i = 0; str[i] != '\0'; i++)
        {
            cnt++;
            temp[j] = str[i];

            if (cnt % 3 == 0)
            {
                temp[j + 1] = '@';
                temp[j + 2] = '@';
                j += 2;
            }
            j++;
        }

        for (int i = strlen(temp) - 1; ; i--)
        {
            if (temp[i] == '@')
                temp[i] = '\0';
            else break;
        }

        for (int i = 0; temp[i] != '\0'; i++)
        {
            str[i] = temp[i];
        }

    }
    else
    {
        for (int i = 0; str[i] != '\0'; i++)
        {
            if (str[i] == '@')
            {
                for (int j = i; str[j] != '\0'; j++)
                {
                    str[j] = str[j + 2];
                    str[j + 1] = str[j + 3];
                }
            }
        }
    }

    if (flag)
        flag = false;
    else if (flag == false)
        flag = true;

    PrintInfo(str);
}

void ReverseJump(char str[])
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != '@' && flag == true)
        {
            if (str[i] == ' ')
            {
                char temp = str[i + 1];
                str[i + 1] = str[i + 2];
                str[i + 2] = temp;
            }
            else if (str[i + 2] == ' ')
            {
                char temp = str[i];
                str[i] = str[i + 1];
                str[i + 1] = temp;
            }
            else
            {
                char temp = str[i];
                str[i] = str[i + 2];
                str[i + 2] = temp;
            }
            i += 2;
        }
    }

    PrintInfo(str);
}

void ChangeWord(char str[])
{
    char old_word;
    char new_word;

    cout << "Enter the Word you want to change : ";
    cin >> old_word;
    cout << "Enter the new Word : ";
    cin >> new_word;
    cout << endl;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == old_word)
            str[i] = new_word;
    }

    PrintInfo(str);
}

//void Samesame(char str[])
//{
//    char temp[MAX] = {};
//    char sameword[MAX] = {};
//    int i = 0;
//    int index = 0;
//
//    for (;;)
//    {
//        if (str[i] == ' ' || str[i] == '\0' || str[i] == '\n')
//        {
//            index = 0;
//
//            for (int j = 0; temp[j] != '\0'; j++)
//            {
//                if (j > strlen(temp) / 2 - 1)
//                    break;
//
//                if (temp[j] == temp[strlen(temp) - j - 1])
//                {
//                    sameword[j] = temp[j];
//                    cout << sameword[j];
//                }
//            }
//            cout << endl;
//
//        }
//
//        temp[index++] = str[i];
// 
//        if (str[i] == '\0')
//            break;
//
//        i++;
//    }
//}

void Samesame(char str[])
{
    char temp[100][100] = {};
    char temp2[100][100] = {};


    int index = 0;
    int index2 = 0;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ' ' || str[i] == '\n')
        {
            index2 = 0;
            index++;
            continue;
        }

        temp[index][index2++] = str[i];

    }

    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < strlen(temp[i]); j++)
        {
            int a = 0;
        }
    }
    
    for (int i = 0; i < 100; i++)
    {
        int len = strlen(temp[i]);

        for (int j = 0; j < strlen(temp[i]) / 2; j++)
        {
            if (temp[i][j] == temp[i][strlen(temp[i]) - 1 - j])
            {
                temp2[i][j] = temp[i][j];
            }
            else break;
        }
    }
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            if (temp2[i][j] != 0)
            {
                cout << temp2[i] << endl;
                break;
            }
        }
    }

    cout << endl;

}

void PrintInfo(char str[])
{
    int len = strlen(str);

    for (int i = 0; i <= len; i++)
    {
        cout << str[i];
    }

    cout << endl << endl;
}

