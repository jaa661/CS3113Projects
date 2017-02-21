//Projects File
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
#include <vector>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
class Entity{
    public:
        void Draw();
    
        float x;
        float y;
        float rotation;
        int textureID;
        float width;
        float height;
        float speed;
        float direction_x;
        float direction_y;
};

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

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        }); }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    // draw this data (use the .data() method of std::vector to get pointer to data)
}
bool collideRect(Entity A, Entity B){
    if(((A.y - A.height)>(B.y + B.height))
       ||((A.y + A.height)<(B.y - B.height))
       ||((A.x - A.width)>(B.x + B.width))
       ||((A.x + A.width)<(B.x - B.width))
       )
        return false;
    else
        return true;
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
    GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER"winky.png");
    GLuint grassTexture = LoadTexture(RESOURCE_FOLDER"grass.png");
    glUseProgram(program.programID);
    glViewport(0, 0, 640, 360);
    glEnable(GL_BLEND);
    float lastFrameTicks = 0.0f;
    Entity paddle;
    paddle.x = 7;
    paddle.y = 0;
    paddle.height = 1;
    paddle.width = .25;
    paddle.speed = 7;
    Entity player;
    player.x = -7;
    player.y = 0;
    player.height = 1;
    player.width = .25;
    player.speed = 8;
    Entity ball;
    ball.rotation = 0;
    ball.x = 0;
    ball.y = 0;
    ball.speed = 5;
    ball.height = .25;
    ball.width = .25;
    //int score1 = 0;
    //int score2 = 0;
/////////////////////////////////////////loop initiation
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {//check for quit
                done = true;
            }else if(event.type == SDL_MOUSEMOTION) {
                // event.motion.x is the new x position
                // event.motion.y is the new y position
            }else if(event.type == SDL_MOUSEBUTTONDOWN) {
                // event.button.x is the click x position
                // event.button.y is the click y position
                // event.button.button is the mouse button that was clicked (1,2,3,etc.)
            }
        }
////////////////////////////////////////////////////////////game loop
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        /*if(keys[SDL_SCANCODE_RIGHT]){
            positionX += .5;
        }*/
        /*if(keys[SDL_SCANCODE_LEFT]){
            positionX -= .5;
        }*/
        if(keys[SDL_SCANCODE_UP]){
            player.y += elapsed*player.speed;
        }
        if(keys[SDL_SCANCODE_DOWN]){
            player.y -= elapsed*player.speed;
        }
        if(keys[SDL_SCANCODE_SPACE]){
            if (ball.speed == 0){
                ball.speed = 5;
                done = true;
            }
        }
        
        glClearColor(0.4f, 0.2f, 0.4f, 1.0f);//clear color of screen
        glClear(GL_COLOR_BUFFER_BIT);//clear screen
        projectionMatrix.setOrthoProjection(-8.0, 8.0, -4.5f, 4.5f, -1.0f, 1.0f);//orthographic
        
        /////////////////draw phase
        if(ball.y < (-4.5 + ball.height)){
            if(ball.rotation > 180 && ball.rotation < 270){
                ball.rotation = ((int)ball.rotation - 2*(((int)ball.rotation % 360) - 180)) % 360;
            }
            else if(ball.rotation > 270 && ball.rotation < 360){
                ball.rotation = ((int)ball.rotation + 2*(360 - ((int)ball.rotation % 360))) % 360;
            }
        }
        if(ball.y > (4.5 - ball.height)){
            if(ball.rotation < 180 && ball.rotation > 90){
                ball.rotation = ((int)ball.rotation + 2*(180 - ((int)ball.rotation % 360)))%360;
            }
            else if(ball.rotation > 0 && ball.rotation < 90){
                ball.rotation = (360 - (((int)ball.rotation % 360)))%360;
            }
        }
        
        
        if(collideRect(ball, player)){
            if(ball.rotation < 180 && ball.rotation >= 90){
                ball.rotation = ((int)ball.rotation + 2*(90 - ((int)ball.rotation % 360)))%360;
                ball.rotation += 25*(ball.y - player.y);
                ball.rotation = (int)ball.rotation%360;
            }
            if(ball.rotation >= 180 && ball.rotation < 270){
                ball.rotation = ((int)ball.rotation + 2*(270 - ((int)ball.rotation % 360))) % 360;
                ball.rotation += 25*(ball.y - player.y);
                ball.rotation = (int)ball.rotation%360;
            }
            ball.speed = ball.speed + .5;
        }
        if(collideRect(ball, paddle)){
            if(ball.rotation <= 360 && ball.rotation >= 270){
                ball.rotation = ((int)ball.rotation - 2*(((int)ball.rotation % 360)-270))%360;
                ball.rotation += 100*(ball.y - paddle.y)*(ball.y - paddle.y)*(ball.y - paddle.y);
                ball.rotation = (int)ball.rotation%360;

            }
            if(ball.rotation >= 0 && ball.rotation <= 90){
                ball.rotation = ((int)ball.rotation +2*(90-((int)ball.rotation % 360)))%360;
                ball.rotation += 100*(ball.y - paddle.y)*(ball.y - paddle.y)*(ball.y - paddle.y);
                ball.rotation = (int)ball.rotation%360;
            }
            ball.speed = ball.speed + .5;
        }
        /*if(ball.x + ball.width > 8 ){
            ball.x = 0;
            ball.y = 0;
            ball.speed = 0;
            score1++;
        } if(ball.x - ball.width < -8 ){
            ball.x = 0;
            ball.y = 0;
            ball.speed = 0;
            score2++;
        }*/
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        if(ball.x + ball.width > 8 ){
                ball.speed = 0;
                DrawText(&program, 1, "Game Over!", 2, 1);
         } if(ball.x - ball.width < -8 ){
             ball.speed = 0;
             DrawText(&program, 1, "Game Over!", 2, 1);
         }
        if(ball.rotation <0)
            ball.rotation = 360 + ball.rotation;
            
        ball.x += cos(ball.rotation*(3.14159/180)) * elapsed * ball.speed;
        ball.y += sin(ball.rotation*(3.14159/180)) * elapsed * ball.speed;
        modelMatrix.identity();
        modelMatrix.Translate(ball.x, ball.y, 0.0f);
        //modelMatrix.Scale(1.0f, 1.0f, 1.0f);
        //modelMatrix.Rotate(.001 * angle* angle * angle * (3.14159/180));
        //rotates in radians and CCW
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        glBindTexture(GL_TEXTURE_2D, emojiTexture);
        float vertices[] = {-0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        //////////////////
        modelMatrix.identity();
        modelMatrix.Translate(player.x, player.y, 0.0f);
        //changes
        program.setModelMatrix(modelMatrix);

        glBindTexture(GL_TEXTURE_2D, grassTexture);
        float vertices2[] = {-0.25, -1.0, 0.25, -1.0, 0.25, 1.0, -0.25, -1.0, 0.25, 1.0, -0.25, 1.0};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords2[] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        ///////////////////
        modelMatrix.identity();
        if(ball.y > paddle.y)
            paddle.y += elapsed*paddle.speed;
        if(ball.y < paddle.y)
            paddle.y -= elapsed*paddle.speed;
        
        modelMatrix.Translate(paddle.x, paddle.y, 0.0f);
        //changes
        program.setModelMatrix(modelMatrix);
        
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
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

