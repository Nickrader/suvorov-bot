#include "strategies/terran/ZappBrannigan.h"

#include <sc2api/sc2_map_info.h>

#include "Historican.h"
#include "Hub.h"
#include "core/API.h"

// when supply hits 200; marines added to front of queue are never built, should
// dump the orders list to std::output when minerals climb.  See if rax on the books???

// I do have problem that 2nd supply depot may be built early (far from
// optimal) need to find out where in logic of game it
// makes depots.
//   start at builder, dispatcher, the usual suspects.
//  I am not adding them to 'ObligatoryOrder' so not as simple as counting
// in my functions.

namespace {
Historican gHistory("strategy.ZappBrannigan");
}  // namespace

Killbots::Killbots() : Strategy(20.0f) {}

void Killbots::OnGameStart(Builder* builder_) {
  std::cout << "Now, like all great plans, \
my strategy is so simple an idiot could have devised it."
            << std::endl;
}

void Killbots::OnStep(Builder* builder_) {  // FIXME: consolidate/ refactor
  uint32_t minerals = gAPI->observer().GetMinerals();
  to_build_or_not_to_build =
      Should_Build_Expansion();  //  probably a better way to control flow, this
                                 //  is very simple implementatoin.

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

  // this does not fix issue, as there are lots of without orders after destroy
  // main. may not be full cause of issue, but seems to contribute. so I need
  // condition if
  if (gAPI->observer().GetFoodUsed() == 200) {
    auto targets = gAPI->observer().GameInfo().enemy_start_locations;
    for (auto i : m_units) {
      if (i->orders.empty()) {
        gAPI->action().Attack(m_units, targets.front());
      }
    }
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
      if (number_of_barracks >= 1)
        return true;
      else
        return false;
      break;
    case 2:
      if (number_of_barracks >= 5)
        return true;
      else
        return false;
      break;
    case 3:
      if (number_of_barracks >= 9)
        return true;
      else
        return false;
      break;
    case 4:
      if (number_of_barracks >= 14)
        return true;
      else
        return false;
      break;
    case 5:
      if (number_of_barracks >= 20)
        return true;
      else
        return false;
      break;
    case 6:
      if (number_of_barracks >= 25)
        return true;
      else
        return false;
      break;
    case 7:
      if (number_of_barracks >= 30)
        return true;
      else
        return false;
      break;
    case 8:
      if (number_of_barracks >= 35)
        return true;
      else
        return false;
      break;
    default:
      return true;  // true covers case max CC
      break;
  }
}
