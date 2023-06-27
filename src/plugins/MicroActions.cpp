#include "plugins/MicroActions.h"

#include "core/API.h"
#include "sc2api/sc2_unit_filters.h"

StutterStep::StutterStep() : stutter(false) {}

// still have mixed feelings about span_ parameter, but no wrapping
// my head around alternative.  When close enough, don't want move command.
void StutterStep::StutterStepAttack(const sc2::Units& units_,
                                    sc2::Point2D target_, float span_) {
  uint32_t x = gAPI->observer().GetGameLoop();
  if (x > (stutter_frame_attack + stutter_steps)) {
      stutter = false;
  }
  if (units_.size() > 0) {
    if (!stutter) {
      stutter_frame_move = gAPI->observer().GetGameLoop();
      stutter_frame_attack =
          gAPI->observer().GetGameLoop() + (stutter_steps / 2);
      stutter = true;
    }

    if (stutter_frame_move == x && span_ > 9) {
      gAPI->action().Move(units_, target_);
    }

    if (stutter_frame_attack == x) {
      gAPI->action().Attack(units_, target_);
    }

    if ((stutter_frame_attack + stutter_steps) == x) {
      gAPI->action().Attack(units_, target_);
      stutter = false;
    }
  }
}

void FocusFire::FFInitiate(const sc2::Unit* target_, bool main_) {
  if (!ff && main_) {
    uint32_t duration_gameloops = 12;
    break_focus_frame = gAPI->observer().GetGameLoop() + duration_gameloops;
    ff = true;
    target = target_;
  }
}

// 1. Want to pick closest target, right now using a gameloop to wait.
// 2. Want to incorporate stutter into the target.

void FocusFire::FFTarget(const sc2::Units& army_) {
  if (ff) {
    if (cnt < 1) {
      gAPI->action().Attack(army_, target, false);
      ++cnt;
    }
    if (break_focus_frame == gAPI->observer().GetGameLoop()) {
      ff = false;
      cnt = 0;
    }
  }
}
