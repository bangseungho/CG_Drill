#define SIZE 10
#define DATA_NUM 3

#include <stdio.h>
#include <iostream>
#include <math.h>
using namespace std;

class Node
{
public:
	Node() : _data{ 0 }, _InputData(false), _distance(0)
	{

	}

public:
	int _data[DATA_NUM];
	bool _InputData;
	double _distance;
	int _total;
};

class List
{
public:
	List() : _index(0)
	{
		for (int i = 0; i < SIZE; i++)
		{
			_node[i] = new Node();
		}
	}
	List(const List& c)
	{
		for (int i = 0; i < SIZE; i++)
		{
			_node[i] = (Node*)malloc(sizeof(Node));

			for(int j = 0; j < DATA_NUM; j++)
			_node[i]->_data[j] = c._node[i]->_data[j];

			_node[i]->_InputData = c._node[i]->_InputData;
		}
	}

	void AddData()
	{
		for (int i = 0; i < DATA_NUM; i++)
		{
			int input;
			cin >> input;

			for (int j = 0; j < SIZE; j++)
			{
				if (_node[j]->_InputData == false)
				{
					_index = j;
					break;
				}
			}

			if (_index < 10)
			{
				_node[_index]->_data[i] = input;
			}
			else
			{
				cout << "FULL DATA" << endl;
				rewind(stdin);
				return;
			}
		}
	

		for (int i = _index; i < SIZE; i++)
		{
			if (_node[i]->_InputData == false)
			{
				_node[_index]->_InputData = true;
				_index++;
				break;
			}
		}
	}

	void DelData()
	{
		for (int i = SIZE - 1; i >= 0; i--)
		{
			if (_node[i]->_InputData)
			{
				_node[i]->_InputData = false;
				_index--;
				return;
			}
			
		}
	}

	void EleData()
	{
		for (int i = SIZE -2; i >= 0; i--)
		{
			if (_node[i]->_InputData)
			{
				for (int j = 0; j < DATA_NUM; j++)
				{
					_node[i + 1]->_data[j] = _node[i]->_data[j];
				}
				_node[i + 1]->_InputData = true;
				_node[i]->_InputData = false;
			}
		}

		for (int i = 0; i < DATA_NUM; i++)
		{
			cin >> _node[0]->_data[i];
			_node[0]->_InputData = true;
		}

		for (int i = 0; i < SIZE - 1; i++)
		{
			if (_node[i]->_InputData == false)
			{
				_index = i;
				return;
			}
		}
	}

	void FDelDate()
	{
		for (int i = 0; i < SIZE; i++)
		{
			if (_node[i]->_InputData)
			{
				_node[i]->_InputData = false;
				return;
			}
		}
	}

	void Length()
	{
		int cnt = 0;

		for (int i = 0; i < 10; i++)
		{
			if (_node[i]->_InputData) cnt++;

		}

		cout << "Length : " << cnt << endl;
	}

	void CanDate()
	{
		for (int i = 0; i < 10; i++)
		{
			if (_node[i]->_InputData)
			{
				_node[i]->_InputData = false;
				_index = 0;
			}
		}
	}

	void FarData()
	{
		GetDistance();

		int max = 0;
		int j = 0;

		for (int i = 0; i < SIZE; i++)
		{
			if (_node[i]->_distance > max)
			{
				max = _node[i]->_distance;
				j = i;
			}
		}
		cout << "MAX : (" << _node[j]->_data[0] << " " << _node[j]->_data[1] << " " << _node[j]->_data[2] << ")" << endl;
	}

	void CloseData()
	{
		int min = 1000;
		int j = 0;

		GetTotalDate();

		for (int i = 0; i < SIZE; i++)
		{
			if (_node[i]->_total < min && _node[i]->_InputData)
			{
				min = _node[i]->_total;
				j = i;
			}
		}
		cout << "MIN : (" << _node[j]->_data[0] << " " << _node[j]->_data[1] << " " << _node[j]->_data[2] << ")" << endl;
	}

	void GetDistance()
	{
		for (int i = 0; i < SIZE; i++)
		 {
			if (_node[i]->_InputData)
			{
				_node[i]->_distance = sqrt(pow(_node[i]->_data[0], 2) + pow(_node[i]->_data[1], 2) + pow(_node[i]->_data[2], 2));
			}
		}
	}

	void SortDate()
	{
		GetTotalDate();

		for (int i = 0; i < SIZE; i++)
		{
			for (int j = i; j < SIZE; j++)
			{
				if (_node[i]->_total > _node[j]->_total)
				{
					Node* temp;
					temp = _node[i];
					_node[i] = _node[j];
					_node[j] = temp;
				}
			}
		}

		for (int i = 0; i < SIZE; i++)
		{
			if (_node[i]->_InputData == false)
			{
				for (int j = i; j < SIZE; j++)
				{
					if (_node[j]->_InputData)
					{
						Node* temp;
						temp = _node[i];
						_node[i] = _node[j];
						_node[j] = temp;
					}
				}
			}
		}
	}

	void GetTotalDate()
	{
		for (int i = 0; i < SIZE; i++)
		{
			_node[i]->_total = _node[i]->_data[0] + _node[i]->_data[1] + _node[i]->_data[2];
		}
	}

	void PrintInfo()
	{
		GetDistance();

		for (int i = SIZE - 1; i >= 0; i--)
		{
			cout << i << " : ";

			if (_node[i]->_InputData)
			{
				cout << _node[i]->_data[0] << " " << _node[i]->_data[1] << " " << _node[i]->_data[2] << " (length : " << _node[i]->_distance << ")";
			}

			cout << endl;

		}
		cout << endl;
	}
	
public:
	Node* _node[SIZE];
	int _index;
};

int main()
{
	List li;

	static bool trans = false;
	char input;

	while (1)
	{
		cout << "Command : ";
		cin >> input;

		if (input == 'q') break;

		switch (input)
		{
		case '+':
			li.AddData();
			li.PrintInfo();
			break;
		case '-':
			li.DelData();
			li.PrintInfo();
			break;
		case 'e':
			li.EleData();
			li.PrintInfo();
			break;
		case 'd':
			li.FDelDate();
			li.PrintInfo();
			break;
		case 'l':
			li.Length();
			li.PrintInfo();
			break;
		case 'c':
			li.CanDate();
			li.PrintInfo();
			break;
		case 'm':
			li.FarData();
			break;
		case 'n':
			li.CloseData();
			break;
		case 's':
			static List temp = li;

			if(trans)
			{
				li = temp;
				trans = false;
			}
			else
			{
				li.SortDate();
				trans = true;
			}

			li.PrintInfo();
			break;
		}

	}


	return 0;
}