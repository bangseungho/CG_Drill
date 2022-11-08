#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"

using namespace std;

GLuint win_width = 1280;
GLuint win_height = 800;
GLfloat mx;
GLfloat my;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> urd{ 0.2, 1.0 };

GLvoid Reshape(int w, int h);
GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);

static int x_move;
static int y_move;
static int arm_move;
static int z_cam_move;
static int y_cam_move;
static int x_cam_move;
static int y_cam_rotate;
static int cam_rotate;

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

enum class ShaderType {
	vertexshader,
	fragmentshader,
};

class Shader {
	const GLchar* _source;
	ShaderType _type;
	GLuint _shader;
	GLint result;
	GLchar errorLog[512];

public:
	Shader(const GLchar* source, ShaderType type) : _shader{}, result{}, errorLog{} {
		_source = filetobuf(source);
		_type = type;
	}

	GLuint& getShader() {
		return _shader;
	}

	void make_shader() {
		switch (_type) {
		case ShaderType::vertexshader:
			_shader = glCreateShader(GL_VERTEX_SHADER);
			break;
		case ShaderType::fragmentshader:
			_shader = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		}
		glShaderSource(_shader, 1, (const GLchar**)&_source, 0);
		glCompileShader(_shader);
		error_check();
	}

	void error_check() {
		glGetShaderiv(_shader, GL_COMPILE_STATUS, &result);
		if (!result)
		{
			glGetShaderInfoLog(_shader, 512, NULL, errorLog);
			cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
			return;
		}
	}

	void delete_shader() {
		glDeleteShader(_shader);
	}
};

Shader coord_v_shader{ "coord_vertex.glsl", ShaderType::vertexshader };
Shader obj1_v_shader{ "obj1_vertex.glsl", ShaderType::vertexshader };
Shader temp_f_shader{ "fragment.glsl", ShaderType::fragmentshader };
Shader view_v_shader{ "viewport_border_vertex.glsl", ShaderType::vertexshader };

class ShaderProgram {
	GLuint _s_program;

public:
	ShaderProgram() : _s_program{} {

	}

	GLuint& getSprogram() {
		return _s_program;
	}

	void make_s_program(GLint vertex, GLint fragment) {
		_s_program = glCreateProgram();

		glAttachShader(_s_program, vertex);
		glAttachShader(_s_program, fragment);
		glLinkProgram(_s_program);
	}
};

ShaderProgram obj1_s_program;
ShaderProgram coord_s_program;
ShaderProgram viewport_s_program;

void InitShader()
{
	coord_v_shader.make_shader();
	obj1_v_shader.make_shader();
	temp_f_shader.make_shader();
	view_v_shader.make_shader();

	obj1_s_program.make_s_program(obj1_v_shader.getShader(), temp_f_shader.getShader());
	coord_s_program.make_s_program(coord_v_shader.getShader(), temp_f_shader.getShader());
	viewport_s_program.make_s_program(view_v_shader.getShader(), temp_f_shader.getShader());

	coord_v_shader.delete_shader();
	obj1_v_shader.delete_shader();
	temp_f_shader.delete_shader();
	view_v_shader.delete_shader();
}

class Camera {
	glm::vec3 Pos;
	glm::vec3 Dir;
	glm::vec3 Up;
	glm::mat4 view;
	glm::vec4 Pos_trans;
	glm::vec4 Dir_trans;
	glm::mat4 pos_RT;
	glm::mat4 dir_RT;
	glm::vec4 pivot;
	glm::vec4 pivot_dir;

	GLfloat R_degree;
	glm::vec3 T_trans;

	GLfloat dir_R_degree;
	glm::vec3 dir_T_trans;

public:
	Camera() {

	}

	void rotate(GLfloat degree) {
		R_degree += degree;
	}

	void reset() {
		Pos_trans = glm::vec4(pivot.x, pivot.y, pivot.z, pivot.w);
		Dir_trans = glm::vec4(pivot_dir.x, pivot_dir.y, pivot_dir.z, pivot_dir.w);

		Pos = glm::vec3(0.0, 1.0, 2.0);
		Dir = glm::vec3(0.0, -3.0, -5.0);

		view = glm::mat4(1.0f);

		R_degree = 0;
		dir_R_degree = 0;
		T_trans = glm::vec3(0.0f);
		dir_T_trans = glm::vec3(0.0f);

		pos_RT = glm::mat4(1.0f);
		dir_RT = glm::mat4(1.0f); 
	}

	void dir_rotate(GLfloat degree) {
		dir_R_degree += degree;
	}

	void dir_translate(glm::vec3 trans) {
		dir_T_trans.x += trans.x;
		dir_T_trans.y += trans.y;
		dir_T_trans.z += trans.z;
	}

	void translate(glm::vec3 trans) {
		T_trans.x += trans.x;
		T_trans.y += trans.y;
		T_trans.z += trans.z;
	}

	void setting(glm::vec4 Pos_pivot, glm::vec4 Dir_pivot, glm::vec3 Up_pivot) {
		pivot = Pos_pivot;
		pivot_dir = Dir_pivot;

		Pos_trans = glm::vec4(pivot.x, pivot.y, pivot.z, pivot.w);
		Dir_trans = glm::vec4(pivot_dir.x, pivot_dir.y, pivot_dir.z, pivot_dir.w);

		pos_RT = glm::mat4(1.0f);
		pos_RT = glm::rotate(pos_RT, glm::radians(R_degree), glm::vec3(0.0, 1.0, 0.0));
		pos_RT = glm::translate(pos_RT, glm::vec3(T_trans.x, T_trans.y, T_trans.z));
		Pos_trans = pos_RT * Pos_trans;

		dir_RT = glm::mat4(1.0f);
		dir_RT = glm::translate(dir_RT, glm::vec3(dir_T_trans.x, dir_T_trans.y, dir_T_trans.z));
		dir_RT = glm::rotate(dir_RT, (GLfloat)glm::radians(dir_R_degree), glm::vec3(0.0, 1.0, 0.0));
		Dir_trans = pos_RT * dir_RT * Dir_trans;

		Pos = glm::vec3(Pos_trans.x, Pos_trans.y, Pos_trans.z);
		Dir = glm::vec3(Dir_trans.x, Dir_trans.y, Dir_trans.z);
		Up = Up_pivot;
		view = glm::mat4(1.0f);
		view = glm::lookAt(Pos, Dir, Up);
	}

	glm::mat4 getView() {
		return view;
	}
};

Camera cam_1;
Camera cam_2;

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
	Vec3d _scale_info; // 현재 신축 변환 정보
	const char* modelTransform;
	unsigned int modelLocation;

public:
	glm::mat4 T = glm::mat4(1.0f); // 이동 변환
	glm::mat4 R = glm::mat4(1.0f); // 회전 변환
	glm::mat4 S = glm::mat4(1.0f); // 신축 변환
	glm::mat4 SRT = glm::mat4(1.0f); // 변환 행렬
	glm::vec4 center = glm::vec4(1.0f);
	glm::mat4 nT = glm::mat4(1.0f);

	Object() : _trans_info{}, _rotate_info{}, _scale_info{} {

	}
	~Object() {

	}

	Vec3d& get_translate() { return _trans_info; };
	Vec3d& get_rotate() { return _rotate_info; };
	Vec3d& get_sacle() { return _scale_info; };

	GLvoid push_back(Vertice vertices) {
		this->vertices.push_back({ vertices._pos, vertices._color });
	}

	GLvoid reset() {
		SRT = glm::mat4(1.0f);
		S = glm::mat4(1.0f);
		T = glm::mat4(1.0f);
		R = glm::mat4(1.0f);
		nT = glm::mat4(1.0f);
	}

	GLvoid transformation() {
		SRT = T * R * nT * S;
	}

	GLvoid translate(GLfloat x, GLfloat y, GLfloat z) {
		T = glm::translate(T, glm::vec3(x, y, z));
		_trans_info._posX += x;
		_trans_info._posY += y;
		_trans_info._posZ += z;

		transformation();
	}

	GLvoid rotate(GLfloat degree, GLfloat x, GLfloat y, GLfloat z) {
		R = glm::rotate(R, glm::radians(degree), glm::vec3(x, y, z));

		if (x == 1.0f)
			_rotate_info._posX += degree;
		if (y == 1.0f)
			_rotate_info._posY += degree;
		if (z == 1.0f)
			_rotate_info._posZ += degree;

		transformation();
	}

	GLvoid scale(GLfloat x, GLfloat y, GLfloat z) {
		S = glm::scale(S, glm::vec3(x, y, z));
		_scale_info._posX += x;
		_scale_info._posY += y;
		_scale_info._posZ += z;

		transformation();
	}

	GLvoid print_info() {
		cout << "Translate : (" << _trans_info._posX << ", " << _trans_info._posY << ", " << _trans_info._posZ << ")" << endl;
		cout << "Rotate : (" << _rotate_info._posX << ", " << _rotate_info._posY << ", " << _rotate_info._posZ << ")" << endl;
		cout << "Rotate : (" << _scale_info._posX << ", " << _scale_info._posY << ", " << _scale_info._posZ << ")" << endl;
	}
};

class Obj : public Object
{
	objRead objReader;
	GLint obj;
	GLuint ebo;
	vector<Color> colors;

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

	GLvoid set_vbo() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao); //--- VAO를 바인드하기

		glGenBuffers(1, &ebo);
		glGenBuffers(1, &vbo_position);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
		glBufferData(GL_ARRAY_BUFFER, objReader.nr_outvertex.size() * sizeof(glm::vec3), &objReader.nr_outvertex[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, objReader.vertexIndices.size() * sizeof(glm::vec3), &objReader.vertexIndices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &vbo_color);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
		glBufferData(GL_ARRAY_BUFFER, objReader.nr_outvertex.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
		GLint cAttribute = glGetAttribLocation(obj1_s_program.getSprogram(), "vColor");
		glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(cAttribute);
	}

	GLvoid draw(ShaderProgram s_program, Camera cam, glm::mat4& projection) {
		glUseProgram(s_program.getSprogram());
		glBindVertexArray(vao); //--- VAO를 바인드하기
		unsigned int obj1_modelLocation = glGetUniformLocation(s_program.getSprogram(), modelTransform);
		glUniformMatrix4fv(obj1_modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glPointSize(10);
		unsigned int viewLocation_obj1 = glGetUniformLocation(s_program.getSprogram(), "viewTransform"); //--- 뷰잉 변환 설정
		unsigned int projLoc_obj1 = glGetUniformLocation(s_program.getSprogram(), "projection");
		glUniformMatrix4fv(viewLocation_obj1, 1, GL_FALSE, &cam.getView()[0][0]);
		glUniformMatrix4fv(projLoc_obj1, 1, GL_FALSE, &projection[0][0]);
		glDrawElements(GL_TRIANGLES, obj, GL_UNSIGNED_INT, 0);
	}
};

Obj under_body;
Obj up_body;
Obj plane;
Obj arm[2];
vector<Obj*> objs;

void InitBuffer()
{
	under_body.set_vbo();
	up_body.set_vbo();
	plane.set_vbo();

	for (int i = 0; i < 2; ++i) {
		arm[i].set_vbo();
	}

	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(0);
}

GLvoid display()
{
	//=======================================================뷰포트 1=============================================================
	unsigned int viewLocation_coord = glGetUniformLocation(coord_s_program.getSprogram(), "viewTransform"); //--- 뷰잉 변환 설정
	unsigned int projLoc_coord = glGetUniformLocation(coord_s_program.getSprogram(), "projection");

	glViewport(100, 100, 600, 600);
	//--- 변경된 배경색 설정
	glm::mat4 projection = glm::mat4(1.0f);
	glClearColor(0.1, 0.1, 0.1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- border
	glUseProgram(viewport_s_program.getSprogram());
	glPointSize(10);
	glDrawArrays(GL_LINES, 0, 8);
	
	//--- 카메라
	cam_1.setting({ 0.0, 1.0, 2.0, 1.0 }, { 0.0, -3.0, -5.0, 1.0 }, {0.0, 1.0, 0.0});

	//--- 좌표계
	projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 100.0f);
	glUseProgram(coord_s_program.getSprogram());
	glUniformMatrix4fv(viewLocation_coord, 1, GL_FALSE, &cam_1.getView()[0][0]);
	glUniformMatrix4fv(projLoc_coord, 1, GL_FALSE, &projection[0][0]);
	glDrawArrays(GL_LINES, 0, 6);

	//--- 아래 몸통
	under_body.draw(obj1_s_program, cam_1, projection);

	//--- 위 몸통
	up_body.draw(obj1_s_program, cam_1, projection);

	//--- 바닥
	plane.draw(obj1_s_program, cam_1, projection);

	//--- 팔
	for (int i = 0; i < 2; ++i) {
		arm[i].draw(obj1_s_program, cam_1, projection);
	}

	//=======================================================뷰포트 2=============================================================
	glViewport(830, 400, 350, 350);

	glUseProgram(viewport_s_program.getSprogram());
	glPointSize(10);
	glDrawArrays(GL_LINES, 0, 8);

	cam_1.setting({ 0.0, 5.0, 0.0, 1.0 }, { 0.0, 0.0, 0.0, 1.0 }, {0.0, 0.0, -1.0});

	//--- 카메라
	glUseProgram(coord_s_program.getSprogram());
	glUniformMatrix4fv(viewLocation_coord, 1, GL_FALSE, &cam_1.getView()[0][0]);
	glDrawArrays(GL_LINES, 0, 6);

	projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 100.0f);

	//--- 아래 몸통
	under_body.draw(obj1_s_program, cam_1, projection);

	//--- 위 몸통
	up_body.draw(obj1_s_program, cam_1, projection);

	////--- 바닥
	plane.draw(obj1_s_program, cam_1, projection);

	//--- 팔
	for (int i = 0; i < 2; ++i) {
		arm[i].draw(obj1_s_program, cam_1, projection);
	}


	//=======================================================뷰포트 3=============================================================
	glViewport(830, 50, 350, 350);

	glUseProgram(viewport_s_program.getSprogram());
	glPointSize(10);
	glDrawArrays(GL_LINES, 0, 8);

	cam_1.setting({ 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0 });

	//--- 카메라
	glUseProgram(coord_s_program.getSprogram());
	glUniformMatrix4fv(viewLocation_coord, 1, GL_FALSE, &cam_1.getView()[0][0]);
	glDrawArrays(GL_LINES, 0, 6);

	//--- 아래 몸통
	under_body.draw(obj1_s_program, cam_1, projection);

	//--- 위 몸통
	up_body.draw(obj1_s_program, cam_1, projection);

	//--- 바닥
	plane.draw(obj1_s_program, cam_1, projection);

	//--- 팔
	for (int i = 0; i < 2; ++i) {
		arm[i].draw(obj1_s_program, cam_1, projection);
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
	case 'b':
		x_move = ((x_move + 1) % 3);
		break;
	case 'm':
		y_move = ((y_move + 1) % 3);
		break;
	case 't':
		arm_move = ((arm_move + 1) % 3);
		break;
	case 'z':
		z_cam_move = ((z_cam_move + 1) % 3);
		break;
	case 'y':
		y_cam_move = ((y_cam_move + 1) % 3);
		break;
	case 'x':
		x_cam_move = ((x_cam_move + 1) % 3);
		break;
	case 'r':
		y_cam_rotate = ((y_cam_rotate + 1) % 3);
		break;
	case 'a':
		cam_rotate = ((cam_rotate + 1) % 3);
		break;
	case 's':
		x_move = 0;
		y_move = 0;
		arm_move = 0;
		z_cam_move = 0;
		y_cam_move = 0;
		x_cam_move = 0;
		cam_rotate = 0;
		break;
	case 'c':
		x_move = 0;
		y_move = 0;
		arm_move = 0;
		z_cam_move = 0;
		y_cam_move = 0;
		x_cam_move = 0;
		cam_rotate = 0;

		cam_1.reset();

		for (auto it = objs.begin(); it != objs.end();) {
			(*it)->reset();
			++it;
		}

		under_body.scale(0.2, 0.05, 0.2);
		under_body.translate(0.0, 0.05, 0.0);

		up_body.scale(0.12, 0.04, 0.12);
		up_body.translate(0.0, 0.1, 0.0);

		for (int i = 0; i < 2; ++i) {
			arm[i].scale(0.035, 0.1, 0.035);
		}
		arm[0].translate(-0.05, 0.25, 0.0);
		arm[1].translate(0.05, 0.25, 0.0);

		arm[0].nT = glm::translate(arm[0].nT, glm::vec3(0.05f, 0.15f, 0.0f));
		arm[1].nT = glm::translate(arm[1].nT, glm::vec3(-0.05f, 0.15f, 0.0f));

		glutPostRedisplay();
		break;
	case 'q':
		exit(1);
		break;
	}
}

void KeyUp(int z, int x, int y)
{

}


void special(int key, int x, int y)
{

}

void TimerFunction(int value)
{
	//cam_1.rotate(1.0f);


	if (x_move == 1) {
		for (auto it = objs.begin(); it != objs.end();) {
			(*it)->translate(0.01f, 0, 0);
			it++;
		}
	}
	else if (x_move == 2) {
		for (auto it = objs.begin(); it != objs.end();) {
			(*it)->translate(-0.01f, 0, 0);
			it++;
		}
	}


	if (y_move == 1) {
		for (auto it = objs.begin() + 1; it != objs.end();) {
			(*it)->rotate(1.0f, 0, 1, 0);
			it++;
		}
	}
	else if (y_move == 2) {
		for (auto it = objs.begin() + 1; it != objs.end();) {
			(*it)->rotate(-1.0f, 0, 1, 0);
			it++;
		}
	}

	int cnt = 0;

	if (arm[0].get_rotate()._posX > 90 || arm[0].get_rotate()._posX < -90) {
		if (arm_move == 1) arm_move = 2;
		else if (arm_move == 2) arm_move = 1;
	}

	if (arm_move == 1) {
		for (auto it = objs.begin() + 2; it != objs.end();) {
			(*it)->rotate(0.5f * (pow(-1, cnt)), 1, 0, 0);
			cnt++;
			it++;
		}
	}
	else if (arm_move == 2) {
		for (auto it = objs.begin() + 2; it != objs.end();) {
			(*it)->rotate(-0.5f * (pow(-1, cnt)), 1, 0, 0);
			cnt++;
			it++;
		}
	}

	if (z_cam_move == 1) {
		cam_1.translate({ 0, 0, 0.01f });
	}
	else if (z_cam_move == 2) {
		cam_1.translate({ 0, 0, -0.01f });
	}

	if (y_cam_move == 1) {
		cam_1.translate({ 0, 0.01f, 0 });
	}
	else if (y_cam_move == 2) {
		cam_1.translate({ 0, -0.01f, 0 });
	}

	if (x_cam_move == 1) {
		cam_1.translate({ 0.01f, 0, 0 });
	}
	else if (x_cam_move == 2) {
		cam_1.translate({ -0.01f, 0, 0 });
	}

	if (y_cam_rotate == 1) {
		cam_1.rotate(1.0f);
	}
	else if (y_cam_rotate == 2) {
		cam_1.rotate(-1.0f);
	}

	if (cam_rotate == 1) {
		cam_1.dir_rotate(0.5f);
	}
	else if (cam_rotate == 2) {
		cam_1.dir_rotate(-0.5f);
	}

	arm[0].SRT = up_body.T * arm[0].R * arm[0].nT * arm[0].S;
	arm[1].SRT = up_body.T * arm[1].R * arm[1].nT * arm[1].S;

	glutPostRedisplay();
	glutTimerFunc(1, TimerFunction, 1);
}


void Init()
{
	under_body.init("obj1_modelTransform", "body.obj");
	under_body.scale(0.2, 0.05, 0.2);
	under_body.translate(0.0, 0.05, 0.0);

	up_body.init("obj1_modelTransform", "body.obj");
	up_body.scale(0.12, 0.04, 0.12);
	up_body.translate(0.0, 0.1, 0.0);

	plane.init("obj1_modelTransform", "plane.obj");
	plane.scale(3.0, -0.0001, 3.0);

	for (int i = 0; i < 2; ++i) {
		arm[i].init("obj1_modelTransform", "arm.obj");
		arm[i].scale(0.035, 0.15, 0.035);
	}
	arm[0].translate(-0.05, 0.25, 0.0);
	arm[1].translate(0.05, 0.25, 0.0);

	arm[0].nT = glm::translate(arm[0].nT, glm::vec3(0.05f, 0.15f, 0.0f));
	arm[1].nT = glm::translate(arm[1].nT, glm::vec3(-0.05f, 0.15f, 0.0f));

	objs.push_back(&under_body);
	objs.push_back(&up_body);
	objs.push_back(&arm[0]);
	objs.push_back(&arm[1]);
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(win_width, win_height);
	glutCreateWindow("cg_1-18");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	//--- My_init
	Init();
	//---
	InitShader();
	InitBuffer();
	glutDisplayFunc(display);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(special);
	glutSpecialUpFunc(KeyUp);
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop();
}