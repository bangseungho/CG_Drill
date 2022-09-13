#include <iostream>
#include <windows.h>
#include <iomanip>
#define SIZE 30

using namespace std;

enum Direction
{
	left = 0,
	right = 1,
	up = 2,
	down = 3,
};

class Path
{
public:
	int Init()
	{
		_map[y][x] = _path_num;
		int i = 0;
		int cnt = 1;

		while (true)
		{
			if (cango())
			_prev_dir = _dir;

			if(rand() % 5 == 0)
				_dir = rand() % 4;

			if (cango())
			{
				if (_prev_dir == _dir)
					cnt++;
				else
					cnt = 0;

				if (cnt < 7)
				{
					switch (_dir)
					{
					case Direction::left:
						_lflag = true;
						x -= 1;
						break;
					case Direction::right:
						_rflag = true;
						x += 1;
						break;
					case Direction::up:
						_uflag = true;
						y -= 1;
						break;
					case Direction::down:
						_dflag = true;
						y += 1;
						break;
					}

					_map[y][x] = ++_path_num;

					i++;
				}
			}

			if (_map[29][29] != 0)
			{
				if (_lflag = false || _rflag == false || _uflag == false || _dflag == false) return 1;
			}

			if (_map[29][29] != 0)
				break;
		}
	
	}

	bool cango()
	{
		if (_dir == Direction::left && x == 0)
		{
			return false;
		}
		if (_dir == Direction::right && x == 29)
		{
			return false;
		}
		if (_dir == Direction::up && y == 0)
		{
			return false;
		}
		if (_dir == Direction::down && y == 29)
		{
			return false;
		}


	}

	void PrintInfo()
	{
		for (int i = 0; i < SIZE; i++)
		{
			for (int j = 0; j < SIZE; j++)
			{
				if (_map[i][j] != 0)
				{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
				}
				else
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				cout << setw(4) << _map[i][j] << "   ";
			}
			cout << endl << endl;
		}

	}

private:
	int _map[SIZE][SIZE]{};
	int x = 0;
	int y = 0;
	int _num = 0;
	int _path_num = 1;
	int _dir = down;
	int _prev_dir = 0;
	bool _lflag = false;
	bool _rflag = false;
	bool _uflag = false;
	bool _dflag = false;
};

int main()
{
	Path p;

	srand((unsigned int)time(nullptr));

	char input;
	int replay = 0;

	replay = p.Init();
	p.PrintInfo();

	while (1)
	{
		if (replay == 1)
			p.Init();

		cin >> input;

		if (input == 'q')
			break;

		switch (input)
		{
		case 's':
			break;
		case 'l':
			break;
		}

		p.PrintInfo();
	}






	return 0;
}
