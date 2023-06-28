#include "plugins/MicroActions.h"

#include "core/API.h"
#include "sc2api/sc2_unit_filters.h"

MicroActions::MicroActions() : stutter(false) {}

// still have mixed feelings about span_ parameter, but no wrapping
// my head around alternative.  When close enough, don't want move command.
void MicroActions::StutterStepAttack(const sc2::Units& units_,
                                    sc2::Point2D target_, float span_,
                                    const sc2::Unit* enemy_) {
  uint32_t x = gAPI->observer().GetGameLoop();
  if (x > (stutter_frame_attack + steps_per_loop)) {
    stutter = false;
  }
  if (units_.size() > 0) {
    if (!stutter) {
      stutter_frame_move = gAPI->observer().GetGameLoop();
      stutter_frame_attack =
          gAPI->observer().GetGameLoop() + (steps_per_loop / 2);
      stutter = true;
    }

    if (stutter_frame_move == x && span_ > 9) {
      gAPI->action().Move(units_, target_);
    }

    if (stutter_frame_attack == x) {
      gAPI->action().Attack(units_, target_);
      FocusFire(units_, enemy_);
    }

    if ((stutter_frame_attack + steps_per_loop) == x) {
      gAPI->action().Attack(units_, target_);
      FocusFire(units_, enemy_);
      stutter = false;
    }
  }
}

void MicroActions::FocusFire(const sc2::Units& units_,
                            const sc2::Unit* target_) {
  if (target_) {
    sc2::Units tmp;
    for (const sc2::Unit* a : units_) {
      if (sc2::Distance2D(a->pos, target_->pos) <= GetRange(a))
        tmp.push_back(a);
    }
    gAPI->action().Attack(tmp, target_, true);
  }
}
