#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"

using namespace std;

GLuint win_width = 1920;
GLuint win_height = 1080;
GLfloat aspect_ratio{};
GLfloat fovy{};
GLfloat mx;
GLfloat my;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> urd_color{ 0.2, 1.0 };
uniform_real_distribution<float> urd_speed{ 0.2, 1.0 };

GLvoid Reshape(int w, int h);
GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);

static int width_num;
static int height_num;

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

void InitShader()
{
	coord_v_shader.make_shader();
	obj1_v_shader.make_shader();
	temp_f_shader.make_shader();

	obj1_s_program.make_s_program(obj1_v_shader.getShader(), temp_f_shader.getShader());
	coord_s_program.make_s_program(coord_v_shader.getShader(), temp_f_shader.getShader());

	coord_v_shader.delete_shader();
	obj1_v_shader.delete_shader();
	temp_f_shader.delete_shader();
}

class Camera {
	glm::vec4 pivot;
	glm::vec4 target;

	glm::vec3 Pos;
	glm::vec3 Dir;
	glm::vec3 Up;
	glm::mat4 view;

	glm::vec4 final_transform;
	glm::vec4 final_dir_transform;

	GLfloat rotate_degree;
	glm::vec3 translate_amount;

	GLfloat dir_rotate_degree;
	glm::vec3 dir_translate_amount;
public:
	Camera() {

	}

	void translate(glm::vec3 trans) {
		translate_amount += trans;
	}

	void rotate(GLfloat degree) {
		rotate_degree += degree;
	}

	void dir_translate(glm::vec3 trans) {
		dir_translate_amount += trans;
	}

	void dir_rotate(GLfloat degree) {
		dir_rotate_degree += degree;
	}

	void setting(glm::vec4 pos_pivot, glm::vec4 target_pivot, glm::vec3 up_pivot) {
		pivot = pos_pivot;
		target = target_pivot;
		Up = up_pivot;

		final_transform = glm::vec4(pivot);
		final_dir_transform = glm::vec4(target);

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians(rotate_degree), glm::vec3(0.0, 1.0, 0.0));
		trans = glm::translate(trans, glm::vec3(translate_amount));
		final_transform = trans * final_transform;

		glm::mat4 dir_trans = glm::mat4(1.0f);
		dir_trans = glm::translate(dir_trans, glm::vec3(dir_translate_amount));
		dir_trans = glm::rotate(dir_trans, (GLfloat)glm::radians(dir_rotate_degree), glm::vec3(0.0, 1.0, 0.0));
		//final_dir_transform = trans * dir_trans * final_dir_transform;
		final_dir_transform = dir_trans * final_dir_transform;

		Pos = glm::vec3(final_transform);
		Dir = glm::vec3(final_dir_transform);
		view = glm::mat4(1.0f);
		view = glm::lookAt(Pos, Dir, Up);
	}

	glm::mat4 getView() {
		return view;
	}
};

class Object
{
	//======================transform========================//
	glm::vec4 pivot;
	glm::vec4 cur_loc;
	GLfloat height_randValue;
	GLfloat falling_speed;

	glm::vec3 local_translate;
	GLfloat local_rotate;
	glm::vec3 local_scale = glm::vec3(1.0f);
	glm::mat4 local_model = glm::mat4(1.0f);

	glm::vec3 world_translate;
	GLfloat world_rotate;
	glm::vec3 world_scale = glm::vec3(1.0f);
	glm::mat4 world_model = glm::mat4(1.0f);

	glm::mat4 final_transform = glm::mat4(1.0f);
	vector<Color> colors;



	objRead objReader;
	GLint obj;
	unsigned int modelLocation;
	const char* modelTransform;

	//======================VAO, VBO========================//
	GLuint vao;
	GLuint ebo;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_color;

	//====================Mountain info======================//
	bool start_animation = true;

public:
	Object(glm::vec4 Pos_pivot) : pivot{ Pos_pivot } {
		obj = objReader.loadObj_normalize_center("cube.obj");
		this->modelTransform = "obj1_modelTransform";
		height_randValue = urd_speed(dre);
		falling_speed = urd_speed(dre);
		GLfloat red_color = urd_color(dre);
		GLfloat green_color = urd_color(dre);
		GLfloat blue_color = urd_color(dre);
		for (int i{}; i < objReader.nr_outvertex.size(); ++i) {
			colors.push_back({ red_color, green_color, blue_color });
		}
	}
	~Object() {

	}

	GLvoid set_vbo() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

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

	void rotate(GLfloat degree) {
		world_rotate += degree;
	}

	void translate(glm::vec3 trans) {
		world_translate += trans;
	}

	void scale(glm::vec3 scale) {
		world_scale += scale;
	}

	void lrotate(GLfloat degree) {
		local_rotate += degree;
	}

	void ltranslate(glm::vec3 trans) {
		local_translate += trans;
	}

	void lscale(glm::vec3 scale) {
		local_scale += scale;
	}

	void setting() {
		cur_loc = pivot;
		final_transform = glm::mat4(1.0f);

		local_model = glm::mat4(1.0f);
		local_model = glm::translate(local_model, glm::vec3(local_translate));
		local_model = glm::rotate(local_model, glm::radians(local_rotate), glm::vec3(0.0, 1.0, 0.0));
		local_model = glm::translate(local_model, glm::vec3(pivot.x, pivot.y, pivot.z));
		local_model = glm::scale(local_model, glm::vec3(local_scale));

		world_model = glm::mat4(1.0f);
		world_model = glm::translate(world_model, glm::vec3(world_translate));
		world_model = glm::rotate(world_model, glm::radians(world_rotate), glm::vec3(0.0, 1.0, 0.0));
		world_model = glm::scale(world_model, glm::vec3(world_scale));

		cur_loc = world_model * cur_loc;
		final_transform = world_model * local_model;
	}

	void print_cur_loc() {
		//cout << "x: " << cur_loc.x << " " << "y: " << cur_loc.y << " " << "z: " << cur_loc.z << endl;
		cout << "y: " << cur_loc.y - height_randValue << endl;
	}

	glm::vec4 get_cur_loc() const {
		return cur_loc;
	}

	GLfloat get_height_randValue() const {
		return height_randValue;
	}

	GLfloat get_falling_speed() const {
		return falling_speed / 10;
	}

	bool get_start_animation() const {
		return start_animation;
	}

	void set_start_animation() {
		start_animation = false;
	}

	GLvoid draw(ShaderProgram s_program, Camera cam, glm::mat4& projection) {
		glUseProgram(s_program.getSprogram());
		glBindVertexArray(vao);
		unsigned int obj1_modelLocation = glGetUniformLocation(s_program.getSprogram(), modelTransform);
		glUniformMatrix4fv(obj1_modelLocation, 1, GL_FALSE, glm::value_ptr(final_transform));
		glPointSize(10);
		unsigned int viewLocation_obj1 = glGetUniformLocation(s_program.getSprogram(), "viewTransform"); //--- 뷰잉 변환 설정
		unsigned int projLoc_obj1 = glGetUniformLocation(s_program.getSprogram(), "projection");
		glUniformMatrix4fv(viewLocation_obj1, 1, GL_FALSE, &cam.getView()[0][0]);
		glUniformMatrix4fv(projLoc_obj1, 1, GL_FALSE, &projection[0][0]);
		glDrawElements(GL_TRIANGLES, obj, GL_UNSIGNED_INT, 0);
	}
};

list<Object> cube;
Camera cam_1;
Object plane(Object{ glm::vec4(0.0f) });

void InitBuffer()
{
	plane.set_vbo();

	for (auto it = cube.begin(); it != cube.end(); ++it) {
		it->set_vbo();
	}

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

bool isProjection = true;
bool isMove = false;

GLvoid display()
{
	//====================set viewport======================//
	//glViewport(0, 0, win_width, win_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//=====================set cam_1, projection=============//
	cam_1.setting({ 0.0, 4.0, 5.0, 1.0 }, { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0 });
	glm::mat4 projection = glm::mat4(1.0f);
	unsigned int projLoc_coord = glGetUniformLocation(coord_s_program.getSprogram(), "projection");

	//===================set cooordinate====================//
	if (isProjection)
		projection = glm::perspective(glm::radians(60.0f), aspect_ratio, 0.1f, 100.0f);
	else
		projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 100.0f);
	glUseProgram(coord_s_program.getSprogram());
	unsigned int viewLocation_coord = glGetUniformLocation(coord_s_program.getSprogram(), "viewTransform"); //--- 뷰잉 변환 설정
	glUniformMatrix4fv(viewLocation_coord, 1, GL_FALSE, &cam_1.getView()[0][0]);
	glUniformMatrix4fv(projLoc_coord, 1, GL_FALSE, &projection[0][0]);
	glDrawArrays(GL_LINES, 0, 6);

	//======================set object======================//
	for (auto it = cube.begin(); it != cube.end(); ++it) {
		it->setting();
		it->draw(obj1_s_program, cam_1, projection);
	}

	plane.setting();
	plane.draw(obj1_s_program, cam_1, projection);

	//======================set mode========================//
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	if (h == 0)
		h = 1;
	aspect_ratio = GLfloat(w) / h;
	glViewport(0, 0, w, h);
}

static int arrive_cnt = 0;

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'L':
	case 'l':
		for (auto it = cube.begin(); it != cube.end(); ++it)
			it->print_cur_loc();
		cout << arrive_cnt << endl;
		break;
	case 'd':
	case 'D':
		cam_1.translate(glm::vec3(0, -0.1f, 0));
		break;
	case 'o':
	case 'O':
		isProjection = isProjection ? false : true;
		break;
	case 'm':
	case 'M':
		isMove = isMove ? false : true;
		break;
	}
}

void TimerFunction(int value)
{
	if (arrive_cnt < width_num * height_num) {
		for (auto it = cube.begin(); it != cube.end(); ++it) {
			if (it->get_start_animation())
			{
				if (it->get_cur_loc().y - it->get_height_randValue() > 0.0f) {
					it->translate(glm::vec3(0.0f, -it->get_falling_speed(), 0.0f));
				}
			}
			if (it->get_cur_loc().y - it->get_height_randValue() < 0.0f) {
				if (it->get_start_animation()) arrive_cnt += 1;
				it->set_start_animation();
				it->translate(glm::vec3(0.0f, -(it->get_cur_loc().y - it->get_height_randValue()), 0.0f));
			}
		}
	}
	for (auto it = cube.begin(); it != cube.end(); ++it) {
		//it->rotate(1.0f);
	}
	if (isMove) {
		for (auto it = cube.begin(); it != cube.end(); ++it) {
			it->ltranslate (glm::vec3(0.0f, 0.001f, 0.0f));
			it->lscale(glm::vec3(0.0f, 0.001f, 0.0f));
		}
	}
	glutPostRedisplay();
	glutTimerFunc(1, TimerFunction, 1);
}

void Init()
{
	for (int i = 0; i < width_num; ++i)
		for (int j = 0; j < height_num; ++j)
			cube.push_back(Object{ glm::vec4(2 * i - (width_num - 1), 5, 2 * j - (height_num - 1), 1) });

	for (auto it = cube.begin(); it != cube.end(); ++it) {
		it->scale(glm::vec3(-0.9f, it->get_height_randValue(), -0.9f));
	}

	plane.scale(glm::vec3(0.1f * width_num, 0.001f, 0.1f * height_num));

}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	cout << "input width_num : ";
	cin >> width_num;
	cout << "input height_num : ";
	cin >> height_num;
	cout << width_num << " : " << height_num;
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(400, 200);
	glutInitWindowSize(win_width, win_height);
	glutCreateWindow("MOUNTAIN MAGE");
	//--- GLEW 초기화하기
	glClearColor(0.1, 0.1, 0.1, 1.0f);
	glewExperimental = GL_TRUE;
	glewInit();
	Init();
	InitShader();
	InitBuffer();
	glutDisplayFunc(display);
	//glutMouseFunc(Mouse);
	//glutSpecialFunc(special);
	//glutSpecialUpFunc(KeyUp);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(100, TimerFunction, 1);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}