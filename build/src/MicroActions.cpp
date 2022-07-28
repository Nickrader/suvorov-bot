#include "src/MicroActions.h"

#include "core/API.h"
#include "sc2api/sc2_unit_filters.h"
#include "strategies/terran/ZappBrannigan.h"

StutterStep::StutterStep() : stutter(false), enemy_main_destroyed(false) {}

// StutterStepAttack is trying to do too much and thus become very brittle.
// I think some of the overall logic has been worked out
// but too specialized, not general enough.
// s/b less parameters?

// should probably pick off a target priority in addition to distance.
// attack priority: game engine has it's own, generally good enough.
// however sometimes focus firing damage dealer will help.
void StutterStep::StutterStepAttack(const sc2::Units& units_,
                                    sc2::Point2D enemy_main_,
                                    bool enemy_main_destroyed_) {
  uint32_t x = gAPI->observer().GetGameLoop();
  if (units_.size() > 0) {
    enemy_main_destroyed = enemy_main_destroyed_;
    Units wutang_clan = gAPI->observer().GetUnits(sc2::Unit::Enemy);

    const sc2::Unit* target =
        wutang_clan.GetClosestUnit({units_[0]->pos.x, units_[0]->pos.y});
    if (target) {
      auto span = sc2::DistanceSquared2D(target->pos, units_[0]->pos);

      if (sc2::IsVisible()(*target)) {
        // changeling is visible.  Causes problems. ChanglingMarine is 15.
        if (target->unit_type != 15) {
          if (!stutter) {
            stutter_frame_move = gAPI->observer().GetGameLoop() + 1;
            stutter_frame_attack =
                gAPI->observer().GetGameLoop() + (stutter_steps / 2);
            stutter = true;
          }
          if (stutter_frame_move == x && span > 25 &&
              span < 100) {  // magic constant squared
            gAPI->action().Move(units_, {target->pos.x, target->pos.y});
          }
          if (stutter_frame_attack == x && span < 64) {
            gAPI->action().Attack(units_, {target->pos.x, target->pos.y});
          }
          if ((stutter_frame_attack + stutter_steps) == x) {
            // enemy_main_destroyed not being updated.
            // Without pointer to zapp. How to fix?
            if (!enemy_main_destroyed)
              gAPI->action().Attack(units_, enemy_main_);
            else
              gAPI->action().Attack(units_, {target->pos.x, target->pos.y});
          }
        }
      }
    }
    if (x > stutter_frame_attack + stutter_steps) {
      stutter = false;
      // if (units_.size() > 0) {
      if (!target || !sc2::IsVisible()(*target)) {
        if (!enemy_main_destroyed)
          gAPI->action().Attack(units_, enemy_main_);
        else if (target && sc2::IsBuilding()(*target))
          gAPI->action().Attack(units_, target->pos);
      }
    }
    //}
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
