#include <iostream>
#include <vector>

#include "opengl.hh"
#include "object_vbo.hh"
#include "image_io.hh"
#include "shaders.hh"
#include "obj_loader.hh"
#include "vao.hh"
#include "camera.hh"

#define CAUSTICS_SIZE 512

Vao *floor_vao;
Vao *surface_vao;
Vao *background_vao;

std::vector<Vao *> fish_vaos;
std::vector<Vao *> fish_2_vaos;
std::vector<Vao *> fish_3_vaos;
std::vector<Vao *> fish_4_vaos;
std::vector<Vao *> rock_vaos;
std::vector<Vao *> grass_vaos;
std::vector<Vao *> grass_2_vaos;
std::vector<Vao *> grass_3_vaos;

program *surface_program;
program *background_program;
program *obj_program;

GLuint caustic_idx = 0;
GLuint caustic_begin = 1;
GLuint timer = 1000 / 60;

GLuint blue_texture_id;
GLuint floor_texture_id;

Camera camera(glm::vec3(0.0f, -40.0f, 0.0f),
              glm::vec3(0.0f, 1.0f, 0.0f));

int lastXMouse = 0;
int lastYMouse = 0;
bool firstMouse = true;

int delta_time = 0;
int last_time = 0;

struct key_being_pressed {
    bool w;
    bool a;
    bool d;
    bool s;
} keyBeingPressed;

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
    // Wheel input reports as 3 (scroll up) and 4 (scroll down)
    if (button == 3 || button == 4) {
        if (state == GLUT_UP) return;
        float offset = 1.f;
        if (button == 3)
            camera.processScroll(offset);
        else
            camera.processScroll(-offset);
    }
    else if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN)
            firstMouse = true;
    }
}

void keyboardPressFunctionCallback(unsigned char c, int x, int y) {
    switch (c) {
        // w to move forward.
        case 'w':
            keyBeingPressed.w = true;
            break;
        // s to move backward.
        case 's':
            keyBeingPressed.s = true;
            break;

        // a to straff left.
        case 'a':
            keyBeingPressed.a = true;
            break;
        // d to straff right.
        case 'd':
            keyBeingPressed.d = true;
            break;

        // Escape button to exit.
        case 27:
            exit(0);
            break;
    }
}

void keyboardReleaseFunctionCallback(unsigned char c, int x, int y) {
    switch (c) {
        // w to move forward.
        case 'w':
            keyBeingPressed.w = false;
            break;
        // s to move backward.
        case 's':
            keyBeingPressed.s = false;
            break;

        // a to straff left.
        case 'a':
            keyBeingPressed.a = false;
            break;
        // d to straff right.
        case 'd':
            keyBeingPressed.d = false;
            break;

        // Escape button to exit.
        case 27:
            exit(0);
            break;
    }
}

void keyboardToCamera() {
    float dt = delta_time / 1000.0f;
    if (keyBeingPressed.w)
        camera.processKeyboard(Camera_Movement::FORWARD, dt);
    if (keyBeingPressed.s)
        camera.processKeyboard(Camera_Movement::BACKWARD, dt);
    if (keyBeingPressed.a)
        camera.processKeyboard(Camera_Movement::LEFT, dt);
    if (keyBeingPressed.d)
        camera.processKeyboard(Camera_Movement::RIGHT, dt);
}

void updateCamera() {
    surface_program->use();
    GLuint mv_loc = glGetUniformLocation(surface_program->id, "view_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &camera.view_matrix()[0][0]);TEST_OPENGL_ERROR();

    GLuint cp_loc = glGetUniformLocation(surface_program->id, "cameraPos");TEST_OPENGL_ERROR();
    glUniform3f(cp_loc, camera.position.x, camera.position.y, camera.position.z);TEST_OPENGL_ERROR();

    obj_program->use();
    mv_loc = glGetUniformLocation(obj_program->id, "view_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &camera.view_matrix()[0][0]);TEST_OPENGL_ERROR();

    cp_loc = glGetUniformLocation(obj_program->id, "cameraPos");TEST_OPENGL_ERROR();
    glUniform3f(cp_loc, camera.position.x, camera.position.y, camera.position.z);TEST_OPENGL_ERROR();

    background_program->use();
    mv_loc = glGetUniformLocation(background_program->id, "view_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &camera.view_matrix()[0][0]);TEST_OPENGL_ERROR();

    cp_loc = glGetUniformLocation(background_program->id, "cameraPos");TEST_OPENGL_ERROR();
    glUniform3f(cp_loc, camera.position.x, camera.position.y, camera.position.z);TEST_OPENGL_ERROR();
}

void timerFunc(int value) {
    caustic_idx = (caustic_idx + 1) % CAUSTICS_SIZE;
    glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, caustic_idx + caustic_begin);TEST_OPENGL_ERROR();
    glutPostRedisplay();TEST_OPENGL_ERROR();
    glutTimerFunc(timer, timerFunc, value);TEST_OPENGL_ERROR();
}


void display() {
    int elapsed_time = glutGet(GLUT_ELAPSED_TIME);
    delta_time = elapsed_time - last_time;
    last_time = elapsed_time;

    keyboardToCamera();
    updateCamera();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();

    surface_program->use();

    floor_vao->draw();

    surface_vao->draw();

    obj_program->use();

    GLuint mv_loc = glGetUniformLocation(obj_program->id, "model_matrix");TEST_OPENGL_ERROR();
    auto model_matrix = glm::rotate(glm::translate(glm::vec3(-20.0f, -40.f, -40.f)), glm::radians(90.f), glm::vec3(0,1,0));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: fish_vaos) {
        vao->draw();
    }

    model_matrix = glm::rotate(glm::translate(glm::vec3(30.0f, -42.f, -45.f)), -glm::radians(60.f), glm::vec3(0,1,0));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: fish_vaos) {
        vao->draw();
    }

    model_matrix = glm::rotate(glm::scale(glm::translate(glm::vec3(5.0f, -45.f, -60.f)), glm::vec3(.5f)),
                               -glm::radians(60.f), glm::vec3(0,1,0));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: fish_2_vaos) {
        vao->draw();
    }

    model_matrix = glm::rotate(glm::scale(glm::translate(glm::vec3(0.f, -792.f, 1050.f)), glm::vec3(4.f)),
                               glm::radians(180.f), glm::vec3(0,1,0));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: fish_3_vaos) {
        vao->draw();
    }

    model_matrix = glm::rotate(glm::scale(glm::translate(glm::vec3(0.f, -300.f, 600.f)), glm::vec3(7.f)),
                               glm::radians(90.f), glm::vec3(0,1,0));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: fish_4_vaos) {
        vao->draw();
    }

    model_matrix = glm::scale(glm::translate(glm::vec3(0.0f, -49.5f, -30.f)), glm::vec3(3.0f));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: rock_vaos) {
        vao->draw();
    }

    model_matrix = glm::rotate(glm::scale(glm::translate(glm::vec3(10.0f, -49.5f, -50.f)), glm::vec3(2.5f)),
                               glm::radians(90.f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: rock_vaos) {
        vao->draw();
    }

    model_matrix = glm::scale(glm::translate(glm::vec3(-25.0f, -49.5f, -70.f)), glm::vec3(3.f));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: rock_vaos) {
        vao->draw();
    }

    model_matrix = glm::scale(glm::translate(glm::vec3(15.0f, -49.5f, -30.f)), glm::vec3(10.0f));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: grass_vaos) {
        vao->draw();
    }

    model_matrix = glm::scale(glm::translate(glm::vec3(-30.0f, -49.5f, -50.f)), glm::vec3(20.0f));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: grass_2_vaos) {
        vao->draw();
    }

    model_matrix = glm::scale(glm::translate(glm::vec3(0.0f, -49.5f, -70.f)), glm::vec3(15.0f));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: grass_2_vaos) {
        vao->draw();
    }

    model_matrix = glm::scale(glm::translate(glm::vec3(-15.0f, -49.5f, -55.f)), glm::vec3(20.0f));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: grass_3_vaos) {
        vao->draw();
    }

    model_matrix = glm::scale(glm::translate(glm::vec3(30.0f, -49.5f, -45.f)), glm::vec3(17.0f));
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: grass_3_vaos) {
        vao->draw();
    }

    background_program->use();

    background_vao->draw();

    surface_program->use();

    glutSwapBuffers();
}


void init_glut(int &argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitContextVersion(4,6);
    glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);

    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition ( 100, 100 );
    glutCreateWindow("Underwater Scene");

    glutDisplayFunc(display);
    glutReshapeFunc(window_resize);

    glutTimerFunc(0, timerFunc, 0);

    glutIgnoreKeyRepeat(1);
    glutMotionFunc(motionFunctionCallback);
    glutMouseFunc(mouseFunctionCallback);
    glutKeyboardFunc(keyboardPressFunctionCallback);
    glutKeyboardUpFunc(keyboardReleaseFunctionCallback);

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
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);TEST_OPENGL_ERROR();
    glClearColor(0.0,0.35,0.75,1.0);TEST_OPENGL_ERROR();
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glPixelStorei(GL_PACK_ALIGNMENT,1);
}

void init_object_vbo() {
    GLint vertex_location = glGetAttribLocation(surface_program->id, "position");TEST_OPENGL_ERROR();
    GLint uv_location = glGetAttribLocation(surface_program->id, "uv");TEST_OPENGL_ERROR();

    // Floor
    floor_vao = Vao::make_vao(vertex_location, floor_vbo, floor_texture_id, uv_location, uv_buffer_data);

    // Surface
    surface_vao = Vao::make_vao(vertex_location, surface_vbo, blue_texture_id, uv_location, surface_uv_buffer_data);
}

std::vector<Vao *> init_obj_vao(const char *filename) {
    GLint vertex_location = glGetAttribLocation(obj_program->id, "position");TEST_OPENGL_ERROR();
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
        std::vector<GLfloat> uv;
        std::vector<GLuint> indices;

        for (auto index : mesh.Indices) {
            indices.push_back(index);
        }

        for (auto vertex : mesh.Vertices) {
            vertices.push_back(vertex.Position.X);
            vertices.push_back(vertex.Position.Y);
            vertices.push_back(vertex.Position.Z);

            uv.push_back(vertex.Position.X);
            uv.push_back(vertex.Position.Z);
        }

        obj_vaos.push_back(Vao::make_vao(vertex_location, vertices, texture_id, uv_location, uv, indices));
    }

    glBindVertexArray(0);
    return obj_vaos;
}

void init_background_vao() {
    background_program->use();
    GLint vertex_location = glGetAttribLocation(background_program->id, "position");TEST_OPENGL_ERROR();
    background_vao = Vao::make_vao(vertex_location, background_vbo, blue_texture_id);
}

void init_uniform(GLuint program_id) {
    glm::mat4 projection_matrix = glm::perspective(
            camera.fov_camera,
            16.f/9.f,
            5.f, 17500.0f
    );

    GLuint mp_loc = glGetUniformLocation(program_id, "projection_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mp_loc, 1, GL_FALSE, &projection_matrix[0][0]);TEST_OPENGL_ERROR();

    glm::mat4 model_matrix = glm::mat4(1.0f);

    GLuint m_loc = glGetUniformLocation(program_id, "model_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(m_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
}

void init_textures() {
    int width, height;
    auto *caustics_id = new GLuint[CAUSTICS_SIZE];
    GLint tex_location;
    GLint texture_units, combined_texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &combined_texture_units);
    std::cout << "Limit 1 " <<  texture_units << " limit 2 " << combined_texture_units << std::endl;

    // Floor texture.
    GLubyte *floor_texture = tifo::load_image("../image_test/sand-texture-seamless.tga", &width, &height)->get_buffer();

    std::cout << "floor texture " << width << ", " <<  height << "\n";


    glGenTextures(1, &floor_texture_id);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, floor_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, floor_texture);TEST_OPENGL_ERROR();
    glGenerateMipmap(GL_TEXTURE_2D);

    tex_location = glGetUniformLocation(surface_program->id, "floor_sampler");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 0);TEST_OPENGL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);TEST_OPENGL_ERROR();

    delete floor_texture;
    // Caustics textures.
    glGenTextures(CAUSTICS_SIZE, caustics_id);TEST_OPENGL_ERROR();
    caustic_begin = caustics_id[0];
    char *filename = new char[64];
    for (unsigned i = 0; i < CAUSTICS_SIZE; ++i) {
        std::sprintf(filename, "../image_test/caustics_v3/CausticsRender_%03d.tga", i + 1);
        auto *caustic_texture = tifo::load_gray_image(filename, &width, &height)->get_buffer();

        glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
        glBindTexture(GL_TEXTURE_2D, caustics_id[i]);TEST_OPENGL_ERROR();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, caustic_texture);TEST_OPENGL_ERROR();
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);TEST_OPENGL_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);TEST_OPENGL_ERROR();
        delete caustic_texture;
    }
    delete[] filename;

    glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, caustic_idx + 1);TEST_OPENGL_ERROR();
    tex_location = glGetUniformLocation(surface_program->id, "caustic_sampler");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 1);TEST_OPENGL_ERROR();

    unsigned char blue[3] = {0, 140, 179};

    glGenTextures(1, &blue_texture_id);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, blue_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, blue);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);TEST_OPENGL_ERROR();

    delete[] caustics_id;
}

bool init_shaders() {
    surface_program = program::make_program("../shaders/surface/surface.vert",
                                            "../shaders/surface/surface.frag");
    if (!surface_program->is_ready()) {
        std::cerr << "Surface Program Creation Failed:" << surface_program->get_log();
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

    surface_program->use();
    return true;
}


int main(int argc, char *argv[]) {
    init_glut(argc, argv);
    if (!init_glew())
        std::exit(1);

    init_GL();

    init_shaders();

    init_textures();
    init_object_vbo();
    init_uniform(surface_program->id);

    obj_program->use();
    init_uniform(obj_program->id);

    fish_vaos = init_obj_vao("../image_test/objs/Fish1.obj");
    fish_2_vaos = init_obj_vao("../image_test/objs/Fish2.obj");
    fish_3_vaos = init_obj_vao("../image_test/objs/Manta ray.obj");
    fish_4_vaos = init_obj_vao("../image_test/objs/Shark.obj");
    rock_vaos = init_obj_vao("../image_test/objs/Rock1.obj");
    grass_vaos = init_obj_vao("../image_test/objs/Grass1.obj");
    grass_2_vaos = init_obj_vao("../image_test/objs/Grass2.obj");
    grass_3_vaos = init_obj_vao("../image_test/objs/Grass3.obj");

    background_program->use();
    init_background_vao();
    init_uniform(background_program->id);

    surface_program->use();

    glutMainLoop();
}