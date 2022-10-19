#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <random>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

using namespace std;

GLchar* vertexsource, * fragmentsource; //--- 소스코드 저장 변수
GLuint vertexshader, fragmentshader; //--- 세이더 객체
GLuint VAO, VBO, VBO_line, EBO;
GLvoid Reshape(int w, int h);
const double pi = 3.14159265358979;
GLfloat mx;
GLfloat my;
void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> d(-1.0, 1.0);
uniform_real_distribution<float> cd(0, 1.0);
GLuint s_program;
bool draw_on[10]{ false };
GLfloat color[10][3];

int cross_shape[3][2];

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
	//--- 버텍스 세이더 객체 만들기
	vertexshader = glCreateShader(GL_VERTEX_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexshader);
	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexshader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
		return;
	}
}

void make_fragmentShader()
{
	fragmentsource = filetobuf("fragment.glsl");
	//--- 프래그먼트 세이더 객체 만들기
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentshader);
	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentshader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << endl;
		return;
	}
}

void InitShader()
{
	make_vertexShader(); //--- 버텍스 세이더 만들기
	make_fragmentShader(); //--- 프래그먼트 세이더 만들기
	//-- shader Program
	s_program = glCreateProgram();
	glAttachShader(s_program, vertexshader);
	glAttachShader(s_program, fragmentshader);
	glLinkProgram(s_program);
	//--- 세이더 삭제하기
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
	//--- Shader Program 사용하기
	glUseProgram(s_program);
}

void InitBuffer()
{
	GLfloat vertices[] = {
		-1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, -1.0, 0.0,

		-0.5, -0.5, -0.5, 
		 0.5, -0.5, -0.5, 
		 -0.5, -0.5, 0.5, 
		 0.5, -0.5, 0.5, 

		 -0.5, 0.5, -0.5, 
		 0.5, 0.5, -0.5, 
		 -0.5, 0.5, 0.5, 
		 0.5, 0.5, 0.5,

		 0.0, -0.3, -0.3, 
		 0.4, -0.3, 0.3, 
		 -0.4, -0.3, 0.3,
		 0.0, 0.3, 0.0,

	};

	unsigned int index[] = {
		0, 1, 
		2, 3, 

		4, 5, 6, // 아랫면
		5, 6, 7,

		8, 9, 10, // 윗면
		9, 10, 11,

		5, 7, 9, // 우측면
		7, 9, 11, 

		4, 6, 8, // 왼쪽면
		6, 8, 10, 

		6, 7, 10, // 뒷면
		7, 10, 11, 

		4, 5, 8, // 앞면
		5, 8, 9,

		//================

		12, 13, 14, 
		15, 14, 13, 
		15, 12, 14, 
		15, 13, 12,
	};

	glGenVertexArrays(1, &VAO); //--- VAO 를 지정하고 할당하기
	glBindVertexArray(VAO); //--- VAO를 바인드하기
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(0.0, 0.0, 0.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);
	//--- 사용할 VAO 불러오기
	//--- 삼각형 그리기
	glBindVertexArray(VAO);

	glm::mat4 Rz = glm::mat4(1.0f); //--- 회전 행렬 선언
	glm::mat4 Tx = glm::mat4(1.0f); //--- 이동 행렬 선언
	glm::mat4 TR = glm::mat4(1.0f); //--- 합성 변환 행렬

	Tx = glm::translate(Tx, glm::vec3(0.0, 0.0, 0.0));
	Rz = glm::rotate(Rz, glm::radians(10.0f), glm::vec3(1.0, 1.0, 0.0));

	TR = Rz * Tx;

	glm::mat4 unit = glm::mat4(1.0f);
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");

	int vColorLocation = glGetUniformLocation(s_program, "shape_color");

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(unit)); // line
	glUniform3f(vColorLocation, 1.0, 0.0, 0.0);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
	glUniform3f(vColorLocation, 0.0, 1.0, 0.0);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * 2));

	glPointSize(5);

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR)); // triangle


	for (int i = 0; i < 6; ++i)
	{
		if (draw_on[i]) {
			glUniform3f(vColorLocation, color[i][0], color[i][1], color[i][2]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * (4 + 6 * i)));
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		if (draw_on[i + 6]) {
			glUniform3f(vColorLocation, color[i][0], color[i][1], color[i][2]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * (40 + 3 * i)));
		}
	}


	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (button == GLUT_LEFT_BUTTON)
	{
		
	}
}

void Keyboard(unsigned char key, int x, int y)
{
		switch (key)
		{
		case '1':
			if (!draw_on[0])
				draw_on[0] = true;
			else draw_on[0] = false;
			break;
		case '2':
			if (!draw_on[1])
				draw_on[1] = true;
			else draw_on[1] = false;
			break;
		case '3':
			if (!draw_on[2])
				draw_on[2] = true;
			else draw_on[2] = false;
			break;
		case '4':
			if (!draw_on[3])
				draw_on[3] = true;
			else draw_on[3] = false;
			break;
		case '5':
			if (!draw_on[4])
				draw_on[4] = true;
			else draw_on[4] = false;
			break;
		case '6':
			if (!draw_on[5])
				draw_on[5] = true;
			else draw_on[5] = false;
			break;

		case '7':
			if (!draw_on[6])
				draw_on[6] = true;
			else draw_on[6] = false;
			break;
		case '8':
			if (!draw_on[7])
				draw_on[7] = true;
			else draw_on[7] = false;
			break;
		case '9':
			if (!draw_on[8])
				draw_on[8] = true;
			else draw_on[8] = false;
			break;
		case '0':
			if (!draw_on[9])
				draw_on[9] = true;
			else draw_on[9] = false;
			break;
		case 'a':
			if (!draw_on[0])
			{
				draw_on[0] = true;
				draw_on[1] = true;
			}
			else 
			{
				draw_on[0] = false;
				draw_on[1] = false;
			}
			break;
		case 'b':
			if (!draw_on[2])
			{
				draw_on[2] = true;
				draw_on[3] = true;
			}
			else
			{
				draw_on[2] = false;
				draw_on[3] = false;
			}
			break;
		case 'c':
			if (!draw_on[4])
			{
				draw_on[4] = true;
				draw_on[5] = true;
			}
			else
			{
				draw_on[4] = false;
				draw_on[5] = false;
			}
			break;
		case 'e':
			if (!draw_on[7])
			{
				draw_on[6] = true;
				draw_on[7] = true;
			}
			else
			{
				draw_on[6] = false;
				draw_on[7] = false;
			}
			break;
		case 'f':
			if (!draw_on[8])
			{
				draw_on[6] = true;
				draw_on[8] = true;
			}
			else
			{
				draw_on[6] = false;
				draw_on[8] = false;
			}
			break;
		case 'g':
			if (!draw_on[9])
			{
				draw_on[6] = true;
				draw_on[9] = true;
			}
			else
			{
				draw_on[6] = false;
				draw_on[9] = false;
			}
			break;
		}
}

void TimerFunction(int value)
{

	InitBuffer();
	glutPostRedisplay();
	glutTimerFunc(100, TimerFunction, 1);
}

void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = 800;
	int h = 600;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void Init()
{

}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	for (int i = 0; i < 10; ++i)
		for (int j = 0; j < 3; ++j)
		{
			color[i][j] = cd(dre);
		}
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Example1");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	Init();
	glewInit();
	InitShader();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop();
}

