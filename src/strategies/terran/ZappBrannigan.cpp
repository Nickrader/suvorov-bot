#include "strategies/terran/ZappBrannigan.h"

#include <sc2api/sc2_map_info.h>

#include "Historican.h"
#include "Hub.h"
#include "core/API.h"

namespace {
Historican gHistory("strategy.ZappBrannigan");
}  // namespace

Killbots::Killbots() : Strategy(20.0f) {}

void Killbots::OnGameStart(Builder* builder_) {
  std::cout << "Now, like all great plans, \
my strategy is so simple an idiot could have devised it."
            << std::endl;
}

// FIXME(nickrader): consolidate/ refactor
void Killbots::OnStep(Builder* builder_) {
  Strategy::OnStep(builder_);
  // want minerals to update on step?  Or some more delay?
  uint32_t minerals = gAPI->observer().GetMinerals();
  //  probably a better way to control flow, this is very simple implementatoin.
  build_cc = Should_Build_Expansion();

  if (build_cc) build_commandcenter(minerals, builder_);

  if (!build_cc) build_barracks(minerals, builder_);

  // FIXME(nickrader): possible cause of extra lag issues at max army supply?
  // lag more pronounced in Debug rather than Release compilation.
  // if (gAPI->observer().GetFoodUsed() == 200) {
  //  auto& targets = gAPI->observer().GameInfo().enemy_start_locations;
  //  for (auto i : m_units) {
  //    if (i->orders.empty()) {
  //      gAPI->action().Attack(m_units, targets.front());
  //    }
  //  }
  //}
}

void Killbots::OnUnitIdle(const sc2::Unit* unit_, Builder* builder_) {
  switch (unit_->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_MARINE);
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
  // converst sc2::Unit to sc2::Units b/c that is what Attack takes as arg
  sc2::Units units{};

  switch (unit_->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_MARINE:
      units.push_back(unit_);
      break;
    default:
      break;
  }

  gAPI->action().Attack(units, rally);
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

void Killbots::OnUnitEnterVision(const sc2::Unit* unit_, Builder* builder_) {
    gHub->
}

void Killbots::OnGameEnd() {
  std::cout << "\nEnd Game:\n Barracks: " << number_of_barracks
            << "\n TownHalls: " << number_of_townhalls << std::endl;
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
void Killbots::build_commandcenter(const uint32_t& minerals,
                                   Builder* builder_) {
  if (minerals >= 400) {
    // not sure best supply to make urgent, try max (200) for now.
    if (gAPI->observer().GetFoodUsed() >= 200) {
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER,
                                        true);
    } else {
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
    }
    ++number_of_townhalls;
  }
}
void Killbots::build_barracks(const uint32_t& minerals, Builder* builder_) {
  {
    if (gAPI->observer().CountUnitType(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT) >
        0)
      if (minerals >= 150) {
        builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_BARRACKS,
                                          true);
        ++number_of_barracks;
      }
  }
}
