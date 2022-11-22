#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"
#include "camera.h"

using namespace glm;
using namespace std;

GLuint win_width = 1920;
GLuint win_height = 1080;
static int width_num;
static int height_num;
GLfloat mx;
GLfloat my;

GLuint global_vbo, global_vao, global_ebo;
objRead global_objReader;
GLint global_obj;

GLfloat aspect_ratio{};

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> urd_color{ 0.2, 1.0 };
uniform_real_distribution<float> urd_speed{ 0.2, 1.0 };

GLvoid Reshape(int w, int h);
GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);

Camera* camera;
Camera fps_camera(Person_View::FPS, vec3(0.0f, 0.0f, 0.0f));
Camera quarter_camera(Person_View::QUARTER, 0.0f, 8.0f, 8.0f, 0.0f, 1.0f, 0.0f, -90, -20);
Camera top_camera(Person_View::QUARTER, 0.0f, 11.0f, 0.0f, 0.0f, 1.0f, 0.0f, 90, -100);

bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = win_width / 2.0;
float lastY = win_height / 2.0;

enum class Keyboard_Event {
	KEYUP,

	FOWARD,
	BACKWARD,
	LEFTSIDE,
	RIGHTSIDE,

	UP,
	DOWN,
	LEFT,
	RIGHT
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

class Cameratemp {
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
	Cameratemp(glm::vec4 pos_pivot, glm::vec4 target_pivot) : pivot{ pos_pivot }, target{ target_pivot }{

	}

	void set_pivot(glm::vec4 trans) {
		this->pivot = trans;
	}

	void set_target(glm::vec4 target) {
		this->target = target;
	}

	void reset_pivot() {
		pivot = vec4(0);
	}

	void translate(glm::vec3 trans) {
		translate_amount += trans;
	}

	void set_translate(glm::vec3 trans) {
		translate_amount = trans;
	}

	void rotate(GLfloat degree) {
		rotate_degree += degree;
	}

	void set_dir_translate(glm::vec3 trans) {
		target = (vec4(0.0, 0.0, -3.0, 1.0));
		dir_translate_amount = trans;
	}

	void dir_translate(glm::vec3 trans) {
		dir_translate_amount += trans;
	}

	void dir_rotate(GLfloat degree) {
		dir_rotate_degree += degree;
	}

	void setting(glm::vec3 up_pivot) {
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
		dir_trans = glm::rotate(dir_trans, (GLfloat)glm::radians(dir_rotate_degree), glm::vec3(0.0, 1.0, 0.0));
		final_dir_transform = trans * dir_trans * final_dir_transform;

		Pos = glm::vec3(final_transform);
		Dir = glm::vec3(final_dir_transform);
		view = glm::mat4(1.0f);
		view = glm::lookAt(Pos, Dir, Up);
	}

	glm::mat4 getView() {
		return view;
	}
};

struct Color
{
	GLfloat	_r;
	GLfloat	_g;
	GLfloat	_b;
};

enum class Type {
	wall,
	load,
	past_load,
	start_point,
	end_point,
	crush_wall,
};

class Object
{
	vec3 world_position;
	vec3 local_position;

	vec3 world_scale;
	vec3 local_scale;

	vec3 world_rotation;
	vec3 local_rotation;

	vec4 world_pivot;
	vec4 local_pivot;

	mat4 model;

	vector<Color> colors;

	glm::mat4 final_transform;

	glm::vec4 cur_loc;

	objRead objReader;
	GLint obj;
	unsigned int modelLocation;
	const char* modelTransform;

	GLfloat height_randValue;
	GLfloat falling_speed;
	GLfloat grow_speed;

	bool start_animation = true;
	int grow = 1;

	Type type = Type::wall;
	bool check_load = false;

	//======================VAO, VBO========================//
	GLuint vao;
	GLuint ebo;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_color;


public:
	Object* next_cube;
	Object(vec4 pivot) {
		world_pivot = local_pivot = vec4(0);
		world_position = local_position = vec3(0);
		world_rotation = local_rotation = vec3(0);
		world_scale = local_scale = vec3(1);
		local_pivot = pivot;
		final_transform = mat4(1);
		next_cube = nullptr;

		this->modelTransform = "obj1_modelTransform";
		height_randValue = urd_speed(dre);
		falling_speed = urd_speed(dre);
		grow_speed = urd_speed(dre);
		GLfloat red_color = urd_color(dre);
		GLfloat green_color = urd_color(dre);
		GLfloat blue_color = urd_color(dre);
		for (int i{}; i < 9; ++i) {
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
		glBufferData(GL_ARRAY_BUFFER, global_objReader.nr_outvertex.size() * sizeof(glm::vec3), &global_objReader.nr_outvertex[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, global_objReader.vertexIndices.size() * sizeof(glm::vec3), &global_objReader.vertexIndices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &vbo_color);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
		glBufferData(GL_ARRAY_BUFFER, global_objReader.nr_outvertex.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
		GLint cAttribute = glGetAttribLocation(obj1_s_program.getSprogram(), "vColor");
		glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(cAttribute);
	}
	
	void rotate(vec3 degree) {
		world_rotation.x += degree.x;
		world_rotation.y += degree.y;
		world_rotation.z += degree.z;
	}

	void translate(glm::vec3 trans) {
		world_position += trans;
	}

	void set_translate(glm::vec3 trans) {
		world_position = trans;
	}

	void scale(glm::vec3 scale) {
		world_scale = scale;
	}

	void lrotate(vec3 degree) {
		local_rotation.x += degree.x;
		local_rotation.y += degree.y;
		local_rotation.z += degree.z;
	}

	void ltranslate(glm::vec3 trans) {
		local_position += trans;
	}

	void lscale(glm::vec3 scale) {
		local_scale = scale;
	}

	void set_type(Type type) {
		this->type = type;
	}

	Type get_type() {
		return type;
	}

	void setting() {
		mat4 local_model = mat4(1.0);
		mat4 world_model = mat4(1.0);

		local_model = glm::translate(local_model, vec3(local_position));
		local_model = glm::rotate(local_model, radians(local_rotation.x), glm::vec3(1.0, 0.0, 0.0));
		local_model = glm::rotate(local_model, radians(local_rotation.y), glm::vec3(0.0, 1.0, 0.0));
		local_model = glm::rotate(local_model, radians(local_rotation.z), glm::vec3(0.0, 0.0, 1.0));
		local_model = glm::translate(local_model, vec3(local_pivot));
		local_model = glm::scale(local_model, vec3(local_scale));

		world_model = glm::translate(world_model, vec3(world_position));
		world_model = glm::rotate(world_model, radians(world_rotation.x), glm::vec3(1.0, 0.0, 0.0));
		world_model = glm::rotate(world_model, radians(world_rotation.y), glm::vec3(0.0, 1.0, 0.0));
		world_model = glm::rotate(world_model, radians(world_rotation.z), glm::vec3(0.0, 0.0, 1.0));
		world_model = glm::translate(world_model, vec3(world_pivot));
		world_model = glm::scale(world_model, vec3(world_scale));

		cur_loc = local_pivot;
		cur_loc = world_model * cur_loc;
		final_transform = world_model * local_model;
	}

	glm::vec4 get_cur_loc() const {
		return cur_loc;
	}

	glm::vec3 get_cur_positon() const {
		return world_position;
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

	GLfloat get_grow_speed() const {
		return grow_speed;
	}

	void add_sub_grow_speed(GLfloat value) {
		grow_speed += value;
	}

	void print_cur_loc() const {
		//cout << "x: " << cur_loc.x << " " << "y: " << cur_loc.y << " " << "z: " << cur_loc.z << endl;
		//cout << "y: " << cur_loc.y - height_randValue << endl;
		cout << "y: " << local_scale.y << endl;
	}

	void set_start_animation() {
		start_animation = false;
	}

	void reset_scale() {
	}

	void grow_cube(vec3 grow_value) {
		if (local_scale.y > 1.5f or local_scale.y < 0)
			grow *= -1;
		local_scale.x += grow * grow_value.x;
		local_scale.y += grow * grow_value.y;
		local_scale.z += grow * grow_value.z;
		local_position.x += grow * grow_value.x;
		local_position.y += grow * grow_value.y;
		local_position.z += grow * grow_value.z;
	}

	void low_cube(vec3 grow_value) {
		if (local_scale.y > 0.2) {
			local_scale.x += grow_value.x;
			local_scale.y += grow_value.y;
			local_scale.z += grow_value.z;
			local_position.x += grow_value.x;
			local_position.y += grow_value.y;
			local_position.z += grow_value.z;
		}
	}

	GLvoid draw(ShaderProgram s_program, Camera cam, glm::mat4& projection) {
		glUseProgram(s_program.getSprogram());
		glBindVertexArray(vao);
		unsigned int obj1_modelLocation = glGetUniformLocation(s_program.getSprogram(), modelTransform);
		glUniformMatrix4fv(obj1_modelLocation, 1, GL_FALSE, glm::value_ptr(final_transform));
		glPointSize(10);
		unsigned int viewLocation_obj1 = glGetUniformLocation(s_program.getSprogram(), "viewTransform"); //--- 뷰잉 변환 설정
		unsigned int projLoc_obj1 = glGetUniformLocation(s_program.getSprogram(), "projection");
		glUniformMatrix4fv(viewLocation_obj1, 1, GL_FALSE, &cam.GetViewMatrix()[0][0]);
		glUniformMatrix4fv(projLoc_obj1, 1, GL_FALSE, &projection[0][0]);
		glDrawElements(GL_TRIANGLES, global_obj, GL_UNSIGNED_INT, 0);
	}
};

vector<vector<Object>> cube;
Object player(Object{ glm::vec4(0.0f) });
Object plane(Object{ glm::vec4(0.0f) });


void InitBuffer()
{
	for (int i = 0; i < width_num; ++i) {
		for (int j = 0; j < height_num; ++j) {
			cube[i][j].set_vbo();
		}
	}
	player.set_vbo();
	plane.set_vbo();

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

bool isProjection = true;
bool isMove = false;
bool isCamMove = false;
bool islowHeight = false;
bool create_player = false;
bool cam_fps = false;
bool made_mage = false;

GLvoid display()
{
	float currentFrame = static_cast<float>(glutGet(GLUT_ELAPSED_TIME));
	deltaTime = (currentFrame - lastFrame) / 1000;
	lastFrame = currentFrame;
	glViewport(0, 0, win_width, win_height);
	//====================set viewport======================//
	//glViewport(0, 0, win_width, win_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//=====================set cam_1, projection=============//
	glm::mat4 projection = glm::mat4(1.0f);
	unsigned int projLoc_coord = glGetUniformLocation(coord_s_program.getSprogram(), "projection");

	glm::mat4 view = camera->GetViewMatrix();

	//===================set cooordinate====================//
	if (isProjection)
		projection = glm::perspective(glm::radians(60.0f), aspect_ratio, 0.1f, 100.0f);
	else
		projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 100.0f);
	glUseProgram(coord_s_program.getSprogram());
	unsigned int viewLocation_coord = glGetUniformLocation(coord_s_program.getSprogram(), "viewTransform"); //--- 뷰잉 변환 설정
	glUniformMatrix4fv(viewLocation_coord, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_coord, 1, GL_FALSE, &projection[0][0]);
	glDrawArrays(GL_LINES, 0, 6);

	//======================set object======================//
	for (auto& n : cube) {
		for (auto& val : n) {
			if (val.get_type() == Type::wall or val.get_type() == Type::crush_wall) {
				val.setting();
				val.draw(obj1_s_program, *camera, projection);
			}
		}
	}

	if (create_player) {
		player.setting();
		player.draw(obj1_s_program, *camera, projection);
	}

	plane.setting();
	plane.draw(obj1_s_program, *camera, projection);


	//=======================================================맵 뷰포트=============================================================
	GLfloat map_size = 200;
	glViewport(win_width - win_width * 0.3, win_height - win_height * 0.3, win_width * 0.3, win_height * 0.3);
	//projection = glm::perspective(glm::radians(60.0f), aspect_ratio, 0.1f, 100.0f);
	projection = glm::ortho(-5.0f * aspect_ratio, 5.0f * aspect_ratio, -5.0f, 5.0f, -2.0f, 100.0f);

	//======================set object======================//
	for (auto& n : cube) {
		for (auto& val : n) {
			if (val.get_type() == Type::wall or val.get_type() == Type::crush_wall) {
				val.setting();
				val.draw(obj1_s_program, top_camera, projection);
			}
		}
	}

	if (create_player) {
		player.setting();
		player.draw(obj1_s_program, top_camera, projection);
	}

	plane.setting();
	plane.draw(obj1_s_program, top_camera, projection);



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

inline void load_all_cube() {
	for (auto& n : cube) {
		for (auto& val : n) {
			val.set_type(Type::crush_wall);
		}
	}
}

inline void set_border_cube() {
	for (int i = 0; i < width_num; ++i) {
		cube[i][0].set_type(Type::wall);
		cube[i][height_num - 1].set_type(Type::wall);
	}
	for (int i = 0; i < height_num; ++i) {
		cube[width_num - 1][i].set_type(Type::wall);
		cube[0][i].set_type(Type::wall);
	}

}

static int start_x = -1;
static int start_y = -1;
static int fstart_x = -1;
static int fstart_y = -1;

inline void set_start_end() {
	uniform_int_distribution<int> uid{ 1, width_num - 2 };
	cube[uid(dre)][height_num - 1].set_type(Type::start_point);
	cube[uid(dre)][0].set_type(Type::end_point);

	for (int y = 0; y < height_num; ++y) {
		for (int x = 0; x < width_num; ++x) {
			if (cube[x][y].get_type() == Type::start_point) {
				start_x = x;
				start_y = y;
				fstart_x = x;
				fstart_y = y;
				break;
			}
		}
	}

	for (int y = 0; y < height_num; ++y) {
		for (int x = 0; x < width_num; ++x) {
			if (cube[x][y].get_type() == Type::end_point) {
				cube[x][y + 1].set_type(Type::load);
				break;
			}
		}
	}
}

enum Dir {
	left,
	right,
	top,
	bottom,
};

void set_mage() {

	int reset_cnt = 0;


	while (true) {
		uniform_int_distribution<int> uid_dir{ 0, 4 };
		int randvalue = uid_dir(dre);


		switch (randvalue)
		{
		case Dir::left:
			if (start_x == 0) break;

			cube[start_x][start_y].next_cube = &cube[start_x - 1][start_y];
			if (cube[start_x][start_y].next_cube->get_type() == Type::end_point) return;

			if (cube[start_x][start_y].next_cube->get_type() == Type::wall) {
				reset_cnt++;
				break;
			}

			reset_cnt = 0;
			cube[start_x][start_y].set_type(Type::load);
			start_x -= 1;
			break;
		case Dir::right:

			if (start_x == width_num - 1) break;

			cube[start_x][start_y].next_cube = &cube[start_x + 1][start_y];
			if (cube[start_x][start_y].next_cube->get_type() == Type::end_point) return;

			if (cube[start_x][start_y].next_cube->get_type() == Type::wall) {
				reset_cnt++;
				break;
			}

			reset_cnt = 0;
			cube[start_x][start_y].set_type(Type::load);
			start_x += 1;
			break;
		case Dir::top:

			if (start_y == 0) break;

			cube[start_x][start_y].next_cube = &cube[start_x][start_y - 1];
			if (cube[start_x][start_y].next_cube->get_type() == Type::end_point) return;

			if (cube[start_x][start_y].next_cube->get_type() == Type::wall) {
				reset_cnt++;
				break;
			};
			reset_cnt = 0;

			cube[start_x][start_y].set_type(Type::load);
			start_y -= 1;
			break;
		case Dir::bottom:

			if (start_x == height_num - 1) break;

			cube[start_x][start_y].next_cube = &cube[start_x][start_y + 1];
			if (cube[start_x][start_y].next_cube->get_type() == Type::end_point) return;

			if (cube[start_x][start_y].next_cube->get_type() == Type::wall) {
				reset_cnt++;
				break;
			}
			reset_cnt = 0;
			cube[start_x][start_y].set_type(Type::load);
			start_y += 1;
			break;
		}

		if (reset_cnt > 1) {
			load_all_cube();
			set_border_cube();
			set_start_end();
			//cout << "Roading the Mage..." << endl;
		}

		if (cube[start_x + 1][start_y].get_type() == Type::wall &&
			cube[start_x - 1][start_y].get_type() == Type::wall &&
			cube[start_x][start_y + 1].get_type() == Type::wall &&
			cube[start_x][start_y - 1].get_type() == Type::wall)
		{
			load_all_cube();
			set_border_cube();
			set_start_end();
		}


	}

	std::cout << "x : " << start_x << " " << "y: " << start_y << endl;

}

void make_mage() {

	load_all_cube();

	set_border_cube();

	set_start_end();

	set_mage();

}

static Keyboard_Event key_down;
static int cnt = 0;

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'p':
	case 'P':
		isProjection = true;
		break;
	case 'o':
	case 'O':
		isProjection = false;
		break;
	case 'm':
	case 'M':
		isMove = isMove ? false : true;
		islowHeight = false;
		break;
	case 'y':
	case 'Y':
		isCamMove = isCamMove ? false : true;
		break;
	case 'v':
	case 'V':
		if (islowHeight) {
			isMove = true;
			islowHeight = false;
		}
		else {
			isMove = false;
			islowHeight = true;
		}
		break;
	case 'r':
	case 'R':
		create_player = false;
		made_mage = true;
		make_mage();
		break;
	case 'q':
	case 'Q':
		exit(1);
		break;
	case '+':
		for (auto& n : cube) {
			for (auto& val : n) {
				val.add_sub_grow_speed(0.3);
			}
		}
		break;
	case '-':
		for (auto& n : cube) {
			for (auto& val : n) {
				if (val.get_grow_speed() > 0.3)
					val.add_sub_grow_speed(-0.3);
			}
		}
		break;
	case 'b':
	case 'B':
		if (!create_player && made_mage) {
			create_player = true;
			player.set_translate(vec3(cube[fstart_x][fstart_y].get_cur_loc().x, 0.05, cube[fstart_x][fstart_y].get_cur_loc().z));
		}
		break;
	case '1':
		camera = &fps_camera;
		cam_fps = true;
		camera->Position = vec3(player.get_cur_positon().x, 0.1, player.get_cur_positon().z);
		break;
	case '3':
		cam_fps = false;
		camera = &quarter_camera;
		break;
	case 's':
	case 'S':
		key_down = Keyboard_Event::BACKWARD;
		break;
	case 'w':
	case 'W':
		key_down = Keyboard_Event::FOWARD;
		break;
	case 'a':
	case 'A':
		key_down = Keyboard_Event::LEFTSIDE;
		break;
	case 'd':
	case 'D':
		key_down = Keyboard_Event::RIGHTSIDE;
		break;
	case 'c':
	case 'C':
		for (auto& n : cube) {
			for (auto& val : n) {
				val.set_type(Type::crush_wall);
			}
		}
		made_mage = false;
		create_player = false;
		islowHeight = false;
		isMove = true;
		cam_fps = false;
		camera = &quarter_camera;
		camera->Position = vec3(0.0f, 8.0f, 8.0f);
		player.set_translate(vec3(cube[fstart_x][fstart_y].get_cur_loc().x, 0.05, cube[fstart_x][fstart_y].get_cur_loc().z));
		break;
	}

	glutPostRedisplay();
}

void KeyboardUp(unsigned char key, int x, int y)
{
	key_down = Keyboard_Event::KEYUP;
}

void SpecialKeyboardUp(int key, int x, int y)
{
	key_down = Keyboard_Event::KEYUP;
}

void SpecialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		key_down = Keyboard_Event::UP;
		break;
	case GLUT_KEY_DOWN:
		key_down = Keyboard_Event::DOWN;
		break;
	case GLUT_KEY_LEFT:
		key_down = Keyboard_Event::LEFT;
		break;
	case GLUT_KEY_RIGHT:
		key_down = Keyboard_Event::RIGHT;
		break;
	}
}

void MoveTimerFuction(int value) {
	if (cam_fps) {
		fps_camera.Position = vec3(player.get_cur_positon().x, 0.1, player.get_cur_positon().z);

		switch (key_down) {
		case Keyboard_Event::FOWARD:
			player.translate(vec3(fps_camera.Front.x * 0.01, 0, fps_camera.Front.z * 0.01));
			break;
		case Keyboard_Event::BACKWARD:
			player.translate(vec3(-fps_camera.Front.x * 0.01, 0, -fps_camera.Front.z * 0.01));
			break;
		case Keyboard_Event::LEFTSIDE:
			player.translate(vec3(-fps_camera.Right.x * 0.01, 0, -fps_camera.Right.z * 0.01));
			break;
		case Keyboard_Event::RIGHTSIDE:
			player.translate(vec3(fps_camera.Right.x * 0.01, 0, fps_camera.Right.z * 0.01));
			break;
		}
	}
	else {
		switch (key_down) {
		case Keyboard_Event::UP:
			player.translate(vec3(0, 0, -0.01));
			break;
		case Keyboard_Event::DOWN:
			player.translate(vec3(0, 0, 0.01));
			break;
		case Keyboard_Event::LEFT:
			player.translate(vec3(-0.01, 0, 0));
			break;
		case Keyboard_Event::RIGHT:
			player.translate(vec3(0.01, 0, 0));
			break;
		}
	}

	glutTimerFunc(10, MoveTimerFuction, 1);
}

void PassiveMotion(int x, int y) {
	if (cam_fps) {
		float xpos = static_cast<float>(x);
		float ypos = static_cast<float>(y);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		camera->ProcessMouseMovement(xoffset, yoffset);

		if (x < 100 || x > win_width - 100) {
			lastX = win_width / 2;
			lastY = win_height / 2;
			glutWarpPointer(win_width / 2, win_height / 2);
		}
		else if (y < 100 || y > win_height - 100) {
			lastX = win_width / 2;
			lastY = win_height / 2;
			glutWarpPointer(win_width / 2, win_height / 2);
		}
	}
}

void TimerFunction(int value)
{
	if (arrive_cnt < width_num * height_num) {
		for (auto& n : cube) {
			for (auto& val : n) {
				if (val.get_start_animation())
				{
					if (val.get_cur_loc().y - val.get_height_randValue() > 0.0f) {
						val.translate(glm::vec3(0.0f, -0.1, 0.0f));
					}
				}
				if (val.get_cur_loc().y - val.get_height_randValue() < 0.0f) {
					if (val.get_start_animation()) arrive_cnt += 1;
					val.set_start_animation();
					val.translate(glm::vec3(0.0f, -(val.get_cur_loc().y - val.get_height_randValue()), 0.0f));
				}
			}
		}
	}

	if (isMove) {
		for (auto& n : cube) {
			for (auto& val : n) {
				val.grow_cube(glm::vec3(0.0f, val.get_grow_speed() / 100, 0.0f));
			}
		}
	}


	if (isCamMove) {
		quarter_camera.Position.x = 8 * static_cast<float>(sin(cnt / 360.0 * 2 * 3.141592));
		quarter_camera.Position.z = 8 * static_cast<float>(cos(cnt / 360.0 * 2 * 3.141592));
		cnt++;
	}

	if (islowHeight) {
		for (auto& n : cube) {
			for (auto& val : n) {
				val.low_cube(vec3(0, -0.1, 0));
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(1, TimerFunction, 1);
}


void Init()
{
	cube.resize(width_num);

	for (int i = 0; i < width_num; ++i) {
		for (int j = 0; j < height_num; ++j) {
			cube[i].push_back(Object{ glm::vec4(2 * i - (width_num - 1), 50, 2 * j - (height_num - 1), 1) });
		}
	}

	for (int i = 0; i < width_num; ++i) {
		for (int j = 0; j < height_num; ++j) {
			cube[i][j].scale(glm::vec3(0.2f, cube[i][j].get_height_randValue(), 0.2f));
		}
	}
	player.lscale(vec3(0.05, 0.05, 0.05));

	global_obj = global_objReader.loadObj_normalize_center("cube.obj");

	plane.scale(glm::vec3(0.2f * width_num, 0.001f, 0.2f * height_num));

	camera = &quarter_camera;
}


void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	while (true) {
		cout << "input width_num : ";
		cin >> width_num;
		cout << "input height_num : ";
		cin >> height_num;

		if (width_num <= 50 and height_num <= 50 and
			width_num >= 5 and height_num >= 5) {
			cout << width_num << " : " << height_num << endl;
			break;
		}
		cout << "Please enter again. maximum is 50, ";
		cout << "minimum is 5." << endl;
	}
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
	glutSetCursor(GLUT_CURSOR_NONE);
	glutSpecialFunc(SpecialKeys);
	//glutSpecialUpFunc(KeyUp);
	//glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(PassiveMotion);
	glutKeyboardFunc(Keyboard);
	glutSpecialUpFunc(SpecialKeyboardUp);
	//glutSpecialUpFunc(KeyUp);
	glutKeyboardUpFunc(KeyboardUp);
	glutTimerFunc(100, TimerFunction, 1);
	glutTimerFunc(10, MoveTimerFuction, 1);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}