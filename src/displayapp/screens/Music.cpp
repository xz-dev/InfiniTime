/*  Copyright (C) 2020 JF, Adam Pigg, Avamander

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "displayapp/screens/Music.h"
#include "displayapp/screens/Symbols.h"
#include <cstdint>
#include "displayapp/DisplayApp.h"
#include "components/ble/MusicService.h"
#include "displayapp/icons/music/disc.c"
#include "displayapp/icons/music/disc_f_1.c"
#include "displayapp/icons/music/disc_f_2.c"

using namespace Pinetime::Applications::Screens;

static void ButtonClickHandlerVolUp(lv_event_t* event) {
  Music* screen = static_cast<Music*>(lv_event_get_user_data(event));
  screen->OnVolUpButtonEvent(event);
}

static void ButtonClickHandlerVolDown(lv_event_t* event) {
  Music* screen = static_cast<Music*>(lv_event_get_user_data(event));
  screen->OnVolDownButtonEvent(event);
}

static void ButtonClickHandlerPrev(lv_event_t* event) {
  Music* screen = static_cast<Music*>(lv_event_get_user_data(event));
  screen->OnPrevButtonEvent(event);
}

static void ButtonClickHandlerNext(lv_event_t* event) {
  Music* screen = static_cast<Music*>(lv_event_get_user_data(event));
  screen->OnNextButtonEvent(event);
}

static void ButtonClickHandlerPlayPause(lv_event_t* event) {
  Music* screen = static_cast<Music*>(lv_event_get_user_data(event));
  screen->OnPlayPauseButtonEvent(event);
}

/**
 * Set the pixel array to display by the image
 * This just calls lv_img_set_src but adds type safety
 *
 * @param img pointer to an image object
 * @param data the image array
 */
inline void lv_img_set_src_arr(lv_obj_t* img, const lv_img_dsc_t* src_img) {
  lv_img_set_src(img, src_img);
}

/**
 * Music control watchapp
 *
 * TODO: Investigate Apple Media Service and AVRCPv1.6 support for seamless integration
 */
Music::Music(Pinetime::Controllers::MusicService& music) : musicService(music) {
  lv_obj_t* label;

  lv_style_init(&btn_style);
  lv_style_set_radius(&btn_style, 20);
  lv_style_set_bg_color(&btn_style, LV_COLOR_AQUA);
  lv_style_set_bg_opa(&btn_style, LV_OPA_50);

  btnVolDown = lv_btn_create(lv_screen_active());
  btnVolDown->user_data = this;
  lv_obj_add_event_cb(btnVolDown, ButtonClickHandlerVolDown, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnVolDown, 76, 76);
  lv_obj_align_to(btnVolDown, nullptr, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_add_style(btnVolDown, &btn_style, LV_STATE_DEFAULT);
  label = lv_label_create(btnVolDown);
  lv_label_set_text_static(label, Symbols::volumDown);
  lv_obj_add_flag(btnVolDown, LV_OBJ_FLAG_HIDDEN);

  btnVolUp = lv_btn_create(lv_screen_active());
  btnVolUp->user_data = this;
  lv_obj_add_event_cb(btnVolUp, ButtonClickHandlerVolUp, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnVolUp, 76, 76);
  lv_obj_align_to(btnVolUp, nullptr, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_add_style(btnVolUp, &btn_style, LV_STATE_DEFAULT);
  label = lv_label_create(btnVolUp);
  lv_label_set_text_static(label, Symbols::volumUp);
  lv_obj_add_flag(btnVolUp, LV_OBJ_FLAG_HIDDEN);

  btnPrev = lv_btn_create(lv_screen_active());
  btnPrev->user_data = this;
  lv_obj_add_event_cb(btnPrev, ButtonClickHandlerPrev, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnPrev, 76, 76);
  lv_obj_align_to(btnPrev, nullptr, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_add_style(btnPrev, &btn_style, LV_STATE_DEFAULT);
  label = lv_label_create(btnPrev);
  lv_label_set_text_static(label, Symbols::stepBackward);

  btnNext = lv_btn_create(lv_screen_active());
  btnNext->user_data = this;
  lv_obj_add_event_cb(btnNext, ButtonClickHandlerNext, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnNext, 76, 76);
  lv_obj_align_to(btnNext, nullptr, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_add_style(btnNext, &btn_style, LV_STATE_DEFAULT);
  label = lv_label_create(btnNext);
  lv_label_set_text_static(label, Symbols::stepForward);

  btnPlayPause = lv_btn_create(lv_screen_active());
  btnPlayPause->user_data = this;
  lv_obj_add_event_cb(btnPlayPause, ButtonClickHandlerPlayPause, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnPlayPause, 76, 76);
  lv_obj_align_to(btnPlayPause, nullptr, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_style(btnPlayPause, &btn_style, LV_STATE_DEFAULT);
  txtPlayPause = lv_label_create(btnPlayPause);
  lv_label_set_text_static(txtPlayPause, Symbols::play);

  txtTrackDuration = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(txtTrackDuration, LV_LABEL_LONG_SCROLL);
  lv_obj_align_to(txtTrackDuration, nullptr, LV_ALIGN_TOP_LEFT, 12, 20);
  lv_label_set_text_static(txtTrackDuration, "--:--/--:--");
  lv_obj_set_style_text_align(txtTrackDuration, LV_TEXT_ALIGN_LEFT | LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(txtTrackDuration, LV_HOR_RES);

  constexpr uint8_t FONT_HEIGHT = 12;
  constexpr uint8_t LINE_PAD = 15;
  constexpr int8_t MIDDLE_OFFSET = -25;
  txtArtist = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(txtArtist, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_align_to(txtArtist, nullptr, LV_ALIGN_LEFT_MID, 12, MIDDLE_OFFSET + 1 * FONT_HEIGHT);
  lv_obj_set_style_text_align(txtArtist, LV_TEXT_ALIGN_LEFT | LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(txtArtist, LV_HOR_RES - 12);
  lv_label_set_text_static(txtArtist, "Artist Name");

  txtTrack = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(txtTrack, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_align_to(txtTrack, nullptr, LV_ALIGN_LEFT_MID, 12, MIDDLE_OFFSET + 2 * FONT_HEIGHT + LINE_PAD);

  lv_obj_set_style_text_align(txtTrack, LV_TEXT_ALIGN_LEFT | LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(txtTrack, LV_HOR_RES - 12);
  lv_label_set_text_static(txtTrack, "This is a very long getTrack name");

  /** Init animation */
  imgDisc = lv_img_create(lv_screen_active());
  lv_img_set_src_arr(imgDisc, &disc);
  lv_obj_align_to(imgDisc, nullptr, LV_ALIGN_TOP_RIGHT, -15, 15);

  imgDiscAnim = lv_img_create(lv_screen_active());
  lv_img_set_src_arr(imgDiscAnim, &disc_f_1);
  lv_obj_align_to(imgDiscAnim, nullptr, LV_ALIGN_TOP_RIGHT, -15 - 32, 15);

  frameB = false;

  musicService.event(Controllers::MusicService::EVENT_MUSIC_OPEN);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Music::~Music() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  lv_style_reset(&btn_style);
  lv_obj_clean(lv_screen_active());
}

void Music::Refresh() {
  if (artist != musicService.getArtist()) {
    artist = musicService.getArtist();
    lv_label_set_text(txtArtist, artist.data());
  }

  if (track != musicService.getTrack()) {
    track = musicService.getTrack();
    lv_label_set_text(txtTrack, track.data());
  }

  if (album != musicService.getAlbum()) {
    album = musicService.getAlbum();
  }

  if (playing != musicService.isPlaying()) {
    playing = musicService.isPlaying();
  }

  if (currentPosition != musicService.getProgress()) {
    currentPosition = musicService.getProgress();
    UpdateLength();
  }

  if (totalLength != musicService.getTrackLength()) {
    totalLength = musicService.getTrackLength();
    UpdateLength();
  }

  if (playing) {
    lv_label_set_text_static(txtPlayPause, Symbols::pause);
    if (xTaskGetTickCount() - 1024 >= lastIncrement) {

      if (frameB) {
        lv_img_set_src(imgDiscAnim, &disc_f_1);
      } else {
        lv_img_set_src(imgDiscAnim, &disc_f_2);
      }
      frameB = !frameB;

      if (currentPosition >= totalLength) {
        // Let's assume the getTrack finished, paused when the timer ends
        //  and there's no new getTrack being sent to us
        playing = false;
      }
      lastIncrement = xTaskGetTickCount();
    }
  } else {
    lv_label_set_text_static(txtPlayPause, Symbols::play);
  }
}

void Music::UpdateLength() {
  if (totalLength > (99 * 60 * 60)) {
    lv_label_set_text_static(txtTrackDuration, "Inf/Inf");
  } else if (totalLength > (99 * 60)) {
    lv_label_set_text_fmt(txtTrackDuration,
                          "%02d:%02d/%02d:%02d",
                          (currentPosition / (60 * 60)) % 100,
                          ((currentPosition % (60 * 60)) / 60) % 100,
                          (totalLength / (60 * 60)) % 100,
                          ((totalLength % (60 * 60)) / 60) % 100);
  } else {
    lv_label_set_text_fmt(txtTrackDuration,
                          "%02d:%02d/%02d:%02d",
                          (currentPosition / 60) % 100,
                          (currentPosition % 60) % 100,
                          (totalLength / 60) % 100,
                          (totalLength % 60) % 100);
  }
}

void Music::OnVolUpButtonEvent([[maybe_unused]] lv_event_t* event) {
  musicService.event(Controllers::MusicService::EVENT_MUSIC_VOLUP);
}

void Music::OnVolDownButtonEvent([[maybe_unused]] lv_event_t* event) {
  musicService.event(Controllers::MusicService::EVENT_MUSIC_VOLDOWN);
}

void Music::OnPrevButtonEvent([[maybe_unused]] lv_event_t* event) {
  musicService.event(Controllers::MusicService::EVENT_MUSIC_PREV);
}

void Music::OnNextButtonEvent([[maybe_unused]] lv_event_t* event) {
  musicService.event(Controllers::MusicService::EVENT_MUSIC_NEXT);
}

void Music::OnPlayPauseButtonEvent([[maybe_unused]] lv_event_t* event) {
  if (playing == Pinetime::Controllers::MusicService::MusicStatus::Playing) {
    musicService.event(Controllers::MusicService::EVENT_MUSIC_PAUSE);

    // Let's assume it stops playing instantly
    playing = Controllers::MusicService::NotPlaying;
  } else {
    musicService.event(Controllers::MusicService::EVENT_MUSIC_PLAY);

    // Let's assume it starts playing instantly
    // TODO: In the future should check for BT connection for better UX
    playing = Controllers::MusicService::Playing;
  }
}

bool Music::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeUp: {
      lv_obj_add_flag(btnVolDown, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(btnVolUp, LV_OBJ_FLAG_HIDDEN);

      lv_obj_add_flag(btnNext, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(btnPrev, LV_OBJ_FLAG_HIDDEN);
      return true;
    }
    case TouchEvents::SwipeDown: {
      if (lv_obj_has_flag(btnNext, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(btnNext, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(btnPrev, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btnVolDown, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btnVolUp, LV_OBJ_FLAG_HIDDEN);
        return true;
      }
      return false;
    }
    case TouchEvents::SwipeLeft: {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_NEXT);
      return true;
    }
    case TouchEvents::SwipeRight: {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_PREV);
      return true;
    }
    default: {
      return false;
    }
  }
}
