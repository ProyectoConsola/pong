#include "fabgl.h"
#include "fabutils.h"
#include <Ps3Controller.h>
#include "sprites.h"
#include "sounds.h"
#include "WiFiGeneric.h"

// Ambas barras se mueven con los dos sticks del mando
// La pelota comienza a moverse una vez se haya pulsado D-Pad abajo, o X en el mando

//Las variables van a ser globales para que no cause proplema
constexpr unsigned long TIEMPO_LIMITE = 90000;
unsigned long tiempoInicio;

using fabgl::iclamp;

fabgl::VGAController DisplayController;
fabgl::Canvas canvas(&DisplayController);
SoundGenerator soundGenerator;

int centerText(const char* text, int charLength){
  return (DisplayController.getViewPortWidth() - strlen(text) * charLength)/2;
}

// IntroScene
struct IntroScene : public Scene
{
  static const int PADDLE1_START_X = 303;
  static const int PADDLE2_START_X = 9;
  static const int PADDLE_START_Y = 81;
  static const int BALL_START_X = 157;
  static const int BALL_START_Y = 95;

  int starting_ = 0;

  SamplesGenerator *music_ = nullptr;

  IntroScene()
      : Scene(0, 20, DisplayController.getViewPortWidth(), DisplayController.getViewPortHeight())
  {
  }

  void init()
  {
    canvas.setBrushColor(0, 0, 0);
    canvas.clear();
    canvas.setGlyphOptions(GlyphOptions().FillBackground(true));
    canvas.selectFont(&fabgl::FONT_10x20);
    canvas.setPenColor(217, 245, 255);
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(1));
    canvas.drawText(centerText("PONG", 20), 20, "PONG");

    canvas.selectFont(&fabgl::FONT_8x8);
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(0));
    canvas.setPenColor(224, 158, 16);
    canvas.drawText(centerText("con ESP32 por FIE", 8), 65, "con ESP32 por FIE");
    canvas.drawText(centerText("Facultad de Ingenieria Electrica.", 8), 80, "Facultad de Ingenieria Electrica.");

    canvas.drawBitmap(PADDLE1_START_X, PADDLE_START_Y - 20, &bmpPaddle);
    canvas.drawBitmap(PADDLE2_START_X, PADDLE_START_Y + 15, &bmpPaddle);

    canvas.drawBitmap(BALL_START_X - 60, BALL_START_Y + 55, &bmpBall_2);
    canvas.drawBitmap(BALL_START_X - 58, BALL_START_Y + 57, &bmpBall_1);
    canvas.drawBitmap(BALL_START_X - 56, BALL_START_Y + 59, &bmpBall);

    canvas.setPenColor(255, 255, 255);
    canvas.drawRectangle(17, 4, 301, 5);
    canvas.drawRectangle(17, 188, 301, 189);

    music_ = soundGenerator.playSamples(themeSoundSamples, sizeof(themeSoundSamples), 100, -1);

  }

  void update(int updateCount)
  {
    
    if (starting_)
    {
      if (starting_ > 50){
        soundGenerator.detach(music_);
        stop();
      }
      ++starting_;
      canvas.scroll(0, -5);
    }
    else
    {
      if (updateCount % 20 == 0)
      {
        canvas.setPenColor(255, random(255), random(255));
        canvas.drawText(centerText("Presiona [START] para jugar", 8), 100, "Presiona [START] para jugar");
      }

      if (updateCount > 50)
      {
        if (Ps3.event.button_down.start)
          starting_ = true;
      }
    }
  }

  void collisionDetected(Sprite *spriteA, Sprite *spriteB, Point collisionPoint)
  {
  }
};
unsigned long TiempoInicio;
unsigned long TiempoTranscurrido;
// GameScene
struct GameScene : public Scene
{

  enum SpriteType
  {
    TYPE_PADDLE1,
    TYPE_PADDLE2,
    TYPE_BALL,
  };

  enum GameState
  {
    GAMESTATE_PLAYING,
    GAMESTATE_PLAYERSCORE,
    GAMESTATE_GAMEOVER,
  };

  GameState gameState_ = GAMESTATE_PLAYING;


  struct SISprite : Sprite
  {
    SpriteType type;
  };

  static const int PADDLECOUNT = 1;
  static const int BALLCOUNT = 1;
  static const int SPRITESCOUNT = 2 * PADDLECOUNT + BALLCOUNT;

  static const int BALL_START_X = 157;
  static const int BALL_START_Y = 95;
  static const int PADDLE1_START_X = 303;
  static const int PADDLE2_START_X = 9;
  static const int PADDLE_START_Y = 81;
  static const int POINT_START_X = 158;
  static const int POINT_END_X = 159;
  int POINT_START_Y = 8;
  int POINT_END_Y = 9;

  static int scoreP1_;
  static int scoreP2_;

  SISprite *sprites_ = new SISprite[SPRITESCOUNT];
  SISprite *player1_ = sprites_;
  SISprite *player2_ = player1_ + PADDLECOUNT;
  SISprite *ball_ = player2_ + PADDLECOUNT;

  int player1VelY_ = 0;
  int player2VelY_ = 0;
  
  int ballVelX = 0;
  int ballVelY = 0;
  
  bool updateScore_ = true;
  bool scored_ = false;
  bool reseted_ = true;
  bool impulsed_ = false;
  bool collided_ = false;
  
  GameScene()
      : Scene(3, 20, DisplayController.getViewPortWidth(), DisplayController.getViewPortHeight())
  {
  }

  ~GameScene()
  {
    delete[] sprites_;
  }

  void init()
  {
    TiempoInicio=millis();
    // setup player 1
    player1_->addBitmap(&bmpPaddle);
    player1_->moveTo(PADDLE1_START_X, PADDLE_START_Y);
    player1_->type = TYPE_PADDLE1;
    addSprite(player1_);

    // setup player 2
    player2_->addBitmap(&bmpPaddle);
    player2_->moveTo(PADDLE2_START_X, PADDLE_START_Y);
    player2_->type = TYPE_PADDLE2;
    addSprite(player2_);

    // setup ball
    ball_->addBitmap(&bmpBall);
    ball_->moveTo(BALL_START_X, BALL_START_Y);
    ball_->type = TYPE_BALL;
    addSprite(ball_);
    
    DisplayController.setSprites(sprites_, 3);

    canvas.setBrushColor(0, 0, 0);
    canvas.clear();

    canvas.setPenColor(255, 255, 255);
    canvas.drawRectangle(17, 4, 301, 5);
    canvas.drawRectangle(17, 188, 301, 189);
    for(int i = 1; i <= 46; i++){
      canvas.drawRectangle(POINT_START_X, POINT_START_Y, POINT_END_X, POINT_END_Y);
      POINT_START_Y += 4;
      POINT_END_Y += 4;
    }
    canvas.setGlyphOptions(GlyphOptions().FillBackground(true));
    canvas.selectFont(&fabgl::FONT_8x16);
  }
  void gameOver(){
    // disable enemies drawing, so text can be over them
    canvas.clear();
    stop();
    // show game over
    
    // change state
    gameState_ = GAMESTATE_GAMEOVER;
    
    
  }
  void moveBall(int rn){
    if((ball_->y < 7 || ball_->y > 184) && !collided_){
      soundGenerator.playSamples(invadersSoundSamples[0], invadersSoundSamplesSize[0]);      
      ballVelY = -ballVelY;
      collided_ = true;
      impulsed_ = false;
    }
    if (!impulsed_ && rn % 2){
      if(ballVelY > 3){
        ballVelY--;
      }
      else if (ballVelY < -3){
        ballVelY++;
      }
      else if(ballVelX > 3){
        ballVelX--;
      }
      else if (ballVelX < -3){
        ballVelX++;
      }
    }
    else{
      collided_ = false;
    }
    ball_->x += ballVelX;
    ball_->y += ballVelY;
    updateSpriteAndDetectCollisions(ball_);
  }

  void movePlayer(){
    if (Ps3.data.analog.stick.ry < -80){
      player1VelY_ = -2;
    }
    else if (Ps3.data.analog.stick.ry > 80){
      player1VelY_ = +2;
    }
    else{
      player1VelY_ = 0;
    }
    if (Ps3.data.analog.stick.ly < -80){
      player2VelY_ = -2;
    }
    else if (Ps3.data.analog.stick.ly > 80){
      player2VelY_ = +2;
    }
    else{
      player2VelY_ = 0;
    }
  }

  void drawScore()
  {
    canvas.setPenColor(255, 255, 255);
    canvas.drawTextFmt(130, 10, "%02d" , scoreP2_);
    canvas.drawTextFmt(172, 10, "%02d" ,scoreP1_);
  }

  void scoreGoal(){
    if (ball_->x > 308 && !scored_){
      soundGenerator.playSamples(shootSoundSamples, sizeof(shootSoundSamples));
      scoreP2_++;
      updateScore_ = true;
      scored_ = true;
    }
    else if (ball_->x < 10 && !scored_){
      soundGenerator.playSamples(shootSoundSamples, sizeof(shootSoundSamples));
      scoreP1_++;
      updateScore_ = true;
      scored_ = true;
    }
  }

  void resetBall(){
    ballVelX = 0;
    ballVelY = 0;
    ball_->moveTo(BALL_START_X, BALL_START_Y);
    reseted_ = true;
  }

  void startBall(int rn){
    int random = rand() % 100;
    if (rn % 2){
      ballVelX = 2;
    }
    else {
      ballVelX = -2;
    }
    if (random % 2){
      ballVelY = -2;
    }
    else{
      ballVelY = 2;
    }
    reseted_ = false;
  }

  void update(int updateCount)
  {
    if(updateScore_){
      drawScore();
      updateScore_ = false;
    }
    if (scored_){
      resetBall();
      scored_ = false;
    }
    if (reseted_ && (Ps3.event.analog_changed.button.cross || Ps3.event.analog_changed.button.down)){
      startBall(updateCount);
    }
    movePlayer();
    moveBall(updateCount);
    scoreGoal();
    player1_->y += player1VelY_;
    player1_->y= iclamp(player1_->y, 0, getHeight() - player1_->getHeight());
    updateSpriteAndDetectCollisions(player1_);
    player2_->y += player2VelY_;
    player2_->y= iclamp(player2_->y, 0, getHeight() - player2_->getHeight());
    updateSpriteAndDetectCollisions(player2_);

    DisplayController.refreshSprites();
    TiempoTranscurrido=(millis()-TiempoInicio)/1000;
    if(90-TiempoTranscurrido==0){
      gameOver();
      TiempoTranscurrido=0;
    }
    canvas.setPenColor(255, 255, 255);
    canvas.drawTextFmt(150, 14, "%2d", 90 - TiempoTranscurrido);
  }

  void collisionDetected(Sprite *spriteA, Sprite *spriteB, Point collisionPoint)
  {
    SISprite *sA = (SISprite *)spriteA;
    SISprite *sB = (SISprite *)spriteB;

    if (sA->type == TYPE_PADDLE1 && sB->type == TYPE_BALL){
      soundGenerator.playSamples(invadersSoundSamples[3], invadersSoundSamplesSize[3]);      
      ballVelX = -ballVelX;
      if (ballVelX >= -4)
        ballVelX--;
      if (player1VelY_){
        ballVelY += player1VelY_;
      }
      else{
        ballVelY/=2;
      }
      impulsed_ = true;
    }

    if (sA->type == TYPE_PADDLE2 && sB->type == TYPE_BALL){
      soundGenerator.playSamples(invadersSoundSamples[3], invadersSoundSamplesSize[3]);      
      ballVelX = -ballVelX;
      if (ballVelX <= 4)
        ballVelX++;
      if (player2VelY_){
        ballVelY += player2VelY_;
      }
      else{
        ballVelY/=2;
      }
      impulsed_ = true;
    }
  }
};

int GameScene::scoreP1_ = 0;
int GameScene::scoreP2_ = 0;
struct FinalScene : public Scene{

  FinalScene()
      : Scene(0, 20, DisplayController.getViewPortWidth(), DisplayController.getViewPortHeight())
  {
  }
  void init(){
    canvas.clear();
    canvas.setBrushColor(0, 0, 0);
    canvas.setPenColor(248, 252, 167);//248, 252, 67
    canvas.setBrushColor(28, 35, 92);
    canvas.fillRectangle(40, 60, 270, 130);
    canvas.drawRectangle(40, 60, 270, 130);
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(0));
    canvas.setPenColor(255, 255, 255);
    canvas.drawText(80, 72, "TIEMPO TERMINADO!");//55, 80
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(0));
   // canvas.setPenColor(248, 252, 167);
   // canvas.drawText(100, 110, "Presiona [START]");

  }
  void update(int updateCount)
  {
  }
  void collisionDetected(Sprite *spriteA, Sprite *spriteB, Point collisionPoint)
  {
  }

};
void setup()
{
  Ps3.begin("24:6f:28:af:1c:66");
  DisplayController.begin();
  DisplayController.setResolution(VGA_320x200_75Hz);
}

void loop()
{
  IntroScene introScene;
  introScene.start();
  GameScene gameScene;
  gameScene.start();
  FinalScene finalScene;
  finalScene.start();
}