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
GLuint VAO, VBO_position, VBO_color;
static bool DrawLine = false;
GLfloat mx;
GLfloat my;
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> d(-1.0, 1.0);
uniform_real_distribution<float> dis(-0.005, 0.005);
uniform_real_distribution<float> cd(0.0, 1.0);
static int tri_index;

GLfloat vertexData[] = { //--- �ﰢ�� ��ġ ��
	-0.5, 0.8, 0.0, // �� ��
	-0.6, 0.4, 0.0, // ����
	-0.4, 0.4, 0.0, // ������

	0.5,  0.8, 0.0,
	0.4,  0.4, 0.0,
	0.6,  0.4, 0.0,

	-0.5, -0.4, 0.0,
	-0.6, -0.8, 0.0,
	-0.4, -0.8, 0.0,

	0.5, -0.4, 0.0,
	0.4, -0.8, 0.0,
	0.6, -0.8, 0.0,
};

GLfloat colorData[] = { //--- �ﰢ�� ��ġ ��
	1.0, 0.0, 0.0,
	1.0, 0.0, 0.0,
	1.0, 0.0, 0.0,

	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,

	0.0, 0.0, 1.0,
	0.0, 0.0, 1.0,
	0.0, 0.0, 1.0,

	0.4, 0.0, 0.3,
	0.4, 0.0, 0.3,
	0.4, 0.0, 0.3,
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

void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = 800;
	int h = 800;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void MoveVertex(int tri_index, GLfloat moveX, GLfloat moveY)
{
	for (int i{ 0 }; i < 3; ++i)
	{
		vertexData[tri_index * 9 + i * 3] += moveX;
		vertexData[tri_index * 9 + i * 3 + 1] += moveY;
	}
}

void RotateVertex(int digree, int index)
{
	float center_x = vertexData[index * 9];
	float center_y = vertexData[index * 9 + 4];

	vertexData[index * 9 + 0] = center_x + 0.4 * sin(digree / 360.0 * 2 * 3.141592);
	vertexData[index * 9 + 1] = center_y + 0.4 * cos(digree / 360.0 * 2 * 3.141592);

	vertexData[index * 9 + 3] = center_x + 0.1 * sin((digree - 90) / 360.0 * 2 * 3.141592);
	vertexData[index * 9 + 4] = center_y + 0.1 * cos((digree - 90) / 360.0 * 2 * 3.141592);

	vertexData[index * 9 + 6] = center_x + 0.1 * sin((digree + 90) / 360.0 * 2 * 3.141592);
	vertexData[index * 9 + 7] = center_y + 0.1 * cos((digree + 90) / 360.0 * 2 * 3.141592);
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

GLuint s_program;

void InitBuffer()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}


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

	//--- �ﰢ�� �׸���
	glDrawArrays(GL_TRIANGLES, 0, sizeof(vertexData) / 3);

	//--- ȭ�鿡 ����ϱ�
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

static GLfloat randMove[8];

void TimerFunction(int value)
{

	for (int i = 0; i < 4; ++i)
	{
		MoveVertex(i, randMove[i], randMove[i + 4]);

		for (int j = 0; j < 3; j++)
		{
			if (vertexData[i * 9 + j * 3] <= -1.0)
			{
				randMove[i] *= -1;
				RotateVertex(90, i);
				break;
			}
			else if (vertexData[i * 9 + j * 3] >= 1.0)
			{
				randMove[i] *= -1;
				RotateVertex(270, i);
				break;
			}
			else if (vertexData[i * 9 + j * 3 + 1] <= -1.0)
			{
				randMove[i + 4] *= -1;
				RotateVertex(0, i);
				break;
			}
			else if (vertexData[i * 9 + j * 3 + 1] >= 1.0)
			{
				randMove[i + 4] *= -1;
				RotateVertex(180, i);
				break;
			}
		}
	}

	InitBuffer();

	glutPostRedisplay();
	glutTimerFunc(10, TimerFunction, 1);
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

void Mouse(int button, int state, int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		vertexData[tri_index * 9 + 0 * 3] = mx;
		vertexData[tri_index * 9 + 0 * 3 + 1] = my + 0.2;
		vertexData[tri_index * 9 + 1 * 3] = mx - 0.1;
		vertexData[tri_index * 9 + 1 * 3 + 1] = my - 0.2;
		vertexData[tri_index * 9 + 2 * 3] = mx + 0.1;
		vertexData[tri_index * 9 + 2 * 3 + 1] = my - 0.2;

		for (int i{ 0 }; i < 3; ++i)
		{
			float randColor = cd(dre);

			for (int j{ 0 }; j < 3; ++j)
				colorData[tri_index * 9 + j * 3 + i] = randColor;
		}

		InitBuffer();
		tri_index = (tri_index + 1) % 4;
	}
}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
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

	for (int i = 0; i < 8; i++)
		randMove[i] = dis(dre);

	glutDisplayFunc(drawScene);
	glutMouseFunc(Mouse);
	glutReshapeFunc(Reshape);
	glutTimerFunc(100, TimerFunction, 1);
	glutKeyboardFunc(Keyboard);
	glutMainLoop();
}