#include "ZappBrannigan.h"

#include "Historican.h"
#include "Hub.h"
#include "core/API.h"

namespace {
Historican gHistory("strategy.marine_push");
}  // namespace

Killbots::Killbots() : Strategy(20.0f) {}

void Killbots::OnGameStart(Builder* builder_) {
  std::cout << "Now, like all great plans, \
my strategy is so simple an idiot could have devised it."
            << std::endl;
}

void Killbots::OnStep(Builder* builder_) {
  uint32_t minerals = gAPI->observer().GetMinerals();
  if (!Should_Build_Expansion) {
    if (minerals >= 400)
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
    if (minerals >= 150)
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_BARRACKS);
  }

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

bool Killbots::Should_Build_Expansion() {
  uint32_t number_of_barracks =
      gAPI->observer().CountUnitType(sc2::UNIT_TYPEID::TERRAN_BARRACKS);
  uint32_t number_of_townhalls =
      gAPI->observer().CountUnitType(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
  switch (number_of_townhalls) {
    case 1:
      if (number_of_barracks >= 3) return true;
    case 2:
      if (number_of_barracks >= 7) return true;
    case 3:
      if (number_of_barracks >= 11) return true;
    default:  // this should mean we only expo to 3 CC total ???
      return false;
      break;
  }
}
