#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#define RANDNUM (float)rand() / (float)RAND_MAX

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
const int WindowWidth = 800;
const int WindowHeight = 800;

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

	enum class Animation {
		stop,
		cross,
		zigzag,
		size,
	};

	Color _color;
	Position _pos;
	static Animation _animation1;
	static Animation _animation2;
	static Animation _animation3;
	GLfloat _moveX = 0.01f;
	GLfloat _moveY = 0.01f;
	GLfloat _transSize = 0.05f;
	GLfloat _mx;
	GLfloat _my;

public:
	Rec(GLfloat _x, GLfloat _y) : _pos{ _x, _y } { _color = { RANDNUM, RANDNUM, RANDNUM }; _mx = _x, _my = _y; }
	void Move(GLfloat x, GLfloat y) { _pos._x += x, _pos._y += y; }
	void DrawObject(GLfloat mx, GLfloat my) 
	{ 
		glRectf(mx - _transSize, my - _transSize, mx + _transSize, my + _transSize);
	}
	void InitializePosition() { _pos._x = _mx, _pos._y = _my; }
};

static Rec* rec[5];

GLvoid Keyboard(unsigned char key, int x, int y);
void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);
 void Motion(int x, int y);
 void TimerFunction(int value);
 void Mouse(int button, int state, int x, int y);
 GLfloat mx;
 GLfloat my;
 static int index = 0;
 Rec::Animation _animation1 = Rec::Animation::stop;
 Rec::Animation _animation2 = Rec::Animation::stop;
 Rec::Animation _animation3 = Rec::Animation::stop;

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(WindowWidth, WindowHeight); // 윈도우의 크기 지정
	glutCreateWindow("cg_1-4"); // 윈도우 생성(윈도우 이름)
	glewExperimental = GL_TRUE; //--- GLEW 초기화하기
	if (glewInit() != GLEW_OK) // glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene); // 출력 콜백함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard); // 키보드 입력 콜백함수 지정
	glutMouseFunc(Mouse);
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //--- 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT); //--- 설정된 색으로 전체를 칠하기

	for (int i = 0; i < index; i++)
	{
		if (rec[i] != nullptr)
		{
			glColor3f(rec[i]->_color._r, rec[i]->_color._g, rec[i]->_color._b);
			rec[i]->DrawObject(rec[i]->_pos._x, rec[i]->_pos._y);
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
	case 'A':
	case 'a':
		if (_animation1 == Rec::Animation::cross)
			_animation1 = Rec::Animation::stop;
		else
		{
			_animation1 = Rec::Animation::cross;
			for (int i = 0; i < index; ++i)
			{
				rec[i]->_moveX = 0.01f;
				rec[i]->_moveY = 0.01f;
			}
		}
		break;
	case 'I':
	case 'i':
		if (_animation2 == Rec::Animation::zigzag)
			_animation2 = Rec::Animation::stop;
		else
		{
			_animation2 = Rec::Animation::zigzag;
			for (int i = 0; i < index; ++i)
			{
				rec[i]->_moveX = 0.05f;
				rec[i]->_moveY = 0.02f;
			}
		}
		break;
	case 'C':
	case 'c':
		if (_animation3 == Rec::Animation::size)
			_animation3 = Rec::Animation::stop;
		else
		{
			_animation3 = Rec::Animation::size;
		}
		break;
	case 'S':
	case 's':
		_animation1 = Rec::Animation::stop;
		_animation2 = Rec::Animation::stop;
		_animation3 = Rec::Animation::stop;
		break;
	case 'M':
	case 'm':
		for (int i = 0; i < index; ++i)
			rec[i]->InitializePosition();
		break;
	case 'R':
	case 'r':
		if (index > 0)
		{
			index--;
			delete rec[index];
		}
		
		break;
	case 'Q':
	case 'q':
		exit(1);
	}
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
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
	// 윈도우 좌표에서 OpenGL좌표로 변환
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (index < 5)
		{
			rec[index] = new Rec(mx, my);
			index++;
		}
	}
}

void TimerFunction(int value)
{
	if (_animation1 == Rec::Animation::cross)
	{
		for (int i = 0; i < index; i++)
		{
			if (rec[i]->_pos._x + rec[i]->_transSize > 1.0f || rec[i]->_pos._x - rec[i]->_transSize < -1.0f)
				rec[i]->_moveX *= -1;
			if (rec[i]->_pos._y + rec[i]->_transSize > 1.0f || rec[i]->_pos._y - rec[i]->_transSize < -1.0f)
				rec[i]->_moveY *= -1;

			rec[i]->Move(rec[i]->_moveX, rec[i]->_moveY);
		}
	}
	if (_animation2 == Rec::Animation::zigzag)
	{
		static int zigzag_cnt = 0;
		for (int i = 0; i < index; i++)
		{
			if (rec[i]->_pos._x + 0.05f > 1.0f || rec[i]->_pos._x - 0.05f < -1.0f)
				rec[i]->_moveX *= -1;
			if (rec[i]->_pos._y + 0.05f > 1.0f || rec[i]->_pos._y - 0.05f < -1.0f)
				rec[i]->_moveY *= -1;
			
			zigzag_cnt++;

			if (zigzag_cnt > 10)
			{
				rec[i]->_moveX *= -1;
				zigzag_cnt = 0;
			}

			rec[i]->Move(rec[i]->_moveX, rec[i]->_moveY);
		}
	}
	if (_animation3 == Rec::Animation::size)
	{
		for (int i = 0; i < index; i++)
		{
			int randValue = rand() % 10;
			
			switch (randValue)
			{
			case 0:
				rec[i]->_transSize += 0.005f;
				break;
			case 1:
				rec[i]->_transSize += 0.002f;
				break;
			case 2:
				rec[i]->_transSize -= 0.001f;
				break;
			case 3:
				rec[i]->_transSize -= 0.0005f;
				break;
			case 4:
				rec[i]->_transSize -= 0.005f;
				break;
			}
		}
	}


	glutPostRedisplay(); // 화면 재 출력
	glutTimerFunc(100, TimerFunction, 1);
}