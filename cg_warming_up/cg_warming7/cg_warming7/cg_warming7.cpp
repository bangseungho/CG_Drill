#include <iostream>
#include <Windows.h>
using namespace std;

#define COL 5
#define LOW 5
#define BOARD_SIZE 16

class Node
{
public:
	char _data;
	bool _check;
};

class Game
{
public:
	Node* _node[COL][LOW];
	int _aflag = 0;
	int _bflag = 0;
	int _cflag = 0;
	int _dflag = 0;
	int _eflag = 0;
	int _fflag = 0;
	int _gflag = 0;
	int _hflag = 0;

public:
	void Init()
	{
		for (int i = 0; i < COL; i++)
		{
			for (int j = 0; j < LOW; j++)
			{
				_node[i][j] = new Node();
			}
		}

		for (int i = 1; i < COL; i++)
		{
			for (int j = 1; j < LOW; j++)
			{

				int randvalue = 1 + rand() % 8;

				switch (randvalue)
				{
				case 1:
					_node[i][j]->_data = 'A';
					_aflag++;
					if (_aflag > 2)
					{
						_aflag = 2;
						j--;
					}
					break;
				case 2:
					_node[i][j]->_data = 'B';
					_bflag++;
					if (_bflag > 2)
					{
						_bflag = 2;
						j--;
					}
					break;
				case 3:
					_node[i][j]->_data = 'C';
					_cflag++;
					if (_cflag > 2)
					{
						_cflag = 2;
						j--;
					}
					break;
				case 4:
					_node[i][j]->_data = 'D';
					_dflag++;
					if (_dflag > 2)
					{
						_dflag = 2;
						j--;
					}
					break;
				case 5:
					_node[i][j]->_data = 'E';
					_eflag++;
					if (_eflag > 2)
					{
						_eflag = 2;
						j--;
					}
					break;
				case 6:
					_node[i][j]->_data = 'F';
					_fflag++;
					if (_fflag > 2)
					{
						_fflag = 2;
						j--;
					}
					break;
				case 7:
					_node[i][j]->_data = 'G';
					_gflag++;
					if (_gflag > 2)
					{
						_gflag = 2;
						j--;
					}
					break;
				case 8:
					_node[i][j]->_data = 'H';
					_hflag++;
					if (_hflag > 2)
					{
						_hflag = 2;
						j--;
					}
					break;
				}

			}
		}
	}

	void PrintInfo()
	{
		_node[0][0]->_data = ' ';
		_node[0][1]->_data = 'a';
		_node[0][2]->_data = 'b';
		_node[0][3]->_data = 'c';
		_node[0][4]->_data = 'd';
		_node[1][0]->_data = '1';
		_node[2][0]->_data = '2';
		_node[3][0]->_data = '3';
		_node[4][0]->_data = '4';

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		cout << "=============" << endl;

		/*for (int i = 0; i < COL; i++)
		{
			for (int j = 0; j < LOW; j++)
			{
					cout << _node[i][j]->_data << "  ";
			}
			cout << endl << endl;
		}

		cout << "=============" << endl;*/

		for (int i = 0; i < COL; i++)
		{
			for (int j = 0; j < LOW; j++)
			{
				if (i == 0 || j == 0)
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				else 
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);

				if (i > 0 && j > 0 && _node[i][j]->_check == false)
					cout << "*" << "  ";
				else
				{
					switch (_node[i][j]->_data)
					{
					case 'A':
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
						break;
					case 'B':
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 3);
						break;
					case 'C':
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
						break;
					case 'D':
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 5);
						break;
					case 'E':
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
						break;
					case 'F':
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
						break;
					case 'G':
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
						break;
					case 'H':
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 9);
						break;
					}
					cout << _node[i][j]->_data << "  ";
				}
			}
			cout << endl << endl;
		}

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		cout << "=============" << endl;
	}
};

int main()
{
	srand(static_cast<int>(time(nullptr)));

	char input1[2];
	char input2[2];
	int score = 0;
	int cnt = 0;
	Game game;

	game.Init();

	while(true)
	{
		cout << "input card 1 : ";
		cin >> input1;

		if (input1[0] == 'r')
		{
			cout << "Reset!" << endl;
			game._aflag = 0;
			game._bflag = 0;
			game._cflag = 0;
			game._dflag = 0;
			game._eflag = 0;
			game._fflag = 0;
			game._gflag = 0;
			game._hflag = 0;

			for (int i = 0; i < COL; i++)
			{
				for (int j = 0; j < LOW; j++)
				{
					delete game._node[i][j];
				}
			}

			score = 0;
			cnt = 0;
			game.Init();
		}
		else
		{
			cout << "input card 2 : ";
			cin >> input2;
			cout << endl << endl;

			game._node[input1[1] - 48][input1[0] - 96]->_check = true;
			game._node[input2[1] - 48][input2[0] - 96]->_check = true;

			if (game._node[input1[1] - 48][input1[0] - 96]->_data == game._node[input2[1] - 48][input2[0] - 96]->_data)
			{
				game._node[input1[1] - 48][input1[0] - 96]->_check = true;
				game._node[input2[1] - 48][input2[0] - 96]->_check = true;
				score++;
			}
			else
			{	
				game.PrintInfo();
				Sleep(2000);
				game._node[input1[1] - 48][input1[0] - 96]->_check = false;
				game._node[input2[1] - 48][input2[0] - 96]->_check = false;
			}

			game.PrintInfo();
			cout << "SCORE : " << score << " " << "남은 횟수 : " << 10 - cnt << endl;
			cnt++;

			if (cnt == 10)
			{
				cout << "게임이 종료됩니다." << endl;
				break;
			}
		}
	}

	

	return 0;
}