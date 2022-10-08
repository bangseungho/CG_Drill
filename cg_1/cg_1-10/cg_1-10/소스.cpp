#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h> // �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <random>
using namespace std;

#define ROW 3
#define COL 1

GLchar* vertexsource, * fragmentsource; //--- �ҽ��ڵ� ���� ����
GLuint vertexshader, fragmentshader; //--- ���̴� ��ü
GLuint VAO, VBO[3];
GLvoid Reshape(int w, int h);
const double pi = 3.14159265358979;
GLfloat mx;
GLfloat my;
void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> d(-1.0, 1.0);
GLuint s_program;
static int shape = 0;
static bool draw_point = false;

const GLfloat colors[] = {
	 0.0, 0.5, 1.0,
	 0.0, 0.5, 1.0,
	 0.0, 0.5, 1.0,
	 0.0, 0.5, 1.0,

	 1.0, 0.0, 0.0,
	 1.0, 0.0, 0.0,
	 1.0, 0.0, 0.0,

	 0.0, 1.0, 0.0,

};

GLfloat points[4][42] = { 	//--- �� ��� ��
	-1.0, 0.0, 0.0,
	 1.0, 0.0, 0.0,

	 0.0, -1.0, 0.0,
	 0.0,  1.0, 0.0,

	 -0.2, -0.2, 0.0, // left bottom vertex
	  0.2,  0.2, 0.0, // left top vertex
	 -0.2, -0.2, 0.0, // right bottom vertex

	  0.0,  0.2, 0.0, // left top vertex
	  0.2, -0.2, 0.0, // right bottom vertex
	  0.0,  0.2, 0.0, // right top vertex

	  0.0, 0.1, 0.0,
	 -0.2, 0.1, 0.0,
	  0.2, 0.1, 0.0, 

	  0.0, 0.0, 0.0,
};

GLfloat movePos[] = { 
	0.0, 0.0, 
	0.0, 0.0, 
	0.0, 0.0, 
	0.0, 0.0, 

	-0.5, 0.5,  
	-0.5, 0.5,  
	-0.5, 0.5,  

	-0.5, 0.5,  
	-0.5, 0.5,  
	-0.5, 0.5,  

	-0.5, 0.5,  
	-0.5, 0.5,  
	-0.5, 0.5,  

	-0.5, 0.5,   
};

GLfloat copy_points[]{ 0 };

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
	glGenVertexArrays(1, &VAO); //--- VAO �� �����ϰ� �Ҵ��ϱ�
	glBindVertexArray(VAO); //--- VAO�� ���ε��ϱ�

	glGenBuffers(3, VBO); //--- 1���� VBO�� �����ϰ� �Ҵ��ϱ�

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(movePos), movePos, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
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
	glClearColor(1.0, 1.0, 1.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);
	//--- ����� VAO �ҷ�����
	glBindVertexArray(VAO);
	//--- �ﰢ�� �׸���
	glPointSize(5);

	int movePos = glGetUniformLocation(s_program, "in_movePos");
	int Color = glGetUniformLocation(s_program, "new_Color");
	glUniform2f(movePos, 0, 0);
	glUniform3f(Color, 0.0, 0.5, 1.0);
	glDrawArrays(GL_LINES, 0, 4);

	glUniform2f(movePos, -0.5, 0.5);
	glUniform3f(Color, 1.0, 0.0, 0.0);
	glDrawArrays(GL_LINES, 4, 2);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 3);

	glUniform2f(movePos, 0.5, 0.5);
	glUniform3f(Color, 0.0, 1.0, 0.0);
	glDrawArrays(GL_TRIANGLES, 1 * 14 + 4, 6);

	glUniform2f(movePos, -0.5, -0.5);
	glUniform3f(Color, 0.0, 0.0, 1.0);
	glDrawArrays(GL_TRIANGLES, 2 * 14 + 4, 9);

	glUniform2f(movePos, 0.5, -0.5);
	glUniform3f(Color, 1.0, 0.0, 1.0);
	if (draw_point)
		glDrawArrays(GL_POINTS, 3 * 14 + 13, 1);
	glDrawArrays(GL_TRIANGLES, 3 * 14 + 4, 9);


	glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		shape = true;
	}
}

void move_point(float value, int row, int column, int index)
{
	points[index][row * ROW + column * COL] += value;
}

void line_to_triangle()
{
	if (points[0][6 * ROW + 0 * COL] - 0.2 <= points[0][4 * 3 + 3])
	{
		move_point(-0.01, 5, 0, 0);
		move_point(0.01, 6, 0, 0);
		move_point(0.01, 6, 0, 0);
	}
}

void triangle_to_rectangle()
{
	if (points[1][4 * ROW + 0 * COL] <= points[1][5 * ROW + 0 * COL])
	{
		move_point(-0.01, 5, 0, 1);
		move_point(-0.01, 7, 0, 1);
		move_point(0.01, 9, 0, 1);
	}
}

void rectangle_to_pentagon()
{
	if (points[2][5 * ROW + 1 * COL] >= 0.1)
	{
		move_point(-0.01, 5, 1, 2);
		move_point(-0.01, 7, 1, 2);
		move_point(-0.01, 9, 1, 2);

	}
	else if (points[2][10 * ROW + 1 * COL] <= 0.2)
	{
		move_point(0.01, 10, 1, 2);
		move_point(0.01, 4, 0, 2);
		move_point(-0.01, 6, 0, 2);
		move_point(-0.01, 8, 0, 2);
		move_point(0.01, 4, 1, 2);
		move_point(0.01, 6, 1, 2);
		move_point(0.01, 8, 1,2);
	}
}

void pentagon_to_point()
{
	if (points[3][7 * ROW + 0 * COL] <= -0.01)
	{
		for (int i{ 12 }; i < 39; ++i)
		{
			if (points[3][i] != 0)
			{
				if (i == 31 || i == 15 || i == 21 || i == 27 || i == 33 || i == 36)
				{
					if (points[3][i] < 0.01)
						points[3][i] += 0.02;
					else if (points[3][i] > 0.01)
						points[3][i] -= 0.02;
				}
				else if (points[3][i] < 0.01)
					points[3][i] += 0.01;
				else if (points[3][i] > 0.01)
					points[3][i] -= 0.01;
			}
		}
	}
	else
	{
		draw_point = true;
	}
}


void TimerFunction(int value)
{
	if (shape)
	{
		line_to_triangle();
		triangle_to_rectangle();
		rectangle_to_pentagon();
		pentagon_to_point();
	}


	InitBuffer();
	glutPostRedisplay();
	glutTimerFunc(100, TimerFunction, 1);
}

void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = 800;
	int h = 800;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void Init()
{
	for (int i = 1; i < 4; ++i)
	{
		for (int j = 0; j < 42; ++j)
		{
			points[i][j] = points[0][j];
		}
	}

	while (points[1][6 * ROW + 0 * COL] - 0.2 <= points[1][4 * 3 + 3])
	{
		move_point(-0.01, 5, 0, 1);
		move_point(0.01, 6, 0, 1);
		move_point(0.01, 6, 0, 1);
	}

	while (points[2][4 * ROW + 0 * COL] <= points[2][5 * ROW + 0 * COL])
	{
		if (points[2][6 * ROW + 0 * COL] - 0.2 <= points[2][4 * 3 + 3])
		{
			move_point(-0.01, 5, 0, 2);
			move_point(0.01, 6, 0, 2);
			move_point(0.01, 6, 0, 2);
		}
		else if (points[2][4 * ROW + 0 * COL] <= points[2][5 * ROW + 0 * COL])
		{
			move_point(-0.01, 5, 0, 2);
			move_point(-0.01, 7, 0, 2);
			move_point(0.01, 9, 0, 2);
		}
	}

	while (points[3][10 * ROW + 1 * COL] <= 0.2)
	{
		if (points[3][6 * ROW + 0 * COL] - 0.2 <= points[3][4 * 3 + 3])
		{
			move_point(-0.01, 5, 0, 3);
			move_point(0.01, 6, 0, 3);
			move_point(0.01, 6, 0, 3);
		}
		else if (points[3][4 * ROW + 0 * COL] <= points[3][5 * ROW + 0 * COL])
		{
			move_point(-0.01, 5, 0, 3);
			move_point(-0.01, 7, 0, 3);
			move_point(0.01, 9, 0, 3);
		}
		else if (points[3][5 * ROW + 1 * COL] >= 0.1)
		{
			move_point(-0.01, 5, 1, 3);
			move_point(-0.01, 7, 1, 3);
			move_point(-0.01, 9, 1, 3);

		}
		else if (points[3][10 * ROW + 1 * COL] <= 0.2)
		{
			move_point(0.01, 10, 1, 3);
			move_point(0.01, 4, 0, 3);
			move_point(-0.01, 6, 0, 3);
			move_point(-0.01, 8, 0, 3);
			move_point(0.01, 4, 1, 3);
			move_point(0.01, 6, 1, 3);
			move_point(0.01, 8, 1, 3);
		}
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
	Init();
	glewInit();
	InitShader();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop();
}

