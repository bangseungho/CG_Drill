#include <iostream>
#include <gl/glew.h> // �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);

const int WindowWidth = 800;
const int WindowHeight = 800;
void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);
void Motion(int x, int y);
void Mouse(int button, int state, int x, int y);

class Rec {
public:
	struct Color {
		float _r;
		float _g;
		float _b;
	};

	struct Position {
		GLfloat _left;
		GLfloat _bottom;
		GLfloat _right;
		GLfloat _top;
	};

	Color _color;
	Position _pos;

public:
	Rec() : _color{ (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX }, _pos{ -0.5f, -0.5f, 0.5f, 0.5f }{};
	void Move(GLfloat left, GLfloat bottom, GLfloat right, GLfloat top) { _pos._left = left, _pos._bottom = bottom, _pos._right = right, _pos._top = top; }
};

Rec* r[5];
static int index{ 0 };
static int rec_num{ 0 };
static bool left_button{false};

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	for (int i{ 0 }; i < 5; ++i) r[i] = new Rec();

	srand(static_cast<unsigned int>(time(nullptr)));
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0); // �������� ��ġ ����
	glutInitWindowSize(WindowWidth, WindowHeight); // �������� ũ�� ����
	glutCreateWindow("cg_1-3"); // ������ ����(������ �̸�)
	glewExperimental = GL_TRUE; //--- GLEW �ʱ�ȭ�ϱ�
	if (glewInit() != GLEW_OK) // glew �ʱ�ȭ
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene); // ��� �ݹ��Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �ݹ��Լ� ����
	glutKeyboardFunc(Keyboard); // Ű���� �Է� �ݹ��Լ� ����
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMainLoop(); // �̺�Ʈ ó�� ����
} 

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	//--- ����� ���� ����
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //--- �������� ����
	glClear(GL_COLOR_BUFFER_BIT); //--- ������ ������ ��ü�� ĥ�ϱ�

	for (int i{ 0 }; i < rec_num + 1; ++i)
	{
		if (rec_num > 4) rec_num = 4;
		glColor3f(r[i]->_color._r, r[i]->_color._g, r[i]->_color._b);
		glRectf(r[i]->_pos._left, r[i]->_pos._bottom, r[i]->_pos._right, r[i]->_pos._top);
	}


	glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'A':
	case 'a':
		index++;
		rec_num++;
		if (index > 4)
		{
			rec_num = 4;
			index = 4;
		}
		break;
	}
	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
}

GLfloat mx;
GLfloat my;

void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = WindowWidth;
	int h = WindowHeight;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

GLfloat lgap{ 0.0f };
GLfloat bgap{ 0.0f };
GLfloat rgap{ 0.0f };
GLfloat tgap{ 0.0f };

void Mouse(int button, int state, int x, int y)
{
	// ������ ��ǥ���� OpenGL��ǥ�� ��ȯ
	
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	left_button = false;

	if (button == GLUT_LEFT_BUTTON)
	{
		for (int i = rec_num; i >= 0; --i)
		{
			if (mx >= r[i]->_pos._left && mx <= r[i]->_pos._right &&
				my >= r[i]->_pos._bottom && my <= r[i]->_pos._top)
			{
				index = i;
				i = 0;

				lgap = mx - r[index]->_pos._left;
				bgap = my - r[index]->_pos._bottom;
				rgap = r[index]->_pos._right - mx;
				tgap = r[index]->_pos._top - my;
				left_button = true;
			}
		}

	}
}

void Motion(int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (left_button == true)
	{
		r[index]->Move(mx - lgap, my - bgap, mx + rgap, my + tgap);
	}
	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
}
