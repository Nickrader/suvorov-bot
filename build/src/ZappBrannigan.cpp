#include "strategies/terran/ZappBrannigan.h"

#include "Historican.h"
#include "Hub.h"
//#include "core/API.cpp"  // why does this fix type error, incomoplete class type??? for gAPI...Creates ungodly amt of linker errors, tho.
#include "core/API.h"
#include <sc2api/sc2_map_info.h>

// I do have problem that 2nd supply depot may be built early (far from
// optimal), because we only count completed structures, unless we implement a
// counter, like we did for barracks. need to find out where in logic of game it
// makes depots.  start at builder, dispatcher, the usual suspects.
//  I am not adding them to 'ObligatoryOrder' so not as simple as counting
// in my functions.

//  Towards end in a long game, default condition 'build cc' happens over marine
//  training???

//  Also, need trigger to attack all army anytime supply == 200

namespace {
Historican gHistory("strategy.ZappBrannigan");
}  // namespace

Killbots::Killbots() : Strategy(20.0f) {}

void Killbots::OnGameStart(Builder* builder_) {
  std::cout << "Now, like all great plans, \
my strategy is so simple an idiot could have devised it."
            << std::endl;
}

void Killbots::OnStep(Builder* builder_) {
  uint32_t minerals = gAPI->observer().GetMinerals();
  to_build_or_not_to_build = Should_Build_Expansion();

  if (to_build_or_not_to_build) {
    if (minerals >= 400) {
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
      ++number_of_townhalls;
    }
  }

  if (!to_build_or_not_to_build)
    if (minerals >= 150) {
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_BARRACKS);
      ++number_of_barracks;
    }

  Strategy::OnStep(builder_);
  if (gAPI->observer().GetFoodUsed() == 200) {
     auto targets = gAPI->observer().GameInfo().enemy_start_locations;
    gAPI->action().Attack(m_units, targets.front());
  }
}

void Killbots::OnUnitIdle(const sc2::Unit* unit_, Builder* builder_) {
  switch (unit_->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_MARINE, unit_);
      gHistory.info() << "Schedule Marine training\n";
      break;
    default:
      break;
  }
}

void Killbots::OnUnitCreated(const sc2::Unit* unit_, Builder* builder_) {
  const Expansions& expansions = gHub->GetExpansions();
  sc2::Point3D natural_expansion =
      expansions[1].town_hall_location;  // works at [0] not [1] for realtime.
  sc2::Point2D rally(natural_expansion.x, natural_expansion.y);
  sc2::Units units_{};

  switch (unit_->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_MARINE:
      units_.push_back(unit_);
      break;
    default:
      break;
  }

  gAPI->action().Attack(units_, rally);
  Strategy::OnUnitCreated(unit_, builder_);
}

void Killbots::OnUnitDestroyed(const sc2::Unit* unit_, Builder* builder_) {
  switch (unit_->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
      --number_of_townhalls;
      break;
    case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
      --number_of_barracks;
      break;
  }
}

bool Killbots::Should_Build_Expansion() {
  switch (number_of_townhalls) {
    case 1:
      if (number_of_barracks >= 3)
        return true;
      else
        return false;
      break;
    case 2:
      if (number_of_barracks >= 7)
        return true;
      else
        return false;
      break;
    case 3:
      if (number_of_barracks >= 11)
        return true;
      else
        return false;
      break;
    default:
      return true;  // true covers case max CC
      break;
  }
}
