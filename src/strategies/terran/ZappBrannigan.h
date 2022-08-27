#pragma once
/* "The Killbots?... A trifle.  It was simply a matter of outsmarting them. ...
Knowing their weakness, I sent wave after wave of my own men at them, until they
reached their limit, and shutdown." -Captain Zapp Brannigan */

#include "plugins/MicroActions.h"
#include "strategies/Strategy.h"

struct Zapp : Strategy {
  Zapp();

  void OnGameStart(Builder*) final;

  void OnStep(Builder*) final;

  void OnUnitIdle(const sc2::Unit*, Builder*) final;

  void OnUnitCreated(const sc2::Unit*, Builder*) final;

  void OnUnitDestroyed(const sc2::Unit*, Builder*) final;

  void OnUnitEnterVision(const sc2::Unit*, Builder*) final;

  void OnGameEnd() final;

 private:
  bool ShouldBuildExpansion();

  void IsMainDestroyed(sc2::Units::iterator it);

  // is *& needed/wanted.  Auto-refactor did this along with adding a bool.
  // work as simple pointer
  void AddEnemyBuilding(const sc2::Unit* unit_);

  void AttackNextBuilding();

  void BuildBarracks(const uint32_t& minerals, Builder* builder_);

  void BuildCommandcenter(const uint32_t& minerals, Builder* builder_);

  void DestroyedEnemyBuildings(const sc2::Unit* unit_);

  void DestroyedEnemyUnits(const sc2::Unit* unit_);

  void SeekEnemy();

  void UpdateGoal();

  bool build_cc = false;
  int number_of_townhalls = 1;  // we start with one command center.
  int number_of_barracks = 0;
  int number_of_supplydepots = 0;
  sc2::Units buildings_enemy{};
  bool enemy_main_destroyed = false;
  sc2::Point2D enemy_main = {0, 0};
  bool attacked = false;
  sc2::Point2D goal = { 0, 0 };
  float span = 0.0f;
  StutterStep stutter;
  FocusFire ff;
};

struct SortAttackBuildings {  // copy of logic from Hub.cpp
  SortAttackBuildings(sc2::Units& army_);
  bool operator()(const sc2::Unit* lhs_, const sc2::Unit* rhs_);

 private:
  sc2::Point2D kb_point;
};

