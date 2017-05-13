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
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
//due to poor planning everything will be called space invaders from now on
//but tey will be in the right folders
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "Space_Invader.app/Contents/Resources/"
#endif

int LEVEL_HEIGHT =18;
int LEVEL_WIDTH =32;
int SPRITE_COUNT_X =30;
float SPRITE_COUNT_Y =30;
float TILE_SIZE = 1;
unsigned int levelData[100][100];

SDL_Window* displayWindow;
enum GameState { WIN, MENU, PLAY, NEW, GAME_OVER};
int SCORE = 0;


float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

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
    int spriteCountX = 30;//can be changed if neccesary
    int spriteCountY = 30;
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

class Bullet{
public:
    Bullet();
    void draw(ShaderProgram *program);
    void update(float elapsed);
    
    V3 position;
    V3 velocity;
    V3 accel;
    float rotation;
    int textureID;
    SheetSprite sprite;
    float width;
    float height;
    float timeAlive;
    Matrix modelMatrix;
    int index;
    bool fired;
};
Bullet::Bullet(){
    timeAlive = 0;
    position.x = 0;
    position.y= 0;
    position.z= 0;
    velocity.x= 0;
    velocity.y= 0;
    velocity.z= 0;
    height = .1;
    width = .1;
    accel.x= 0;
    accel.y= 0;
    accel.z= 0;
    fired = false;
    index = 92;
}
void Bullet::update(float elapsed){
    timeAlive += elapsed;
    if(timeAlive > 1.4) {
        fired = false;
    }
    position.y += velocity.y;
}
void Bullet::draw(ShaderProgram *program){
    modelMatrix.identity();
    modelMatrix.Translate(position.x, position.y, 0);
    modelMatrix.Scale(width, height, 1);
    program->setModelMatrix(modelMatrix);
    sprite.DrawSpriteSheetSprite(program, index);;
}

class Entity{
    public:
        Entity();
        void draw(ShaderProgram *program);
        void update(float elapsed);
    
    std::string type;
        V3 position;
        V3 velocity;
        float friction_x = 20;
        float friction_y = 5;
        V3 accel;
        float rotation;
        int textureID;
        SheetSprite sprite;
        float width;
        float height;
        Matrix modelMatrix;
        int index;
        int ammo;
        Bullet bullets[100];
        int fired;
        bool solid = false;
        bool ground = true;
};
Entity::Entity(){
    position.x = 0;
    position.y= 0;
    position.z= 0;
    velocity.x= 0;
    velocity.y= 0;
    velocity.z= 0;
    height = 1;
    width = 1;
    accel.x= 0;
    accel.y= 0;
    accel.z= 0;
    ammo = 1;
    fired = 0;
}
void Entity::draw(ShaderProgram *program){
    modelMatrix.identity();
    modelMatrix.Translate(position.x, position.y, 0);
    modelMatrix.Scale(width+.5, height+.5, 1);
    program->setModelMatrix(modelMatrix);
    sprite.DrawSpriteSheetSprite(program, index);;
    for(int i=0; i < ammo; i++) {
        if(bullets[i].fired == true)
            bullets[i].draw(program);
    }
}
void Entity::update(float elapsed){
    velocity.x += accel.x * elapsed;
    velocity.y += accel.y * elapsed;
    position.x += velocity.x * elapsed;
    position.y +=velocity.y * elapsed;
    accel.x = 0;
    accel.y = 0;
    velocity.x = lerp(velocity.x, 0.0f, elapsed * friction_x);
    velocity.y = lerp(velocity.y, 0.0f, elapsed * friction_y);
}

class grounded : public Entity{
public:
    void update(float elapsed, Entity tiles[100][100]){
        velocity.x += accel.x* elapsed;
        velocity.y += accel.y* elapsed;
        position.x += velocity.x* elapsed;
        accel.x = 0;
        for(int i = 0; i < LEVEL_HEIGHT;i++){
            for(int j = 0; j < LEVEL_WIDTH;j++){
                if ((checkTile(tiles[i][j]))&&(tiles[i][j].solid == true)){
                   position.x -= velocity.x* elapsed;
                    break;
                }
            }
        }
        position.y += velocity.y* elapsed;
        if(velocity.y>0)
            ground = false;
        accel.y -= 100 * elapsed;
        for(int i = 0; i < LEVEL_HEIGHT;i++){
            for(int j = 0; j < LEVEL_WIDTH;j++){
                if ((checkTile(tiles[i][j]))&&(tiles[i][j].solid == true)){
                    ground = true;
                    position.y -= velocity.y* elapsed;
                    accel.y = 0;
                    velocity.y=0;
                    break;
                }
            }
        }
        velocity.x = lerp(velocity.x, 0.0f, elapsed * friction_x);
        velocity.y = lerp(velocity.y, 0.0f, elapsed * friction_y);
        for(int i=0; i < ammo; i++) {
            bullets[i].update(elapsed);
        }
        std::cout<<position.x<<" "<<position.y<<std::endl;
    }
    void shootBullet() {
        if (!bullets[fired].fired){
        Bullet newBullet;
        newBullet.position.x = position.x;
        newBullet.position.y = position.y;
        newBullet.fired = true;
        newBullet.velocity.y = .1f;
        newBullet.timeAlive = 0.0f;
        bullets[fired] = newBullet;
        fired++;
        fired = fired%ammo;
        }
    }
    bool checkTile(Entity B){
        if(((position.y - height)>(B.position.y + B.height))
           ||((position.y + height)<(B.position.y - B.height))
           ||((position.x - width)>(B.position.x + B.width))
           ||((position.x + width)<(B.position.x - B.width))
           )
            return false;//does collide
        else
            return true;
    }
    
};
class floating : public Entity{
public:
    floating(){
        position.x = 0;
        position.y= 0;
        position.z= 0;
        velocity.x= 0;
        velocity.y= 0;
        velocity.z= 0;
        height = 1;
        width = 1;
        accel.x= 0;
        accel.y= 0;
        accel.z= 0;
        ammo = 1;
        fired = 0;
    }
    floating(std::string typex, int placex, int placey){
        position.x = placex;
        position.y= placey;
        position.z= 0;
        velocity.x= 0;
        velocity.y= 0;
        velocity.z= 0;
        height = 1;
        width = 1;
        accel.x= 0;
        accel.y= 0;
        accel.z= 0;
        ammo = 1;
        fired = 0;
        type = typex;
    }

    void update(float elapsed){
        velocity.x += accel.x * elapsed;
        velocity.y += accel.y * elapsed;
        position.x += velocity.x * elapsed;
        position.y +=velocity.y * elapsed;
        accel.x = 0;
        accel.y = 0;
        
        velocity.x = lerp(velocity.x, 0.0f, elapsed * friction_x);
        velocity.y = lerp(velocity.y, 0.0f, elapsed * friction_y);
        for(int i=0; i < ammo; i++) {
            bullets[i].update(elapsed);
        }
    }
    void shootBullet() {
        if (!bullets[fired].fired){
            Bullet newBullet;
            newBullet.position.x = position.x;
            newBullet.position.y = position.y;
            newBullet.fired = true;
            newBullet.velocity.y = -.1f;
            newBullet.timeAlive = 0.0f;
            bullets[fired] = newBullet;
            fired++;
            fired = fired%ammo;
        }
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
bool collideRect(Bullet A, Entity B){
    if(((A.position.y - A.height)>(B.position.y + B.height))
       ||((A.position.y + A.height)<(B.position.y - B.height))
       ||((A.position.x - A.width)>(B.position.x + B.width))
       ||((A.position.x + A.width)<(B.position.x - B.width))
       )
        return false;
    else
        return true;
}

bool readHeader(std::ifstream &stream) {
    std::string line;
    LEVEL_WIDTH = -1;
    LEVEL_HEIGHT = -1;
    while(getline(stream, line)) {
        if(line == "") { break; }
        std::istringstream sStream(line);
        std::string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "width") {
            LEVEL_WIDTH = atoi(value.c_str());
        } else if(key == "height"){
            LEVEL_HEIGHT = atoi(value.c_str());
        } }
    if(LEVEL_WIDTH == -1 || LEVEL_HEIGHT == -1) {
        return false;
    } else { // allocate our map data
        return true;
    }
}

bool readLayerData(std::ifstream &stream) {
    std::string line;
    while(getline(stream, line)) {
        if(line == "") { break; }
        std::istringstream sStream(line);
        std::string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data") {
            for(int y=0; y < LEVEL_HEIGHT; y++) {
                getline(stream, line);
                std::istringstream lineStream(line);
                std::string tile;
                for(int x=0; x < LEVEL_WIDTH; x++) {
                    getline(lineStream, tile, ',');
                    unsigned int val =  (unsigned int)atoi(tile.c_str());
                    if(val > 0) {
                        // be careful, the tiles in this format are indexed from 1 not 0
                        levelData[y][x] = val-1;
                    } else {
                        levelData[y][x] = 0;
                    }
                }
            } }
    }
    return true;
}
void placeEntity(std::string type, int placeX, int placeY, std::vector<floating> &entities){
    unsigned int spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"spritesheet_rgba.png");
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    floating e;
    e.position.x = placeX;
    e.position.y = placeY;
    e.height = .5;
    e.width = .5;
    e.velocity.x = 0;
    e.velocity.y = 0;
    int ind = stoi(type);
    
    e.index = ind;
    e.textureID = spriteSheetTexture;
    e.sprite = mySprite;
    entities.push_back(e);
}
bool readEntityData(std::ifstream &stream, std::vector<floating> &entities) {
    std::string line;
    std::string type;
    while(getline(stream, line)) {
        if(line == "") { break; }
        std::istringstream sStream(line);
        std::string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type") {
            type = value;
        } else if(key == "location") {
            std::istringstream lineStream(value);
            std::string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            float placeX = atoi(xPosition.c_str())*TILE_SIZE;
            float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
            placeEntity(type, placeX, placeY, entities);
        }
    }
return true;
}
void init(grounded &player, GameState &state, std::vector<floating> &entities, Entity tiles[100][100]){
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    entities.clear();
    unsigned int spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"spritesheet_rgba.png");
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    player.position.x = 2.1;
    player.position.y = -12.9;
    player.height = .25 ;
    player.width = .25;
    player.velocity.x = 0;
    player.velocity.y = 0;
    player.index = 30*4 -10;
    player.textureID = spriteSheetTexture;
    player.sprite = mySprite;
    state = PLAY;
    for(int i = 0; i < 100;i++){
        for(int j = 0; j < 100;j++){
            floating e;
            e.position.x = j;
            e.position.y = -i;
            e.height = .5;
            e.width = .5;
            e.solid = false;
            e.velocity.x = 0;
            e.velocity.y = 0;
            e.index = 13*30;
            e.textureID = spriteSheetTexture;
            e.sprite = mySprite;
            tiles[i][j] = e;
        }
    }
    for(int i = 0; i < LEVEL_HEIGHT;i++){
        for(int j = 0; j < LEVEL_WIDTH;j++){
            floating e;
            e.position.x = j;
            e.position.y = -i;
            e.height = .5;
            e.width = .5;
            e.velocity.x = 0;
            e.velocity.y = 0;
            int ind = int(levelData[i][j]);
            e.index = ind;
            e.textureID = spriteSheetTexture;
            e.sprite = mySprite;
            e.solid = true;
            if((int)levelData[i][j] != 0)
                tiles[i][j] = e;
            std::cout<<tiles[i][j].solid<<std::endl;
        }
    }
}
void RenderTileMap(ShaderProgram *program, unsigned int spriteSheetTexture){
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int y=0; y < LEVEL_HEIGHT; y++) {
        for(int x=0; x < LEVEL_WIDTH; x++) {
            float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
            float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
            float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
            float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
            vertexData.insert(vertexData.end(), {
                TILE_SIZE * x, -TILE_SIZE * y,
                TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
                (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                TILE_SIZE * x, -TILE_SIZE * y,
                (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                (TILE_SIZE * x)+TILE_SIZE, -TILE_SIZE * y
            });
            texCoordData.insert(texCoordData.end(), {
                u, v,
                u, v+(spriteHeight),
                u+spriteWidth, v+(spriteHeight),
                u, v,
                u+spriteWidth, v+(spriteHeight),
                u+spriteWidth, v
            });
        }
    }
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
    glDrawArrays(GL_TRIANGLES, 0, LEVEL_HEIGHT * LEVEL_WIDTH * 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
void RenderMainMenu(ShaderProgram program, GLuint font, unsigned int spriteSheetTexture){
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    modelMatrix.identity();
    modelMatrix.Translate(-6.0f, 3.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
    DrawText(&program, font, "Platformer Demo", 0.50f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-6.0f, 1.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    std::string message = "arrow keys to move";
    DrawText(&program, font, message, 0.40f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-6.0f, 1.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
    message = "spacebar to start";
    DrawText(&program, font, message, 0.40f, 0);
}
void RenderLoseMenu(ShaderProgram program, GLuint font, unsigned int spriteSheetTexture){
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    modelMatrix.identity();
    modelMatrix.Translate(-6.0f, 3.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
    DrawText(&program, font, "YOU LOSE :(", 0.50f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-6.0f, 1.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    std::string message = "Your score was: " + std::to_string(SCORE);
    DrawText(&program, font, message, 0.40f, 0);
}
void RenderWinMenu(ShaderProgram program, GLuint font, unsigned int spriteSheetTexture ){
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    modelMatrix.identity();
    modelMatrix.Translate(-6.0f, 3.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
    DrawText(&program, font, "YOU WIN :)", 0.50f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-6.0f, 1.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    std::string message = "Your score was: " + std::to_string(SCORE);
    DrawText(&program, font, message, 0.40f, 0);
}
void RenderGameLevel(grounded player, ShaderProgram *program, std::vector<floating> entities, Entity tiles[100][100], GLuint font, unsigned int spriteSheetTexture){
    std::string score;
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    //RenderTileMap(program, spriteSheetTexture);//draw map
    modelMatrix.identity();
    modelMatrix.Translate(-7.0f, 4.0f, 0.0f);
    program->setModelMatrix(modelMatrix);
    score = "Score: " + std::to_string(SCORE);
    DrawText(program, font, score, 0.30f, 0);
    viewMatrix.identity();
    viewMatrix.Translate(-player.position.x, -player.position.y, 0.0f);
    program->setViewMatrix(viewMatrix);
    for(int i = 0; i < LEVEL_HEIGHT;i++){
        for(int j = 0; j < LEVEL_WIDTH;j++){
            tiles[i][j].draw(program);
        }
    }
    for(int i=0; i < entities.size(); i++) {
        entities[i].draw(program);
    }//draw entities
    player.draw(program);//draw player

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
void UpdateGameLevel(int &modifier, grounded &player, std::vector<floating> &entities, float elapsed, GameState &state, Entity tiles[100][100]){
    for(int i=0; i < entities.size(); i++) {
        entities[i].update(elapsed);
    }
    player.update(elapsed, tiles);
    for(int i = 0; i <entities.size();i++){
        for(int j = 0; j <player.ammo;j++){
            if(player.bullets[j].fired)
            if(collideRect(player.bullets[j],entities[i])){
                
                entities.erase(entities.begin() + i);
                player.bullets[j].position.x = 0;
                player.bullets[j].position.y = 0;
                player.bullets[j].fired = false;
                player.bullets[j].velocity.y = 0.0f;
                player.bullets[j].timeAlive = 0.0f;
                SCORE++;
            }
        }
    }
    for(int i = 0; i <entities.size();i++){
        for(int j = 0; j <entities[i].ammo;j++){
            if(entities[i].bullets[j].fired)
                if(collideRect(entities[i].bullets[j],player)){
                    state = GAME_OVER;
                }
        }
    }
    if (SCORE >=48)
        state = WIN;
    //else if (entities[entities.size()-1].position.y < .45)
        //state = GAME_OVER;
}
void ProcessMainMenuInput(grounded player, GameState &state, const Uint8 *keys){
    if(keys[SDL_SCANCODE_SPACE]){
        state = NEW;
    }
}
void ProcessGameLevelInput(grounded &player, GameState &state, const Uint8 *keys, float elapsed ){
    if(keys[SDL_SCANCODE_RIGHT]){
        player.accel.x = 100;
    }
    if(keys[SDL_SCANCODE_LEFT]){
        player.accel.x = -100;
    }
    if(keys[SDL_SCANCODE_UP]&&(player.ground == true)){
        player.velocity.y += 10;
        player.ground = false;
    }

}
void ProcessGameLevelEnemy(std::vector<floating> &entities, float elapsed){
    if(entities.size()>0){
    if (entities[entities.size()-1].position.x > 7 || entities[0].position.x < -7){
        for(int i=0; i < entities.size(); i++) {
            entities[i].velocity.x = -entities[i].velocity.x;
            entities[i].accel.y = -.1;
        }
    }
    int r = rand() % (entities.size()*int(1/elapsed));
    std::cout<<1/elapsed<<std::endl;
    for(int i=0; i < entities.size(); i++) {
        if(i == r)
            entities[i].shootBullet();
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
    glUseProgram(program.programID);
    glViewport(0, 0, 640, 360);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.4f, 0.2f, 0.4f, 1.0f);//clear color of screen
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    projectionMatrix.setOrthoProjection(-8.0, 8.0, -4.5f, 4.5f, -1.0f, 1.0f);//orthographic
    GLuint font = LoadTexture(RESOURCE_FOLDER"font2.png");
    unsigned int spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"spritesheet_rgba.png");
    int modifier = 0;
    float lastFrameTicks = 0.0f;
    float animationElapsed = 0.0f;
    float framesPerSecond = 5.0f;
    std::vector<floating> entities;
    std::string levelFile = "/Users/pierules53/Desktop/Current\ Homework/Game\ Programming/CS3113\ Projects/project_template/Xcode/Platformer/Space_invaders/Map.txt";
    std::ifstream infile(levelFile);
    std::string line;
    //placeEntity("0", 0, 0, entities);
    while (getline(infile, line)) {
        std::cout<<line<<std::endl;
        if(line == "[header]") {
            readHeader(infile);
        } else if(line == "[layer]") {
            //unsigned char levelData[LEVEL_HEIGHT][LEVEL_WIDTH];
            readLayerData(infile);
        } else if(line == "[ObjectsLayer]") {
            readEntityData(infile, entities);
        }
    }
    //const int lHeight = LEVEL_HEIGHT;
    //const int lWidth = LEVEL_WIDTH;
    Entity (*tiles)[100] = new Entity[100][100];
    //std::vector<std::vector<Entity>> tiles;
    
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
                RenderMainMenu(program, font, spriteSheetTexture);
                UpdateMainMenu();
                ProcessMainMenuInput(player, state, keys);
                break;
                
            case NEW:
                init(player, state, entities, tiles);
                //placeEntity("0", 0, 0, entities);
                break;
                
            case GAME_OVER:
                RenderLoseMenu(program, font, spriteSheetTexture);
                break;
                break;
                
            case WIN:
                RenderWinMenu(program, font, spriteSheetTexture);
                break;
                
            case PLAY:
                RenderGameLevel(player, &program, entities, tiles, font, spriteSheetTexture);
                animationElapsed += elapsed;
                if(animationElapsed > 1.0/framesPerSecond) {
                    animate(modifier, player, entities);
                    animationElapsed = 0.0;
                }
                ProcessGameLevelInput(player, state, keys, elapsed);
                ProcessGameLevelEnemy(entities, elapsed);
                UpdateGameLevel(modifier, player, entities, elapsed, state, tiles);
                break;

        }
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}

