#include <Arduino.h>
#include <Ps3Controller.h>
#include <scene.h>
#include "fabgl.h"
#include "fabglconf.h"
#include "fabutils.h"
#include "sprites.h"
#include "sounds.h"
#include <math.h>

using fabgl::iclamp;

fabgl::VGAController DisplayController;
fabgl::Canvas canvas(&DisplayController);
SoundGenerator soundGenerator;


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
    canvas.setBrushColor(21, 26, 70);
    canvas.clear();
    canvas.setGlyphOptions(GlyphOptions().FillBackground(true));
    canvas.selectFont(&fabgl::FONT_8x8);
    canvas.setPenColor(217, 245, 255);
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(5));
    canvas.drawText(135, 20, "PONG");
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(0));

    canvas.setPenColor(59, 167, 204);
    canvas.drawText(30, 55, "Facultad de Ingenieria Electrica.");

    canvas.setPenColor(248, 252, 167);
    canvas.setBrushColor(0, 0, 0);
    canvas.drawText(60,140, "Jugador 1");
    canvas.drawText(170,140, "Jugador 2");
    canvas.drawBitmap(1, 135, &bmpWall);

    music_ = soundGenerator.playSamples(themeSoundSamples, sizeof(themeSoundSamples), 100, -1);
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


struct GameScene : public Scene
{

  enum SpriteType
  {
    TYPE_PLAYER,
    TYPE_PLAYER2,
    TYPE_SHIELD,
  };

  struct SISprite : Sprite
  {
    SpriteType type;
    uint8_t enemyPoints;
  };

  enum GameState
  {
    GAMESTATE_PLAYING,
    GAMESTATE_GAMEOVER,
  };

  static const int PLAYERSCOUNT = 1;
  static const int SHIELDSCOUNT = 4;
  static const int ROWENEMIESCOUNT = 11;
  static const int PLAYERFIRECOUNT = 1;
  static const int ENEMIESFIRECOUNT = 1;
  static const int ENEMYMOTHERCOUNT = 1;
  static const int SPRITESCOUNT = 2 * PLAYERSCOUNT + SHIELDSCOUNT + 5 * ROWENEMIESCOUNT + 2 * PLAYERFIRECOUNT + ENEMIESFIRECOUNT + ENEMYMOTHERCOUNT;

  static const int ENEMIES_X_SPACE = 16; // Espacio entre enemigos
  static const int ENEMIES_Y_SPACE = 10;
  static const int ENEMIES_START_X = 0;
  static const int ENEMIES_START_Y = 30;
  static const int ENEMIES_STEP_X = 6;
  static const int ENEMIES_STEP_Y = 8;

  static const int PLAYER_Y = 170;
  static const int PLAYER2_Y = 170;

  static int Score1_;
  static int level_;
  static int Score2_;

  SISprite *sprites_ = new SISprite[SPRITESCOUNT];
  SISprite *player_ = sprites_;
  SISprite *player2_ = player_ + PLAYERSCOUNT;
  SISprite *shields_ = player2_ + PLAYERSCOUNT;
  SISprite *enemies_ = shields_ + SHIELDSCOUNT;
  SISprite *enemiesR1_ = enemies_;
  SISprite *enemiesR2_ = enemiesR1_ + ROWENEMIESCOUNT;
  SISprite *enemiesR3_ = enemiesR2_ + ROWENEMIESCOUNT;
  SISprite *enemiesR4_ = enemiesR3_ + ROWENEMIESCOUNT;
  SISprite *enemiesR5_ = enemiesR4_ + ROWENEMIESCOUNT;
  SISprite *playerFire_ = enemiesR5_ + ROWENEMIESCOUNT;
  SISprite *playerFire2_ = playerFire_ + PLAYERFIRECOUNT;
  SISprite *enemiesFire_ = playerFire2_ + PLAYERFIRECOUNT;
  SISprite *enemyMother_ = enemiesFire_ + ENEMIESFIRECOUNT;

  int playerVelX_ = 0; // 0 = no move
  int player2VelX_ = 0;
  int enemiesX_ = ENEMIES_START_X;
  int enemiesY_ = ENEMIES_START_Y;

  // enemiesDir_
  //   bit 0 : if 1 moving left
  //   bit 1 : if 1 moving right
  //   bit 2 : if 1 moving down
  //   bit 3 : if 0 before was moving left, if 1 before was moving right
  // Allowed cases:
  //   1  = moving left
  //   2  = moving right
  //   4  = moving down (before was moving left)
  //   12 = moving down (before was moving right)

  static constexpr int ENEMY_MOV_LEFT = 1;
  static constexpr int ENEMY_MOV_RIGHT = 2;
  static constexpr int ENEMY_MOV_DOWN_BEFORE_LEFT = 4;
  static constexpr int ENEMY_MOV_DOWN_BEFORE_RIGHT = 12;

  int enemiesDir_ = ENEMY_MOV_RIGHT;

  int enemiesAlive_ = ROWENEMIESCOUNT * 5;
  int enemiesSoundCount_ = 0;
  SISprite *lastHitEnemy_ = nullptr;
  GameState gameState_ = GAMESTATE_PLAYING;

  bool updateScore_ = true;
  int64_t pauseStart_;

  Bitmap bmpShield[4] = {
      Bitmap(22, 16, shield_data, PixelFormat::Mask, RGB888(47, 93, 130), true),
      Bitmap(22, 16, shield_data, PixelFormat::Mask, RGB888(47, 93, 130), true),
      Bitmap(22, 16, shield_data, PixelFormat::Mask, RGB888(47, 93, 130), true),
      Bitmap(22, 16, shield_data, PixelFormat::Mask, RGB888(47, 93, 130), true),
  };

  GameScene()
      : Scene(SPRITESCOUNT, 20, DisplayController.getViewPortWidth(), DisplayController.getViewPortHeight())
  {
  }

  ~GameScene()
  {
    delete[] sprites_;
  }

  void init()
  {
    // setup player 1
    player_->addBitmap(&bmpPaddle1);
    player_->moveTo(225, PLAYER_Y);
    player_->type = TYPE_PLAYER;
    addSprite(player_);

    // setup player 2
    player2_->addBitmap(&bmpPaddle2);
    player2_->moveTo(75, PLAYER_Y);
    player2_->type = TYPE_PLAYER2;
    addSprite(player2_);


    // setup enemies
   
    DisplayController.setSprites(sprites_, SPRITESCOUNT);

    canvas.setBrushColor(21, 26, 70);
    canvas.clear();

    canvas.setPenColor(47, 93, 130);
    canvas.drawLine(0, 180, 320, 180);

    canvas.setGlyphOptions(GlyphOptions().FillBackground(true));
    canvas.selectFont(&fabgl::FONT_4x6);
    canvas.setPenColor(108, 155, 245);
    canvas.drawText(110, 20, "Bienvenidos al espacio");
    canvas.selectFont(&fabgl::FONT_8x8);
    canvas.setPenColor(69, 142, 237);
    canvas.drawText(2, 2, "SCORE");
    canvas.setPenColor(248, 252, 167);
    canvas.drawText(254, 2, "HI-SCORE");
    canvas.setPenColor(255, 255, 255);
    canvas.drawTextFmt(256, 181, "Nivel %02d", level_);

    drawScore();
  }

  void drawScore()
  {
    canvas.setPenColor(255, 255, 255);
    canvas.drawTextFmt(2, 14, "%05d", Score1_);
    //if (score_ > hiScore_)
    //  hiScore_ = score_;
    canvas.setPenColor(255, 255, 255);
    canvas.drawTextFmt(266, 14, "%05d", Score2_);
  }

  void gameOver()
  {
    // disable enemies drawing, so text can be over them
    for (int i = 0; i < ROWENEMIESCOUNT * 5; ++i)
      enemies_[i].allowDraw = false;

    // show game over
    canvas.setPenColor(248, 252, 167);
    canvas.setBrushColor(28, 35, 92);
    canvas.fillRectangle(40, 60, 270, 130);
    canvas.drawRectangle(40, 60, 270, 130);
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(1));
    canvas.setPenColor(255, 255, 255);
    canvas.drawText(55, 80, "FIN DEL JUEGO");
    canvas.setGlyphOptions(GlyphOptions().DoubleWidth(0));
    canvas.setPenColor(248, 252, 167);
    canvas.drawText(95, 100, "Presiona [START]");
    // change state
    gameState_ = GAMESTATE_GAMEOVER;
    level_ = 1;
    Score1_ = 0;
    Score2_ = 0;
  }


  void update(int updateCount)
  {

    if (updateScore_)
    {
      updateScore_ = false;
      drawScore();
    }

    if (gameState_ == GAMESTATE_PLAYING || gameState_ == GAMESTATE_PLAYERKILLED)
    {

      // move enemies and shoot
      if ((updateCount % max(3, 21 - level_ * 2)) == 0)
      {
        // handle enemy explosion
        if (lastHitEnemy_)
        {
          lastHitEnemy_->visible = false;
          lastHitEnemy_ = nullptr;
        }
        // handle enemies movement
        enemiesX_ += (-1 * (enemiesDir_ & 1) + (enemiesDir_ >> 1 & 1)) * ENEMIES_STEP_X;
        enemiesY_ += (enemiesDir_ >> 2 & 1) * ENEMIES_STEP_Y;
        bool touchSide = false;
        for (int i = 0; i < ROWENEMIESCOUNT; ++i)
        {
          moveEnemy(&enemiesR1_[i], enemiesX_ + i * ENEMIES_X_SPACE, enemiesY_ + 0 * ENEMIES_Y_SPACE, &touchSide);
          moveEnemy(&enemiesR2_[i], enemiesX_ + i * ENEMIES_X_SPACE, enemiesY_ + 1 * ENEMIES_Y_SPACE, &touchSide);
          moveEnemy(&enemiesR3_[i], enemiesX_ + i * ENEMIES_X_SPACE, enemiesY_ + 2 * ENEMIES_Y_SPACE, &touchSide);
          moveEnemy(&enemiesR4_[i], enemiesX_ + i * ENEMIES_X_SPACE, enemiesY_ + 3 * ENEMIES_Y_SPACE, &touchSide);
          moveEnemy(&enemiesR5_[i], enemiesX_ + i * ENEMIES_X_SPACE, enemiesY_ + 4 * ENEMIES_Y_SPACE, &touchSide);
        }
        switch (enemiesDir_)
        {
        case ENEMY_MOV_DOWN_BEFORE_LEFT:
          enemiesDir_ = ENEMY_MOV_RIGHT;
          break;
        case ENEMY_MOV_DOWN_BEFORE_RIGHT:
          enemiesDir_ = ENEMY_MOV_LEFT;
          break;
        default:
          if (touchSide)
            enemiesDir_ = (enemiesDir_ == ENEMY_MOV_LEFT ? ENEMY_MOV_DOWN_BEFORE_LEFT : ENEMY_MOV_DOWN_BEFORE_RIGHT);
          break;
        }
        // sound
        ++enemiesSoundCount_;
        soundGenerator.playSamples(invadersSoundSamples[enemiesSoundCount_ % 4], invadersSoundSamplesSize[enemiesSoundCount_ % 4]);
        // handle enemies fire generation
        if (!enemiesFire_->visible)
        {
          int shottingEnemy = random(enemiesAlive_);
          for (int i = 0, a = 0; i < ROWENEMIESCOUNT * 5; ++i)
          {
            if (enemies_[i].visible)
            {
              if (a == shottingEnemy)
              {
                enemiesFire_->x = enemies_[i].x + enemies_[i].getWidth() / 2;
                enemiesFire_->y = enemies_[i].y + enemies_[i].getHeight() / 2;
                enemiesFire_->visible = true;
                break;
              }
              ++a;
            }
          }
        }
      }

      if (gameState_ == GAMESTATE_PLAYER2KILLED)
      {
        // animate player explosion or restart playing other lives
        if ((updateCount % 20) == 0)
        {
          if (player2_->getFrameIndex() == 1)
          {
            player2_->setFrame(2);
          }
          else
          {
            player2_->setFrame(0);
            gameState_ = GAMESTATE_PLAYING;
          }
        }
      }
      else if (gameState_ == GAMESTATE_PLAYERKILLED)
      {
        // animate player explosion or restart playing other lives
        if ((updateCount % 20) == 0)
        {
          if (player_->getFrameIndex() == 1)
          {
            player_->setFrame(2);
          }
          else
          {
            player_->setFrame(0);
            gameState_ = GAMESTATE_PLAYING;
          }
        }
      } else if (playerVelX_ != 0 || player2VelX_ != 0)
      {
        // move player using PS3
        player_->x += playerVelX_;
        player_->x = iclamp(player_->x, 0, getWidth() - player_->getWidth());
        updateSprite(player_);

        player2_->x += player2VelX_;
        player2_->x = iclamp(player2_->x, 0, getWidth() - player2_->getWidth());
        updateSprite(player2_);
      }

      // move player fire
      if (playerFire_->visible)
      {
        playerFire_->y -= 3;
        if (playerFire_->y < ENEMIES_START_Y)
          playerFire_->visible = false;
        else
          updateSpriteAndDetectCollisions(playerFire_);
      }

      if (playerFire2_->visible)
      {
        playerFire2_->y -= 3;
        if (playerFire2_->y < ENEMIES_START_Y)
          playerFire2_->visible = false;
        else
          updateSpriteAndDetectCollisions(playerFire2_);
      }

      // move enemies fire
      if (enemiesFire_->visible)
      {
        enemiesFire_->y += 2;
        enemiesFire_->setFrame(enemiesFire_->getFrameIndex() ? 0 : 1);
        if (enemiesFire_->y > PLAYER_Y + player_->getHeight())
          enemiesFire_->visible = false;
        else
          updateSpriteAndDetectCollisions(enemiesFire_);
      }

      // move enemy mother ship
      if (enemyMother_->visible && enemyMother_->getFrameIndex() == 0)
      {
        enemyMother_->x -= 1;
        if (enemyMother_->x < -enemyMother_->getWidth())
          enemyMother_->visible = false;
        else
          updateSprite(enemyMother_);
      }

      // start enemy mother ship
      if ((updateCount % 800) == 0)
      {
        soundGenerator.playSamples(motherShipSoundSamples, sizeof(motherShipSoundSamples), 100, 7000);
        enemyMother_->x = getWidth();
        enemyMother_->setFrame(0);
        enemyMother_->visible = true;
      }

      // Uso del control de PS3 de jugador 1.
      if (Ps3.data.analog.stick.rx > 90 || Ps3.data.analog.stick.rx < -90)
      {
        if (Ps3.data.analog.stick.rx > 90)
        {
          playerVelX_ = +1;
        }
        else if (Ps3.data.analog.stick.rx < -90)
        {
          playerVelX_ = -1;
        }
      }
      else
      {
        playerVelX_ = 0;
      }

      if (abs(Ps3.event.analog_changed.button.cross) && !playerFire_->visible) // player fire?
        fire();

      // Uso del control de PS3 de jugador 2.
      if (Ps3.data.analog.stick.lx > 90 || Ps3.data.analog.stick.lx < -90)
      {
        if (Ps3.data.analog.stick.lx > 90)
        {
          player2VelX_ = +1;
        }
        else if (Ps3.data.analog.stick.lx < -90)
        {
          player2VelX_ = -1;
        }
      }
      else
      {
        player2VelX_ = 0;
      }

      if (abs(Ps3.event.analog_changed.button.down) && !playerFire2_->visible) // player fire?
        fire2();
    }

    if (gameState_ == GAMESTATE_ENDGAME)
      gameOver();

    if (gameState_ == GAMESTATE_GAMEOVER)
    {

      // animate player burning
      if ((updateCount % 20) == 0)
      {
        player_->setFrame(player_->getFrameIndex() == 1 ? 2 : 1);
        player_->setFrame(player_->getFrameIndex() == 1 ? 2 : 1);
      }

      if (Ps3.event.button_down.start)
      {
        stop();
        DisplayController.removeSprites();
      }
    }

    DisplayController.refreshSprites();
  }


  void collisionDetected(Sprite *spriteA, Sprite *spriteB, Point collisionPoint)
  {
    
  }
};


int GameScene::level_ = 1;
int GameScene::Score1_= 0;
int GameScene::Score2_ = 0;

void setup()
{
    Serial.begin(115200);
    Ps3.begin("24:6f:28:af:1c:66");
    DisplayController.begin();
    DisplayController.setResolution(VGA_320x200_75Hz);


}

void loop()
{
    delay(100);
    IntroScene introScene;
    introScene.start();
    DisplayController.refreshSprites();

}