#include "displayapp/screens/Paddle.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/LittleVgl.h"

#include <cstdlib> // for rand()

using namespace Pinetime::Applications::Screens;

Paddle::Paddle(Pinetime::Components::LittleVgl& lvgl) : lvgl {lvgl} {
  background = lv_obj_create(lv_screen_active());
  lv_obj_set_size(background, LV_HOR_RES + 1, LV_VER_RES);
  lv_obj_set_pos(background, -1, 0);
  static lv_style_t style_background;
  lv_style_set_radius(&style_background, 0);
  lv_style_set_bg_color(&style_background, LV_COLOR_BLACK);
  lv_style_set_border_color(&style_background, LV_COLOR_WHITE);
  lv_style_set_border_width(&style_background, 1);
  lv_obj_add_style(background, &style_background, LV_STATE_DEFAULT);

  points = lv_label_create(lv_screen_active());
  static lv_style_t style_points;
  lv_style_set_text_font(&style_points, &jetbrains_mono_42);
  lv_label_set_text_static(points, "0000");
  lv_obj_align_to(points, lv_screen_active(), LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_add_style(points, &style_points, LV_STATE_DEFAULT);

  paddle = lv_obj_create(lv_screen_active());
  static lv_style_t style_paddle;
  lv_style_set_bg_color(&style_paddle, LV_COLOR_WHITE);
  lv_style_set_radius(&style_paddle, 0);
  lv_obj_add_style(paddle, &style_paddle, LV_STATE_DEFAULT);
  lv_obj_set_size(paddle, 4, 60);

  ball = lv_obj_create(lv_screen_active());
  static lv_style_t style_ball;
  lv_style_set_bg_color(&style_ball, LV_COLOR_WHITE);
  lv_style_set_radius(&style_ball, LV_RADIUS_CIRCLE);
  lv_obj_add_style(ball, &style_ball, LV_STATE_DEFAULT);
  lv_obj_set_size(ball, ballSize, ballSize);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Paddle::~Paddle() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  lv_obj_clean(lv_screen_active());
}

void Paddle::Refresh() {
  ballX += dx;
  ballY += dy;

  lv_obj_set_pos(ball, ballX, ballY);

  // checks if it has touched the sides (floor and ceiling)
  if (ballY <= 1 || ballY >= LV_VER_RES - ballSize - 2) {
    dy *= -1;
  }

  // checks if it has touched the side (right side)
  if (ballX >= LV_HOR_RES - ballSize - 1) {
    dx *= -1;
    dy += rand() % 3 - 1; // add a little randomization in wall bounce direction, one of [-1, 0, 1]
    if (dy > 5) {         // limit dy to be in range [-5 to 5]
      dy = 5;
    }
    if (dy < -5) {
      dy = -5;
    }
  }

  // checks if it is in the position of the paddle
  if (dx < 0 && ballX <= 4) {
    if (ballX >= -ballSize / 4) {
      if (ballY <= (paddlePos + 30 - ballSize / 4) && ballY >= (paddlePos - 30 - ballSize + ballSize / 4)) {
        dx *= -1;
        score++;
      }
    }
    // checks if it has gone behind the paddle
    else if (ballX <= -ballSize * 2) {
      ballX = (LV_HOR_RES - ballSize) / 2;
      ballY = (LV_VER_RES - ballSize) / 2;
      score = 0;
    }
  }
  lv_label_set_text_fmt(points, "%04d", score);
}

bool Paddle::OnTouchEvent(Pinetime::Applications::TouchEvents /*event*/) {
  return true;
}

bool Paddle::OnTouchEvent(uint16_t /*x*/, uint16_t y) {
  // sets the center paddle pos. (30px offset) with the the y_coordinate of the finger
  // but clamp it such that the paddle never clips off screen
  if (y < 31) {
    lv_obj_set_pos(paddle, 0, 1);
  } else if (y > LV_VER_RES - 31) {
    lv_obj_set_pos(paddle, 0, LV_VER_RES - 61);
  } else {
    lv_obj_set_pos(paddle, 0, y - 30);
  }
  paddlePos = y;
  return true;
}
