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

class IntroScene : public Scene
{
private:
    static constexpr uint8_t bitmapd[128] = {0xff};
    Bitmap bitmap = Bitmap(16, 64, bitmapd, PixelFormat::Mask, RGB888(255, 255, 0));
    Sprite sprite;


public:
    IntroScene(): Scene(0, 20, DisplayController.getViewPortWidth(), DisplayController.getViewPortHeight())
    {
          sprite.addBitmap(&bitmap);
          sprite.moveTo(0, 150);
          sprite.visible = true;
        DisplayController.setSprites(&sprite,1);
    }

    void init()
    {
        canvas.setBrushColor(21, 26, 70);
        canvas.clear();
        canvas.setGlyphOptions(GlyphOptions().FillBackground(true));
        canvas.selectFont(&fabgl::FONT_8x8);
        canvas.setPenColor(217, 245, 255);
        canvas.setGlyphOptions(GlyphOptions().DoubleWidth(5));
        canvas.drawText(50, 20, "PONG");
        canvas.setGlyphOptions(GlyphOptions().DoubleWidth(0));

        canvas.setBrushColor(21, 26, 70);
    };

    virtual void collisionDetected(Sprite *spriteA, Sprite *spriteB, Point collisionPoint) = 0;
    
    virtual void update(int updateCount){
         sprite.x += 1;
    if (sprite.x >= DisplayController.getViewPortWidth())
        sprite.x = 0;
    float count = 0.0;
    sprite.y = 150 + 35.0 * sin(count);

    count += 0.10;
    };
};



void setup()
{
    Serial.begin(115200);
    Ps3.begin("24:6f:28:af:1c:66");
    DisplayController.begin();
    DisplayController.setResolution(VGA_320x200_75Hz);

    Canvas cv(&DisplayController);
    cv.setBrushColor(RGB888(0, 0, 64));
    cv.clear();
    cv.setPenColor(RGB888(64, 64, 0));
    for (int i = 0; i < cv.getWidth(); i += 10)
        cv.drawLine(i, 0, i, cv.getHeight() - 1);
    for (int i = 0; i < cv.getHeight(); i += 10)
        cv.drawLine(0, i, cv.getWidth() - 1, i);

}

void loop()
{
    delay(100);
   

    // update sprites positions
    DisplayController.refreshSprites();

}
