#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
#glewInit();
#endif
//////////////////////////////////////////////////////setup
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    ShaderProgram program2(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER"P!.png");
    GLuint grassTexture = LoadTexture(RESOURCE_FOLDER"grass.png");
    glUseProgram(program.programID);
    glViewport(0, 0, 640, 360);
    glEnable(GL_BLEND);
    float lastFrameTicks = 0.0f;
    float positionX = 0;
    float positionY = 0;
    float angle = 0;
/////////////////////////////////////////loop initiation
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {//check for quit
                done = true;
            }
        }
////////////////////////////////////////////////////////////game loop
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        if(keys[SDL_SCANCODE_RIGHT]){
            positionX += .5;
        }
        if(keys[SDL_SCANCODE_LEFT]){
            positionX -= .5;
        }
        if(keys[SDL_SCANCODE_UP]){
            positionY += .5;
        }
        if(keys[SDL_SCANCODE_DOWN]){
            positionY -= .5;
        }
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        angle += 50 * -elapsed;
        
        glClearColor(0.4f, 0.2f, 0.4f, 1.0f);//clear color of screen
        glClear(GL_COLOR_BUFFER_BIT);//clear screen
        
        projectionMatrix.setOrthoProjection(-8.0, 8.0, -4.5f, 4.5f, -1.0f, 1.0f);
        
        
        
        
        //modelMatrix.identity();
        modelMatrix.Translate(positionX, positionY, 0.0f);
        //modelMatrix.Scale(1.0f, 1.0f, 1.0f);
        modelMatrix.Rotate(.001 * angle* angle * angle * (3.14159/180));
        //rotates in radians and CCW
        
        
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        
        
        /////////////////draw phase
        
        
        glBindTexture(GL_TEXTURE_2D, emojiTexture);
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        modelMatrix.identity();
        //changes
        program.setModelMatrix(modelMatrix);

        glBindTexture(GL_TEXTURE_2D, grassTexture);
        float vertices2[] = { -1.5, -1.5, -2.0, -1.5, -2.0, -2.0, -2.0, -2.0, -1.5, -2.0, -1.5, -1.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords2[] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0};
        glVertexAttribPointer(program2.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        modelMatrix.identity();
        //changes
        program.setModelMatrix(modelMatrix);
        
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        float vertices3[] = { 2.0, -1.5, 1.5, -1.5, 1.5, -2.0, 1.5, -2.0, 2.0, -2.0, 2.0, -1.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords3[] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0};
        glVertexAttribPointer(program2.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        ////////////////post phase
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}

