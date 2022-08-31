#include "strategies/terran/ZappBrannigan.h"

#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_unit_filters.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

// TODO:  Scout cheese, have alt build, path
// Should want to scout after 1st rax starts?

// TODO: examine logic. right now overall logic is attack main, when main
// destroyed, attack nearest building

// TODO:  Perhaps if first attack is total failure, change army cap that we
// attack at max

// TODO:
// when main destroyed and no targets, attack action is spammed bad.
// I guess this would be one excuse to learn the profiling tools better
// maybe way to automate/hook when acting up???
// right now, investigate using debugger, know it is OnStep problem somewhere

std::ostream& operator<<(std::ostream& os, tm time_) {
  os << time_.tm_year + 1900 << '-' << time_.tm_mon + 1 << '-' << time_.tm_mday
     << "  " << time_.tm_hour << ':' << time_.tm_min << ':' << time_.tm_sec
     << std::endl;
  return os;
}

std::tm timestamp() {
  auto aa =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::time_t const now_c = aa;
  std::tm tnm = {};
  localtime_s(&tnm, &now_c);
  return tnm;
}

namespace {
Historican gHistory("strategy.ZappBrannigan");
}  // namespace

Zapp::Zapp() : Strategy(16.0f) {}

void Zapp::OnGameStart(Builder* builder_) {
  // Initialize variables
  enemy_main = {gAPI->observer().GameInfo().enemy_start_locations.front().x,
                gAPI->observer().GameInfo().enemy_start_locations.front().y};
  std::cout << "\Enemy Main: " << enemy_main.x << " , " << enemy_main.y
            << std::endl;
  goal = enemy_main;

  // Give speech to boost morale
  std::cout << "Now, like all great plans, \
my strategy is so simple an idiot could have devised it."
            << std::endl;
}

// logic probably better driven by orders on units for micro and targeting.
void Zapp::OnStep(Builder* builder_) {
  Strategy::OnStep(builder_);

  uint32_t minerals = gAPI->observer().GetMinerals();

  build_cc = ShouldBuildExpansion();

  if (build_cc) BuildCommandcenter(minerals, builder_);

  if (!build_cc) BuildBarracks(minerals, builder_);

  UpdateGoal();
  stutter.StutterStepAttack(field_units, goal, span);

  // ff.FFTarget(field_units);
  if (buildings_enemy.size() == 0 && enemy_main_destroyed) SeekEnemy();
}

void Zapp::OnUnitIdle(const sc2::Unit* unit_, Builder* builder_) {
  switch (unit_->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
      if (gAPI->observer().GetFoodUsed() < 195) {
        builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_MARINE,
                                          true);
        gHistory.info() << "Schedule Marine conscription\n";
        break;
      } else {
        builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_MARINE);
        gHistory.info() << "Scheulde Marine training\n";
        break;
      }
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
  DestroyedEnemyUnits(unit_);
}

void Zapp::OnUnitEnterVision(const sc2::Unit* unit_, Builder* builder_) {
  if (unit_->Alliance::Enemy) {
    AddEnemyBuilding(unit_);
    if (buildings_enemy.size() > 0 && enemy_main_destroyed)
      AttackNextBuilding();
    //    if (IsWorkerUnit()(*unit_)) {
    // ff.FFInitiate(unit_, enemy_main_destroyed);
    //    }
    //-------------------------------------------------
  }
}

void Zapp::AddEnemyBuilding(const sc2::Unit* unit_) {
  if (sc2::IsBuilding()(unit_->unit_type)) {
    for (auto i : buildings_enemy) {
      if (unit_ == i) return;
    }
    buildings_enemy.push_back(unit_);
  }
}

void Zapp::OnGameEnd() {
  std::string game_file = "data/game_results.txt";
  std::ofstream outFile(game_file, std::ios_base::app);
  std::stringstream ss;
  std::tm ts = timestamp();
  ss << ts;
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
      // ss << sc2::UnitTypeToName(a->unit_type) << std::endl;
    }
  }
  if (buildings_enemy.size() <= 1 && cnt >= 1) {
    std::cout << "Call me cocky, but if there's an alien out there, I can't "
                 "kill. I haven't met him and killed him yet."
              << std::endl;
    ss << "Win\n";
  } else {
    std::cout << "When I'm in command, every mission is a suicide mission."
              << std::endl;
    ss << "Loss\n";
  }
  auto enemy_buildings = buildings_enemy.size();
  ss << "Mine: " << cnt << std::endl;
  ss << "Enemy: " << enemy_buildings << std::endl;
  outFile << ss.rdbuf();
  outFile.close();
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
      if (!enemy_main_destroyed) gAPI->action().Attack(field_units, enemy_main);
      if (enemy_main_destroyed) AttackNextBuilding();
    }
  }
}

void Zapp::DestroyedEnemyUnits(const sc2::Unit* unit_) {
  if (unit_->alliance == sc2::Unit::Alliance::Enemy) {
    AttackNextBuilding();
  }
}

void Zapp::IsMainDestroyed(sc2::Units::iterator iter) {
  if (!enemy_main_destroyed) {
    auto& it_loc = *iter;
    sc2::Point2D unit_it_loc{it_loc->pos.x, it_loc->pos.y};
    if (unit_it_loc == enemy_main) {
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
  } else
    gAPI->action().Attack(field_units, enemy_main);
}

void Zapp::SeekEnemy() {
  auto& expo = gHub->GetExpansions();
  for (int i = 0; i < expo.size() && i < field_units.size(); ++i) {
    sc2::Units xfer{};
    if (field_units.size() > 0) xfer.push_back(field_units[i]);
    gAPI->action().Attack(
        xfer, {expo[i].town_hall_location.x, expo[i].town_hall_location.y});
  }
  uint32_t delay = game_loops_second * 10;
  seek_enemy_delay = gAPI->observer().GetGameLoop() + delay;
}

sc2::Point3D Zapp::offset3D(sc2::Point3D point_, float offset_) {
  sc2::Point3D ret_val;
  ret_val.x = point_.x + offset_;
  ret_val.y = point_.y + offset_;
  ret_val.z = point_.z;
  return ret_val;
}

const sc2::Unit* Zapp::getTarget() {
  /*const sc2::Unit* a_target = target;*/
  return target;
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

// if under attack (townhalls decremented) then we should not build CC
// 'urgent')
void Zapp::BuildCommandcenter(const uint32_t& minerals, Builder* builder_) {
  uint32_t ten_seconds = 224;
  if (minerals >= 400) {
    // just arbitrary number to avoid tons of CC in build queue.
    if (number_of_townhalls >= gHub->GetExpansions().size()) return;

    if (!attacked && gAPI->observer().GetFoodUsed() < 200) {
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
      ++number_of_townhalls;
      return;
    }
    if (!attacked && gAPI->observer().GetFoodUsed() >= 200) {
      builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER,
                                        true);
      uint32_t command_center_delay =
          gAPI->observer().GetGameLoop() + (ten_seconds * 6);
      ++number_of_townhalls;
    }
    if (attacked) {
      builder_->ScheduleOptionalOrder(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
      ++number_of_townhalls;
    }
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

void Zapp::UpdateGoal() {
  if (field_units.size() < 1) return;
  Units wutang_clan =
      gAPI->observer().GetUnits(sc2::IsVisible(), sc2::Unit::Enemy);
  target = wutang_clan.GetClosestUnit(
      {field_units[0]->pos.x, field_units[0]->pos.y});
  if (target) {
    span = sc2::DistanceSquared2D(target->pos, field_units[0]->pos);
    // changeling is visible.  Causes problems. ChanglingMarine ID is 15.
    // should probably be an exclusion class or enum?
    // unless have focus fire, should exit, no update
    if (target->unit_type == 15) return;
    if (sc2::IsVisible()(*target) && (25 < span < 100)) {
      goal = target->pos;
      return;
    }
  }
  if (!enemy_main_destroyed) {
    goal = enemy_main;
    return;
  }
  if (enemy_main_destroyed) {
    if (gAPI->observer().GetGameLoop() > seek_enemy_delay) {
      SeekEnemy();
      return;
    }
  }
}

std::unique_ptr<Zapp> gZapp;
