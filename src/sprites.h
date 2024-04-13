
const uint8_t paddle_data[] = {
  0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
  0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
  0xfc, 0xfc, 0xfc, 0xfc,
};
Bitmap bmpPaddle = Bitmap(6, 36, &paddle_data[0], PixelFormat::Mask, RGB888(255, 255, 255));

const uint8_t ball_data[] = {
  0b11110000, 0b11110000, 0b11110000, 0b11110000, 
};
Bitmap bmpBall = Bitmap(4, 4, &ball_data[0], PixelFormat::Mask, RGB888(255, 255, 255));

const uint8_t ball_data_blurry1[] = {
  0b11110000, 0b11110000, 0b11110000, 0b11110000, 
};
Bitmap bmpBall_1 = Bitmap(4, 4, &ball_data[0], PixelFormat::Mask, RGB888(170, 170, 170));

const uint8_t ball_data_blurry2[] = {
  0b11110000, 0b11110000, 0b11110000, 0b11110000, 
};
Bitmap bmpBall_2 = Bitmap(4, 4, &ball_data[0], PixelFormat::Mask, RGB888(85, 85, 85));