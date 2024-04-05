#include <Arduino.h>
#include <Ps3Controller.h>

#include "fabgl.h"
#include "fabglconf.h"
#include "fabutils.h"
#include "sprites.h"
#include "sounds.h"

using fabgl::iclamp;

fabgl::VGAController DisplayController;
fabgl::Canvas        canvas(&DisplayController);
fabgl::PS2Controller PS2Controller;
SoundGenerator       soundGenerator;


void
setup()
{
    Serial.begin(115200);
    PS2Controller.begin(PS2Preset::KeyboardPort0, KbdMode::GenerateVirtualKeys);

    Ps3.begin("24:6f:28:af:1c:66");
    DisplayController.begin();
    DisplayController.setResolution(VGA_320x200_75Hz);
    Serial.println("¡Listo!");
}

void loop()
{
    delay(100);
}
