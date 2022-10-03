#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h> // �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <random>
using namespace std;

GLchar* vertexsource, * fragmentsource; //--- �ҽ��ڵ� ���� ����
GLuint vertexshader, fragmentshader; //--- ���̴� ��ü
GLvoid Reshape(int w, int h);
GLuint VAO, VBO, EBO;
static bool DrawLine = false;
GLfloat mx;
GLfloat my;
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> d(-1.0, 1.0);

class Shape {
public:
	Shape()
	{

	}
	~Shape()
	{
		delete[] vertexDataList;
	}

	void MoveSet(GLfloat x, GLfloat y, GLfloat z)
	{
		_moveX = x;
		_moveY = y;
		_moveZ = z;
	}

	void ColorSet(GLfloat r, GLfloat g, GLfloat b)
	{
		_color._r = r,
		_color._g = g,
		_color._b = b,
		_color._a = 1.0;
	}

	struct Color {
		GLfloat _r = 0;
		GLfloat _g = 0;
		GLfloat _b = 0;
		GLfloat _a = 0;
	};

	struct Scale {
		GLfloat _x = 1.0;
		GLfloat _y = 1.0;
		GLfloat _z = 1.0;
	};

public:
	GLfloat* vertexDataList = nullptr;
	GLfloat _moveX = 0;
	GLfloat _moveY = 0;
	GLfloat _moveZ = 0;
	Color _color;
	Scale _scale;

};

static Shape s[4];

const GLfloat vertexDataList[] = { //--- �ﰢ�� ��ġ ��
  0.0,  0.2, 0.0,
 -0.2, -0.2, 0.0,
  0.2, -0.2, 0.0,
};

unsigned int index[] = {
	0, 1, 2,
};

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading 
	if (!fptr) // Return NULL on failure 
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
	length = ftell(fptr); // Find out how many bytes into the file we are 
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator 
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
	fclose(fptr); // Close the file 
	buf[length] = 0; // Null terminator 
	return buf; // Return the buffer 
}

void make_vertexShader()
{
	vertexsource = filetobuf("vertex.glsl");
	//--- ���ؽ� ���̴� ��ü �����
	vertexshader = glCreateShader(GL_VERTEX_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
	//--- ���ؽ� ���̴� �������ϱ�
	glCompileShader(vertexshader);
	//--- �������� ����� ���� ���� ���: ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexshader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader ������ ����\n" << errorLog << endl;
		return;
	}
}

void make_fragmentShader()
{
	fragmentsource = filetobuf("fragment.glsl");
	//--- �����׸�Ʈ ���̴� ��ü �����
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
	//--- �����׸�Ʈ ���̴� ������
	glCompileShader(fragmentshader);
	//--- �������� ����� ���� ���� ���: ������ ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentshader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader ������ ����\n" << errorLog << endl;
		return;
	}
}

void InitBuffer()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	for (int i = 0; i < 4; i++)
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexDataList), s[i].vertexDataList, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

GLuint s_program;

void InitShader()
{
	make_vertexShader(); //--- ���ؽ� ���̴� �����
	make_fragmentShader(); //--- �����׸�Ʈ ���̴� �����
	//-- shader Program
	s_program = glCreateProgram();
	glAttachShader(s_program, vertexshader);
	glAttachShader(s_program, fragmentshader);
	glLinkProgram(s_program);
	//--- ���̴� �����ϱ�
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
	//--- Shader Program ����ϱ�
	glUseProgram(s_program);
}

GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(0.2, 0.3, 0.3, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);

	//--- ����� VAO �ҷ�����
	glBindVertexArray(VAO);

	for (int i = 0; i < 4; i++)
	{
		int vertexLocation = glGetUniformLocation(s_program, "move");
		int vertexColorLocation = glGetUniformLocation(s_program, "out_Color");
		int vertexScaleLocation = glGetUniformLocation(s_program, "scale");

		glUseProgram(s_program);
		glUniform3f(vertexLocation, s[i]._moveX, s[i]._moveY, s[i]._moveZ);
		glUniform4f(vertexColorLocation, s[i]._color._r, s[i]._color._g, s[i]._color._b, s[i]._color._a);
		glUniform3f(vertexScaleLocation, s[i]._scale._x, s[i]._scale._x, s[i]._scale._z);

		//--- �ﰢ�� �׸���
		// DrawLine�� true�̸� �������� �׸��� false�̸� �ﰢ������ �׸���
		(DrawLine ? glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, 0) : glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0));
	}

	//--- ȭ�鿡 ����ϱ�
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

void TimerFunction(int value)
{
	glutPostRedisplay();
	glutTimerFunc(100, TimerFunction, 1);
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
	case 'A':
		DrawLine = false;
		break;
	case 'b':
	case 'B':
		DrawLine = true;
		break;
	}

	glutPostRedisplay();
}

void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = 800;
	int h = 800;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void Mouse(int button, int state, int x, int y)
{
	static int mouse_cnt = 0;
	static int scale_cnt = -24;

	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		s[mouse_cnt]._moveX = mx;
		s[mouse_cnt]._moveY = my;

		// �����ϸ� ���� ���� ����
		s[mouse_cnt]._color._r = d(dre);
		s[mouse_cnt]._color._g = d(dre);
		s[mouse_cnt]._color._b = d(dre);

		if (scale_cnt >= 0)
		{
			s[mouse_cnt]._scale._x -= 0.2;
			s[mouse_cnt]._scale._y -= 0.2;
			s[mouse_cnt]._scale._z -= 0.2;
		}
		else
		{
			s[mouse_cnt]._scale._x += 0.2;
			s[mouse_cnt]._scale._y += 0.2;
			s[mouse_cnt]._scale._z += 0.2;
		}

		s[mouse_cnt].vertexDataList[1] = 0.9;

		mouse_cnt++;
		scale_cnt++;

		if (scale_cnt == 24)
			scale_cnt = -24;

		if (mouse_cnt > 3)
			mouse_cnt = 0;
	}
}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	for (int j = 0; j < 4; j++)
	{
		s[j].vertexDataList = new GLfloat[9];
		for (int i = 0; i < 9; i++)
		{
			s[j].vertexDataList[i] = vertexDataList[i];
		}
	}
	s[0].MoveSet(-0.7, 0.7, 0.0);
	s[1].MoveSet(0.7, 0.7, 0.0);
	s[2].MoveSet(-0.7, -0.7, 0.0);
	s[3].MoveSet(0.7, -0.7, 0.0);

	s[0].ColorSet(1.0, 0.0, 0.0);
	s[1].ColorSet(0.0, 1.0, 0.0);
	s[2].ColorSet(0.0, 0.0, 1.0);
	s[3].ColorSet(0.8, 0.2, 0.9);

	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Example1");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();
	InitShader();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutMouseFunc(Mouse);
	glutReshapeFunc(Reshape);
	glutTimerFunc(100, TimerFunction, 1);
	glutKeyboardFunc(Keyboard);
	glutMainLoop();
}

