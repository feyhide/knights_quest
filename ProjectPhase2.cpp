#include "raylib.h"
#include "raymath.h"
#include <iostream>
using namespace std;

const float screenWidth = 1200;
const float screenHeight = 550;
const float gravity = 0.8;
const int groundYpos = 400;
int firststart = 0;

class Character {
    friend class camera;
public:
    Character(const string& texturePath, float screenWidth, float screenHeight, int speed)
        : screenWidth(screenWidth), screenHeight(screenHeight), characterSpeed(speed), attacking(false)
    {
        loadingCharactertextures();
        healthbarpng = LoadTexture("healthbar.png");
        texture = LoadTexture(texturePath.c_str());
        standing();
        position = { (screenWidth / 2)-100, groundYpos };
        isattacking = 0;
        washurting = 0;
        isblocking = 0; attacked = 0;
        health = 100;
    }

    void Update(int stop ) {

        if (wasfacingleft) {
            facingleft();
        }
        else if (wasfacingright) {
            facingright();
        }

        healthbar();
        isblocking = 0; attacked = 0;
        //cout << "position of character : " << position.x << endl;

        isrunning = 0;
        if (stop == 0) {
            if (health == 0) {
                // Character is dead, switch to death animation
                isDead = true;
                playerDead();
                //PlaySound(death);
            }
            if (isontheground() && !isDead) {
                lastposition = position;
                washurting = 0;
                if (eitherpermission() && ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_SPACE)))) {
                    velocity.y = -7 * characterSpeed;
                    isJumping = true;
                    jumping();
                }
                else if ((IsKeyDown(KEY_DOWN)) && !isRunning) {
                    isblocking = 1;
                    blockinganimation();
                }
                else if (IsKeyDown(KEY_RIGHT) && permissiontogoright) {
                    velocity.x = characterSpeed;
                    isrunning = 1;
                    flagleft = 0;
                    flagright = 1;
                    isRunning = true;
                }
                else if (IsKeyDown(KEY_LEFT) && permissiontogoleft) {
                    velocity.x = -characterSpeed;
                    isrunning = 1;
                    flagleft = 1;
                    flagright = 0;
                    isRunning = true;
                }
                else {
                    isRunning = false;
                    velocity.x = 0;
                }
            }
        }

        if (IsKeyPressed(KEY_A) && !isDead) {
            PlaySound(woosh);

            attackinganimation();
            attacked = 1;
        }


        gravitydown();

        position.x += velocity.x;
        position.y += velocity.y;

        position.x = Clamp(position.x, frameWidth - 10, screenWidth - frameWidth);
        position.y = Clamp(position.y, 0, groundYpos);

        if (isJumping && !isontheground()) {
            jumping();
        }
        else {
            isJumping = false;
        }

        bool isRunning = velocity.x != 0.0f;
        bool ontheground = isontheground();
        positioning();

        flippingframe();
        if (attacking) {
            if (frameRunningIndex < numFramesWidth) {
                frameDelayCounter++;
                if (frameDelayCounter >= frameDelay) {
                    frameDelayCounter = 0;
                    frameRunningIndex++;
                    if (frameRunningIndex >= numFramesWidth) {
                        frameRunningIndex = 0;
                        attacking = false;
                    }
                    frameRec.x = frameWidth * frameRunningIndex;
                }
            }
        }
        else if(!isDead){
            ++frameDelayCounter;
            if (frameDelayCounter > frameDelay) {
                frameDelayCounter = 0;
                if (isRunning) {
                    if (ontheground) {
                        running();
                    }
                    else if (isJumping) {
                        if (frameRunningIndex < numFramesWidth) {
                            frameDelayCounter++;
                            if (frameDelayCounter >= frameDelay) {
                                frameDelayCounter = 0;
                                frameRunningIndex++;
                                if (frameRunningIndex >= numFramesWidth) {
                                    frameRunningIndex = 0;
                                    attacking = false;
                                }
                                frameRec.x = frameWidth * frameRunningIndex;
                            }
                        }
                        //jumping(); // Start or continue the jump animation
                    }
                    positioning();
                    ++frameRunningIndex;
                    frameRunningIndex %= numFramesWidth;

                    frameRec.x = (float)frameWidth * frameRunningIndex;
                }
                else {
                    standing();
                    positioning();
                }
            }
        }
    }

    virtual void deadAnimation() {
        textureDead = LoadTexture("Knight_1/Dead.png");
        frameRec.width = deathFrameWidth / numDeathFrames;
        position = lastposition;
    }

    void Draw() const
    {
        if (!isDead) {
            DrawTextureRec(texture, frameRec, position, WHITE);
        }
    }

    void playerDead() {
        if (isDead && quitDeadAnimation == 0) {
            // Load the dead texture and set initial frame parameters
            textureDead = LoadTexture("Knight_1/Dead.png");
            frameRec.width = deathFrameWidth / numDeathFrames;
            position = lastposition;

            cout << "DEAD ANIMATION started " << endl;
            // Draw the dead animation frame by frame
            DrawTextureRec(textureDead, frameRec, position, WHITE);

            // Advance the animation frame index
            frameDelayCounter++;
            cout << "frame dead delay counter " << frameDelayCounter << endl;
            if (frameDelayCounter >= deadframeDelay) {
                cout << "frame dead counter " << frameDeadIndex << endl;
                frameDeadIndex++;
                frameDelayCounter = 0;
            }

            // Check if the animation has completed
            if (frameDeadIndex >= numDeathFrames) {
                frameDeadIndex = 0;
                isPlayerDead = 0; // Reset the dead animation flag
                quitDeadAnimation = 1; // Set to quit dead animation
                cout << "DEAD ANIMATION COMPLETED" << endl;
            }

            // Calculate the frame rectangle for the next frame
            frameRec.x = frameDeadIndex * frameRec.width;
        }
    }

    int getpositionx() {
        return position.x;
    }

public:

    virtual void healthbar() {
        if (!isDead) {
            DrawTextureEx(healthbarpng, { 200,10 }, 0, 0.3, WHITE);
            DrawText("YOU", 155, 18, 20, BLACK);

            float healthBarWidth = ((float)health / 100.0f) * 215.0f;

            DrawRectangle(208, 18, 215, 20, Color{ 230, 41, 55, 255 });

            DrawRectangle(208, 18, healthBarWidth, 20, GREEN);
        }
    }

    virtual void loadingCharactertextures() {
        textureHurting = LoadTexture("Knight_1/Hurt.png");
        textureAttacking = LoadTexture("Knight_1/Attack1.png");
        textureBlocking = LoadTexture("Knight_1/Protect.png");
        textureRunningAttack = LoadTexture("Knight_1/Run+Attack.png");
        textureJumping = LoadTexture("Knight_1/Jump.png");
        textureRunning = LoadTexture("Knight_1/Run.png");
        textureIdle = LoadTexture("Knight_1/Idle.png");
    }


    void setframe() {
        frameRec = { 0.0f, 0.0f, (float)texture.width / numFramesWidth, (float)texture.height / numFramesHeight };
        frameWidth = texture.width / numFramesWidth;
        frameHeight = texture.height / numFramesHeight;
    }

    bool isontheground() {
        return position.y + texture.height >= groundYpos;
    }

    void flippingframe() {
        if (flagleft == 1) {
            facingleft();
            wasfacingleft = 1;
            wasfacingright = 0;
        }
        else if (flagright == 1) {
            facingright();
            wasfacingleft = 0;
            wasfacingright = 1;
        }
    }

    void facingleft() {
        wasfacingright = 0;
        wasfacingleft = 1;
        if (frameRec.width > 0) {
            frameRec.width = -frameRec.width;
        }
    }

    void facingright() {
        wasfacingright = 1;
        wasfacingleft = 0;
        if (frameRec.width < 0) {
            frameRec.width = -frameRec.width;
        }
    }

    void blockinganimation() {
        blocking();
        frameDelayCounter = 0;
        frameRunningIndex = 0;
        positioning();

        frameRec.x = (float)frameWidth;
    }


    void positioning() {
        if (!isontheground()) {
            velocity.y += gravity;
        }
        position = Vector2Add(position, velocity);
        position.x = Clamp(position.x, frameWidth - 10, screenWidth - frameWidth);
        if (isontheground()) {
            position.y = groundYpos - texture.height;
            velocity.y = 0;
        }
    }

    void gravitydown() {
        velocity.y += gravity;
    }

    void running() {
        numFramesWidth = 7;
        numFramesHeight = 1;
        //UnloadTexture(texture); // Unload previous texture
        texture = textureRunning;
        setframe();

    }


    virtual void attackinganimation() {
        if (!attacking && IsKeyDown(KEY_A)) {
            attacking = true;
            frameDelayCounter = 0;
            if (flagleft == 1 || flagright == 1) {
                numFramesWidth = 6;
                texture = textureRunningAttack;
                setframe();
                frameRunningIndex = 0;
            }
            else {
                numFramesWidth = 5;
                texture = textureAttacking;
                setframe();
                frameRunningIndex = 0;
            }
        }
        if (wasfacingleft == 1) {
            facingleft();
        }
        else if (wasfacingright == 1) {
            facingright();
        }
        attacked = 0;
    }

    void wherewasilooking() {
        if (wasfacingleft == 1) {
            facingleft();
        }
        else if (wasfacingright == 1) {
            facingright();
        }
    }

    void blocking() {
        numFramesWidth = 1;
        numFramesHeight = 1;
        //UnloadTexture(texture); // Unload previous texture
        texture = textureBlocking;
        setframe();
        wherewasilooking();
    }

    void jumping() {
        numFramesWidth = 6;
        numFramesHeight = 1;
        //UnloadTexture(texture); // Unload previous texture
        texture = textureJumping;
        setframe();
        wherewasilooking();
    }

    void standing() {
        numFramesWidth = 4;
        numFramesHeight = 1;
        //UnloadTexture(texture); // Unload previous texture
        texture = textureIdle;
        setframe();

        // Adjust facing direction based on previous movement
        wherewasilooking();
    }

    virtual void resetpermission() {
        permissiontogoright = 1;
        permissiontogoleft = 1;
    }
    virtual int eitherpermission() {
        if (permissiontogoright || permissiontogoleft) {
            return 1;
        }
    }
    Sound death = LoadSound("die.mp3");
    Sound woosh = LoadSound("Knight_1/Woosh-Char.wav");

    int permissiontogoright;
    int permissiontogoleft;
    Vector2 lastposition = { 0.0f, 0.0f };
    int isattacking;
    int attacked;
    int isblocking;
    int health;
    int washurting;
    int wasfacingleft;
    int wasfacingright;
    int flagleft;
    int flagright;
    int isrunning;
    bool attacking;
    bool isRunning = false;
    bool isJumping = false;
    const int screenWidth;
    const int screenHeight;
    const int characterSpeed;
    int numFramesWidth;
    int numFramesHeight;
    Texture2D healthbarpng;
    Texture2D textureHurting;
    Texture2D texture;
    Texture2D textureIdle;
    Texture2D textureJumping;
    Texture2D textureAttacking;
    Texture2D textureRunning;
    Texture2D textureRunningAttack;
    Texture2D textureBlocking;
    Texture2D textureDead;
    Rectangle frameRec;
    Vector2 position;
    Vector2 velocity = { 0.0f, 0.0f };
    bool isDead = false;
    int numDeathFrames = 6;
    int deathFrameWidth = 480;
    unsigned frameDelay = 10;
    unsigned deadframeDelay = 6;
    unsigned frameDelayCounter = 0;
    int frameWidth;
    int frameHeight;
    unsigned frameRunningIndex = 0;
    unsigned frameDeadIndex = 0;
    int isPlayerDead = 0;
    int quitDeadAnimation = 0;
};

class Enemy : public Character {

    friend class camera;
public:
    Enemy(const string& texturePath, float screenWidth, float screenHeight, int speed)
        : Character(texturePath, screenWidth, screenHeight, speed)
    {

        stopped = 0;
        enemypermissiontogoleft = 0;
        enemypermissiontogoright = 0;
        isattacking = 0;
        cheatingtimes = 0;
        times = 0;
        isblocking = 0;
        isrunning = 0;
        canrunleft = 0;
        canrunright = 0;
        canattack = 0;
        canjump = 0; 
        canblock = 0;
        canrunattack = 0;
        attacked = 0;
        attacking = false;
        health = 100;
        position = { (screenWidth / 2)+200, groundYpos };
        srand(static_cast<unsigned>(time(nullptr))); // Seed for random number generation
        loadingCharactertextures();
    }

    void resetpermission() override {
        enemypermissiontogoright = 1;
        enemypermissiontogoleft = 1;
    }

    virtual int eitherpermission() {
        if (enemypermissiontogoright || enemypermissiontogoleft) {
            return 1;
        }
    }
    void Update(int stop) {
        int randomBehavior;
        if (firststart == 0) {
            times = 0;
            prevrandom = 0;
        }
        isblocking = 0;
        isrunning = 0;
        int selectdone = 0;
        attacked = 0;
        healthbar();
        cout << "position of enemy : " << position.x << endl;
        lastposition = position;

        if (stop == 0) {
            int maxrandomnum = canattack + canblock + canjump + canrunattack + canrunleft + canrunright;

            randomBehavior = rand() % 4;
            while (selectdone == 0 && firststart != 0) {
                randomBehavior = rand() % 4;
                if (times == 0 && (prevrandom != randomBehavior)) {
                    randomBehavior = prevrandom;
                    times++;
                }
                else {
                    times = 0;
                }
                selectdone = 1;
            }


            if (health <= 0) {
                isDead = true;
                playerDead();
            }
            else {
                switch (randomBehavior) {

                case 0:
                    //Move left

                    if (canrunleft == 1 && enemypermissiontogoleft) {

                        isrunning = 1;
                        attacking = false;
                        facingleft();
                        velocity.x = -characterSpeed;
                        running();
                    }

                    break;
                case 1:
                    // Attack

                    if (canattack == 1) {
                        PlaySound(woosh);
                        attacked = 1;
                        attacking = true;
                        attackinganimation();
                    }


                    break;
                case 2:
                    // Move right
                    if (canrunright == 1 && enemypermissiontogoright) {

                        isrunning = 1;
                        attacking = false;
                        facingright();
                        velocity.x = characterSpeed;
                        running();
                    }

                    break;
                case 3:
                    // Jump

                    if (canjump == 1) {
                        attacking = false;
                        if (isontheground()) {
                            velocity.y = -5 * characterSpeed;
                            isJumping = true;
                            jumping();
                        }
                    }

                    break;
                case 4:
                    // Block
                    if (canblock == 1) {

                        isblocking = 0;
                        attacking = 0;
                        blockinganimation();
                    }
                    break;
                default:

                    break;
                }

            }
            gravitydown();

            position.x += velocity.x;
            position.y += velocity.y;

            position.x = Clamp(position.x, frameWidth - 10, screenWidth - frameWidth);
            position.y = Clamp(position.y, 0, groundYpos);

            if (isJumping && !isontheground()) {
                jumping();
            }
            else {
                isJumping = false;
            }

            bool isRunning = velocity.x != 0.0f;
            bool ontheground = isontheground();
            positioning();

            flippingframe();
            if (attacking) {
                if (frameRunningIndex < numFramesWidth) {
                    frameDelayCounter++;
                    if (frameDelayCounter >= frameDelay) {
                        frameDelayCounter = 0;
                        frameRunningIndex++;
                        if (frameRunningIndex >= numFramesWidth) {
                            frameRunningIndex = 0;
                            attacking = false;
                        }
                        frameRec.x = frameWidth * frameRunningIndex;
                    }
                }
            }

            ++frameDelayCounter;
            if (frameDelayCounter > frameDelay) {
                frameDelayCounter = 0;
                if (isRunning) {
                    if (ontheground) {
                        running();
                    }
                    else {
                        jumping(); // Start or continue the jump animation
                    }
                    positioning();
                    ++frameRunningIndex;
                    frameRunningIndex %= numFramesWidth;

                    frameRec.x = (float)frameWidth * frameRunningIndex;
                }
                else {
                    standing();
                    positioning();
                }

            }
        }
    }

    
    void stop() {
        stopped = 1;
    }

     void deadAnimation() override {
        textureDead = LoadTexture("Knight_3/Dead.png");
        frameRec.width = deathFrameWidth / numDeathFrames;
     }

     void playerDead() {
         if (isDead && quitDeadAnimation == 0) {
             textureDead = LoadTexture("Knight_3/Dead.png");
             frameRec.width = deathFrameWidth / numDeathFrames;
             position = lastposition;

             cout << "DEAD ANIMATION started" << endl;

             // Set facing direction for the dead animation
             if (wasfacingleft == 1) {
                 facingleft();
             }
             else if (wasfacingright == 1) {
                 facingright();
             }

             // Draw the dead animation frame by frame
             DrawTextureRec(textureDead, frameRec, position, WHITE);

             // Advance the animation frame index
             frameDelayCounter++;
             if (frameDelayCounter >= deadframeDelay) {
                 frameDeadIndex++;
                 frameDelayCounter = 0;
             }

             // Check if the animation has completed
             if (frameDeadIndex >= numDeathFrames) {
                 frameDeadIndex = 0;
                 isPlayerDead = 0; // Reset the dead animation flag
                 quitDeadAnimation = 1; // Set to quit dead animation
                 cout << "DEAD ANIMATION COMPLETED" << endl;
             }

             // Calculate the frame rectangle for the next frame
             frameRec.x = frameDeadIndex * frameRec.width;
         }
     }



    void Draw() const {
        if (isDead) {
            DrawTextureRec(textureDead, frameRec, position, WHITE);
        }
        else {
            DrawTextureRec(texture, frameRec, position, WHITE);
        }
    }

    void setCanRunLeft(int value) {
        canrunleft = value;
    }

    void setCanRunRight(int value) {
        canrunright = value;
    }

    void setCanAttack(int value) {
        canattack = value;
    }

    void setCanJump(int value) {
        canjump = value;
    }

    void setCanBlock(int value) {
        canblock = value;
    }

    void setCanRunAttack(int value) {
        canrunattack = value;
    }

    void attackinganimation() {
        numFramesWidth = 6;
        //UnloadTexture(texture);
        texture = textureRunningAttack;
        setframe();
        frameRunningIndex = 0;
        if (wasfacingleft == 1) {
            facingleft();
        }
        else if (wasfacingright == 1) {
            facingright();
        }
        attacked = 1;
    }


    int enemypermissiontogoleft;
    int enemypermissiontogoright;


    void healthbar() override {
        if (!isDead) {
            DrawTextureEx(healthbarpng, { 800,10 }, 0, 0.3, WHITE);

            DrawText("ANGY", 740, 18, 20, BLACK);

            float healthBarWidth = ((float)health / 100.0f) * 215.0f;

            DrawRectangle(808, 18, 215, 20, Color{ 230, 41, 55, 255 });

            DrawRectangle(808, 18, healthBarWidth, 20, GREEN);
        }

    }

    int canrunleft;
    int canrunright;
    int canattack;
    int canjump; 
    int prevrandom;
    int cheatingtimes;
    Vector2 lastposition = { 0.0f, 0.0f };
    int stopped;
    int canblock;
    int times;
    int canrunattack; 
    Sound woosh = LoadSound("Knight_3/Woosh-Enemy.wav");

protected:

    void loadingCharactertextures() override {
        textureHurting = LoadTexture("Knight_3/Hurt.png");
        textureAttacking = LoadTexture("Knight_3/Attack.png");
        textureBlocking = LoadTexture("Knight_3/Protect.png");
        textureRunningAttack = LoadTexture("Knight_3/Run+Attack.png");
        textureJumping = LoadTexture("Knight_3/Jump.png");
        textureRunning = LoadTexture("Knight_3/Run.png");
        textureIdle = LoadTexture("Knight_3/Idle.png");
    }

};

class relativemotion {
public:
    relativemotion(Character& player, Enemy& enemy) : player(player), enemy(enemy) {
        returnflag = 0;
    }
    int checkforcamera() {
        if ((player.getpositionx() <= 200 && player.getpositionx() >= 0)) {
            return 0; // left
        }
        else if ((player.getpositionx() <= 1200 && player.getpositionx() >= 900)) {
            return 1; // right
        }
        else {
            return 2; // reset camera
        }
    }


    void damage() {
        if (returnflag == 1) {
            if (enemy.attacked && player.isblocking == 0) {
                cout << "enemyattack" << endl;
                player.health -= 0.00001;
            }
            else if (player.attacked && enemy.isblocking == 0) {
                cout << "playerattack" << endl;
                enemy.health -= 6;
            }
        }
        cout << "player attacking "<< player.attacked << endl;
        cout << "enemy attacking " << enemy.attacked << endl;
        cout << "player blocking " << player.isblocking <<endl;
        cout << "enemy blocking " << enemy.isblocking << endl;
        cout << "playerHealth : " << player.health << endl;
        cout << "enemyHealth : " << enemy.health << endl;
    }


    void check() {

        //cout << "Enemy abilities:" << endl;
        //cout << "Can run left: " << enemy.canrunleft << endl;
        //cout << "Can run right: " << enemy.canrunright << endl;
        //cout << "Can attack: " << enemy.canattack << endl;
        //cout << "Can jump: " << enemy.canjump << endl;
        //cout << "Can block: " << enemy.canblock << endl;
        //cout << "Can run and attack: " << enemy.canrunattack << endl;
        

        returnflag = 0;
        distance = player.getpositionx() - enemy.getpositionx();

        cout << "distance between : " << distance << endl;
        if (distance < 0) {
            enemy.setCanAttack(0);
            enemy.setCanBlock(0);
            enemy.setCanJump(0);
            enemy.setCanRunLeft(1);
            enemy.setCanRunAttack(0);
            enemy.setCanRunRight(0);
            enemy.facingleft();
        }
        else if (distance > 0) {
            enemy.setCanAttack(0);
            enemy.setCanBlock(0);
            enemy.setCanJump(0);
            enemy.setCanRunLeft(0);
            enemy.setCanRunAttack(0);
            enemy.setCanRunRight(1);
            enemy.facingright();
        }
        
        
        if (distance <= 100 && distance >= 50) {
            enemy.setCanAttack(1);
            enemy.setCanBlock(1);
            enemy.setCanJump(1);
            enemy.setCanRunLeft(0);
            enemy.setCanRunAttack(1);
            enemy.setCanRunRight(1);
        }
        else if (distance >= -100 && distance <= 50) {
            enemy.setCanAttack(1);
            enemy.setCanBlock(1);
            enemy.setCanJump(1);
            enemy.setCanRunLeft(1);
            enemy.setCanRunAttack(1);
            enemy.setCanRunRight(0);
        }

        if (distance <= 50 && distance >= 0) {
            enemy.setCanAttack(1);
            enemy.setCanBlock(0);
            enemy.setCanJump(0);
            enemy.setCanRunLeft(1);
            enemy.setCanRunAttack(0);
            enemy.setCanRunRight(0);
            returnflag = 1;
        }
        else if (distance >= -50 && distance <= 0) {
            enemy.setCanAttack(1);
            enemy.setCanBlock(0);
            enemy.setCanJump(0);
            enemy.setCanRunLeft(0);
            enemy.setCanRunAttack(0);
            enemy.setCanRunRight(1);
            returnflag = 1;
        }

        
        if (distance <= 10 && distance >= 0) {

            //character is on right side   
            player.permissiontogoleft = 0;
            player.permissiontogoright = 1;
            enemy.enemypermissiontogoleft = 1;
            enemy.enemypermissiontogoright = 0;
            returnflag = 1;
        }
        else if (distance >= -10 && distance < 0) {
            //character is on left side
            player.permissiontogoleft = 1;
            player.permissiontogoright = 0;
            enemy.enemypermissiontogoleft = 0;
            enemy.enemypermissiontogoright = 1;
            returnflag = 1;
        }
        else {
            enemy.resetpermission();
            player.resetpermission();
        }

        if (enemy.getpositionx() == 61) {
            enemy.setCanRunLeft(0);
        }
        else if (enemy.getpositionx() == 725) {
            enemy.setCanRunRight(0);
        }
    }

private:
    int returnflag;
    int distance;
    int distanceforcamera;
    Character& player;
    Enemy& enemy;
};


class MainMenu {
public:
    Font font = LoadFont("resources/fonts/alpha_beta.png");


    MainMenu(int width, int height) : screenWidth(width), screenHeight(height) {
        mainmenu = LoadTexture("mainmenu.png");
        final = LoadTexture("final.png");
        paused = LoadTexture("paused.png");
        control = LoadTexture("controls.png");
        mainsound = LoadSound("soundtrack.wav");
        pause = 0;
        gotomainmenu = 0;
    }

    void Draw() {
        // Draw menu items
        PlaySound(mainsound);
        DrawTexture(mainmenu, 0, 0, WHITE);
    }
    void DrawControl() {
        // Draw menu items
        DrawTexture(control, 0, 0, WHITE);
    }

    void finaldraw() {
        PlaySound(mainsound);
        // Draw menu items
            DrawTexture(final, 0, 0, WHITE);
    }

    

    int Update() {
        // Check for user input
        if (IsKeyPressed(KEY_ENTER)) {
            return 1; // Start the game
        }
        else if (IsKeyPressed(KEY_ESCAPE)) {
            return -1; // Quit the game
        }
        return 0; // Stay in the menu
    }
    int controlUpdate() {
        // Check for user input
        if (IsKeyPressed(KEY_ENTER)) {
            return 1; // Start the game
        }
        else if (IsKeyPressed(KEY_ESCAPE)) {
            return -1; // Quit the game
        }
        return 0; // Stay in the menu
    }
    int getpause() {
        return pause;
    }
    void setpause(int value) {
        pause = value;
    }

    int getgotomainmenu() {
        return gotomainmenu;
    }

    void setgotomainmenu(int value) {
        gotomainmenu = value;
    }

    void drawpause() {

        PlaySound(mainsound);
        DrawTexture(paused, 0, 0, WHITE);
    }

    int pauseupdate() {
        // Check for user input
        if (IsKeyPressed(KEY_ENTER)) {
            pause = 0; // Start the game
        }
        else if (IsKeyPressed(KEY_Z)) {
            //Something
        }
        return 0; // Stay in the menu
    }

    int finalupdate() {
        // Check for user input
        if (IsKeyPressed(KEY_R)) {
            return 1; // Start the game
        }
        else if (IsKeyPressed(KEY_E)) {
            //Something
            return 0;
        }
        return 0; // Stay in the menu
    }



private:
    Sound mainsound;
    Texture2D win;
    Texture2D lose;
    Texture2D paused;
    Texture2D mainmenu;
    Texture2D final;
    Texture2D control;
    int gotomainmenu;
    int pause;
    int screenWidth;
    int screenHeight;
};


class camera {
public:
    camera() {
        cameraXaxisBattle = -350;
        MIDDLEcameraXaxisBattle = -350;
    }

    void setcameraXaxisBattle(int value) {
        cameraXaxisBattle = value;
    }

    int getcameraXaxisBattle() {
        return cameraXaxisBattle;
    }

    int getcameramovement() {
        return cameramovement;
    }

    void setcameramovement(int value) {
        cameramovement = value;
    }

    void parallex(Character& player, Enemy& enemy) {
        if (cameramovement == 1 && player.isrunning == 1 && enemy.isrunning == 1) {
            if (cameraXaxisBattle > - 400) {
                cameraXaxisBattle -= 3;
            }
        }
        else if (cameramovement == 0 && player.isrunning == 1 && enemy.isrunning == 1) {
            if (cameraXaxisBattle < -300) {
                cameraXaxisBattle += 3;
            }
        }
        else if (cameramovement == 2 && player.isrunning == 1 && enemy.isrunning == 1) {
            if (cameraXaxisBattle != MIDDLEcameraXaxisBattle) {
                if (cameraXaxisBattle < MIDDLEcameraXaxisBattle) {
                    cameraXaxisBattle += 3;
                }
                else if (cameraXaxisBattle > MIDDLEcameraXaxisBattle) {
                    cameraXaxisBattle -= 3;
                }
            }
        }
    }

private:
    int cameramovement;
    int cameraXaxisBattle;
    int MIDDLEcameraXaxisBattle;
};



int plsstop1 = 0;
int plsstop2 = 0;
int winner = 0;

int main(void)
{
    //int cameraXaxisBattle = -350;
    //int MIDDLEcameraXaxisBattle = -350;

    InitWindow(screenWidth, screenHeight, "KNIGHT'S QUEST - THE BATTLE FOR THE KINGDOM");

    InitAudioDevice();

    camera cam;
    MainMenu mainMenu(screenWidth, screenHeight);
    Character player("Knight_1/Idle.png", screenWidth, screenHeight, 2);
    Enemy enemy("Knight_3/Idle.png", screenWidth, screenHeight, 1);
    relativemotion movement(player, enemy); // Pass references to player and enemy
    cout << "camera : " << cam.getcameraXaxisBattle() << endl;
    SetTargetFPS(60);

    Sound stabC = LoadSound("Knight_1/Stab-Char.wav");
    Sound gruntC = LoadSound("Knight_1/Grunt-Char.wav");
    Sound stabE = LoadSound("Knight_3/Stab-Enemy.wav");
    Sound gruntE = LoadSound("Knight_3/Grunt-Enemy.wav");

    int weretheyclose = 0;
    int playerishurting = 0;
    int someonedead = 0;

    Texture2D background = LoadTexture("background1.png");
    Vector2 camerapositionfinal = { 0 , 0};


    if (background.id <= 0)
    {
        // Texture loading failed, print an error message or handle the failure
        cerr << "Failed to load background texture!" << endl;
    }

    int menuResult = 0; // Result from the main menu, 0: stay in menu, 1: start game, -1: quit
    int finalResult = 0;
    int menuResult2 = 0;
    while (!WindowShouldClose()) {
        

        while (!WindowShouldClose() && menuResult == 0) {
            BeginDrawing();
            finalResult = 0;
            someonedead = 0; plsstop1 = 0; plsstop2 = 0;
            ClearBackground(RAYWHITE);
            // Update and draw the main menu
            menuResult = mainMenu.Update();
            mainMenu.Draw();

            EndDrawing();
        }

        while (!WindowShouldClose() && menuResult == 1) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            // Update and draw the main menu
            menuResult2 = mainMenu.controlUpdate();
            if (menuResult2 == 1) {
                menuResult = 0;
            }
            mainMenu.DrawControl();

            EndDrawing();
        }

        if(menuResult2==2){
            someonedead = 1; 
        }

        if (menuResult2 == 1) {
            while (mainMenu.getgotomainmenu() == 0 && !WindowShouldClose())
            {
                if (IsKeyPressed(KEY_Z)) {
                    mainMenu.setpause(1);
                }

                if (mainMenu.getpause() == 0) {
                    cam.setcameramovement(movement.checkforcamera());
                    ClearBackground(RAYWHITE);
                    cam.parallex(player,enemy);
                    int camXaxis = cam.getcameraXaxisBattle();
                    cout << "camera : " << camXaxis << endl;
                    Vector2 cameraposition = { camXaxis , 0 };
                    camerapositionfinal = cameraposition;
                    DrawTextureEx(background, cameraposition, 0.0f, 1.0f, WHITE);
                    if (player.isDead) {
                        winner = 0;
                        plsstop1 = 1;
                        menuResult2 = 2;
                    }
                    else if (enemy.isDead) {
                        winner = 1;
                        plsstop2 = 1;
                        menuResult2 = 2;
                    }
                    player.Update(plsstop2);
                    enemy.Update(plsstop1);
                    movement.check();
                    movement.damage();
                    
                }
                else {
                    ClearBackground(RAYWHITE);
                    mainMenu.pauseupdate();
                }

                BeginDrawing();
                if (mainMenu.getpause() == 0) {
                    player.Draw();
                    enemy.Draw();
                }
                else {
                    mainMenu.drawpause();
                }

                while (!WindowShouldClose() && menuResult2 == 2) {
                    BeginDrawing();
                    Texture2D background2 = LoadTexture("win.png");
                    ClearBackground(RAYWHITE);
                    if (!plsstop2) {
                        background2 = LoadTexture("lose.png");
                    }
                    else {
                        background2 = LoadTexture("win.png");
                    }
                    DrawTexture(background2, 0, 0, WHITE);
                    // Update and draw the main menu
                    //mainMenu.finaldraw(winner);
                    if (player.isDead && player.quitDeadAnimation == 0) {
                        player.Draw();
                        player.Update(plsstop2);
                    }
                    else if (enemy.isDead && enemy.quitDeadAnimation == 0) {
                        enemy.Draw();
                        enemy.Update(plsstop1);
                    }

                    if (player.quitDeadAnimation || enemy.quitDeadAnimation) {
                        mainMenu.finaldraw();
                        finalResult = mainMenu.finalupdate();
                    }

                    EndDrawing();
                }


                EndDrawing();
            }
        }
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

