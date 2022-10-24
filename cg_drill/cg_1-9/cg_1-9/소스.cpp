#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <random>
using namespace std;

GLchar* vertexsource, * fragmentsource; //--- 소스코드 저장 변수
GLuint vertexshader, fragmentshader; //--- 세이더 객체
GLuint vao, vbo[2];
GLvoid Reshape(int w, int h);
const double pi = 3.14159265358979;
static bool mouse_flag = false;
GLfloat mx;
GLfloat my;
void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);
static GLfloat theta = 0;
static GLfloat radius = 0;
static int cnt = 0;
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> d(-1.0, 1.0);
GLfloat back_ground_r = d(dre);
GLfloat back_ground_g = d(dre);
GLfloat back_ground_b = d(dre);

GLfloat point[480] = { 	//--- 삼각형 위치 값
};

const GLfloat colors[] = { //--- 삼각형 꼭지점 색상
 1.0, 1.0, 1.0
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

void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
	glBindVertexArray(vao); //--- VAO를 바인드하기
	glGenBuffers(2, vbo); //--- 2개의 VBO를 지정하고 할당하기
	//--- 1번째 VBO를 활성화하여 바인드하고, 버텍스 속성 (좌표값)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	//--- 변수 diamond 에서 버텍스 데이터 값을 버퍼에 복사한다.
	//--- triShape 배열의 사이즈: 9 * float
	glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
	//--- 좌표값을 attribute 인덱스 0번에 명시한다: 버텍스 당 3* float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
	//--- attribute 인덱스 0번을 사용가능하게 함
	glEnableVertexAttribArray(0);
	//--- 2번째 VBO를 활성화 하여 바인드 하고, 버텍스 속성 (색상)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	//--- 변수 colors에서 버텍스 색상을 복사한다.
	//--- colors 배열의 사이즈: 9 *float 
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
	//--- 색상값을 attribute 인덱스 1번에 명시한다: 버텍스 당 3*float
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//--- attribute 인덱스 1번을 사용 가능하게 함.
	glEnableVertexAttribArray(1);
}

GLuint s_program;

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

GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(back_ground_r, back_ground_g, back_ground_b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);
	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);
	//--- 삼각형 그리기
	glPointSize(5);

	if (mouse_flag)
		glDrawArrays(GL_LINE_SMOOTH, 0, cnt);

	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		int value_theta = 20;
		GLfloat value_radius = 0.005;
		mouse_flag = true;
		theta = 0;
		radius = 0;
		cnt = 0;
		int j = 0;

		back_ground_r = d(dre);
		back_ground_g = d(dre);
		back_ground_b = d(dre);

		int vColor = glGetUniformLocation(s_program, "select_Color");
		glUseProgram(s_program);
		srand(unsigned int(time(nullptr)));
		int randValue = rand() % 2;

		if (randValue == 0)
		{
			glUniform3f(vColor, 1.0, 0.0, 0.0);

			for (int i = 0; i < 120; i++)
			{
				if (i == 60)
				{
					mx += 0.585;
					my += 0.05;
					theta = 190;
					value_radius *= -1;
					value_theta = 20;
				}

				if (i < 60)
				{
					point[j] = mx + radius * sin(theta / 360 * 2 * pi);
					point[j + 1] = my + radius * cos(theta / 360 * 2 * pi);
				}
				else
				{
					point[j] = mx + radius * cos(theta / 360 * 2 * pi);
					point[j + 1] = my + radius * sin(theta / 360 * 2 * pi);
				}

				j += 3;
				theta += value_theta;
				radius += value_radius;
			}
		}
		else
		{
			glUniform3f(vColor, 1.0, 1.0, 1.0);

			theta = 270;
			for (int i = 0; i < 120; i++)
			{
				if (i == 60)
				{
					mx += 0.585;
					theta = 270;
					value_radius *= -1;
					value_theta = 20;
				}

				if (i < 60)
				{
					point[j] = mx + radius * cos(theta / 360 * 2 * pi);
					point[j + 1] = my + radius * sin(theta / 360 * 2 * pi);
				}
				else
				{
					point[j] = mx + radius * sin(theta / 360 * 2 * pi);
					point[j + 1] = my + radius * cos(theta / 360 * 2 * pi);
				}

				j += 3;
				theta += value_theta;
				radius += value_radius;
			}
		}
	
	}
}

void TimerFunction(int value)
{
	if (mouse_flag)
	{
		if(cnt < 120)
			cnt++;
	}


	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);

	glutPostRedisplay();
	glutTimerFunc(10, TimerFunction, 1);
}

void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = 800;
	int h = 800;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Example1");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	InitShader();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop();
}

