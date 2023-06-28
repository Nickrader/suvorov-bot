#pragma once
#include <sc2api/sc2_api.h>

struct MicroActions {
  MicroActions();

  void StutterStepAttack(const sc2::Units& units_, sc2::Point2D target_,
                         float span_, const sc2::Unit* enemy_);

  void FocusFire(const sc2::Units& units_, const sc2::Unit* target_);

  uint32_t GetRange(const sc2::Unit* unit_) {
     auto& a = ranges.find(unit_->unit_type);
     return a->second;
  }

 private:
     std::map<sc2::UNIT_TYPEID, uint32_t> ranges = { {sc2::UNIT_TYPEID::TERRAN_MARINE, 5} };

  bool stutter;
  uint32_t stutter_frame_attack = 0;  // TODO: fix garbage value
  uint32_t stutter_frame_move = 0;    // or is OK b/c game start on 0 ???
  uint32_t steps_per_loop = 12;  // gameloops for 1 full stutter (move + attack)
};
