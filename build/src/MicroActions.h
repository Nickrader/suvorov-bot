#pragma once
#include <sc2api/sc2_api.h>

struct StutterStep {
  StutterStep();

  // void StutterStepInitiate(sc2::Point2D point_, bool main_);

  void StutterStepAttack(const sc2::Units& units_, sc2::Point2D enemy_main_,
                         bool enemy_main_destroyed_);

 private:
  bool stutter;
  bool enemy_main_destroyed;
  uint32_t stutter_frame_attack = 0;  // TODO: fix garbage value
  uint32_t stutter_frame_move = 0;    // or is OK b/c game start on 0 ???
  uint32_t stutter_steps = 12;  // gameloops for 1 full stutter (move + attack)
};

struct FocusFire {
  FocusFire() {}

  void FFInitiate(const sc2::Unit* target_, bool main_);

  void FFTarget(const sc2::Units& army_);

 private:
  bool ff = false;
  uint32_t cnt = 0;
  const sc2::Unit* target = nullptr;
  uint32_t break_focus_frame = 0;
};
