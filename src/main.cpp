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

// IntroScene
struct IntroScene : public Scene
{

  static const int TEXTROWS = 4;
  static const int TEXT_X = 130;
  static const int TEXT_Y = 122;

  int textRow_ = 0;
  int textCol_ = 0;
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
    canvas.selectFont(&fabgl::FONT_8x8);
    canvas.setPenColor(217, 245, 255);
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(5));
    canvas.drawText(135, 20, "PONG");
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(0));

    canvas.setPenColor(59, 167, 204);
    canvas.drawText(85, 40, "con ESP32 por FIE");
    canvas.drawText(30, 55, "Facultad de Ingenieria Electrica.");

    canvas.setPenColor(248, 252, 167);
    canvas.setBrushColor(0, 0, 0);
    canvas.setPenColor(248, 252, 167);

    canvas.setBrushColor(21, 26, 70);
    canvas.drawText(60,140, "Jugador 1");
    canvas.drawText(170,140, "Jugador 2");




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
        canvas.setPenColor(random(255), random(255), 255);
        canvas.drawText(50, 75, "Presiona [START] para jugar");
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

  struct SISprite : Sprite
  {
    SpriteType type;
  };

  enum GameState
  {
    GAMESTATE_PLAYING,
    GAMESTATE_PLAYERKILLED,
    GAMESTATE_PLAYER2KILLED,
    GAMESTATE_ENDGAME,
    GAMESTATE_GAMEOVER,
  };

  static const int PADDLECOUNT = 1;
  static const int BALLCOUNT = 1;
  static const int SPRITESCOUNT = 2 * PADDLECOUNT + BALLCOUNT;

  static const int BALL_START_X = 159;
  static const int BALL_START_Y = 95;
  static const int PADDLE1_START_X = 303;
  static const int PADDLE2_START_X = 9;
  static const int PADDLE_START_Y = 81;

  static int scoreP1_;
  static int scoreP2_;

  SISprite *sprites_ = new SISprite[SPRITESCOUNT];
  SISprite *player1_ = sprites_;
  SISprite *player2_ = player1_ + PADDLECOUNT;
  SISprite *ball_ = player2_ + PADDLECOUNT;

  int player1VelY_ = 0; // 0 = no move
  int player2VelY_ = 0;
  
  GameState gameState_ = GAMESTATE_PLAYING;


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

    }
  }

  void moveBall(){
   
  }

  void update(int updateCount)
  {
  }

  void collisionDetected(Sprite *spriteA, Sprite *spriteB, Point collisionPoint)
  {
  }
};

int GameScene::scoreP1_ = 0;
int GameScene::scoreP2_ = 0;

void setup()
{
  Ps3.begin("24:6f:28:af:1c:66");
  DisplayController.begin();
  DisplayController.setResolution(VGA_320x200_75Hz);
}

void loop()
{
  if (true)
  {
    IntroScene introScene;
    introScene.start();
  }
  GameScene gameScene;
  gameScene.start();
}