#include <iostream>
#include <ctime>

#include "object_vbo.hh"
#include "image_io.hh"
#include "obj_loader.hh"
#include "camera.hh"
#include "boids.hh"
#include "program.hh"

#define CAUSTICS_SIZE 512
#define WAVE_SIZE 1024

// PROGRAMS
program *surface_program;
program *background_program;
program *obj_program;
program *wave_program;
program *caustic_program;
program *depth_program;
program *combine_program;
program *floor_program;
program *blur_program;
program *godray_program;

std::vector<program *> programs;

// VAOS IDS
Vao *floor_vao;
Vao *surface_vao;
Vao *background_vao;

Object *floor_object;

// OBJECTS
std::vector<Object> objects;
std::vector<Boid> boids;

// TEXTURES VARIABLES
GLuint timer = 1000 / 60;

GLuint floor_texture_id;
GLuint depth_texture_id;
GLuint depth_framebuffer_id;
GLuint caustic_texture_id;
GLuint caustic_blurred_texture_id;
GLuint caustic_framebuffer_id;
GLuint godray_texture_id;
GLuint godray_framebuffer_id;
GLuint color_texture_id;
GLuint color_framebuffer_id;

// CAMERA VARIABLES
Camera camera(glm::vec3(0.f, -40.f, 0.f),
              glm::vec3(0.f, 1.f, 0.f));

int lastXMouse = 0;
int lastYMouse = 0;
bool firstMouse = true;


// GLUT HANDLING FUNCTIONS (CAMERA + TIMER)
void window_resize(int width, int height) {
    glViewport(0,0,width,height);TEST_OPENGL_ERROR();
}

void motionFunctionCallback(int x, int y) {
    if (firstMouse) {
        lastXMouse = x;
        lastYMouse = y;
        firstMouse = false;
    }

    int xoffset = x - lastXMouse;
    int yoffset = lastYMouse - y;
    lastXMouse = x;
    lastYMouse = y;
    camera.processMouse(xoffset, yoffset);
}

void mouseFunctionCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN)
            firstMouse = true;
    }
}

void keyboardPressFunctionCallback(unsigned char c, int x, int y) {
    switch (c) {
        // w to move forward.
        case 'w':
            camera.processKeyboard(Input_Keys::FORWARD, true);
            break;
        // s to move backward.
        case 's':
            camera.processKeyboard(Input_Keys::BACKWARD, true);
            break;

        // a to straff left.
        case 'a':
            camera.processKeyboard(Input_Keys::LEFT, true);
            break;
        // d to straff right.
        case 'd':
            camera.processKeyboard(Input_Keys::RIGHT, true);
            break;

        // Escape button to exit.
        case 27:
            exit(0);

        default:
            return;
    }
}

void keyboardSpecialPressFunctionCallback(int c, int x, int y) {
    // shift to move faster.
    if (c == GLUT_KEY_SHIFT_L || c == GLUT_KEY_SHIFT_R)
        camera.processKeyboard(Input_Keys::SHIFT, true);
    keyboardPressFunctionCallback(c, x, y);
}

void keyboardReleaseFunctionCallback(unsigned char c, int x, int y) {
    switch (c) {
        // w to move forward.
        case 'w':
            camera.processKeyboard(Input_Keys::FORWARD, false);
            break;
            // s to move backward.
        case 's':
            camera.processKeyboard(Input_Keys::BACKWARD, false);
            break;

            // a to straff left.
        case 'a':
            camera.processKeyboard(Input_Keys::LEFT, false);
            break;
            // d to straff right.
        case 'd':
            camera.processKeyboard(Input_Keys::RIGHT, false);
            break;

        default:
            return;
    }
}

void keyboardSpecialReleaseFunctionCallback(int c, int x, int y) {
    // shift to move faster.
    if (c == GLUT_KEY_SHIFT_L || c == GLUT_KEY_SHIFT_R)
        camera.processKeyboard(Input_Keys::SHIFT, false);
    keyboardReleaseFunctionCallback(c, x, y);
}

void updateCamera() {
    camera.update_camera(programs, (float) glutGet(GLUT_ELAPSED_TIME));
}

void animate_waves() {
    GLint anim_time_location;
    wave_program->use();
    anim_time_location =
            glGetUniformLocation(wave_program->id, "anim_time");TEST_OPENGL_ERROR();
    glUniform1f(anim_time_location, 0.005f * (float) glutGet(GLUT_ELAPSED_TIME));TEST_OPENGL_ERROR();
}

void timerFunc(int value) {
    animate_waves();
    glutPostRedisplay();TEST_OPENGL_ERROR();
    glutTimerFunc(timer, timerFunc, value);TEST_OPENGL_ERROR();
}


// DISPLAY FUNCTIONS
void object_draw(GLuint program_id, bool move = true, GLuint tex = -1) {
    GLint mv_loc = glGetUniformLocation(program_id, "model_matrix");TEST_OPENGL_ERROR();
    GLint vert_loc = glGetAttribLocation(program_id, "position");TEST_OPENGL_ERROR();
    GLint uv_loc = glGetAttribLocation(program_id, "uv");TEST_OPENGL_ERROR();
    GLint normal_loc = glGetAttribLocation(program_id, "normal");TEST_OPENGL_ERROR();

    for (auto& obj : objects)
        obj.draw(mv_loc, vert_loc, uv_loc, normal_loc, tex);

    for (auto &boid : boids) {
        if (move) {
            boid.move(boids, objects, 2 * (float) glutGet(GLUT_ELAPSED_TIME));
        }
        boid.draw(mv_loc, vert_loc, uv_loc, normal_loc, tex);
    }
}

void display() {
    updateCamera();
    glDisable(GL_BLEND);TEST_OPENGL_ERROR();
    glEnable(GL_DEPTH_TEST);TEST_OPENGL_ERROR();
    glClearColor(0, 0, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();

    // Update the surface waves.
    wave_program->use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, surface_vao->vbo_ids[0]);TEST_OPENGL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, surface_vao->vbo_ids[2]);TEST_OPENGL_ERROR();
    glDispatchCompute((WAVE_SIZE + 7) / 8, (WAVE_SIZE + 7) / 8, 1);TEST_OPENGL_ERROR();
    glMemoryBarrier(GL_ALL_BARRIER_BITS);TEST_OPENGL_ERROR();

    // Render the environment map
    glBindFramebuffer(GL_FRAMEBUFFER, depth_framebuffer_id);TEST_OPENGL_ERROR();

    glViewport(0, 0, WAVE_SIZE, WAVE_SIZE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();

    depth_program->use();

    GLint vert_loc = glGetAttribLocation(depth_program->id, "position");TEST_OPENGL_ERROR();
    GLint mv_loc = glGetUniformLocation(depth_program->id, "model_matrix");TEST_OPENGL_ERROR();
    GLint uv_loc = glGetAttribLocation(depth_program->id, "uv");TEST_OPENGL_ERROR();
    GLint normal_loc = glGetAttribLocation(depth_program->id, "normal");TEST_OPENGL_ERROR();
    floor_object->draw(mv_loc, vert_loc, uv_loc, normal_loc);
    object_draw(depth_program->id, true);

    // Render the color buffer
    glBindFramebuffer(GL_FRAMEBUFFER, color_framebuffer_id);
    // fixed viewport since we don't want to resize the framebuffer texture everytime.
    glViewport(0, 0, 1920, 1080);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();

    floor_program->use();
    floor_vao->draw();

    surface_program->use();
    surface_vao->draw();

    obj_program->use();
    object_draw(obj_program->id, false);

    background_program->use();

    background_vao->draw();

    // Render the godrays frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, godray_framebuffer_id);TEST_OPENGL_ERROR();
    glClearColor(0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);TEST_OPENGL_ERROR();
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(5.f);
    // Blending functions, additive for the color and min for the alpha since we want to keep the minimal depth (unused for godrays).
    glBlendEquationSeparate(GL_FUNC_ADD, GL_MIN);TEST_OPENGL_ERROR();
    glBlendFuncSeparate(GL_SRC_COLOR, GL_ONE, GL_ONE, GL_ZERO);TEST_OPENGL_ERROR();

    godray_program->use();

    vert_loc = glGetAttribLocation(godray_program->id, "position");TEST_OPENGL_ERROR();
    uv_loc = glGetAttribLocation(godray_program->id, "uv");TEST_OPENGL_ERROR();
    normal_loc = glGetAttribLocation(godray_program->id, "normal");TEST_OPENGL_ERROR();
    surface_vao->draw(vert_loc, uv_loc, normal_loc, depth_texture_id);


    // Render the caustic frame buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, caustic_framebuffer_id);TEST_OPENGL_ERROR();
    glClear(GL_COLOR_BUFFER_BIT);TEST_OPENGL_ERROR();

    glEnable(GL_PROGRAM_POINT_SIZE);TEST_OPENGL_ERROR();

    // Blending functions, additive for the color and min for the alpha since we want to keep the minimal depth.
    glBlendEquationSeparate(GL_FUNC_ADD, GL_MIN);TEST_OPENGL_ERROR();
    glBlendFuncSeparate(GL_SRC_COLOR, GL_ONE, GL_ONE, GL_ZERO);TEST_OPENGL_ERROR();

    caustic_program->use();

    vert_loc = glGetAttribLocation(caustic_program->id, "position");TEST_OPENGL_ERROR();
    uv_loc = glGetAttribLocation(caustic_program->id, "uv");TEST_OPENGL_ERROR();
    normal_loc = glGetAttribLocation(caustic_program->id, "normal");TEST_OPENGL_ERROR();
    surface_vao->draw(vert_loc, uv_loc, normal_loc, depth_texture_id);

    // Blur caustics and godrays (two gaussian blur passes).
    glClearTexImage(caustic_blurred_texture_id, 0, GL_RGBA, GL_FLOAT, nullptr);TEST_OPENGL_ERROR();
    blur_program->use();
    glBindImageTexture(0, caustic_texture_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(1, caustic_blurred_texture_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute((1920 + 7) / 8, (1080 + 7) / 8, 1);TEST_OPENGL_ERROR();
    glMemoryBarrier(GL_ALL_BARRIER_BITS);TEST_OPENGL_ERROR();

    glBindImageTexture(0, caustic_blurred_texture_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(1, caustic_texture_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute((1920 + 7) / 8, (1080 + 7) / 8, 1);TEST_OPENGL_ERROR();
    glMemoryBarrier(GL_ALL_BARRIER_BITS);TEST_OPENGL_ERROR();

    glBindImageTexture(0, godray_texture_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(1, caustic_blurred_texture_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute((1920 + 7) / 8, (1080 + 7) / 8, 1);TEST_OPENGL_ERROR();
    glMemoryBarrier(GL_ALL_BARRIER_BITS);TEST_OPENGL_ERROR();

    glBindImageTexture(0, caustic_blurred_texture_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(1, godray_texture_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute((1920 + 7) / 8, (1080 + 7) / 8, 1);TEST_OPENGL_ERROR();
    glMemoryBarrier(GL_ALL_BARRIER_BITS);TEST_OPENGL_ERROR();

    // Draw the display buffer by combining the textures of all other frame buffers.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);TEST_OPENGL_ERROR();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    combine_program->use();

    vert_loc = glGetAttribLocation(combine_program->id, "position");TEST_OPENGL_ERROR();
    uv_loc = glGetAttribLocation(combine_program->id, "uv");TEST_OPENGL_ERROR();
    normal_loc = glGetAttribLocation(combine_program->id, "normal");TEST_OPENGL_ERROR();
    mv_loc = glGetUniformLocation(combine_program->id, "model_matrix");TEST_OPENGL_ERROR();

    floor_object->draw(mv_loc, vert_loc, uv_loc, normal_loc, -2);
    background_vao->draw(vert_loc, uv_loc, normal_loc, -2);
    surface_vao->draw(vert_loc, uv_loc, normal_loc, -2);

    object_draw(combine_program->id, false, -2);

    glutSwapBuffers();
}


// INIT FUNCTIONS
void init_glut(int &argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitContextVersion(4,5);
    glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);

    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition ( 100, 100 );
    glutCreateWindow("Underwater Scene");

    glutDisplayFunc(display);
    glutReshapeFunc(window_resize);

    glutTimerFunc(0, timerFunc, 0);

    glutIgnoreKeyRepeat(0);
    glutMotionFunc(motionFunctionCallback);
    glutMouseFunc(mouseFunctionCallback);
    glutKeyboardFunc(keyboardPressFunctionCallback);
    glutSpecialFunc(keyboardSpecialPressFunctionCallback);
    glutKeyboardUpFunc(keyboardReleaseFunctionCallback);
    glutSpecialUpFunc(keyboardSpecialReleaseFunctionCallback);

    glutReportErrors();
}

bool init_glew() {
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        std::cerr << " Error while initializing glew: " << glewGetErrorString(error) << std::endl;
        return false;
    }
    return true;
}

void init_GL() {
    glEnable(GL_DEPTH_TEST);TEST_OPENGL_ERROR();
    //glEnable(GL_BLEND);TEST_OPENGL_ERROR();
    //glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);TEST_OPENGL_ERROR();
    glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );TEST_OPENGL_ERROR();

    glClearColor(0.0,0.0,0.0,0.0);TEST_OPENGL_ERROR();
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glPixelStorei(GL_PACK_ALIGNMENT,1);
}

// Initialize surface grid according to the wave size parameter.
void init_surface_vao() {

    int N = WAVE_SIZE;
    std::vector<GLfloat> surface_vertices;
    std::vector<GLfloat> surface_uvs;
    std::vector<GLuint> surface_indices;
    std::vector<GLfloat> surface_normals;

    float xmin = -1000;
    float xmax = 1000;

    for (int i = 0; i < N; i++) {
        float a = i / (N - 1.f);
        GLfloat x = (1 - a) * xmin + a * xmax;
        for (int j = 0; j < N; j++) {
            float b = j / (N - 1.f);
            GLfloat y = (1 - b) * xmin + b * xmax;
            surface_vertices.push_back(x);
            surface_vertices.push_back(0.f);
            surface_vertices.push_back(y);
            surface_uvs.push_back(a);
            surface_uvs.push_back(b);
            surface_normals.push_back(0.f);
            surface_normals.push_back(1.f);
            surface_normals.push_back(0.f);
        }
    }

    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N - 1; j++) {
            int index = i * N + j;
            surface_indices.push_back(index);
            surface_indices.push_back(index + N);
            surface_indices.push_back(index + N + 1);
            surface_indices.push_back(index + N + 1);
            surface_indices.push_back(index + 1);
            surface_indices.push_back(index);
        }
    }
    GLint vertex_location = glGetAttribLocation(surface_program->id, "position");TEST_OPENGL_ERROR();
    GLint uv_location = glGetAttribLocation(surface_program->id, "uv");TEST_OPENGL_ERROR();
    GLint normal_location = glGetAttribLocation(surface_program->id, "normal");TEST_OPENGL_ERROR();
    surface_vao = Vao::make_vao(vertex_location, surface_vertices, -1, uv_location, surface_uvs, surface_indices, normal_location, surface_normals);
}

void init_object_vbo() {

    // Surface
    surface_program->use();
    init_surface_vao();

    // Floor
    floor_program->use();
    GLint vertex_location = glGetAttribLocation(floor_program->id, "position");TEST_OPENGL_ERROR();
    GLint uv_location = glGetAttribLocation(floor_program->id, "uv");TEST_OPENGL_ERROR();

    floor_vao = Vao::make_vao(vertex_location, floor_vbo, floor_texture_id, uv_location, uv_buffer_data);
    floor_object = new Object(std::vector<Vao *>(1, floor_vao), glm::vec3(0, 0, 0), 0, 0, 0, 1.f, 1.f);
}

// Load object files.
std::vector<Vao *> init_obj_vao(const char *filename) {
    GLint vertex_location = glGetAttribLocation(obj_program->id, "position");TEST_OPENGL_ERROR();
    GLint normal_location = glGetAttribLocation(obj_program->id, "normal");TEST_OPENGL_ERROR();
    GLint uv_location = glGetAttribLocation(obj_program->id, "uv");TEST_OPENGL_ERROR();
    objl::Loader loader;
    bool is_loaded = loader.LoadFile(filename);

    if (!is_loaded) {
        std::cerr << "Could not load obj or mtl file";
        exit(1);
    }

    std::vector<Vao *> obj_vaos;

    for (size_t i = 0; i < loader.LoadedMeshes.size(); ++i) {
        auto mesh = loader.LoadedMeshes[i];
        GLuint texture_id = -1;

        if (!loader.LoadedMaterials.empty()) {
            auto texture = mesh.MeshMaterial;

            glGenTextures(1, &texture_id);TEST_OPENGL_ERROR();
            glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
            glBindTexture(GL_TEXTURE_2D, texture_id);TEST_OPENGL_ERROR();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, &texture.Kd);TEST_OPENGL_ERROR();
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);TEST_OPENGL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);TEST_OPENGL_ERROR();
        }

        std::vector<GLfloat> vertices;
        std::vector<GLfloat> normals;
        std::vector<GLfloat> uv;
        std::vector<GLuint> indices;

        for (auto index : mesh.Indices) {
            indices.push_back(index);
        }

        for (auto vertex : mesh.Vertices) {
            vertices.push_back(vertex.Position.X);
            vertices.push_back(vertex.Position.Y);
            vertices.push_back(vertex.Position.Z);
            normals.push_back(vertex.Normal.X);
            normals.push_back(vertex.Normal.Y);
            normals.push_back(vertex.Normal.Z);

            uv.push_back(vertex.Position.X);
            uv.push_back(vertex.Position.Z);
        }

        obj_vaos.push_back(Vao::make_vao(vertex_location, vertices, texture_id, uv_location, uv, indices, normal_location, normals));
    }

    glBindVertexArray(0);
    return obj_vaos;
}

// Init boids and objects with random position.
void init_objects() {

    std::vector<Vao *> fish_vaos = init_obj_vao("../image_test/objs/Fish1.obj");
    std::vector<Vao *> fish_2_vaos = init_obj_vao("../image_test/objs/Fish2.obj");
    // std::vector<Vao *> fish_3_vaos = init_obj_vao("../image_test/objs/Manta ray.obj");
    // std::vector<Vao *> fish_4_vaos = init_obj_vao("../image_test/objs/Shark.obj");
    std::vector<Vao *> rock_vaos = init_obj_vao("../image_test/objs/Rock1.obj");
    std::vector<Vao *> grass_vaos = init_obj_vao("../image_test/objs/Grass1.obj");
    std::vector<Vao *> grass_2_vaos = init_obj_vao("../image_test/objs/Grass2.obj");
    std::vector<Vao *> grass_3_vaos = init_obj_vao("../image_test/objs/Grass3.obj");

    for (size_t i = 0; i < 100; ++i) {
        boids.emplace_back(
                Boid(objects, fish_vaos,
                     glm::vec3(-100.f, -80.f, -100.f),
                     glm::vec3(100.f, -140.f, 100.f),
                     -45.f, 45.f,
                     0.f, 360.f,
                     0.f, 0.f,
                     1.f, 3.f,
                     .1f, 1.f, 8.f
                ));

        boids.emplace_back(
                Boid(objects, fish_2_vaos,
                     glm::vec3(-100.f, -80.f, -100.f),
                     glm::vec3(100.f, -140.f, 100.f),
                     -45.f, 45.f,
                     0.f, 360.f,
                     0.f, 0.f,
                     1.f, 3.f,
                     .1f, 1.f, 10.f
                ));

        objects.emplace_back(
                Object(objects, rock_vaos,
                       glm::vec3(-500.f, -149.5f, -500.f),
                       glm::vec3(500.f, -149.5f, 10.f),
                       0.f, 0.f,
                       0.f, 360.f,
                       0.f, 0.f,
                       1.f, 10.f, 5.f
                ));

        objects.emplace_back(
                Object(objects, grass_vaos,
                       glm::vec3(-500.f, -149.5f, -500.f),
                       glm::vec3(500.f, -149.5f, 10.f),
                       0.f, 0.f,
                       0.f, 360.f,
                       0.f, 0.f,
                       10.f, 50.f, .5f
                ));

        objects.emplace_back(
                Object(objects, grass_2_vaos,
                       glm::vec3(-500.f, -149.5f, -500.f),
                       glm::vec3(500.f, -149.5f, 10.f),
                       0.f, 0.f,
                       0.f, 360.f,
                       0.f, 0.f,
                       10.f, 50.f, .5f
                ));

        objects.emplace_back(
                Object(objects, grass_3_vaos,
                       glm::vec3(-500.f, -149.5f, -500.f),
                       glm::vec3(500.f, -149.5f, 10.f),
                       0.f, 0.f,
                       0.f, 360.f,
                       0.f, 0.f,
                       10.f, 50.f, .5f
                ));
    }
}

void init_background_vao() {
    background_program->use();
    GLint vertex_location = glGetAttribLocation(background_program->id, "position");TEST_OPENGL_ERROR();
    background_vao = Vao::make_vao(vertex_location, background_vbo);
}

void init_uniform(GLuint program_id) {
    glm::mat4 projection_matrix = camera.projection_matrix();

    GLint mp_loc = glGetUniformLocation(program_id, "projection_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mp_loc, 1, GL_FALSE, &projection_matrix[0][0]);TEST_OPENGL_ERROR();

    glm::mat4 model_matrix = glm::mat4(1.0f);

    GLint m_loc = glGetUniformLocation(program_id, "model_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(m_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
}

void init_deferred_uniform(GLuint program_id) {
    // Initializing light uniforms.
    glm::mat4 projection_matrix = glm::perspective(M_PI_2f32 * 0.8f, 1.f, 5.f, 1000.f);

    GLint mp_loc = glGetUniformLocation(program_id, "projection_matrix_light");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mp_loc, 1, GL_FALSE, &projection_matrix[0][0]);TEST_OPENGL_ERROR();

    auto pos = glm::vec3(0.f, 500.f, 0.f);
    auto view_matrix = glm::lookAt(pos, glm::vec3(0.1f, 20.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
    GLint v_loc = glGetUniformLocation(program_id, "view_matrix_light");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(v_loc, 1, GL_FALSE, &view_matrix[0][0]);TEST_OPENGL_ERROR();


    glm::mat4 model_matrix = glm::mat4(1.0f);

    GLint m_loc = glGetUniformLocation(program_id, "model_matrix_light");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(m_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();

    GLint cp_loc = glGetUniformLocation(program_id, "lightPos");TEST_OPENGL_ERROR();
    glUniform3f(cp_loc, pos.x, pos.y, pos.z);TEST_OPENGL_ERROR();

    GLint tex_location = glGetUniformLocation(program_id, "depth_texture");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 0);TEST_OPENGL_ERROR();
}

// Init textures and frame buffers.
void init_textures() {
    int width, height;
    GLint tex_location;
    GLint texture_units, combined_texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &combined_texture_units);
    std::cout << "Limit 1 " <<  texture_units << " limit 2 " << combined_texture_units << std::endl;

    // Floor texture.
    GLubyte *floor_texture = tifo::load_image("../image_test/sand-texture-seamless.tga", &width, &height)->get_buffer();

    std::cout << "floor texture " << width << ", " <<  height << "\n";

    // Init floor texture.
    floor_program->use();
    glGenTextures(1, &floor_texture_id);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, floor_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, floor_texture);TEST_OPENGL_ERROR();
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);TEST_OPENGL_ERROR();

    tex_location = glGetUniformLocation(floor_program->id, "texture_sampler");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 0);TEST_OPENGL_ERROR();

    delete floor_texture;

    // Init depth framebuffer & texture.
    glGenFramebuffers(1, &depth_framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_framebuffer_id);

    glGenTextures(1, &depth_texture_id);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, depth_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, WAVE_SIZE, WAVE_SIZE, 0, GL_RGBA, GL_FLOAT,nullptr);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);TEST_OPENGL_ERROR();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depth_texture_id, 0);TEST_OPENGL_ERROR();
    // Depth render buffer for depth test.
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WAVE_SIZE, WAVE_SIZE);TEST_OPENGL_ERROR();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);TEST_OPENGL_ERROR();
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Caustic framebuffers & textures.
    glGenFramebuffers(1, &caustic_framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, caustic_framebuffer_id);

    // Caustic framebuffer
    glGenTextures(1, &caustic_texture_id);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_RECTANGLE, caustic_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16F_ARB, 1920, 1080, 0, GL_RGBA, GL_FLOAT,nullptr);TEST_OPENGL_ERROR();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, caustic_texture_id, 0);TEST_OPENGL_ERROR();

    // Godray framebuffers & textures.
    glGenFramebuffers(1, &godray_framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, godray_framebuffer_id);

    glGenTextures(1, &godray_texture_id);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_RECTANGLE, godray_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16F_ARB, 1920, 1080, 0, GL_RGBA, GL_FLOAT,nullptr);TEST_OPENGL_ERROR();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, godray_texture_id, 0);TEST_OPENGL_ERROR();

    // Color framebuffers & textures.
    glGenFramebuffers(1, &color_framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, color_framebuffer_id);

    glGenTextures(1, &color_texture_id);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_RECTANGLE, color_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16F_ARB, 1920, 1080, 0, GL_RGBA, GL_FLOAT,nullptr);TEST_OPENGL_ERROR();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture_id, 0);TEST_OPENGL_ERROR();
    // Depth render buffer pour le color buffer.
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16_ARB, 1920, 1080);TEST_OPENGL_ERROR();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);TEST_OPENGL_ERROR();

    // texture intermediaire pour le blur des caustiques et godrays.
    glGenTextures(1, &caustic_blurred_texture_id);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_RECTANGLE, caustic_blurred_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16F_ARB, 1920, 1080, 0, GL_RGBA, GL_FLOAT,nullptr);TEST_OPENGL_ERROR();

    // On attache les differentes textures au combine program.
    combine_program->use();
    glActiveTexture(GL_TEXTURE2);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_RECTANGLE, color_texture_id);TEST_OPENGL_ERROR();
    tex_location = glGetUniformLocation(combine_program->id, "color_texture");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 2);
    glActiveTexture(GL_TEXTURE3);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_RECTANGLE, caustic_texture_id);TEST_OPENGL_ERROR();
    tex_location = glGetUniformLocation(combine_program->id, "caustic_texture");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 3);
    glActiveTexture(GL_TEXTURE4);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_RECTANGLE, godray_texture_id);TEST_OPENGL_ERROR();
    tex_location = glGetUniformLocation(combine_program->id, "godray_texture");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 4);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);TEST_OPENGL_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);TEST_OPENGL_ERROR();
}

// On initialise les differents shaders.
bool init_shaders() {
    surface_program = program::make_program("../shaders/surface/surface.vert",
                                            "../shaders/surface/surface.frag");
    if (!surface_program->is_ready()) {
        std::cerr << "Surface Program Creation Failed:" << surface_program->get_log();
        return false;
    }

    wave_program = new program;

    auto wave_shader = shader::make_shader("../shaders/surface/wave.shd", GL_COMPUTE_SHADER);
    if (!wave_shader->is_ready()) {
        std::cerr << "Wave shader fail: " << wave_shader->get_log() << '\n';
        return false;
    }
    wave_program->attach(*wave_shader);
    wave_program->link();
    delete wave_shader;

    if (!wave_program->is_ready()) {
        std::cerr << "Wave program Creation Failed:\n" << wave_program->get_log() << '\n';
        return false;
    }

    blur_program = new program;

    auto blur_shader = shader::make_shader("../shaders/surface/blur.shd", GL_COMPUTE_SHADER);
    if (!blur_shader->is_ready()) {
        std::cerr << "Blur shader fail: " << blur_shader->get_log() << '\n';
        return false;
    }
    blur_program->attach(*blur_shader);
    blur_program->link();
    delete blur_shader;

    if (!blur_program->is_ready()) {
        std::cerr << "Blur program Creation Failed:\n" << blur_program->get_log() << '\n';
        return false;
    }


    background_program = program::make_program("../shaders/background/background.vert",
                                               "../shaders/background/background.frag");
    if (!background_program->is_ready()) {
        std::cerr << "Background Program Creation Failed:\n" << background_program->get_log() << '\n';
        return false;
    }

    obj_program = program::make_program("../shaders/object/object.vert",
                                        "../shaders/object/object.frag");
    if (!obj_program->is_ready()) {
        std::cerr << "Obj Program Creation Failed:" << obj_program->get_log();
        return false;
    }

    floor_program = program::make_program("../shaders/object/object.vert",
                                        "../shaders/object/object.frag");
    if (!floor_program->is_ready()) {
        std::cerr << "Floor Program Creation Failed:" << floor_program->get_log();
        return false;
    }

    depth_program = program::make_program("../shaders/surface/depthmap.vert",
                                          "../shaders/surface/depthmap.frag");

    if (!depth_program->is_ready()) {
        std::cerr << "Depth Program Creation Failed:" << depth_program->get_log();
        return false;
    }

    caustic_program = program::make_program("../shaders/caustics/caustic.vert",
                                          "../shaders/caustics/caustic.frag",
                                          "../shaders/caustics/caustic.shd");


    if (!caustic_program->is_ready()) {
        std::cerr << "Caustic Program Creation Failed:" << caustic_program->get_log();
        return false;
    }

    godray_program = program::make_program("../shaders/caustics/caustic.vert",
                                            "../shaders/caustics/godray.frag",
                                            "../shaders/caustics/godray.shd");


    if (!godray_program->is_ready()) {
        std::cerr << "Godray Program Creation Failed:" << godray_program->get_log();
        return false;
    }

    combine_program = program::make_program("../shaders/surface/combine.vert",
                                            "../shaders/surface/combine.frag");

    if (!combine_program->is_ready()) {
        std::cerr << "Combine Program Creation Failed:" << combine_program->get_log();
        return false;
    }

    surface_program->use();
    return true;
}

// MAIN
int main(int argc, char *argv[]) {
    // Initialize random seed, based on exec time.
    srand((unsigned) time(nullptr));

    // Initialize glut and glew functions.
    init_glut(argc, argv);
    if (!init_glew())
        std::exit(1);

    init_GL();

    init_shaders();

    surface_program->use();
    init_uniform(surface_program->id);

    combine_program->use();
    init_uniform(combine_program->id);

    floor_program->use();
    init_uniform(floor_program->id);
    GLint uv_mul = glGetUniformLocation(floor_program->id, "uv_multiplier");TEST_OPENGL_ERROR();
    glUniform2fv(uv_mul, 1, &glm::vec2(320, 320)[0]);

    init_textures();
    init_object_vbo();


    obj_program->use();
    init_uniform(obj_program->id);

    init_objects();

    background_program->use();
    init_background_vao();
    init_uniform(background_program->id);

    depth_program->use();
    init_deferred_uniform(depth_program->id);

    caustic_program->use();
    init_deferred_uniform(caustic_program->id);
    init_uniform(caustic_program->id);

    godray_program->use();
    init_deferred_uniform(godray_program->id);
    init_uniform(godray_program->id);

    // Liste des programmes qui vont avoir besoin des matrices de camera en temps reel.
    programs.emplace_back(surface_program);
    programs.emplace_back(floor_program);
    programs.emplace_back(obj_program);
    programs.emplace_back(background_program);
    programs.emplace_back(caustic_program);
    programs.emplace_back(godray_program);
    programs.emplace_back(combine_program);

    glutMainLoop();
}