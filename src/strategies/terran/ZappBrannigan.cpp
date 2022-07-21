#include "strategies/terran/ZappBrannigan.h"

#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_unit_filters.h>

#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

// TODO: Use my fork of sc2_api to make possible Attack() take Unit* instead of
// vector<Unit*>?
//
// TODO:  Setup Git workflow, may have to be in GitBash?  Get better commits,
// branching, etc.

namespace {
Historican gHistory("strategy.ZappBrannigan");
}  // namespace

Zapp::Zapp() : Strategy(20.0f) {}

void Zapp::OnGameStart(Builder* builder_) {
  // Initialize variables
  the_alamo = {gAPI->observer().GameInfo().enemy_start_locations.front().x,
               gAPI->observer().GameInfo().enemy_start_locations.front().y};
  std::cout << "\The Alamo: " << the_alamo.x << " , " << the_alamo.y
            << std::endl;
  goal = the_alamo;
  // Give speech to boost morale
  std::cout << "Now, like all great plans, \
my strategy is so simple an idiot could have devised it."
            << std::endl;
}

void Zapp::OnStep(Builder* builder_) {
  Strategy::OnStep(builder_);
  // want minerals to update on step?  Or some more delay?
  uint32_t minerals = gAPI->observer().GetMinerals();

  //  probably a better way to control flow, this is very simple implementatoin.
  // probably don't need to execute every step.
  build_cc = ShouldBuildExpansion();

  if (build_cc) BuildCommandcenter(minerals, builder_);

  if (!build_cc) BuildBarracks(minerals, builder_);

  stutter.StutterStepAttack(field_units, the_alamo);
}

void Zapp::OnUnitIdle(const sc2::Unit* unit_, Builder* builder_) {
  switch (unit_->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
      if (!attacked) {
        builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_MARINE);
        gHistory.info() << "Schedule Marine training\n";
        break;
      } else {
        builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_MARINE,
                                          true);
        gHistory.info() << "Scheulde Marine conscription\n";
        break;
      }
    default:
      break;
  }
}

void Zapp::OnUnitCreated(const sc2::Unit* unit_, Builder* builder_) {
  Strategy::OnUnitCreated(unit_, builder_);
  const Expansions& expansions = gHub->GetExpansions();
  sc2::Point3D natural_expansion =
      expansions[1].town_hall_location;  // works at [0] not [1] for realtime.
  sc2::Point2D rally(natural_expansion.x, natural_expansion.y);
  sc2::Units units{};

  switch (unit_->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_MARINE:
      units.push_back(unit_);
      break;
    default:
      break;
  }

  gAPI->action().Attack(units, rally);
}

void Zapp::OnUnitDestroyed(const sc2::Unit* unit_, Builder* builder_) {
  if (unit_->Alliance::Self) {
    switch (unit_->unit_type.ToType()) {
      case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        --number_of_townhalls;
        attacked = true;
        break;
      case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
        --number_of_barracks;
        break;
      default:
        break;
    }
    // probably better here than OnStep, only need to cleanup if UnitDestroyed
    CleanUpBodies(m_units);
    CleanUpBodies(field_units);
  }
  DestroyedEnemyBuildings(unit_);
}

void Zapp::OnUnitEnterVision(const sc2::Unit* unit_, Builder* builder_) {
  if (unit_->Alliance::Enemy && sc2::IsBuilding()(unit_->unit_type)) {
    for (auto i : buildings_enemy) {
      if (unit_ == i) return;
    }
    buildings_enemy.push_back(unit_);
    if (buildings_enemy.size() == 1 && enemy_main_destroyed)
      AttackNextBuilding();
  }
  if (unit_->Alliance::Enemy && IsCombatUnit()(*unit_)) {
    stutter.StutterStepInitiate({unit_->pos.x, unit_->pos.y});
  }
}

void Zapp::OnGameEnd() {
  std::cout << "\nEnd Game:\n Barracks: " << number_of_barracks
            << "\nTownHalls: " << number_of_townhalls << std::endl;
  std::cout << "\nNumTargets: " << buildings_enemy.size() << std::endl;
  for (auto i : buildings_enemy) {
    std::cout << "\n Target: " << i->pos.x << " , " << i->pos.y
              << "\n\tLast Seen: " << i->last_seen_game_loop << std::endl;
  }
  // at the moment, best proxy for win/lose
  auto ii = gAPI->observer().GetUnits();
  auto& i = ii.operator()();
  int cnt = 0;
  for (auto a : i) {
    if (sc2::IsBuilding()(a->unit_type)) {
      ++cnt;
      std::cout << sc2::UnitTypeToName(a->unit_type) << std::endl;
    }
  }
  if (buildings_enemy.size() <= 1 && cnt >= 1) {
    std::cout << "Call me cocky, but if there's an alien out there, I can't "
                 "kill. I haven't met him and killed him yet."
              << std::endl;
  } else
    std::cout << "When I'm in command, every mission is a suicide mission."
              << std::endl;

  auto enemy_buildings = buildings_enemy.size();
  std::cout << "Mine: " << cnt << std::endl;
  std::cout << "Enemy: " << enemy_buildings << std::endl;
}

void Zapp::DestroyedEnemyBuildings(const sc2::Unit* unit_) {
  if (unit_->alliance == sc2::Unit::Alliance::Enemy) {
    if (sc2::IsBuilding()(unit_->unit_type)) {
      for (sc2::Units::iterator it = buildings_enemy.begin();
           it != buildings_enemy.end(); ++it) {
        if (unit_ == *it) {
          IsMainDestroyed(it);
          buildings_enemy.erase(
              remove(buildings_enemy.begin(), buildings_enemy.end(), *it),
              buildings_enemy.end());
          break;  // exception thrown if buildings_enemy.size() == 0
          // if we've found the building to erase, no need to run rest of loop
          // either.
        }
      }
      if (!enemy_main_destroyed) gAPI->action().Attack(field_units, the_alamo);
      if (enemy_main_destroyed) AttackNextBuilding();
    }
  }
}

void Zapp::IsMainDestroyed(sc2::Units::iterator iter) {
  if (!enemy_main_destroyed) {
    auto& it_loc = *iter;
    sc2::Point2D unit_it_loc{it_loc->pos.x, it_loc->pos.y};
    if (unit_it_loc == the_alamo) {
      enemy_main_destroyed = true;
    }
  }
}

void Zapp::AttackNextBuilding() {
  if (buildings_enemy.size() > 0 && field_units.size() > 0) {
    std::sort(buildings_enemy.begin(), buildings_enemy.end(),
              SortAttackBuildings(field_units));

    goal = {buildings_enemy[0]->pos.x, buildings_enemy[0]->pos.y};
    gAPI->action().Attack(field_units, goal);
  }
  if (buildings_enemy.size() == 0) {
    auto& expo = gHub->GetExpansions();
    for (int i = 0; i < expo.size() && i < field_units.size(); ++i) {
      sc2::Units xfer{};
      if (field_units.size() > 0) xfer.push_back(field_units[i]);
      gAPI->action().Attack(
          xfer, {expo[i].town_hall_location.x, expo[i].town_hall_location.y});
    }
  }
}

// copy of logic from Hub.cpp
SortAttackBuildings::SortAttackBuildings(sc2::Units& army_) : kb_point{0, 0} {
  kb_point = {army_[0]->pos.x, army_[0]->pos.y};
}

bool SortAttackBuildings::operator()(const sc2::Unit* lhs_,
                                     const sc2::Unit* rhs_) {
  return sc2::DistanceSquared2D({lhs_->pos.x, lhs_->pos.y}, {kb_point}) <
         sc2::DistanceSquared2D({rhs_->pos.x, rhs_->pos.y}, {kb_point});
}

bool Zapp::ShouldBuildExpansion() {
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

// if under attack (townhalls decremented) then we should not build CC 'urgent')
void Zapp::BuildCommandcenter(const uint32_t& minerals, Builder* builder_) {
  if (minerals >= 400) {
    // just arbitrary number to avoid tons of CC in build queue.
    if (number_of_townhalls >= gHub->GetExpansions().size()) return;

    if (gAPI->observer().GetFoodUsed() >= 200 && !attacked) {
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER,
                                        true);  // this needs a limit. Or wait.
    }

    if (!attacked) {
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
    }

    if (attacked) {
      builder_->ScheduleOptionalOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
    }

    ++number_of_townhalls;
  }
}

void Zapp::BuildBarracks(const uint32_t& minerals, Builder* builder_) {
  {
    if (gAPI->observer().CountUnitType(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT) >
        0)
      if (minerals >= 150) {
        builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_BARRACKS);
        ++number_of_barracks;
      }
  }
}

