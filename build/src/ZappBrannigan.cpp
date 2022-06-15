#include "strategies/terran/ZappBrannigan.h"

#include "Historican.h"
#include "Hub.h"
#include "strategies/terran/ZappBrannigan.h"
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
    default:  // this should mean we only expo to 3 CC total ??? Negative, we keep expanding, it seems...
      return false;
      break;
  }
}
