#define _CRT_SECURE_NO_WARNINGS
#define MAX 1000
#include <iostream>
using namespace std;

int CountWord(char str[]);
int CountNum(char str[]);
int CountCapital(char str[]);

class total
{
public:
    char arr[MAX] = {};
    int index = 0;
};

total ToNum;
total ToCap;

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

    word_cnt = CountWord(str);
    num_cnt = CountNum(str);
    cap_cnt = CountCapital(str);

    cout << "word count : " << word_cnt << endl;
    cout << "number count : " << num_cnt << " " << "( " << ToNum.arr << ")" << endl;
    cout << "capital count : " << cap_cnt << " " << "( " << ToCap.arr << ")" << endl;

    return 0;
}

int CountWord(char str[])
{
    int ret = 0;
    int i = 0;

    while (true)
    {
        i++;
 
        if (str[i] == ' ' || str[i] == '.' || str[i] == '\n' || str[i] == '\0')
        {
            if (str[i - 1] >= 65 && str[i - 1] <= 90 ||
                str[i - 1] >= 97 && str[i - 1] <= 122)
            {
                ret++;

            }
        }

        if (str[i] == '\0')
            break;
    }

    return ret;
}

int CountNum(char str[])
{
    int ret = 0;
    int i = 0;

    while (true)
    {
        i++;

        if (str[i] == ' ' || str[i] == '.' || str[i] == '\n' || str[i] == '\0')
        {
            if (str[i - 1] >= 48 && str[i - 1] <= 57)
            {
                ret++;
            }
        }

        if (str[i] == '\0')
            break;
    }

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] >= 48 && str[i] <= 57)
        {
            if (str[i + 1] >= 65 && str[i + 1] <= 90 ||
                str[i + 1] >= 97 && str[i + 1] <= 122)
                continue;

            ToNum.arr[ToNum.index++] = str[i];
            if (str[i + 1] == ' ' || str[i + 1] == '\n')
                ToNum.arr[ToNum.index++] = ' ';
        }
    }

    return ret;
}

int CountCapital(char str[])
{
    int ret = 0;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i - 1] >= 65 && str[i - 1] <= 90)
        {
            ret++;

            for (int j = i; ; j++)
            { // 3D
                if (str[j - 2] >= 48 && str[i] <= 57)
                    ToCap.arr[ToCap.index++] = str[j - 2];

                ToCap.arr[ToCap.index++] = str[j -1 ];
              
                if (str[j] == ' ' || str[j] == '\n')
                {
                    ToCap.arr[ToCap.index++] = ' ';
                    break;
                }
            }
            
        }
    }

    return ret;
}