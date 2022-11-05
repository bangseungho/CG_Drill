#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"

using namespace std;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> urd{ 0.0, 1.0 };
GLchar* coord_vertexsource, *obj1_vertexsource, * camera_vertexsource;
GLchar* fragmentsource; //--- 소스코드 저장 변수
GLuint coord_vertexshader, obj1_vertexshader, camera_vertexshader;
GLuint fragmentshader; //--- 세이더 객체

GLvoid Reshape(int w, int h);
GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);

GLuint coord_s_program;
GLuint obj1_s_program;
GLuint camera_s_program;
GLfloat mx;
GLfloat my;

static bool depth_draw;
static bool rotation;
static bool rotation_up;
static bool open_front;
static bool hex_open_side;
static bool quad_open_side;
static bool projection_type = true;

enum {
	front,
	left,
	back,
	right,
	up,
	down,
};

enum class Shape {
	hexi,
	quad,
};

static Shape shape = Shape::quad;

struct Vec3d
{
	GLfloat _posX;
	GLfloat _posY;
	GLfloat _posZ;
};

struct Color
{
	GLfloat	_r;
	GLfloat	_g;
	GLfloat	_b;
};

struct Vertice
{
	Vec3d _pos;
	Color _color;
};

class Object
{
protected:
	GLuint vao;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_color;
	vector<Vertice> vertices; // 꼭짓점 모델 좌표 정보
	Vec3d _trans_info; // 현재 이동 변환 정보
	Vec3d _rotate_info; // 현재 회전 변환 정보
	GLfloat _scale_info; // 현재 신축 변환 정보
	glm::vec4 center = glm::vec4(1.0f);
	unsigned int modelLocation;
	const char* modelTransform;
	GLuint s_program;


public:
	glm::mat4 T = glm::mat4(1.0f); // 이동 변환
	glm::mat4 R = glm::mat4(1.0f); // 회전 변환
	glm::mat4 S = glm::mat4(1.0f); // 신축 변환
	glm::mat4 SRT = glm::mat4(1.0f); // 변환 행렬

	Object() : _trans_info{}, _rotate_info{}, _scale_info{} {

	}
	~Object() {

	}

	Vec3d& get_translate() { return _trans_info; };
	Vec3d& get_rotate() { return _rotate_info; };
	GLfloat& get_sacle() { return _scale_info; };

	GLvoid push_back(Vertice vertices) {
		this->vertices.push_back({ vertices._pos, vertices._color });
	}

	GLvoid translate(GLfloat x, GLfloat y, GLfloat z) {
		T = glm::translate(T, glm::vec3(x, y, z));
		_trans_info._posX += x;
		_trans_info._posY += y;
		_trans_info._posZ += z;
	}

	GLvoid rotate(GLfloat degree, GLfloat x, GLfloat y, GLfloat z) {
		R = glm::rotate(R, glm::radians(degree), glm::vec3(x, y, z));

		if (x == 1.0f)
			_rotate_info._posX += degree;
		if (y == 1.0f)
			_rotate_info._posY += degree;
		if (z == 1.0f)
			_rotate_info._posZ += degree;
	}

	GLvoid scale(GLfloat scale) {
		S = glm::scale(S, glm::vec3(scale));
		_scale_info += scale;
	}

	GLvoid print_info() {

	}
};

class Line : public Object
{
public:
	GLvoid init(const char* modelTransform) {
		this->modelTransform = modelTransform;
	}

	GLvoid set_vbo() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo_position);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * 6 * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	GLvoid draw(GLuint s_program) {
		glBindVertexArray(vao);
		glUseProgram(s_program);
		unsigned int modelLocation = glGetUniformLocation(s_program, modelTransform);
		SRT = T * R * S;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glDrawArrays(GL_LINES, 0, 6);
	}
};

static GLint hexi_index;
static GLint quad_index;
static GLint div_s = 15;
vector<Color> colors;

class Obj : public Object
{
	objRead objReader;
	GLint obj;
	GLuint ebo;

public:
	GLvoid init(const char* modelTransform, const char* objfile) {
		obj = objReader.loadObj_normalize_center(objfile);
		this->modelTransform = modelTransform;
		setcolor();
	}

	GLvoid setcolor() {
		for (int i{}; i < objReader.nr_outvertex.size(); ++i) {
			colors.push_back({ urd(dre), urd(dre), urd(dre) });
		}
	}

	GLvoid hexi_set_vbo() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao); //--- VAO를 바인드하기

		glGenBuffers(1, &ebo);
		glGenBuffers(1, &vbo_position);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
		glBufferData(GL_ARRAY_BUFFER, objReader.nr_outvertex.size() * sizeof(glm::vec3), &objReader.nr_outvertex[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, objReader.vertexIndices.size() * sizeof(glm::vec3) / 15, &objReader.vertexIndices[hexi_index], GL_STATIC_DRAW);
		hexi_index += 6;

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &vbo_color);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
		glBufferData(GL_ARRAY_BUFFER, objReader.nr_outvertex.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
		GLint cAttribute = glGetAttribLocation(obj1_s_program, "vColor");
		glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(cAttribute);
	}

	GLvoid quad_set_vbo() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao); //--- VAO를 바인드하기

		glGenBuffers(1, &ebo);
		glGenBuffers(1, &vbo_position);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
		glBufferData(GL_ARRAY_BUFFER, objReader.nr_outvertex.size() * sizeof(glm::vec3), &objReader.nr_outvertex[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, objReader.vertexIndices.size() * sizeof(glm::vec3) / div_s, &objReader.vertexIndices[quad_index], GL_STATIC_DRAW);
		quad_index += 3;
		if (quad_index == 12) div_s = 6;

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &vbo_color);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
		glBufferData(GL_ARRAY_BUFFER, objReader.nr_outvertex.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
		GLint cAttribute = glGetAttribLocation(obj1_s_program, "vColor");
		glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(cAttribute);
	}

	GLvoid draw(GLuint s_program) {
		glUseProgram(s_program);
		glBindVertexArray(vao); //--- VAO를 바인드하기
		unsigned int obj1_modelLocation = glGetUniformLocation(s_program, modelTransform);
		glUniformMatrix4fv(obj1_modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glPointSize(10);
		glDrawElements(GL_TRIANGLES, obj, GL_UNSIGNED_INT, 0);
	}
};

//class Camera {
//public:	
//	glm::vec3 cameraPos;
//	glm::vec3 cameraTarget;
//	glm::vec3 cameraDirection;
//	glm::vec3 up;
//	glm::vec3 cameraRight;
//	glm::vec3 cameraUp;
//
//public:
//	Camera() {
//		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
//		cameraDirection = glm::normalize(cameraPos - cameraTarget);
//		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
//		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));//		glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
//	}
//
//	GLvoid set_cameraPos(glm::vec3 value) {
//		cameraPos = glm::vec3(value.x, value.y, value.z);
//	}
//	GLvoid set_cameraDir(glm::vec3 value) {
//		cameraDirection = glm::vec3(value.x, value.y, value.z);
//	}
//	GLvoid set_cameraUp(glm::vec3 value) {
//		cameraUp = glm::vec3(value.x, value.y, value.z);
//	}
//
//	GLvoid draw(GLuint s_program) {
//		glUseProgram(s_program);
//
//		set_cameraPos(glm::vec3(0.0, 0.0, 0.5));
//		set_cameraDir(glm::vec3(0.0, 0.0, 0.0));
//		set_cameraUp(glm::vec3(0.0, 1.0, 0.0));
//		glm::mat4 view = glm::mat4(1.0f);
//
//		view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
//		unsigned int viewLocation = glGetUniformLocation(s_program, "viewTransform");
//		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
//	}
//};

Line line;
Obj* hexi;
Obj* quad;
//Camera cam;

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


GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = 800;
	int h = 600;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void make_vertexShader()
{
	coord_vertexsource = filetobuf("coord_vertex.glsl");
	obj1_vertexsource = filetobuf("obj1_vertex.glsl");
	camera_vertexsource = filetobuf("camera_vertex.glsl");
	//--- 버텍스 세이더 객체 만들기
	coord_vertexshader = glCreateShader(GL_VERTEX_SHADER);
	obj1_vertexshader = glCreateShader(GL_VERTEX_SHADER);
	camera_vertexshader = glCreateShader(GL_VERTEX_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(coord_vertexshader, 1, (const GLchar**)&coord_vertexsource, 0);
	glShaderSource(obj1_vertexshader, 1, (const GLchar**)&obj1_vertexsource, 0);
	glShaderSource(camera_vertexshader, 1, (const GLchar**)&camera_vertexsource, 0);
	//--- 버텍스 세이더 컴파일하기
	glCompileShader(coord_vertexshader);
	glCompileShader(obj1_vertexshader);
	glCompileShader(camera_vertexshader);
	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(coord_vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(coord_vertexshader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
		return;
	}

	glGetShaderiv(obj1_vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(obj1_vertexshader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
		return;
	}

	glGetShaderiv(camera_vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(camera_vertexshader, 512, NULL, errorLog);
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
	coord_s_program = glCreateProgram();
	obj1_s_program = glCreateProgram();
	camera_s_program = glCreateProgram();
	glAttachShader(coord_s_program, coord_vertexshader);
	glAttachShader(coord_s_program, fragmentshader);
	glLinkProgram(coord_s_program);
	glAttachShader(obj1_s_program, obj1_vertexshader);
	glAttachShader(obj1_s_program, fragmentshader);
	glLinkProgram(obj1_s_program);
	glAttachShader(camera_s_program, camera_vertexshader);
	glLinkProgram(camera_s_program);

	glDeleteShader(coord_vertexshader);
	glDeleteShader(obj1_vertexshader);
	glDeleteShader(camera_vertexshader);
	glDeleteShader(fragmentshader);
}

void InitBuffer()
{
	//--- line vbo
	line.set_vbo();

	//--- obj
	for (int i = 0; i < 6; ++i)
		hexi[i].hexi_set_vbo();

	for (int i = 0; i < 5; ++i)
		quad[i].quad_set_vbo();

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

glm::mat4 nR = glm::mat4(1.0f);
glm::mat4 nR1 = glm::mat4(1.0f);
glm::mat4 nR2 = glm::mat4(1.0f);
glm::mat4 nR3 = glm::mat4(1.0f);
glm::mat4 nR4 = glm::mat4(1.0f);

glm::mat4 nT = glm::mat4(1.0f);
glm::mat4 nT2 = glm::mat4(1.0f);
glm::mat4 nT3 = glm::mat4(1.0f);
glm::mat4 nT4 = glm::mat4(1.0f);
glm::mat4 nTR = glm::mat4(1.0f);

glm::mat4 ori = glm::mat4(1.0f);
glm::mat4 ori1 = glm::mat4(1.0f);
glm::mat4 ori2 = glm::mat4(1.0f);
glm::mat4 ori3 = glm::mat4(1.0f);
glm::mat4 ori4 = glm::mat4(1.0f);

glm::vec3 cameraPos = glm::vec3(0.0f, 0.2f, -2.0f);		 //--- 카메라 위치
glm::mat4 projection = glm::mat4(1.0f);

GLvoid drawScene()
{
	if (!depth_draw)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	//--- 변경된 배경색 설정
	glClearColor(0.1, 0.1, 0.1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- camera
	//cam.draw(camera_s_program);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);		 //--- 카메라 위쪽 방향
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	unsigned int viewLocation_obj = glGetUniformLocation(obj1_s_program, "viewTransform"); //--- 뷰잉 변환 설정
	unsigned int viewLocation_coord = glGetUniformLocation(coord_s_program, "viewTransform"); //--- 뷰잉 변환 설정


	if(!projection_type)
		projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 100.0f);
	else
		projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 100.0f);

	unsigned int projLoc_obj = glGetUniformLocation(obj1_s_program, "projection");
	unsigned int projLoc_coord = glGetUniformLocation(coord_s_program, "projection");

	glUniformMatrix4fv(viewLocation_coord, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_coord, 1, GL_FALSE, &projection[0][0]);

	line.draw(coord_s_program);

	glUniformMatrix4fv(viewLocation_obj, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_obj, 1, GL_FALSE, &projection[0][0]);
	if (shape == Shape::hexi) {
		for (int i = 0; i < 6; ++i) {
			if (i != 4) {
				hexi[i].SRT = hexi[i].S * hexi[i].R * hexi[i].T;
				hexi[i].draw(obj1_s_program);
			}
			else {
				hexi[i].SRT = hexi[i].R * hexi[i].T * nR * ori * hexi[i].S;
				hexi[i].draw(obj1_s_program);
			}
		}
	}
	else {
		for (int i = 0; i < 5; ++i) {

			if (i == 0) {
				quad[i].SRT = quad[i].R * nT * nR1 * quad[i].T * ori1 * quad[i].S;
			}
			else if (i == 2) {
				quad[i].SRT = quad[i].R * nT2 * nR2 * quad[i].T * ori2 * quad[i].S;
			}
			else if (i == 3) {
				quad[i].SRT = quad[i].R * nT3 * nR3 * quad[i].T * ori3 * quad[i].S;
			}
			else if (i == 1) {
				quad[i].SRT = quad[i].R * nT4 * nR4 * quad[i].T * ori4 * quad[i].S;
			}
			else {
				quad[i].SRT = quad[i].S * quad[i].R * quad[i].T;
			}
			quad[i].draw(obj1_s_program);
		}
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	case 'h':
		depth_draw = depth_draw ? false : true;
		break;
	case 'y':
		rotation = rotation ? false : true;
		break;
	case 't':
		rotation_up = rotation_up ? false : true;
		break;
	case 'f':
		open_front = open_front ? false : true;
		break;
	case '1':
		hex_open_side = true;
		break;
	case '2':
		hex_open_side = false;
		break;
	case '0':
		shape = Shape::hexi;
		break;
	case '9':
		shape = Shape::quad;
		break;
	case 'o':
		quad_open_side = quad_open_side ? false : true;
		break;
	case 'p':
		projection_type = projection_type ? false : true;
		break;
	case 'z':
		break;
	case 'q':
		exit(1);
		break;
	}
	glutPostRedisplay();
}

void KeyUp(int z, int x, int y)
{

}


void special(int key, int x, int y)
{

}

static int rotate_cnt = 0;
void TimerFunction(int value)
{
	// rotation to hexis
	if (rotation) {
		for (int i = 0; i < 6; ++i) {
			hexi[i].rotate(1, 0, 1, 0);
		}
	}

	// rotation to up
	if (rotation_up) {
		nR = glm::rotate(nR, glm::radians(1.0f), glm::vec3(1, 0, 0));
	}	// open to front
	if (open_front) {
		if (hexi[front].get_translate()._posY < 1.99)
			hexi[front].translate(0, 0.01, 0);
	}
	else {
		if (hexi[front].get_translate()._posY > 0.01)
			hexi[front].translate(0, -0.01, 0);
	}

	// open to front
	if (hex_open_side) {
		if (hexi[::left].get_translate()._posY < 1.99) {
			hexi[::left].translate(0, 0.01, 0);
			hexi[::right].translate(0, 0.01, 0);
		}
	}
	else {
		if (hexi[::left].get_translate()._posY > 0.01) {
			hexi[::left].translate(0, -0.01, 0);
			hexi[::right].translate(0, -0.01, 0);
		}
	}

	// open to quad
	if (quad_open_side) {
		if (rotate_cnt < 233 ) {
			rotate_cnt++;
			nR1 = glm::rotate(nR1, glm::radians(1.0f), glm::vec3(1, 0, 0));
			nR2 = glm::rotate(nR2, glm::radians(-1.0f), glm::vec3(1, 0, 0));
			nR3 = glm::rotate(nR3, glm::radians(1.0f), glm::vec3(0, 0, 1));
			nR4 = glm::rotate(nR4, glm::radians(-1.0f), glm::vec3(0, 0, 1));
		}
	}
	else {
		if (rotate_cnt > 0) {
			rotate_cnt--;
			nR1 = glm::rotate(nR1, glm::radians(-1.0f), glm::vec3(1, 0, 0));
			nR2 = glm::rotate(nR2, glm::radians(1.0f), glm::vec3(1, 0, 0));
			nR3 = glm::rotate(nR3, glm::radians(-1.0f), glm::vec3(0, 0, 1));
			nR4 = glm::rotate(nR4, glm::radians(1.0f), glm::vec3(0, 0, 1));
		}
	}

	glutPostRedisplay();
	glutTimerFunc(1, TimerFunction, 1);
}

void Init()
{
	hexi = new Obj[6];
	quad = new Obj[5];

	line.init("coord_modelTransform");
	line.push_back({ -1.0, 0.0, 0.0, 1.0, 0.0, 0.0 });
	line.push_back({ 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 });
	line.push_back({ 0.0, 1.0, 0.0, 0.0, 1.0, 0.0 });
	line.push_back({ 0.0, -1.0, 0.0, 0.0, 1.0, 0.0 });
	line.push_back({ 0.0, 0.0, -1.0, 0.0, 0.0, 1.0 });
	line.push_back({ 0.0, 0.0, 1.0, 0.0, 0.0, 1.0 });
	line.rotate(-30, 1, 0, 0);
	line.rotate(-30, 0, 1, 0);
	line.scale(10.0);

	for (int i = 0; i < 6; ++i) {
		hexi[i].init("obj1_modelTransform", "cube.obj");
		hexi[i].rotate(-30, 1, 0, 0);
		hexi[i].rotate(-30, 0, 1, 0);
		hexi[i].scale(0.3);
	}

	ori = glm::translate(ori, glm::vec3(0, -0.3, 0));
	ori1 = glm::translate(ori1, glm::vec3(0, -0.7, -0.3));
	ori2 = glm::translate(ori2, glm::vec3(0, -0.7, 0.3));
	ori3 = glm::translate(ori3, glm::vec3(0.3, -0.7, 0));
	ori4 = glm::translate(ori4, glm::vec3(-0.3, -0.7, 0));

	nT = glm::translate(nT, glm::vec3(0, 0, 0.3));
	nT2 = glm::translate(nT2, glm::vec3(0, 0, -0.3));
	nT3 = glm::translate(nT3, glm::vec3(-0.3, 0, 0));
	nT4 = glm::translate(nT4, glm::vec3(0.3, 0, 0));

	hexi[up].translate(0, 0.3, 0);

	for (int i = 0; i < 5; ++i) {
		quad[i].init("obj1_modelTransform", "quad.obj");
		quad[i].translate(0, 1.0, 0);
		quad[i].rotate(-30, 1, 0, 0);
		quad[i].rotate(-30, 0, 1, 0);
		quad[i].scale(0.3);
	}

	cout << "===============================" << endl;
	cout << "h : 은면제거 설정/해제" << endl;
	cout << "y : y축 자전 애니메이션 시작/정지" << endl;
	cout << "t : 윗면 애니메이션 시작/정지" << endl;
	cout << "f : 앞면 개방/폐쇄" << endl;
	cout << "0 : hexi 그리기" << endl;
	cout << "9 : quad 그리기" << endl;
	cout << "1 : 옆면 개방" << endl;
	cout << "2 : 옆면 폐쇄" << endl;
	cout << "o : 사각뿔 각면 개방/폐쇄" << endl;
	cout << "p : 직각 투영/원근 투영" << endl;
	cout << "===============================" << endl;
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("cg_1-16");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	//--- My_init
	Init();
	line.print_info();
	//---
	InitShader();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(special);
	glutSpecialUpFunc(KeyUp);
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop();
}