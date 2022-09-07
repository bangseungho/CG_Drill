#include <iostream>

using namespace std;



void add_matrix(int a[][3], int b[][3]);

void sub_matrix(int a[][3], int b[][3]);

void new_matrix(int a[][3], int b[][3]);

void mul_matrix(int a[][3], int b[][3]);

void trans_matrix(int a[][3]);

int det_matrix(int a[][3]);

void printinfo(int c[][3]);



int main()

{

	srand(static_cast<unsigned int>(time(nullptr)));



	int a[3][3];

	int b[3][3];

	char input;



	int det_a = 0;

	int det_b = 0;



	for (int i = 0; i < 9; i++)

	{

		a[i / 3][i % 3] = rand() % 3;

		b[i / 3][i % 3] = rand() % 3;

	}



	printinfo(a);

	printinfo(b);



	while (true)

	{

		cin >> input;



		if (input == 'q')

			break;



		switch (input)

		{

		case 'm':

			mul_matrix(a, b);

			break;

		case 'a':

			add_matrix(a, b);

			break;

		case 'd':

			sub_matrix(a, b);

			break;

		case 'r':

			det_a = det_matrix(a);

			det_b = det_matrix(b);

			break;

		case 't':

			trans_matrix(a);

			trans_matrix(b);

			break;

		case 's':

			new_matrix(a, b);

			break;

		}

	}

}



void add_matrix(int a[][3], int b[][3])

{

	int c[3][3];



	for (int i = 0; i < 9; i++)

	{

		c[i / 3][i % 3] = a[i / 3][i % 3] + b[i / 3][i % 3];

	}



	printinfo(c);

}



void sub_matrix(int a[][3], int b[][3])

{

	int c[3][3];



	for (int i = 0; i < 9; i++)

	{

		c[i / 3][i % 3] = a[i / 3][i % 3] - b[i / 3][i % 3];

	}



	printinfo(c);

}



void new_matrix(int a[][3], int b[][3])

{

	for (int i = 0; i < 9; i++)

	{

		a[i / 3][i % 3] = rand() % 3;

		b[i / 3][i % 3] = rand() % 3;

	}



	printinfo(a);

	printinfo(b);

}



void mul_matrix(int a[][3], int b[][3])

{

	int c[3][3] = { 0 };



	for (int i = 0; i < 9; i++)

	{

		for (int j = 0; j < 3; j++)

		{

			c[i / 3][i % 3] += a[i / 3][j] * b[j][i % 3];

		}

	}

	printinfo(c);

}



int det_matrix(int a[][3])

{

	int det_a;



	det_a = a[0][0] * a[1][1] * a[2][2] + a[0][1] * a[1][2] * a[2][0] + a[0][2] * a[1][0] * a[2][1] -

		(a[0][0] * a[1][2] * a[2][1] + a[0][1] * a[1][0] * a[2][2] + a[0][2] * a[1][1] * a[2][0]);



	cout << "det : " << det_a << endl;



	return det_a;

}



void trans_matrix(int a[][3])

{

	int c[3][3];



	int det_c;



	for (int i = 0; i < 9; i++)

	{

		c[i / 3][i % 3] = a[i % 3][i / 3];

	}

	printinfo(c);

	det_c = det_matrix(a);

}



void printinfo(int c[][3])

{

	for (int i = 0; i < 9; i++)

	{

		if (i % 3 == 0)

			cout << endl;



		cout << c[i / 3][i % 3] << " ";

	}

	cout << endl;

}