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
#define RESOURCE_FOLDER "Space_invaders.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
enum GameState { MENU, PLAY, NEW, GAME_OVER};

class SheetSprite {
public:
    SheetSprite();
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size);
    void DrawSpriteSheetSprite(ShaderProgram *program, int index);
    void draw(ShaderProgram *program);
    
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};
SheetSprite::SheetSprite(){}
SheetSprite::SheetSprite(unsigned int textureIDin, float uin, float vin, float widthin, float heightin, float sizein){
    size = sizein;
    textureID = textureIDin;
    u = uin;
    v = vin;
    width = widthin;
    height = heightin;
}
void SheetSprite::DrawSpriteSheetSprite(ShaderProgram *program, int index) {
    int spriteCountX = 12;//can be changed if neccesary
    int spriteCountY = 8;
    float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
    float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;
    float spriteWidth = 1.0/(float)spriteCountX;
    float spriteHeight = 1.0/(float)spriteCountY;
    glBindTexture(GL_TEXTURE_2D, textureID);
    float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,  -0.5f,
        -0.5f, 0.5f, -0.5f};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    GLfloat texCoords[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight
    };
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

class V3 {
public:
    V3(){};
    
    V3(float xin, float yin, float zin){
        x = xin;
        y = yin;
        z = zin;
    }
    float x;
    float y;
    float z;
};

class Entity{
    public:
        Entity();
        void draw(ShaderProgram *program);
        void update();
    
        V3 position;
        V3 velocity;
        V3 accel;
        float rotation;
        int textureID;
        SheetSprite sprite;
        float width;
        float height;
        Matrix modelMatrix;
        int index;
};
Entity::Entity(){
    position.x = 0;
    position.y= 0;
    position.z= 0;
    velocity.x= 0;
    velocity.y= 0;
    velocity.z= 0;
    accel.x= 0;
    accel.y= 0;
    accel.z= 0;
}
void Entity::draw(ShaderProgram *program){
    modelMatrix.identity();
    modelMatrix.Translate(position.x, position.y, 0);
    program->setModelMatrix(modelMatrix);
    sprite.DrawSpriteSheetSprite(program, index);;
}
void Entity::update(){
    velocity.x += accel.x;
    velocity.y += accel.y;
    position.x += velocity.x;
    position.y +=velocity.y;
    accel.x = 0;
    accel.y = 0;
    if (velocity.x > 0)
        velocity.x -= .1;
    if (velocity.x < 0)
        velocity.x -= 0;
    if (velocity.y > 0)
        velocity.y -= .2;
    if (velocity.y < 0)
        velocity.y = 0;
}
class bullet : public Entity{
public:
    void update(){
        
    }
};
class grounded : public Entity{
public:
    void update(){
        std::cout<<velocity.x<<","<<velocity.y<<std::endl;
        velocity.x += accel.x;
        velocity.y += accel.y;
        position.x += velocity.x;
        position.y += velocity.y;
        accel.x = 0;
        accel.y = 0;
        if (velocity.x > -.1 && velocity.x < .1)
            velocity.x = 0;
        if (velocity.x > 0)
            velocity.x -= .1;
        if (velocity.x < 0)
            velocity.x += .1;
        if (velocity.y > 0)
            velocity.y -= .2;
        if (velocity.y < 0)
            velocity.y = 0;
    std::cout<<velocity.x<<","<<velocity.y<<std::endl;
    }
};
class floating : public Entity{
public:
    void update(){
        //std::cout<<velocity.x<<","<<velocity.y<<std::endl;
        velocity.x += accel.x;
        velocity.y += accel.y;
        position.x += velocity.x;
        position.y +=velocity.y;
        accel.x = 0;
        accel.y = 0;
        
        if (velocity.y > -.1 && velocity.y < .1)
            velocity.y = 0;
        if (velocity.y > 0)
            velocity.y -= .05;
        if (velocity.y < 0)
            velocity.y += .05;
        //std::cout<<velocity.x<<","<<velocity.y<<std::endl;
    }
};
GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n"<< &filePath <<std::endl;
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
    float texture_size = 1.0 / 16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for (int i = 0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
bool collideRect(Entity A, Entity B){
    if(((A.position.y - A.height)>(B.position.y + B.height))
       ||((A.position.y + A.height)<(B.position.y - B.height))
       ||((A.position.x - A.width)>(B.position.x + B.width))
       ||((A.position.x + A.width)<(B.position.x - B.width))
       )
        return false;
    else
        return true;
}

void init(grounded &player, GameState &state, std::vector<floating> &entities){
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    entities.clear();
    int enemy_rows = 4;
    int enemy_cols = 12;
    unsigned int spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    player.position.x = 0;
    player.position.y = -3.5;
    player.height = .25;
    player.width = .25;
    player.velocity.x = 0;
    player.velocity.y = 0;
    player.index = 1;
    player.textureID = spriteSheetTexture;
    player.sprite = mySprite;
    floating e;
    for(double i=(-enemy_rows/2); i < (enemy_rows/2); i++) {
        for(int j=(-enemy_cols/2); j < (enemy_cols/2); j++) {
            e.position.x = j + .5;
            e.position.y = i + 2;
            e.height = .25;
            e.width = .25;
            e.velocity.x = -.02;
            e.velocity.y = 0;
            e.index = (i + (enemy_rows/2)+ 16) * 3 + 1;
            e.textureID = spriteSheetTexture;
            e.sprite = mySprite;
            entities.push_back(e);
        }
    }
    state = PLAY;
}

void RenderMainMenu(ShaderProgram program){
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    GLuint font = LoadTexture(RESOURCE_FOLDER"font2.png");
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    modelMatrix.identity();
    modelMatrix.Translate(-6.0f, 3.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
    DrawText(&program, font, "Space Invaders", 0.50f, 0);
}
void RenderGameLevel(grounded player, ShaderProgram *program, std::vector<floating> entities){
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    for(int i=0; i < entities.size(); i++) {
        entities[i].draw(program);
    }
    player.draw(program);
}
void UpdateMainMenu(){

}
void animate(int &modifier, grounded &player, std::vector<floating> &entities){
    modifier = ((modifier + 1)% 4);
    for(int i=0; i < entities.size(); i++) {
        if (modifier > 1)
            entities[i].index--;
        else
            entities[i].index++;
    }
    if (modifier >1)
        player.index--;
    else
        player.index++;
    
}
void UpdateGameLevel(int &modifier, grounded &player, std::vector<floating> &entities){
    for(int i=0; i < entities.size(); i++) {
        entities[i].update();
    }
    player.update();
}
void ProcessMainMenuInput(grounded player, GameState &state, const Uint8 *keys){
    if(keys[SDL_SCANCODE_SPACE]){
        state = NEW;
    }
}
void ProcessGameLevelInput(grounded &player, GameState &state, const Uint8 *keys ){
    if(keys[SDL_SCANCODE_RIGHT]){
        player.accel.x = .1;
    }
    if(keys[SDL_SCANCODE_LEFT]){
        player.accel.x = -.1;
    }
}
void ProcessGameLevelEnemy(std::vector<floating> &entities, GameState &state){
    if (entities[entities.size()-1].position.x > 7 || entities[0].position.x < -7){
        for(int i=0; i < entities.size(); i++) {
            entities[i].velocity.x = -entities[i].velocity.x;
            entities[i].accel.y = -.1;
        }
    }
}


int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
#glewInit();
#endif
//////////////////////////////////////////////////////setup
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    GameState state = MENU;
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER"winky.png");
    GLuint grassTexture = LoadTexture(RESOURCE_FOLDER"grass.png");
    glUseProgram(program.programID);
    glViewport(0, 0, 640, 360);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.4f, 0.2f, 0.4f, 1.0f);//clear color of screen
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    projectionMatrix.setOrthoProjection(-8.0, 8.0, -4.5f, 4.5f, -1.0f, 1.0f);//orthographic
    int modifier = 0;
    float lastFrameTicks = 0.0f;
    const int numFrames = 5;
    float animationElapsed = 0.0f;
    float framesPerSecond = 5.0f;
    
    std::vector<floating> entities;
    grounded player;

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
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
    

        switch(state) {
            case MENU:
                RenderMainMenu(program);
                UpdateMainMenu();
                ProcessMainMenuInput(player, state, keys);
                break;
                
            case NEW:
                init(player, state, entities);
                break;
                
            case GAME_OVER:
                break;
                
            case PLAY:
                RenderGameLevel(player, &program, entities);
                animationElapsed += elapsed;
                if(animationElapsed > 1.0/framesPerSecond) {
                    animate(modifier, player, entities);
                    animationElapsed = 0.0;
                }
                ProcessGameLevelInput(player, state, keys);
                ProcessGameLevelEnemy(entities, state);
                UpdateGameLevel(modifier, player, entities);
                break;

        }
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}

