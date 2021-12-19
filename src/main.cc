#include <iostream>
#include <ctime>

#include "object_vbo.hh"
#include "image_io.hh"
#include "obj_loader.hh"
#include "camera.hh"
#include "boids.hh"

#define CAUSTICS_SIZE 512

// PROGRAMS
program *surface_program;
program *background_program;
program *obj_program;

std::vector<program *> programs;

// VAOS IDS
Vao *floor_vao;
Vao *surface_vao;
Vao *background_vao;

// OBJECTS
std::vector<Object> objects;
std::vector<Boid> boids;

// TEXTURES VARIABLES
GLuint caustic_idx = 0;
GLuint caustic_begin = 1;
GLuint timer = 1000 / 60;

GLuint blue_texture_id;
GLuint floor_texture_id;

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
}

void updateCamera() {
    camera.update_camera(programs, (float) glutGet(GLUT_ELAPSED_TIME));
}

void timerFunc(int value) {
    caustic_idx = (caustic_idx + 1) % CAUSTICS_SIZE;
    glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, caustic_idx + caustic_begin);TEST_OPENGL_ERROR();
    glutPostRedisplay();TEST_OPENGL_ERROR();
    glutTimerFunc(timer, timerFunc, value);TEST_OPENGL_ERROR();
}


// DISPLAY FUNCTIONS
void object_display() {
    obj_program->use();

    GLint mv_loc = glGetUniformLocation(obj_program->id, "model_matrix");TEST_OPENGL_ERROR();

    for (auto& obj : objects)
        obj.draw(mv_loc);

    for (auto &boid : boids) {
        boid.move(boids, (float) glutGet(GLUT_ELAPSED_TIME));
        boid.draw(mv_loc);
    }
}

void display() {
    updateCamera();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();

    surface_program->use();

    floor_vao->draw();
    surface_vao->draw();

    object_display();

    background_program->use();

    background_vao->draw();

    surface_program->use();

    glutSwapBuffers();
}


// INIT FUNCTIONS
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
                     glm::vec3(-100.f, -40.f, -100.f),
                     glm::vec3(100.f, -40.f, 100.f),
                     .1f, 1.f, 0.f, 360.f,
                     glm::vec3(0.f, 1.f, 0.f), 5.f,
                     1.f, 5.f
                ));
    }

        /* objects.emplace_back(
                Object(objects, fish_2_vaos,
                       glm::vec3(-500.f, -49.5f, -500.f),
                       glm::vec3(500.f, -5.f, 500.f),
                       .1f, 1.f, 0.f, 360.f,
                       glm::vec3(0.f, 1.f, 0.f), 5.f
                ));

        objects.emplace_back(
                Object(objects, rock_vaos,
                       glm::vec3(-500.f, -49.5f, -500.f),
                       glm::vec3(500.f, -49.5f, 10.f),
                       5.f, 10.f, 0.f, 360.f,
                       glm::vec3(0.f, 1.f, 0.f), 5.f
                ));

        objects.emplace_back(
                Object(objects, grass_vaos,
                       glm::vec3(-500.f, -49.5f, -500.f),
                       glm::vec3(500.f, -49.5f, 10.f),
                       15.f, 50.f, 0.f, 360.f,
                       glm::vec3(0.f, 1.f, 0.f), 1.f
                ));

        objects.emplace_back(
                Object(objects, grass_2_vaos,
                       glm::vec3(-500.f, -49.5f, -500.f),
                       glm::vec3(500.f, -49.5f, 10.f),
                       15.f, 50.f, 0.f, 360.f,
                       glm::vec3(0.f, 1.f, 0.f), 1.f
                ));

        objects.emplace_back(
                Object(objects, grass_3_vaos,
                       glm::vec3(-500.f, -49.5f, -500.f),
                       glm::vec3(500.f, -49.5f, 10.f),
                       15.f, 50.f, 0.f, 360.f,
                       glm::vec3(0.f, 1.f, 0.f), 1.f
                       ));
    } */

    /* boids.emplace_back(
            Boid(objects, fish_vaos,
                 glm::vec3(-10.f, -40.f, -10.f),
                 glm::vec3(-10.f, -40.f, -10.f),
                 .5f, .5f, 90.f, 90.f,
                 glm::vec3(0.f, 1.f, 0.f), 5.f,
                 1.f, 1.f
            ));

    boids.emplace_back(
            Boid(objects, fish_vaos,
                 glm::vec3(0.f, -40.f, -10.f),
                 glm::vec3(0.f, -40.f, -10.f),
                 .5f, .5f, 180.f, 180.f,
                 glm::vec3(0.f, 1.f, 0.f), 5.f,
                 1.f, 1.f
            ));

    boids.emplace_back(
            Boid(objects, fish_vaos,
                 glm::vec3(10.f, -40.f, -5.f),
                 glm::vec3(10.f, -40.f, -5.f),
                 .5f, .5f, 270.f, 270.f,
                 glm::vec3(0.f, 1.f, 0.f), 5.f,
                 1.f, 1.f
            )); */
}

void init_background_vao() {
    background_program->use();
    GLint vertex_location = glGetAttribLocation(background_program->id, "position");TEST_OPENGL_ERROR();
    background_vao = Vao::make_vao(vertex_location, background_vbo, blue_texture_id);
}

void init_uniform(GLuint program_id) {
    glm::mat4 projection_matrix = camera.projection_matrix();

    GLint mp_loc = glGetUniformLocation(program_id, "projection_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mp_loc, 1, GL_FALSE, &projection_matrix[0][0]);TEST_OPENGL_ERROR();

    glm::mat4 model_matrix = glm::mat4(1.0f);

    GLint m_loc = glGetUniformLocation(program_id, "model_matrix");TEST_OPENGL_ERROR();
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

    init_textures();
    init_object_vbo();
    init_uniform(surface_program->id);

    obj_program->use();
    init_uniform(obj_program->id);

    init_objects();

    background_program->use();
    init_background_vao();
    init_uniform(background_program->id);

    // Initialize programs vector.
    programs.emplace_back(surface_program);
    programs.emplace_back(obj_program);
    programs.emplace_back(background_program);

    surface_program->use();

    glutMainLoop();
}