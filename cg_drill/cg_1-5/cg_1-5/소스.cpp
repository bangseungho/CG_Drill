#include <random>
#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <math.h>

#define Pi 3.141592
#define RANDPOSITION d(dre)
using namespace std;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
const int WindowWidth = 800;
const int WindowHeight = 800;
void TimerFunction(int value);
void Init();

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> d(-1.0, 1.0);

class Rec {
public:
	struct Color {
		float _r;
		float _g;
		float _b;
	};

	struct Position {
		GLfloat _x;
		GLfloat _y;
	};

	struct Dir {
		GLfloat _x;
		GLfloat _y;
	};

	Color _color;
	Position _pos;
	GLfloat _size;
	Dir _dir;
	int _life;
	BOOL _collision;

public:
	Rec(GLfloat _x, GLfloat _y) : _pos{ _x, _y }, _size(0.035f) { _color = { RANDPOSITION, RANDPOSITION, RANDPOSITION }; _dir = { RANDPOSITION, RANDPOSITION }; _life = 3; _collision = false; }
	void DrawObject() { glColor3f(_color._r, _color._g, _color._b); glRectf(_pos._x - _size, _pos._y - _size, _pos._x + _size, _pos._y + _size); }
};

static Rec* rec[100];
GLvoid Keyboard(unsigned char key, int x, int y);
void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);

GLfloat mx;
GLfloat my;
bool left_button;
static int index = 0;

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(WindowWidth, WindowHeight); // 윈도우의 크기 지정
	glutCreateWindow("cg_1-5"); // 윈도우 생성(윈도우 이름)
	glewExperimental = GL_TRUE; //--- GLEW 초기화하기
	if (glewInit() != GLEW_OK) // glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	Init();

	glutDisplayFunc(drawScene); // 출력 콜백함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard); // 키보드 입력 콜백함수 지정
	glutTimerFunc(100, TimerFunction, 1);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //--- 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT); //--- 설정된 색으로 전체를 칠하기

	for (int i = 0; i < 100; i++)
	{
		if (rec[i] != nullptr)
		{
			rec[i]->DrawObject();
		}
	}

	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'R':
	case 'r':
		index = 0;

		for (int i{0}; i<100; ++i)
		{
			if (rec[i] != nullptr)
			{
				rec[i] = nullptr;
				delete rec[i];
			}
		}
		for (int i{ 0 }; i < 100; ++i)
			rec[i] = new Rec(RANDPOSITION, RANDPOSITION);

		break;
	}
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

void Init()
{
	for (int i{ 0 }; i < 100; ++i)
		rec[i] = new Rec(RANDPOSITION, RANDPOSITION);
}

void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = WindowWidth;
	int h = WindowHeight;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void Mouse(int button, int state, int x, int y)
{
	left_button = false;
	// 윈도우 좌표에서 OpenGL좌표로 변환
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (button == GLUT_LEFT_BUTTON)
	{
		for (int i = 99; i >= 0; --i)
		{
			if (rec[i] != nullptr)
			{
				if (mx >= rec[i]->_pos._x - rec[i]->_size && mx <= rec[i]->_pos._x + rec[i]->_size &&
					my >= rec[i]->_pos._y - rec[i]->_size && my <= rec[i]->_pos._y + rec[i]->_size)
				{
					index = i;
					rec[index]->_size = 0.065f;
					
					left_button = true;

					if (rec[index] != nullptr && state == GLUT_UP)
					{
						rec[index] = nullptr;
						delete rec[index];
					}

					break;
				}
			}
		}
	}
}

void Motion(int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (left_button == true)
	{
		rec[index]->_pos._x = mx;
		rec[index]->_pos._y = my;
	}

	for (int i = 0; i < 100; ++i)
	{
		if (i != index && rec[i] != nullptr && rec[index] != nullptr)
		{
			if (rec[index]->_pos._x - rec[index]->_size < rec[i]->_pos._x + rec[i]->_size &&
				rec[index]->_pos._x + rec[index]->_size > rec[i]->_pos._x - rec[i]->_size &&
				rec[index]->_pos._y - rec[index]->_size < rec[i]->_pos._y + rec[i]->_size &&
				rec[index]->_pos._y + rec[index]->_size > rec[i]->_pos._y + rec[i]->_size)
			{
				if (rec[i]->_collision == false)
				{
					rec[i]->_life -= 1;
					rec[i]->_size -= 0.01;
					rec[i]->_collision = true;
				}

				if (rec[i]->_life == 0)
				{
					rec[i] = nullptr;
					delete rec[i];
				}
			}
			else
				rec[i]->_collision = false;

		}
	}

	glutPostRedisplay();
}

static int a = 0;

void TimerFunction(int value)
{
	for (int i = 0; i < 100; ++i)
	{
		if (rec[i] != nullptr && i != index) {

			if (rec[i]->_pos._x + rec[i]->_size > 1.0f || rec[i]->_pos._x - rec[i]->_size < -1.0f)
				rec[i]->_dir._x *= -1;
			if (rec[i]->_pos._y + rec[i]->_size > 1.0f || rec[i]->_pos._y - rec[i]->_size < -1.0f)
				rec[i]->_dir._y *= -1;

			rec[i]->_pos._x += rec[i]->_dir._x / 100;
			rec[i]->_pos._y += rec[i]->_dir._y / 100;
		}
	}

	glutPostRedisplay(); // 화면 재 출력
	glutTimerFunc(100, TimerFunction, 1);
}
