#include "../Include/Common.h"

//For GLUT to handle 
#define MENU_WALK_START 1
#define MENU_WALK_STOP 2
#define MENU_EXIT 3




//GLUT timer variable
float timer_cnt = 0;
bool timer_enabled = false;
unsigned int timer_speed = 16;

using namespace glm;
using namespace std;



mat4 view(1.0f);			// V of MVP, viewing matrix
mat4 projection(1.0f);		// P of MVP, projection matrix
mat4 model(1.0f);			// M of MVP, model matrix
vec3 temp = vec3(0.0, 0.0, 0.0);			// a 3 dimension vector which represents how far did the ladybug should move

// TEXTURE
GLuint usetexture; // determine whether to use texture
vec4 notuse = vec4(0.0, 0.0, 0.0, 0.0);
vec4 use = vec4(1.0, 1.0, 1.0, 1.0);

GLint um4p;
GLint um4mv;
GLuint layer1st;
GLuint layer2nd;
GLuint layer3rd;
GLuint layer4th;
GLuint layer5th;
GLuint layer6th;
GLuint color;

GLuint program;			// shader program id

vec3 head_pos = vec3(0.0, 3.0, 0.0); // head initial position
int head_direction = 1; // head initial direction

// BODY
float cnt_body = 0.0;
float direction_body = 1.0;


// ARM
float cnt_rts = 0.0;
float direction_rts = 1.0;

float cnt_rms = 0.0;
float direction_rms = 1.0;

float cnt_lts = 0.0;
float direction_lts = -1.0;

float cnt_lms = 0.0;
float direction_lms = -1.0;

// LEG
float cnt_rtl = 0.0;
float direction_rtl = 1.0;

float cnt_rml = 0.0;
float direction_rml = 1.0;

float cnt_ltl = 0.0;
float direction_ltl = -1.0;

float cnt_lml = 0.0;
float direction_lml = -1.0;

typedef struct
{
	GLuint vao;			// vertex array object
	GLuint vbo;			// vertex buffer object

	int materialId;
	int vertexCount;
	GLuint m_texture;
} Shape;

Shape m_shape;
Shape shape_sphere;
Shape shape_cube;

vector<string> model_list{"Cube.obj", "Sphere.obj"};


// Load shader file to program
char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

// Free shader file
void freeShaderSource(char** srcp)
{
	delete srcp[0];
	delete srcp;
}

// Load .obj model
void My_LoadModels()
{
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "Cylinder.obj");
	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
	}
	if (!ret) {
		exit(1);
	}

	vector<float> vertices, texcoords, normals;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	for (int s = 0; s < shapes.size(); ++s) {  // for 'ladybug.obj', there is only one object
		int index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
			}
			index_offset += fv;
			m_shape.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &m_shape.vao);
	glBindVertexArray(m_shape.vao);

	glGenBuffers(1, &m_shape.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
	glEnableVertexAttribArray(2);

	shapes.clear();
	shapes.shrink_to_fit();
	materials.clear();
	materials.shrink_to_fit();
	vertices.clear();
	vertices.shrink_to_fit();
	texcoords.clear();
	texcoords.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();

	cout << "Load " << m_shape.vertexCount << " vertices" << endl;

	texture_data tdata = loadImg("metal.png");

	glGenTextures(1, &m_shape.m_texture);
	glBindTexture(GL_TEXTURE_2D, m_shape.m_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete tdata.data;

	// ====================================sphere.obj=============================================
	tinyobj::attrib_t attrib_sph;
	vector<tinyobj::shape_t> shapes_sph;
	vector<tinyobj::material_t> materials_sph;
	string warn_sph;
	string err_sph;
	bool ret_sph = tinyobj::LoadObj(&attrib_sph, &shapes_sph, &materials_sph, &warn_sph, &err_sph, "Sphere.obj");
	if (!warn_sph.empty()) {
		cout << warn_sph << endl;
	}
	if (!err_sph.empty()) {
		cout << err_sph << endl;
	}
	if (!ret_sph) {
		exit(1);
	}

	vector<float> vertices_sph, texcoords_sph, normals_sph;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	for (int s = 0; s < shapes_sph.size(); ++s) {
		int index_offset = 0;
		for (int f = 0; f < shapes_sph[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes_sph[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes_sph[s].mesh.indices[index_offset + v];
				vertices_sph.push_back(attrib_sph.vertices[3 * idx.vertex_index + 0]);
				vertices_sph.push_back(attrib_sph.vertices[3 * idx.vertex_index + 1]);
				vertices_sph.push_back(attrib_sph.vertices[3 * idx.vertex_index + 2]);
				texcoords_sph.push_back(attrib_sph.texcoords[2 * idx.texcoord_index + 0]);
				texcoords_sph.push_back(attrib_sph.texcoords[2 * idx.texcoord_index + 1]);
				normals_sph.push_back(attrib_sph.normals[3 * idx.normal_index + 0]);
				normals_sph.push_back(attrib_sph.normals[3 * idx.normal_index + 1]);
				normals_sph.push_back(attrib_sph.normals[3 * idx.normal_index + 2]);
			}
			index_offset += fv;
			shape_sphere.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &shape_sphere.vao);
	glBindVertexArray(shape_sphere.vao);

	glGenBuffers(1, &shape_sphere.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, shape_sphere.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices_sph.size() * sizeof(float) + texcoords_sph.size() * sizeof(float) + normals_sph.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_sph.size() * sizeof(float), vertices_sph.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices_sph.size() * sizeof(float), texcoords_sph.size() * sizeof(float), texcoords_sph.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices_sph.size() * sizeof(float) + texcoords_sph.size() * sizeof(float), normals_sph.size() * sizeof(float), normals_sph.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices_sph.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices_sph.size() * sizeof(float) + texcoords_sph.size() * sizeof(float)));
	glEnableVertexAttribArray(2);

	shapes_sph.clear();
	shapes_sph.shrink_to_fit();
	materials_sph.clear();
	materials_sph.shrink_to_fit();
	vertices_sph.clear();
	vertices_sph.shrink_to_fit();
	texcoords_sph.clear();
	texcoords_sph.shrink_to_fit();
	normals_sph.clear();
	normals_sph.shrink_to_fit();

	cout << "Load " << shape_sphere.vertexCount << " vertices" << endl;

	texture_data tdata_sph = loadImg("metal.png");

	glGenTextures(1, &shape_sphere.m_texture);
	glBindTexture(GL_TEXTURE_2D, shape_sphere.m_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata_sph.width, tdata_sph.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata_sph.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete tdata_sph.data;

	// ====================================cube.obj=============================================
	tinyobj::attrib_t attrib_cube;
	vector<tinyobj::shape_t> shapes_cube;
	vector<tinyobj::material_t> materials_cube;
	string warn_cube;
	string err_cube;
	bool ret_cube = tinyobj::LoadObj(&attrib_cube, &shapes_cube, &materials_cube, &warn_cube, &err_cube, "Cube.obj");
	if (!warn_cube.empty()) {
		cout << warn_cube << endl;
	}
	if (!err_cube.empty()) {
		cout << err_cube << endl;
	}
	if (!ret_cube) {
		exit(1);
	}

	vector<float> vertices_cube, texcoords_cube, normals_cube;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	for (int s = 0; s < shapes_cube.size(); ++s) {
		int index_offset = 0;
		for (int f = 0; f < shapes_cube[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes_cube[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes_cube[s].mesh.indices[index_offset + v];
				vertices_cube.push_back(attrib_cube.vertices[3 * idx.vertex_index + 0]);
				vertices_cube.push_back(attrib_cube.vertices[3 * idx.vertex_index + 1]);
				vertices_cube.push_back(attrib_cube.vertices[3 * idx.vertex_index + 2]);
				texcoords_cube.push_back(attrib_cube.texcoords[2 * idx.texcoord_index + 0]);
				texcoords_cube.push_back(attrib_cube.texcoords[2 * idx.texcoord_index + 1]);
				normals_cube.push_back(attrib_cube.normals[3 * idx.normal_index + 0]);
				normals_cube.push_back(attrib_cube.normals[3 * idx.normal_index + 1]);
				normals_cube.push_back(attrib_cube.normals[3 * idx.normal_index + 2]);
			}
			index_offset += fv;
			shape_cube.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &shape_cube.vao);
	glBindVertexArray(shape_cube.vao);

	glGenBuffers(1, &shape_cube.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, shape_cube.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices_cube.size() * sizeof(float) + texcoords_cube.size() * sizeof(float) + normals_cube.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_cube.size() * sizeof(float), vertices_cube.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices_cube.size() * sizeof(float), texcoords_cube.size() * sizeof(float), texcoords_cube.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices_cube.size() * sizeof(float) + texcoords_cube.size() * sizeof(float), normals_cube.size() * sizeof(float), normals_cube.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices_cube.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices_cube.size() * sizeof(float) + texcoords_cube.size() * sizeof(float)));
	glEnableVertexAttribArray(2);

	shapes_cube.clear();
	shapes_cube.shrink_to_fit();
	materials_cube.clear();
	materials_cube.shrink_to_fit();
	vertices_cube.clear();
	vertices_cube.shrink_to_fit();
	texcoords_cube.clear();
	texcoords_cube.shrink_to_fit();
	normals_cube.clear();
	normals_cube.shrink_to_fit();

	cout << "Load " << shape_cube.vertexCount << " vertices" << endl;

	texture_data tdata_cube = loadImg("metal.png");

	glGenTextures(1, &shape_cube.m_texture);
	glBindTexture(GL_TEXTURE_2D, shape_cube.m_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata_cube.width, tdata_cube.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata_cube.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete tdata_cube.data;
}

// OpenGL initialization
void My_Init()
{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Create Shader Program
	program = glCreateProgram();

	// Create customize shader by tell openGL specify shader type 
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load shader file
	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");

	// Assign content of these shader files to those shaders we created before
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	// Free the shader file string(won't be used any more)
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);

	// Compile these shaders
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	// Logging
	shaderLog(vertexShader);
	shaderLog(fragmentShader);

	// Assign the program we created before with these shaders
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// Get the id of inner variable 'um4p' and 'um4mv' in shader programs
	um4p = glGetUniformLocation(program, "um4p");
	um4mv = glGetUniformLocation(program, "um4mv");
	layer1st = glGetUniformLocation(program, "layer1st");
	layer2nd = glGetUniformLocation(program, "layer2nd");
	layer3rd= glGetUniformLocation(program, "layer3rd");
	layer4th = glGetUniformLocation(program, "layer4th");
	layer5th = glGetUniformLocation(program, "layer5th");
	layer6th = glGetUniformLocation(program, "layer6th");
	color = glGetUniformLocation(program, "color");
	usetexture = glGetUniformLocation(program, "usetexture");

	// Tell OpenGL to use this shader program now
	glUseProgram(program);

	My_LoadModels();
}

// GLUT callback. Called to draw the scene.
void My_Display()
{
	// Clear display buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Bind a vertex array for OpenGL (OpenGL will apply operation only to the vertex array objects it bind)
	glBindVertexArray(shape_cube.vao);

	// Tell openGL to use the shader program we created before
	glUseProgram(program);

	// =================================BODY====================================
	mat4 identity = mat4((1.0f));
	mat4 translation_matrix = translate(mat4(1.0f), temp);
	// rotation
	GLfloat degree_body = cnt_body;
	if (timer_enabled) cnt_body = cnt_body + 0.02 * direction_body;
	// BODY rotation axis
	vec3 rotate_axis = vec3(0.0, 1.0, 0.0);
	mat4 rotation_matrix = rotate(mat4(1.0f), cnt_body, rotate_axis);
	// scale the body longer
	mat4 scaling;
	vec3 scale_vector = vec3(1.8f, 3.0f, 1.8f);
	scaling = scale(mat4(1.0), scale_vector);
	model = scaling * translation_matrix * rotation_matrix;

	

	// Transfer value of (view*model) to both shader's inner variable 'um4mv'; 
	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));

	// Transfer value of projection to both shader's inner variable 'um4p';
	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glUniform4fv(usetexture, 1, value_ptr(use));

	// Tell openGL to draw the vertex array we had binded before
	glDrawArrays(GL_TRIANGLES, 0, shape_cube.vertexCount);

	// =================================HEAD====================================
	
	mat4 T_head = translate(mat4(1.0f), head_pos);

	
	if(head_pos.y > 3.5) head_direction = -1;
	else if(head_pos.y < 2.7) head_direction = 1;
	if (timer_enabled) head_pos.y += 0.01 * head_direction;
	
	vec3 scaling_vector_head = vec3(1.1, 0.3, 1.1);
	mat4 scaling_head = scale(mat4(1.0), scaling_vector_head);
	mat4 model_matrix_head;
	model_matrix_head = scaling_head * T_head;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_head));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(m_shape.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// =================================EYE RIGHT====================================

	mat4 T_eye_r = translate(mat4(1.0f), vec3(-2.0, 0.0, 0.7)); // (front/back) (top/down) (left/right)
	//vec3 rotate_axis = vec3(0.0, 1.0, 0.0);
	//mat4 rotation_matrix = rotate(mat4(1.0f), radians(timer_cnt), rotate_axis);
	vec3 scaling_vector_eye_r = vec3(0.25, 1.0, 0.25);
	mat4 scaling_eye_r = scale(mat4(1.0), scaling_vector_eye_r);
	mat4 model_matrix_eye_r;
	model_matrix_eye_r = scaling_eye_r * T_eye_r;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_head));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_eye_r));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_sphere.vao);

	glUniform4fv(usetexture, 1, value_ptr(notuse));
	glUniform4fv(color, 1, value_ptr(vec4(0.5, 0.5, 0.5, 1.0)));

	glDrawArrays(GL_TRIANGLES, 0, shape_sphere.vertexCount);

	// =================================EYE LEFT====================================

	mat4 T_eye_l = translate(mat4(1.0f), vec3(-2.0, 0.0, -0.7)); // (front/back) (top/down) (left/right)
	//vec3 rotate_axis = vec3(0.0, 1.0, 0.0);
	//mat4 rotation_matrix = rotate(mat4(1.0f), radians(timer_cnt), rotate_axis);
	vec3 scaling_vector_eye_l = vec3(0.25, 1.0, 0.25);
	mat4 scaling_eye_l = scale(mat4(1.0), scaling_vector_eye_l);
	mat4 model_matrix_eye_l;
	model_matrix_eye_l = scaling_eye_l * T_eye_l;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_head));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_eye_l));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_sphere.vao);

	glUniform4fv(usetexture, 1, value_ptr(notuse));
	glUniform4fv(color, 1, value_ptr(vec4(0.5, 0.5, 0.5, 1.0)));

	glDrawArrays(GL_TRIANGLES, 0, shape_sphere.vertexCount);

	// =================================RIGHT TOP SHOULDER====================================
	
	mat4 T_rts = translate(mat4(1.0f), vec3(0.0, 0.2, 1.25));
	GLfloat degree_arm_rts = cnt_rts;
	// rotation
	if (timer_enabled) cnt_rts = cnt_rts + 0.01 * direction_rts;
	if (degree_arm_rts > 1) direction_rts = -1;
	if (degree_arm_rts < -1) direction_rts = 1;
	vec3 rotate_axis_rts = vec3(0.0, 0.0, 1.0);
	mat4 rotation_matrix_rts = rotate(mat4(1.0f), degree_arm_rts, rotate_axis_rts);
	vec3 scaling_vector_rts = vec3(.8, .6, .8);
	mat4 scaling_rts = scale(mat4(1.0), scaling_vector_rts);
	mat4 model_matrix_rts;
	model_matrix_rts = scaling_rts * T_rts * rotation_matrix_rts;
	// end of rotation


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_rts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_cube.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_cube.vertexCount);

	// =================================RIGHT TOP ARM====================================

	mat4 T_rta = translate(mat4(1.0f), vec3(0.0, -1.2, 0.0));
	//vec3 rotate_axis = vec3(0.0, 1.0, 0.0);
	//mat4 rotation_matrix = rotate(mat4(1.0f), radians(timer_cnt), rotate_axis);
	vec3 scaling_vector_rta = vec3(0.5, 0.5, 0.5);
	mat4 scaling_rta = scale(mat4(1.0), scaling_vector_rta);
	mat4 model_matrix_rta;
	model_matrix_rta = scaling_rta * T_rta;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_rts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_rta));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(m_shape.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// =================================RIGHT MIDDLE SHOULDER====================================

	mat4 T_rms = translate(mat4(1.0f), vec3(0.0, -1.0, 0.0));
	GLfloat degree_arm_rms = cnt_rms;
	// rotation
	if (timer_enabled) cnt_rms = cnt_rms + 0.01 * direction_rms;
	if (degree_arm_rms > 0.3) direction_rms = -1;
	if (degree_arm_rms < -1.7) direction_rms = 1;
	vec3 rotate_axis_rms = vec3(0.0, 0.0, 1.0);
	mat4 rotation_matrix_rms = rotate(mat4(1.0f), degree_arm_rms, rotate_axis_rms);
	vec3 scaling_vector_rms = vec3(1.2, 1.0, 1.2);
	mat4 scaling_rms = scale(mat4(1.0), scaling_vector_rms);
	mat4 model_matrix_rms;
	model_matrix_rms = scaling_rms * T_rms * rotation_matrix_rms;
	// end of rotation


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_rts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_rta));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_rms));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_sphere.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_sphere.vertexCount);

	// =================================RIGHT END ARM====================================

	mat4 T_rea = translate(mat4(1.0f), vec3(0.0, -1.2, 0.0));

	vec3 scaling_vector_rea = vec3(0.5, 0.7, 0.5);
	mat4 scaling_rea = scale(mat4(1.0), scaling_vector_rea);
	mat4 model_matrix_rea;
	model_matrix_rea = scaling_rea * T_rea;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_rts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_rta));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_rms));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_rea));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(m_shape.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// =================================RIGHT HAND====================================

	mat4 T_rhand = translate(mat4(1.0f), vec3(0.0, -1.0, 0.0));
	vec3 scaling_vector_rhand = vec3(1.6, 1.2, 1.6);
	mat4 scaling_rhand = scale(mat4(1.0), scaling_vector_rhand);
	mat4 model_matrix_rhand;
	model_matrix_rhand = scaling_rhand * T_rhand;
	// end of rotation


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_rts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_rta));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_rms));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_rea));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(model_matrix_rhand));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_sphere.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_sphere.vertexCount);

	// =================================LEFT TOP SHOULDER====================================

	mat4 T_lts = translate(mat4(1.0f), vec3(0.0, 0.2, -1.25));
	GLfloat degree_arm_lts = cnt_lts;
	if (timer_enabled) cnt_lts = cnt_lts + 0.01 * direction_lts;
	if (degree_arm_lts > 1) direction_lts = -1;
	if (degree_arm_lts < -1) direction_lts = 1;
	vec3 rotate_axis_lts = vec3(0.0, 0.0, 1.0);
	mat4 rotation_matrix_lts = rotate(mat4(1.0f), degree_arm_lts, rotate_axis_lts);
	vec3 scaling_vector_lts = vec3(.8, .6, .8);
	mat4 scaling_lts = scale(mat4(1.0), scaling_vector_lts);
	mat4 model_matrix_lts;
	model_matrix_lts = scaling_lts * T_lts * rotation_matrix_lts;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_lts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_cube.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_cube.vertexCount);

	// =================================LEFT TOP ARM====================================

	mat4 T_lta = translate(mat4(1.0f), vec3(0.0, -1.2, 0.0));
	//vec3 rotate_axis = vec3(0.0, 1.0, 0.0);
	//mat4 rotation_matrix = rotate(mat4(1.0f), radians(timer_cnt), rotate_axis);
	vec3 scaling_vector_lta = vec3(0.5, 0.5, 0.5);
	mat4 scaling_lta = scale(mat4(1.0), scaling_vector_lta);
	mat4 model_matrix_lta;
	model_matrix_lta = scaling_lta *  T_lta;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_lts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_lta));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(m_shape.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// =================================LEFT MIDDLE SHOULDER====================================

	mat4 T_lms = translate(mat4(1.0f), vec3(0.0, -1.0, 0.0));
	GLfloat degree_arm_lms = cnt_lms;
	// rotation
	if (timer_enabled) cnt_lms = cnt_lms + 0.01 * direction_lms;
	if (degree_arm_lms > 0.3) direction_lms = -1;
	if (degree_arm_lms < -1.7) direction_lms = 1;
	vec3 rotate_axis_lms = vec3(0.0, 0.0, 1.0);
	mat4 rotation_matrix_lms = rotate(mat4(1.0f), degree_arm_lms, rotate_axis_lms);
	vec3 scaling_vector_lms = vec3(1.2, 1.0, 1.2);
	mat4 scaling_lms = scale(mat4(1.0), scaling_vector_lms);
	mat4 model_matrix_lms;
	model_matrix_lms = scaling_lms * T_lms * rotation_matrix_lms;
	// end of rotation


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_lts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_lta));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_lms));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_sphere.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_sphere.vertexCount);

	// =================================LEFT END ARM====================================

	mat4 T_lea = translate(mat4(1.0f), vec3(0.0, -1.2, 0.0));

	vec3 scaling_vector_lea = vec3(0.5, 0.7, 0.5);
	mat4 scaling_lea = scale(mat4(1.0), scaling_vector_lea);
	mat4 model_matrix_lea;
	model_matrix_lea = scaling_lea * T_lea;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_lts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_lta));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_lms));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_lea));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(m_shape.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// =================================LEFT HAND====================================

	mat4 T_lhand = translate(mat4(1.0f), vec3(0.0, -1.0, 0.0));
	vec3 scaling_vector_lhand = vec3(1.6, 1.2, 1.6);
	mat4 scaling_lhand = scale(mat4(1.0), scaling_vector_lhand);
	mat4 model_matrix_lhand;
	model_matrix_lhand = scaling_lhand * T_lhand;
	// end of rotation


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_lts));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_lta));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_lms));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_lea));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(model_matrix_lhand));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_sphere.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_sphere.vertexCount);

	// =================================BOTTOM BODY====================================
	mat4 translation_matrix_bottom = translate(mat4(1.0f), vec3(0.0, -1.7, 0.0));
	// scale the body longer
	mat4 scaling_bottom;
	vec3 scale_vector_bottom = vec3(0.8, 0.5, 0.3);
	scaling_bottom = scale(mat4(1.0), scale_vector_bottom);
	mat4 model_matrix_buttom = scaling_bottom * translation_matrix_bottom;

	// Transfer value of (view*model) to both shader's inner variable 'um4mv'; 
	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));

	// Transfer value of projection to both shader's inner variable 'um4p';
	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glUniform4fv(usetexture, 1, value_ptr(use));
	glBindVertexArray(shape_cube.vao);
	// Tell openGL to draw the vertex array we had binded before
	glDrawArrays(GL_TRIANGLES, 0, shape_cube.vertexCount);
	
	// =================================RIGHT TOP LEG====================================

	mat4 T_rtl = translate(mat4(1.0f), vec3(0.0, -0.2, -1.0));
	GLfloat degree_arm_rtl = cnt_rtl;
	if (timer_enabled) cnt_rtl = cnt_rtl + 0.01 * direction_rtl;
	if (degree_arm_rtl > 1) direction_rtl = -1;
	if (degree_arm_rtl < -1) direction_rtl = 1;
	vec3 rotate_axis_rtl = vec3(0.0, 0.0, 1.0);
	mat4 rotation_matrix_rtl = rotate(mat4(1.0f), degree_arm_rtl, rotate_axis_rtl);
	vec3 scaling_vector_rtl = vec3(1.0, 1.0, 1.5);
	mat4 scaling_rtl = scale(mat4(1.0), scaling_vector_rtl);
	mat4 model_matrix_rtl;
	model_matrix_rtl = scaling_rtl * T_rtl * rotation_matrix_rtl;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_rtl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_cube.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_cube.vertexCount);

	// =================================RIGHT TOP THIGH====================================

	mat4 T_rtt = translate(mat4(1.0f), vec3(0.0, -1.0, 0.0));
	vec3 scaling_vector_rtt = vec3(0.7, 0.5, 0.7);
	mat4 scaling_rtt = scale(mat4(1.0), scaling_vector_rtt);
	mat4 model_matrix_rtt;
	model_matrix_rtt = scaling_rtt * T_rtt;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_rtl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_rtt));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(m_shape.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// =================================RIGHT MIDDLE LEG====================================

	mat4 T_rml = translate(mat4(1.0f), vec3(0.0, -1.1, 0.0));
	GLfloat degree_arm_rml = cnt_rml;
	// rotation
	if (timer_enabled) cnt_rml = cnt_rml + 0.01 * direction_rml;
	if (degree_arm_rml > 2.0) direction_rml = -1;
	if (degree_arm_rml < 0.0) direction_rml = 1;
	vec3 rotate_axis_rml = vec3(0.0, 0.0, 1.0);
	mat4 rotation_matrix_rml = rotate(mat4(1.0f), degree_arm_rml, rotate_axis_rml);
	vec3 scaling_vector_rml = vec3(1.3, 1.3, 1.5);
	mat4 scaling_rml = scale(mat4(1.0), scaling_vector_rml);
	mat4 model_matrix_rml;
	model_matrix_rml = scaling_rml * T_rml * rotation_matrix_rml;
	// end of rotation


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_rtl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_rtt));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_rml));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_sphere.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_sphere.vertexCount);

	// =================================RIGHT CALF====================================

	mat4 T_rc = translate(mat4(1.0f), vec3(0.0, -1.0, 0.0));
	vec3 scaling_vector_rc = vec3(0.7, 0.7, 0.7);
	mat4 scaling_rc = scale(mat4(1.0), scaling_vector_rc);
	mat4 model_matrix_rc;
	model_matrix_rc = scaling_rc * T_rc;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_rtl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_rtt));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_rml));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(model_matrix_rc));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(m_shape.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// =================================RIGHT FOOT====================================

	mat4 T_rf = translate(mat4(1.0f), vec3(-.2, -5.0, 0.0));
	vec3 scaling_vector_rf = vec3(2.0, 0.2, 2.0);
	mat4 scaling_rf = scale(mat4(1.0), scaling_vector_rf);
	mat4 model_matrix_rf;
	model_matrix_rf = scaling_rf * T_rf;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_rtl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_rtt));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_rml));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(model_matrix_rc));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(model_matrix_rf));

	glBindVertexArray(shape_cube.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_cube.vertexCount);

	// =================================LEFT TOP LEG====================================

	mat4 T_ltl = translate(mat4(1.0f), vec3(0.0, -0.2, 1.0));
	GLfloat degree_arm_ltl = cnt_ltl;
	if (timer_enabled) cnt_ltl = cnt_ltl + 0.01 * direction_ltl;
	if (degree_arm_ltl > 1) direction_ltl = -1;
	if (degree_arm_ltl < -1) direction_ltl = 1;
	vec3 rotate_axis_ltl = vec3(0.0, 0.0, 1.0);
	mat4 rotation_matrix_ltl = rotate(mat4(1.0f), degree_arm_ltl, rotate_axis_ltl);
	vec3 scaling_vector_ltl = vec3(1.0, 1.0, 1.5);
	mat4 scaling_ltl = scale(mat4(1.0), scaling_vector_ltl);
	mat4 model_matrix_ltl;
	model_matrix_ltl = scaling_ltl * T_ltl * rotation_matrix_ltl;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_ltl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_cube.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_cube.vertexCount);

	// =================================LEFT TOP THIGH====================================

	mat4 T_ltt = translate(mat4(1.0f), vec3(0.0, -1.0, 0.0));
	vec3 scaling_vector_ltt = vec3(0.7, 0.5, 0.7);
	mat4 scaling_ltt = scale(mat4(1.0), scaling_vector_ltt);
	mat4 model_matrix_ltt;
	model_matrix_ltt = scaling_ltt * T_ltt;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_ltl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_ltt));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(m_shape.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// =================================LEFT MIDDLE LEG====================================

	mat4 T_lml = translate(mat4(1.0f), vec3(0.0, -1.1, 0.0));
	GLfloat degree_arm_lml = cnt_lml;
	// rotation
	if (timer_enabled) cnt_lml = cnt_lml + 0.01 * direction_lml;
	if (degree_arm_lml > 2.0) direction_lml = -1;
	if (degree_arm_lml < 0.0) direction_lml = 1;
	vec3 rotate_axis_lml = vec3(0.0, 0.0, 1.0);
	mat4 rotation_matrix_lml = rotate(mat4(1.0f), degree_arm_lml, rotate_axis_lml);
	vec3 scaling_vector_lml = vec3(1.3, 1.3, 1.5);
	mat4 scaling_lml = scale(mat4(1.0), scaling_vector_lml);
	mat4 model_matrix_lml;
	model_matrix_lml = scaling_lml * T_lml * rotation_matrix_lml;
	// end of rotation


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_ltl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_ltt));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_lml));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(identity));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(shape_sphere.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_sphere.vertexCount);

	// =================================LEFT CALF====================================

	mat4 T_lc = translate(mat4(1.0f), vec3(0.0, -1.0, 0.0));
	vec3 scaling_vector_lc = vec3(0.7, 0.7, 0.7);
	mat4 scaling_lc = scale(mat4(1.0), scaling_vector_lc);
	mat4 model_matrix_lc;
	model_matrix_lc = scaling_lc * T_lc;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_ltl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_ltt));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_lml));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(model_matrix_lc));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(identity));

	glBindVertexArray(m_shape.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// =================================LEFT FOOT====================================

	mat4 T_lf = translate(mat4(1.0f), vec3(-.2, -5.0, 0.0));
	vec3 scaling_vector_lf = vec3(2.0, 0.2, 2.0);
	mat4 scaling_lf = scale(mat4(1.0), scaling_vector_lf);
	mat4 model_matrix_lf;
	model_matrix_lf = scaling_lf * T_lf;


	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));


	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(layer1st, 1, GL_FALSE, value_ptr(model_matrix_buttom));
	glUniformMatrix4fv(layer2nd, 1, GL_FALSE, value_ptr(model_matrix_ltl));
	glUniformMatrix4fv(layer3rd, 1, GL_FALSE, value_ptr(model_matrix_ltt));
	glUniformMatrix4fv(layer4th, 1, GL_FALSE, value_ptr(model_matrix_lml));
	glUniformMatrix4fv(layer5th, 1, GL_FALSE, value_ptr(model_matrix_lc));
	glUniformMatrix4fv(layer6th, 1, GL_FALSE, value_ptr(model_matrix_lf));

	glBindVertexArray(shape_cube.vao);

	glUniform4fv(usetexture, 1, value_ptr(use));

	glDrawArrays(GL_TRIANGLES, 0, shape_cube.vertexCount);

	// Change current display buffer to another one (refresh frame) 
	glutSwapBuffers();
}

// Setting up viewing matrix
void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	float viewportAspect = (float)width / (float)height;

	// perspective(fov, aspect_ratio, near_plane_distance, far_plane_distance)
	// ps. fov = field of view, it represent how much range(degree) is this camera could see 
	projection = perspective(radians(60.0f), viewportAspect, 0.1f, 1000.0f);

	// lookAt(camera_position, camera_viewing_vector, up_vector)
	// up_vector represent the vector which define the direction of 'up'
	view = lookAt(vec3(-15.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

void My_Timer(int val)
{
	timer_cnt += 1.0f;
	glutPostRedisplay();
	if (timer_enabled)
	{
		glutTimerFunc(timer_speed, My_Timer, val);
	}
}

void My_Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	if (!timer_enabled) return;
	if (key == 'd')
	{
		temp = temp + vec3(0, 0, 1);
	}
	else if (key == 'a')
	{
		temp = temp - vec3(0, 0, 1);
	}
	else if (key == 'w')
	{
		temp = temp + vec3(1, 0, 0);
	}
	else if (key == 's')
	{
		temp = temp - vec3(1, 0, 0);
	}
	else if (key == 'z')
	{
		direction_body = -1.0;
	}
	else if (key == 'x')
	{
		direction_body = 1.0;
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch (id)
	{
	case MENU_WALK_START:
		if (!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_WALK_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}

void My_Mouse(int button, int state, int x, int y)
{	
	
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
		}
		else if (state == GLUT_UP)
		{	
			printf("%d", timer_enabled);
			printf("Mouse %d is released at (%d, %d)\n", button, x, y);
		}
	}
}


int main(int argc, char *argv[])
{
#ifdef __APPLE__
	// Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("107062125 GPA HW1"); // You cannot use OpenGL functions before this line;
								  // The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
	dumpInfo();
	My_Init();

	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);
	int menu_new = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Walk", menu_timer);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_WALK_START);
	glutAddMenuEntry("Stop", MENU_WALK_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0);
	
	glutMouseFunc(My_Mouse);

	// Enter main event loop.
	glutMainLoop();

	return 0;
}