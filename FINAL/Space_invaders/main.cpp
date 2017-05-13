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
#include <SDL_mixer.h>
//due to poor planning everything will be called space invaders from now on
//but tey will be in the right folders
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "Space_Invader.app/Contents/Resources/"
#endif

using namespace std;
int LEVEL_HEIGHT =80;
int LEVEL_WIDTH =80;
int SPRITE_COUNT_X = 32;
float SPRITE_COUNT_Y =44;
float TILE_SIZE = 1;
unsigned int levelData[80][80];

SDL_Window* displayWindow;
enum GameState { WIN, MENU, PLAY, NEW, GAME_OVER, SWITCH, PAUSE, FINISH};
int SCORE = 0;


float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

class SheetSprite {
public:
    SheetSprite();
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size);
    void DrawSpriteSheetSprite(ShaderProgram *program, int index, int countX, int countY);
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
void SheetSprite::DrawSpriteSheetSprite(ShaderProgram *program, int index, int countX, int countY) {
    int spriteCountX = countX;//can be changed if neccesary
    int spriteCountY = countY;
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
    sprite.DrawSpriteSheetSprite(program, index, 30, 30);;
}

class Entity{
    public:
        Entity();
        void draw(ShaderProgram *program);
        void update(float elapsed);
        void nextIndex();
    
    std::string type;
        V3 position;
        V3 velocity;
        float friction_x = 5;
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
        bool moving;
        Bullet bullets[80];
        int fired;
        bool solid = false;
        bool ground = true;
    int health;
    int newIndex;
    int direction;
    int changedirection;
    int countX;
    int countY;
    bool enter = false;
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
    countX = SPRITE_COUNT_X;
    countY = SPRITE_COUNT_Y;
    direction = 2;//down
    changedirection = 2;
    moving = false;
    health = 3;
}
void Entity::draw(ShaderProgram *program){
    modelMatrix.identity();
    modelMatrix.Translate(position.x, position.y, 0);
    modelMatrix.Scale(width+.5, height+.5, 1);
    program->setModelMatrix(modelMatrix);
    sprite.DrawSpriteSheetSprite(program, newIndex, countX, countY);;
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
    velocity.y = lerp(velocity.y, 0.0f, elapsed * friction_x);
}
void Entity::nextIndex(){
    if (newIndex%3 >index%3)
        newIndex -= 2;
    else
        newIndex++;
}

class grounded : public Entity{
public:
    void update(float elapsed, Entity tiles[80][80]){
        velocity.x += accel.x* elapsed;
        position.x += velocity.x* elapsed;
        accel.x = 0;
        for(int i = 0; i < LEVEL_HEIGHT;i++){
            for(int j = 0; j < LEVEL_WIDTH;j++){
                if ((checkTile(tiles[i][j]))&&(tiles[i][j].solid == false)){
                    position.x -= velocity.x* elapsed;
                    break;
                }
            }
        }
        velocity.y += accel.y* elapsed;
        position.y += velocity.y* elapsed;
        accel.y = 0;
        for(int i = 0; i < LEVEL_HEIGHT;i++){
            for(int j = 0; j < LEVEL_WIDTH;j++){
                if ((checkTile(tiles[i][j]))&&(tiles[i][j].solid == false)){
                    position.y -= velocity.y* elapsed;
                    break;
                }
            }
        }
        if ((velocity.x !=0)||(velocity.y !=0))
            moving = true;
        else
            moving = false;

        if(direction != changedirection){
            newIndex = (12*(direction-1)) + 3;
            changedirection = direction;
        }
        if(moving == false){
            newIndex = (12*(direction-1)) + 4;
        }
        velocity.x = lerp(velocity.x, 0.0f, elapsed * friction_x);
        velocity.y = lerp(velocity.y, 0.0f, elapsed * friction_x);
        if (velocity.x < .01 && velocity.x > -.01)
            velocity.x =0;
        if (velocity.y < .01 && velocity.y > -.01)
            velocity.y =0;
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
        newBullet.velocity.y = .05f;
        newBullet.timeAlive = 0.0f;
        bullets[fired] = newBullet;
        fired++;
        fired = fired%ammo;
        }
    }
    bool checkTile(Entity B){
        if(((position.y - height)>(B.position.y + B.height))
           ||((position.y + height/4)<(B.position.y - B.height))
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
    bool interact;
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
    bool interacts(grounded &player){
        //if (index == -1)if other interacts are available
            return true;
        //else
           // return false;
    }
    void update(float elapsed, Entity tiles[80][80]){
        velocity.x += accel.x* elapsed;
        position.x += velocity.x* elapsed;
        accel.x = 0;
        for(int i = 0; i < LEVEL_HEIGHT;i++){
            for(int j = 0; j < LEVEL_WIDTH;j++){
                if ((checkTile(tiles[i][j]))&&(tiles[i][j].solid == false)){
                    position.x -= velocity.x* elapsed;
                    break;
                }
            }
        }
        velocity.y += accel.y* elapsed;
        position.y += velocity.y* elapsed;
        accel.y = 0;
        for(int i = 0; i < LEVEL_HEIGHT;i++){
            for(int j = 0; j < LEVEL_WIDTH;j++){
                if ((checkTile(tiles[i][j]))&&(tiles[i][j].solid == false)){
                    position.y -= velocity.y* elapsed;
                    break;
                }
            }
        }
        if ((velocity.x !=0)||(velocity.y !=0))
            moving = true;
        else
            moving = false;
        
        velocity.x = lerp(velocity.x, 0.0f, elapsed * friction_x);
        velocity.y = lerp(velocity.y, 0.0f, elapsed * friction_x);
        if (velocity.x < .01 && velocity.x > -.01)
            velocity.x =0;
        if (velocity.y < .01 && velocity.y > -.01)
            velocity.y =0;
        //std::cout<<newIndex<<" "<<index<<std::endl;

        for(int i=0; i < ammo; i++) {
            bullets[i].update(elapsed);
        }
    }
    
        bool checkTile(Entity B){
            if(((position.y - height)>(B.position.y + B.height))
               ||((position.y + height/4)<(B.position.y - B.height))
               ||((position.x - width)>(B.position.x + B.width))
               ||((position.x + width)<(B.position.x - B.width))
               )
                return false;//does collide
            else
                return true;
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
    if(((A.position.y - A.height/4)>(B.position.y + B.height))
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
    unsigned int spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"characters_1.png");
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    floating e;
    e.position.x = placeX;
    e.position.y = placeY;
    e.height = .5;
    e.width = .5;
    e.velocity.x = 0;
    e.velocity.y = 0;
    e.countX = 12;
    e.countY =8;
    int ind = stoi(type);
    if (ind>=0)
        e.interact = false;
    else
        e.interact = true;
    e.index = ind;
    e.newIndex = ind;
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
void init(grounded &player, GameState &state, std::vector<floating> &entities, Entity tiles[80][80]){
    std::string levelFile = "/Users/pierules53/Desktop/Current\ Homework/Game\ Programming/CS3113\ Projects/project_template/Xcode/Final/Space_invaders/Map1.txt";
    std::ifstream infile(levelFile);
    std::string line;
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
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    entities.clear();
    unsigned int spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sewer_1.png");
    unsigned int characterSheetTexture = LoadTexture(RESOURCE_FOLDER"characters_1.png");
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    SheetSprite hisSprite = SheetSprite(characterSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    player.position.x = 170.5;
    player.position.y = -68;
    player.height = 1 ;
    player.width = 1;
    player.velocity.x = 0;
    player.velocity.y = 0;
    player.index = 4;
    player.textureID = characterSheetTexture;
    player.sprite = hisSprite;
    player.countX = 12;
    player.countY =8;
    state = PLAY;
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
            e.newIndex = ind;
            e.textureID = spriteSheetTexture;
            e.sprite = mySprite;
            e.solid = false;
            int A = (int)levelData[i][j];
            if(((A%SPRITE_COUNT_X) <10)&&((A/SPRITE_COUNT_X)<6))
                e.solid = true;
            //cout<<(int)levelData[i][j]<<endl;
            tiles[i][j] = e;
            //std::cout<<tiles[i][j].solid<<std::endl;
        }
    }
    int positionx = 19.5;
    int positiony = -58;
    placeEntity("53", positionx, positiony, entities);
    positionx = 55.5;
    positiony = -8;
    placeEntity("-1", positionx, positiony, entities);
}
void init2(grounded &player, GameState &state, std::vector<floating> &entities, Entity tiles[80][80]){
    cout<<"Level 2!"<<endl;
    std::string levelFile = "/Users/pierules53/Desktop/Current\ Homework/Game\ Programming/CS3113\ Projects/project_template/Xcode/Final/Space_invaders/Map2.txt";
    std::ifstream infile(levelFile);
    std::string line;
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
    }glClear(GL_COLOR_BUFFER_BIT);//clear screen
    entities.clear();
    unsigned int spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sewer_1.png");
    unsigned int characterSheetTexture = LoadTexture(RESOURCE_FOLDER"characters_1.png");
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    SheetSprite hisSprite = SheetSprite(characterSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    player.position.x = 17.5;
    player.position.y = -68;
    state = PLAY;
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
            e.newIndex = ind;
            e.textureID = spriteSheetTexture;
            e.sprite = mySprite;
            e.solid = false;
            int A = (int)levelData[i][j];
            if(((A%SPRITE_COUNT_X) <10)&&((A/SPRITE_COUNT_X)<6))
                e.solid = true;
            //cout<<(int)levelData[i][j]<<endl;
            tiles[i][j] = e;
            //std::cout<<tiles[i][j].solid<<std::endl;
        }
    }
    int positionx = 19.5;
    int positiony = -58;
    placeEntity("53", positionx, positiony, entities);
    positionx = 55.5;
    positiony = -8;
    placeEntity("-1", positionx, positiony, entities);
}
void init3(grounded &player, GameState &state, std::vector<floating> &entities, Entity tiles[80][80]){
    cout<<"Level 3!"<<endl;
    std::string levelFile = "/Users/pierules53/Desktop/Current\ Homework/Game\ Programming/CS3113\ Projects/project_template/Xcode/Final/Space_invaders/Map3.txt";
    std::ifstream infile(levelFile);
    std::string line;
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
    }glClear(GL_COLOR_BUFFER_BIT);//clear screen
    entities.clear();
    unsigned int spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sewer_1.png");
    unsigned int characterSheetTexture = LoadTexture(RESOURCE_FOLDER"characters_1.png");
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    SheetSprite hisSprite = SheetSprite(characterSheetTexture,425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1);
    player.position.x = 17.5;
    player.position.y = -68;
    state = PLAY;
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
            e.newIndex = ind;
            e.textureID = spriteSheetTexture;
            e.sprite = mySprite;
            e.solid = false;
            int A = (int)levelData[i][j];
            if(((A%SPRITE_COUNT_X) <10)&&((A/SPRITE_COUNT_X)<6))
                e.solid = true;
            //cout<<(int)levelData[i][j]<<endl;
            tiles[i][j] = e;
            //std::cout<<tiles[i][j].solid<<std::endl;
        }
    }
    int positionx = 19.5;
    int positiony = -58;
    placeEntity("53", positionx, positiony, entities);
    positionx = 55.5;
    positiony = -8;
    placeEntity("-1", positionx, positiony, entities);
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
    GLuint backdrop = LoadTexture(RESOURCE_FOLDER"cave.png");
    float x = 18;
    float y = 9;
    modelMatrix.identity();
    glBindTexture(GL_TEXTURE_2D, backdrop);
    float vertices[] = {-x, -y, x, -y, x, y, -x, -y, x, y, -x, y};
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    modelMatrix.Translate(1.5f, 0.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    
    modelMatrix.identity();
    modelMatrix.Translate(-10.0f, 7.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    DrawText(&program, font, "Legends of Lambda", 1.0f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-9.5f, 5.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    std::string message = "Use the Arrow Keys to Move";
    DrawText(&program, font, message, 0.60f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-9.5f, 5.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
     message = "Use space to attack, and Q to quit";
    DrawText(&program, font, message, 0.60f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-9.5f, 4.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    message = "spacebar to start";
    DrawText(&program, font, message, 0.60f, 0);
}
void RenderPauseMenu(ShaderProgram program, GLuint font, unsigned int spriteSheetTexture){
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    
    modelMatrix.identity();
    modelMatrix.Translate(-5.0f, 7.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    DrawText(&program, font, "PAUSED", 2.0f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-4.5f, 5.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    std::string message = "U to Unpause";
    DrawText(&program, font, message, 0.60f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-4.5f, 5.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
    message = "Q to quit";
    DrawText(&program, font, message, 0.60f, 0);
}
void RenderLoseMenu(ShaderProgram program, GLuint font, unsigned int spriteSheetTexture){
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    GLuint backdrop = LoadTexture(RESOURCE_FOLDER"cave.png");
    float x = 18;
    float y = 9;
    modelMatrix.identity();
    glBindTexture(GL_TEXTURE_2D, backdrop);
    float vertices[] = {-x, -y, x, -y, x, y, -x, -y, x, y, -x, y};
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    modelMatrix.Translate(1.5f, 0.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    
    modelMatrix.identity();
    modelMatrix.Translate(-10.0f, 7.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    DrawText(&program, font, "You did not make it to the end.", 0.7f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-10.02f, 6.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    std::string message = "Thanks for Playing!";
    DrawText(&program, font, message, 0.60f, 0);
    }
void RenderWinMenu(ShaderProgram program, GLuint font, unsigned int spriteSheetTexture ){
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    GLuint backdrop = LoadTexture(RESOURCE_FOLDER"cave.png");
    float x = 18;
    float y = 9;
    modelMatrix.identity();
    glBindTexture(GL_TEXTURE_2D, backdrop);
    float vertices[] = {-x, -y, x, -y, x, y, -x, -y, x, y, -x, y};
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    modelMatrix.Translate(1.5f, 0.0f, 0.0f);
    program.setModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    
    modelMatrix.identity();
    modelMatrix.Translate(-10.0f, 7.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    DrawText(&program, font, "Congrats! You Made it!", 0.7f, 0);
    modelMatrix.identity();
    modelMatrix.Translate(-10.02f, 6.5f, 0.0f);
    program.setModelMatrix(modelMatrix);
    std::string message = "Thanks for Playing!";
    DrawText(&program, font, message, 0.60f, 0);
}
void RenderGameLevel(grounded player, ShaderProgram *program, std::vector<floating> entities, Entity tiles[80][80], GLuint font, unsigned int spriteSheetTexture){
    Matrix modelMatrix, projectionMatrix, viewMatrix;
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    viewMatrix.identity();
    viewMatrix.Translate(-player.position.x, -player.position.y, 0.0f);
    program->setViewMatrix(viewMatrix);
    for(int i = 0; i < LEVEL_HEIGHT;i++){
        for(int j = 0; j < LEVEL_WIDTH;j++){
            tiles[i][j].draw(program);
        }
    }
    for(int i=0; i < entities.size(); i++) {
        if (entities[i].index != -1)
            entities[i].draw(program);
    }//draw entities
    player.draw(program);//draw player
    modelMatrix.identity();
    modelMatrix.Translate(-7.0f, 4.0f, 0.0f);
    program->setModelMatrix(modelMatrix);
    string health;
    health = "Health Remaining: " + std::to_string(player.health);
    DrawText(program, font, health, 0.30f, 0);
}
void UpdateMainMenu(){

}
void animate(int &modifier, grounded &player, std::vector<floating> &entities){
    modifier = ((modifier + 1)% 3);
    for(int i=0; i < entities.size(); i++) {
        if(entities[i].moving == true){
            //entities[i].nextIndex();
        }
    }
    if(player.moving == true){
        player.nextIndex();
    }
}
void UpdateGameLevel(int &modifier, grounded &player, std::vector<floating> &entities, float elapsed, GameState &state, Entity tiles[80][80]){
    for(int i=0; i < entities.size(); i++) {
        entities[i].update(elapsed, tiles);
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
    if (player.health <1)
        state = GAME_OVER;
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
        player.accel.x = 40;
        player.direction = 3;
        player.enter = false;

    }
    if(keys[SDL_SCANCODE_LEFT]){
        player.accel.x = -40;
        player.direction = 2;
        player.enter = false;


    }
    if(keys[SDL_SCANCODE_UP]){
        player.accel.y = 40;
        player.direction = 4;
        player.enter = false;
    }
    if(keys[SDL_SCANCODE_DOWN]){
        player.accel.y = -40;
        player.direction = 1;
        player.enter = false;
    }
    if(keys[SDL_SCANCODE_P]){
        state = PAUSE;
    }
    if(keys[SDL_SCANCODE_E]){
        player.enter = true;
    }
}
void ProcessPauseInput(grounded &player, GameState &state, const Uint8 *keys ){
    if(keys[SDL_SCANCODE_U]){
        state = PLAY;
    }
    if(keys[SDL_SCANCODE_Q]){
        state = GAME_OVER;
    }
}
void ProcessGameLevelEnemy(std::vector<floating> &entities, float elapsed, grounded &player, GameState &state, int &level){
if(entities.size()>0){
    for(int i=0; i < entities.size(); i++) {
        int xdis = (((int)entities[i].position.x)-((int)player.position.x))^2;
        int ydis = (((int)entities[i].position.y)-((int)player.position.y))^2;
        int dis = sqrt(xdis+ydis);
        if(entities[i].interact == false){//for enemies
        if (dis < 10){
            if (player.position.x >entities[i].position.x)
                entities[i].accel.x += 7;
            else
                entities[i].accel.x += -7;
            if (player.position.y >entities[i].position.y)
                entities[i].accel.y += 7;
            else
                entities[i].accel.y += -7;

        }
        if (collideRect(player, entities[i])){
            //Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
            Mix_Chunk *Sound;
            Sound = Mix_LoadWAV("Dam.wav");
            Mix_PlayChannel( -1, Sound, 0);
            //float screenShakeValue = 0;
            //screenShakeValue += elapsed;
            //viewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity,
//0.0f);
            player.health--;
            if (player.position.x >entities[i].position.x)
                entities[i].velocity.x -= 10;
            else
                entities[i].velocity.x -= -10;
            if (player.position.y >entities[i].position.y)
                entities[i].velocity.y -= 10;
            else
                entities[i].velocity.y -= -10;
        }
        }
        else{
            cout<<dis<<endl;
            cout<<level<<" "<<player.enter<<endl;
            if (dis < .1){
                    if(entities[i].interacts(player)&&(player.enter==true)){
                        //cout<<"interacts"<<endl;
                        level++;
                        state = NEW;
                    }
            }
        }
    }
    /*int r = rand() % (entities.size()*int(1/elapsed));
    std::cout<<1/elapsed<<std::endl;
    for(int i=0; i < entities.size(); i++) {
        if(i == r)
            entities[i].shootBullet();
    }*/
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//clear color of screen
    glClear(GL_COLOR_BUFFER_BIT);//clear screen
    projectionMatrix.setOrthoProjection(-16.0, 16.0, -9.0f, 9.0f, -1.0f, 1.0f);//orthographic
    GLuint font = LoadTexture(RESOURCE_FOLDER"font2.png");
    unsigned int spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"spritesheet_rgba.png");
    int modifier = 0;
    float lastFrameTicks = 0.0f;
    float animationElapsed = 0.0f;
    float framesPerSecond = 5.0f;
    std::vector<floating> entities;
    int level =1;
    
    //const int lHeight = LEVEL_HEIGHT;
    //const int lWidth = LEVEL_WIDTH;
    Entity (*tiles)[80] = new Entity[80][80];
    //std::vector<std::vector<Entity>> tiles;
    
    grounded player;
    //int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 1, 4096 );
    
    //Mix_Chunk *someSound;
    //someSound = Mix_LoadWAV("");
    //Mix_PlayChannel( -1, someSound, 0);
    //int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);
    //Mix_PlayChannel( -1, someSound, 0);
    
    Mix_Music *music;
    music = Mix_LoadMUS("Fountain.mp3");
    Mix_PlayMusic(music, -1);

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
                
            case PAUSE:
                RenderPauseMenu(program, font, spriteSheetTexture);
                ProcessPauseInput(player, state, keys);
                break;
                
            case SWITCH:
                Mix_FreeMusic(music);
                state = PLAY;
                break;
                
            case NEW:
                if (level == 1){
                    init(player, state, entities, tiles);
                    Mix_FreeMusic(music);
                    music = Mix_LoadMUS("Storms.mp3");
                    Mix_PlayMusic(music, -1);
                }
                if (level == 2){
                    init2(player, state, entities, tiles);
                }
                if (level == 3){
                    init2(player, state, entities, tiles);
                }
                if (level == 4){
                    state = WIN;
                }
                break;
                
            case GAME_OVER:
                //Mix_FreeMusic(music);
                RenderLoseMenu(program, font, spriteSheetTexture);
                state = FINISH;
                break;
                
            case WIN:
                RenderWinMenu(program, font, spriteSheetTexture);
                break;
                
            case FINISH:
                clock_t time_end;
                time_end = clock() + 3 * CLOCKS_PER_SEC/1;
                while (clock() < time_end)
                {
                    RenderLoseMenu(program, font, spriteSheetTexture);
                    cout<<"finishing"<<endl;
                }
                cout<<"done"<<endl;
                done = true;
                break;
                
            case PLAY:
                RenderGameLevel(player, &program, entities, tiles, font, spriteSheetTexture);
                animationElapsed += elapsed;
                if(animationElapsed > 1.0/framesPerSecond) {
                    animate(modifier, player, entities);
                    animationElapsed = 0.0;
                }
                ProcessGameLevelInput(player, state, keys, elapsed);
                ProcessGameLevelEnemy(entities, elapsed, player, state, level);
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

