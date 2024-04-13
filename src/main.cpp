#include "fabgl.h"
#include "fabutils.h"
#include <Ps3Controller.h>
#include "sprites.h"
#include "sounds.h"
#include "WiFiGeneric.h"

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
  static const int POINT_START_X = 158;
  static const int POINT_END_X = 159;
  int POINT_START_Y = 8;
  int POINT_END_Y = 9;

  //int textRow_ = 0;
  //int textCol_ = 0;
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

    canvas.drawBitmap(BALL_START_X - 56 - 4, BALL_START_Y + 59 - 4, &bmpBall_2);
    canvas.drawBitmap(BALL_START_X - 56 - 2, BALL_START_Y + 59 - 2, &bmpBall_1);
    canvas.drawBitmap(BALL_START_X - 56, BALL_START_Y + 59, &bmpBall);

    canvas.setPenColor(255, 255, 255);
    canvas.drawRectangle(17, 4, 301, 5);
    canvas.drawRectangle(17, 188, 301, 189);
  }

  void update(int updateCount)
  {
    
    if (starting_)
    {

      if (starting_ > 50)
      {
        // stop music
        soundGenerator.detach(music_);
        // stop scene
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

      // handle keyboard or mouse (after two seconds)
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

  int player1VelY_ = 0; // 0 = no move
  int player2VelY_ = 0;
  
  bool updateScore_ = true;
  
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
  }

  void moveBall(int x, int y, bool *touchSide){
     if (ball_->visible)
    {
      if (x <= 0 || x >= getWidth() - ball_->getWidth())
        *touchSide = true;
      ball_->moveTo(x, y);
      ball_->setFrame(ball_->getFrameIndex() ? 0 : 1);
      updateSprite(ball_);
    }

  }

  void drawScore()
  {
    canvas.setBrushColor(0, 0, 0);
    canvas.setPenColor(255, 255, 255);
    canvas.selectFont(&fabgl::FONT_8x16);
    canvas.drawTextFmt(130, 10, "%02d" , scoreP1_);
    canvas.drawTextFmt(172, 10, "%02d" ,scoreP2_);
  }

  void update(int updateCount)
  {
    if(updateScore_){
      drawScore();
      updateScore_ = false;
    }
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
    player1_->y += player1VelY_;
    player1_->y= iclamp(player1_->y, 0, getHeight() - player1_->getHeight());
    updateSprite(player1_);
    player2_->y += player2VelY_;
    player2_->y= iclamp(player2_->y, 0, getHeight() - player2_->getHeight());
    updateSprite(player2_);

    


    DisplayController.refreshSprites();
  }

  void GameOver(){
  }



  void collisionDetected(Sprite *spriteA, Sprite *spriteB, Point collisionPoint)
  {
  }
};

int GameScene::scoreP1_ = 0;
int GameScene::scoreP2_ = 0;

void setup()
{
  Ps3.begin("78:dd:08:4d:94:a4");
  DisplayController.begin();
  DisplayController.setResolution(VGA_320x200_75Hz);
}

void loop()
{
  /*if (true)
  {
    IntroScene introScene;
    introScene.start();
  }*/
  GameScene gameScene;
  gameScene.start();
}