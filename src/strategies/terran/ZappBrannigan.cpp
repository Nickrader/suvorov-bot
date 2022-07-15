#include "strategies/terran/ZappBrannigan.h"

#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_unit_filters.h>

#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
// TODO:  Setup Git for some auto-squashing, make cleaner commit history when I
// merge to dev branch
namespace {
Historican gHistory("strategy.ZappBrannigan");
}  // namespace

Killbots::Killbots() : Strategy(20.0f) {}

void Killbots::OnGameStart(Builder* builder_) {
  std::cout << "Now, like all great plans, \
my strategy is so simple an idiot could have devised it."
            << std::endl;
  std::cout << "\nEnemyStartLocations: "
            << gAPI->observer().GameInfo().enemy_start_locations[0].x << ", "
            << gAPI->observer().GameInfo().enemy_start_locations[0].y
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

void Killbots::OnUnitDestroyed(const sc2::Unit* unit_,
                               Builder* builder_) {  // breakpoint, investigate
  if (unit_->Alliance::Self) {
    switch (unit_->unit_type.ToType()) {
      case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        --number_of_townhalls;
        break;
      case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
        --number_of_barracks;
        break;
      default:
        break;
    }
  }
  // probably better here than OnStep, only need to cleanup if UnitDestroyed
  CleanUpBodies(m_units);
  CleanUpBodies(field_units);
  DestroyedEnemyBuildings(unit_);
}

void Killbots::OnUnitEnterVision(const sc2::Unit* unit_, Builder* builder_) {
  if (unit_->Alliance::Enemy && sc2::IsBuilding()(unit_->unit_type)) {
    for (auto i : buildings_enemy) {
      if (unit_ == i) return;
    }
    buildings_enemy.push_back(unit_);
  }
}

void Killbots::OnGameEnd() {
  sc2::Point2D target{gAPI->observer().GameInfo().enemy_start_locations[0].x,
                      gAPI->observer().GameInfo().enemy_start_locations[0].y};
  std::cout << "\nEnd Game:\n Barracks: " << number_of_barracks
            << "\nTownHalls: " << number_of_townhalls << std::endl;
  std::cout << "\nNumTargets: " << buildings_enemy.size() << std::endl;
  std::cout << "\nTargetsFront(): " << target.x << " , " << target.y
            << std::endl;
  for (auto i : buildings_enemy) {
    std::cout << "\n Target: " << i->pos.x << " , " << i->pos.y
              << "\n\tLast Seen: " << i->last_seen_game_loop << std::endl;
  }
}

void Killbots::DestroyedEnemyBuildings(const sc2::Unit* unit_) {
  if (unit_->alliance == sc2::Unit::Alliance::Enemy)
    if (sc2::IsBuilding()(unit_->unit_type)) {
      for (sc2::Units::iterator it = buildings_enemy.begin();
           it != buildings_enemy.end();
           ++it) {
        if (unit_ == *it) {
          OnMainDestroyed(it);
          buildings_enemy.erase(remove(buildings_enemy.begin(), buildings_enemy.end(), *it),
              buildings_enemy.end());
          break; // this might have fixed the out of range exception, unsure single building in vector
        }
      }
      AttackNextBuilding();
    }
}

void Killbots::OnMainDestroyed(sc2::Units::iterator iter) {
  if (!enemy_main_destroyed) {
    auto& targets = gAPI->observer().GameInfo().enemy_start_locations;
    auto& it_loc = *iter;
    sc2::Point2D unit_it_loc{it_loc->pos.x, it_loc->pos.y};
    if (unit_it_loc == targets.front()) {
      enemy_main_destroyed = true;
    }
  }
}
void Killbots::AttackNextBuilding() {
  if (enemy_main_destroyed) {
    gAPI->action().Attack(
        field_units, {buildings_enemy[0]->pos.x, buildings_enemy[0]->pos.y});
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

void Killbots::build_commandcenter(const uint32_t& minerals,
                                   Builder* builder_) {
  if (minerals >= 400) {
    // just try arbitrary number to avoid thousands of CC build orders.
    // TODO: make this expansions.size() test.
    if (number_of_townhalls >= 20) return;
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

// Killbots::OnStep()
// should be a funtion, but is there better way to control flow????
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
