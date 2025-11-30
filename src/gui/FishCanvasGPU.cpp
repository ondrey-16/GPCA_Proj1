//
// Created by ondrey on 26.11.2025.
//

#include "FishCanvasGPU.h"
#include <QOpenGLFunctions>

// Tworzy obiekt Timer'a, a także inicjalizuje pamięć wykorzystywaną do obliczeń na GPU. Liczy czas działania
// inicjalizacji
FishCanvasGPU::FishCanvasGPU(int N, QWidget *parent) : FishCanvas(N, parent) {
    auto start = std::chrono::high_resolution_clock::now();
    cudaError_t status = initializeMemoryOnGPU(&shoalPtrs, &initShoal, N, canvasWidth, canvasHeight);
    if (status != cudaSuccess) {
        std::cerr << "Initializing data error " << cudaGetErrorString(status);
        freeMemoryOnGPU(&shoalPtrs);
        exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    int initDataDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Initializing data on GPU time: " << initDataDuration / 1e9 << "s" << std::endl;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FishCanvasGPU::onUpdate);
}

// Usuwa zaalokowaną pamięć na GPU oraz obiekt Timer'a. Liczy czas zwalniania pamięci
FishCanvasGPU::~FishCanvasGPU() {
    auto start = std::chrono::high_resolution_clock::now();
    freeMemoryOnGPU(&shoalPtrs);
    delete timer;
    auto end = std::chrono::high_resolution_clock::now();
    int freeDataDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Memory release time: " << freeDataDuration / 1e9 << "s" << std::endl;
}

// Inicjalizuje obiekty niezbędne do renderowania obiektów ryb na ekranie przy użyciu karty graficznej. Liczy
// czas działania inicjalizacji
void FishCanvasGPU::initializeGL() {
    auto start = std::chrono::high_resolution_clock::now();
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    const char* vertexShader = R"glsl(
#version 330 core
layout (location = 0) in vec2 instancePos;
layout (location = 1) in float instanceAngle;
layout (location = 2) in float instanceColor;

out VS_OUT {
    float angle;
    float color;
} vs_out;

void main() {
    vs_out.color = instanceColor;
    vs_out.angle = instanceAngle;

    gl_Position = vec4(instancePos.x, instancePos.y, 0.0, 1.0);
}
)glsl";

    const char* geometryShader = R"glsl(
#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
    float angle;
    float color;
} gs_in[];

out float color;

uniform mat4 projMatrix;

void main() {
   color = gs_in[0].color;
    float c = cos(gs_in[0].angle);
    float s = sin(gs_in[0].angle);
    mat2 rot = mat2(c, -s, s, c);

    vec2 base = gl_in[0].gl_Position.xy;
    float size = 8.0;

    vec2 p1 = base + rot * vec2(-size*0.5,  size*0.3);
    vec2 p2 = base + rot * vec2(-size*0.5, -size*0.3);
    vec2 p3 = base + rot * vec2(size,  0.0);

    gl_Position = projMatrix * vec4(p1, 0, 1);
    EmitVertex();
    gl_Position = projMatrix * vec4(p2, 0, 1);
    EmitVertex();
    gl_Position = projMatrix * vec4(p3, 0, 1);
    EmitVertex();
    EndPrimitive();
}
)glsl";

    const char* fragmentShader = R"glsl(
#version 330 core
in float color;
out vec4 fragColor;

void main() {
    if (color == 1.0)
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    else
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)glsl";

    shaderProgram = new QOpenGLShaderProgram(this);
    if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader)) {
        qDebug() << "Vertex Shader Error:" << shaderProgram->log();
    }
    if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Geometry, geometryShader)) {
        qDebug() << "Fragment Shader Error:" << shaderProgram->log();
    }
    if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader)) {
        qDebug() << "Fragment Shader Error:" << shaderProgram->log();
    }
    if (!shaderProgram->link()) {
        qDebug() << "Shader Link Error:" << shaderProgram->log();
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * N, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (
        void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
        (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
        (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    cudaError_t status = cudaGraphicsGLRegisterBuffer(&shoalGPUResource, vbo, cudaGraphicsMapFlagsWriteDiscard);

    if (status != cudaSuccess) {
        std::cerr << "Registration GL/CUDA error " << cudaGetErrorString(status);
        freeMemoryOnGPU(&shoalPtrs);
        delete timer;
        exit(1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    if (!timer) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &FishCanvasGPU::onUpdate);
    }
    auto end = std::chrono::high_resolution_clock::now();
    int intializingGLDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "GL initialization time: " << intializingGLDuration / 1e9 << "s" << std::endl;
    timer->start(33);
}

// Modyfikuje położenia i prędkości ryb w danej chwili. Następnie wywołuje shadery wykorzystywane do renderowania ich
// symulowanych położeń na ekranie. Liczy czas trwania modyfikacji i renderowania
void FishCanvasGPU::paintGL() {
    if (!firstFrame) {
        int duration = frameTimer.nsecsElapsed();
        std::cout << "Frame rendering speed: " << 1e9 / duration << " FPS" << std::endl;
    } else {
        firstFrame = false;
    }

    frameTimer.restart();

    auto start = std::chrono::high_resolution_clock::now();
    glClear(GL_COLOR_BUFFER_BIT);
    shaderProgram->bind();
    QMatrix4x4 proj;
    proj.ortho(0.0f, (float)canvasWidth, (float)canvasHeight, 0.0f, -1.0f, 1.0f);
    shaderProgram->setUniformValue("projMatrix", proj);

    QPoint globalPos = QCursor::pos();
    QPoint localPos = this->mapFromGlobal(globalPos);
    float cx = localPos.x();
    float cy = localPos.y();

    cudaError_t status = updateShoalSceneOnGPU(shoalGPUResource, &shoalPtrs, cx, cy, swR, awR, cwR, swG, awG, cwG, N,
        canvasWidth, canvasHeight);
    if (status != cudaSuccess) {
        std::cerr << "Updating data error " << cudaGetErrorString(status);
        freeMemoryOnGPU(&shoalPtrs);
        delete timer;
        exit(1);
    }

    glBindVertexArray(vao);
    glPointSize(fishesSize);
    glDrawArrays(GL_POINTS, 0, N);
    glBindVertexArray(0);
    shaderProgram->release();

    auto end = std::chrono::high_resolution_clock::now();
    auto paintDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Update + painting speed: " << paintDuration.count() / 1e9 << "s" << std::endl;
}

void FishCanvasGPU::onUpdate() {
    update();
}
