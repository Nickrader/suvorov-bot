#include "strategies/terran/ZappBrannigan.h"

#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_unit_filters.h>

#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
// TODO:  Setup Git for some auto-squashing, make cleaner commit history when I
// merge to dev branch
// TODO: Building placement sometimes traps a grip of marines.  Big job.
//
namespace {
Historican gHistory("strategy.ZappBrannigan");
}  // namespace

Killbots::Killbots() : Strategy(20.0f) {}

void Killbots::OnGameStart(Builder* builder_) {
  // Initialize variables
  the_alamo = {gAPI->observer().GameInfo().enemy_start_locations.front().x,
               gAPI->observer().GameInfo().enemy_start_locations.front().y};
  std::cout << "\The Alamo: " << the_alamo.x << " , " << the_alamo.y
            << std::endl;

  // Give speech to boost morale
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
  build_cc = ShouldBuildExpansion();

  if (build_cc) BuildCommandcenter(minerals, builder_);

  if (!build_cc) BuildBarracks(minerals, builder_);
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
  Strategy::OnUnitCreated(unit_, builder_);
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
}

void Killbots::OnUnitDestroyed(const sc2::Unit* unit_, Builder* builder_) {
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
    if (buildings_enemy.size() == 1) AttackNextBuilding();
  }
}

void Killbots::OnGameEnd() {
  std::cout << "\nEnd Game:\n Barracks: " << number_of_barracks
            << "\nTownHalls: " << number_of_townhalls << std::endl;
  std::cout << "\nNumTargets: " << buildings_enemy.size() << std::endl;
  for (auto i : buildings_enemy) {
    std::cout << "\n Target: " << i->pos.x << " , " << i->pos.y
              << "\n\tLast Seen: " << i->last_seen_game_loop << std::endl;
  }
  // at the moment, best proxy for win/lose
  if (buildings_enemy.size() ==
      1) {  // 1 == win, game ends before removed from list, unless they killed
            // me and I see only 1 building (rarer).
    std::cout << "Call me cocky, but if there’s an alien out there, I can’t "
                 "kill. I haven’t met him and killed him yet."
              << std::endl;
  } else
    std::cout << "When I’m in command, every mission is a suicide mission."
              << std::endl;
}

void Killbots::DestroyedEnemyBuildings(const sc2::Unit* unit_) {
  if (unit_->alliance == sc2::Unit::Alliance::Enemy) {
    if (sc2::IsBuilding()(unit_->unit_type)) {
      for (sc2::Units::iterator it = buildings_enemy.begin();
           it != buildings_enemy.end(); ++it) {
        if (unit_ == *it) {
          OnMainDestroyed(it);
          buildings_enemy.erase(
              remove(buildings_enemy.begin(), buildings_enemy.end(), *it),
              buildings_enemy.end());
          break;
        }
      }
      if (enemy_main_destroyed) AttackNextBuilding();
    }
  }
}

// I get some orphaned units during initial assault after something is
// destroyed?? When it starts going through the kill list (after main
// destroyed), they get adopted
// TODO: Why is this happening, orphaned units before (main_destroyed).
// Idk, how I'd check this, unless I post all orders and last orders, timed with
// building destruction.

void Killbots::OnMainDestroyed(sc2::Units::iterator iter) {
  if (!enemy_main_destroyed) {
    auto& it_loc = *iter;
    sc2::Point2D unit_it_loc{it_loc->pos.x, it_loc->pos.y};
    if (unit_it_loc == the_alamo) {
      enemy_main_destroyed = true;
    }
  }
}

void Killbots::AttackNextBuilding() {
  if (buildings_enemy.size() > 0)
    gAPI->action().Attack(
        field_units, {buildings_enemy[0]->pos.x, buildings_enemy[0]->pos.y});

  if (buildings_enemy.size() == 0) {
    auto& expo = gHub->GetExpansions();
    for (int i = 0; i < expo.size() && i < field_units.size(); ++i) {
      sc2::Units xfer{};
      xfer.push_back(field_units[i]);
      gAPI->action().Attack(
          xfer, {expo[i].town_hall_location.x, expo[i].town_hall_location.y});
    }
  }
}

bool Killbots::ShouldBuildExpansion() {
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

void Killbots::BuildCommandcenter(const uint32_t& minerals, Builder* builder_) {
  if (minerals >= 400) {
    // just arbitrary number to avoid tons of CC in build queue.
    if (number_of_townhalls >= gHub->GetExpansions().size()) return;
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

void Killbots::BuildBarracks(const uint32_t& minerals, Builder* builder_) {
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
