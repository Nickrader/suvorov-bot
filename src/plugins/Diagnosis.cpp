// The MIT License (MIT)
//
// Copyright (c) 2017-2022 Alexander Kurbatov

#include "Diagnosis.h"

#include <sc2api/sc2_map_info.h>

#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
#include "strategies/terran/ZappBrannigan.h"

namespace {
Historican gHistory("plugin.diagnosis");
}  // namespace

Diagnosis::Diagnosis() : m_draw_grids(false), m_draw_target(false) {}

void Diagnosis::OnStep(Builder* builder_) {
  for (const auto& i : gAPI->observer().GetChatMessages()) {
    if (i.message == "gg") {
      gHistory.warning() << "The game was finished forcibly.\n";
      gAPI->debug().EndGame();
      return;
    }

    if (i.message == "grids:dump") {
      auto info = gAPI->observer().GameInfo();
      sc2::PathingGrid(info).Dump("./pathing_grid.txt");
      sc2::PlacementGrid(info).Dump("./placement_grid.txt");
      sc2::HeightMap(info).Dump("./height_map.txt");
    }

    if (i.message == "grids") m_draw_grids = !m_draw_grids;

    if (i.message == "target") m_draw_target = !m_draw_target;
  }

  if (m_draw_grids) {
    auto info = gAPI->observer().GameInfo();
    auto pathing_grid = sc2::PathingGrid(info);
    auto placement_grid = sc2::PlacementGrid(info);
    auto height_map = sc2::HeightMap(info);

    sc2::Point2D camera = gAPI->observer().GetCameraPos();
    int sx = static_cast<int>(camera.x - 12.0f);
    int sy = static_cast<int>(camera.y - 8.0f);

    for (int x = sx; x < sx + 24; ++x) {
      for (int y = sy; y < sy + 20; y++) {
        sc2::Point2DI pos(x, y);

        if (!gAPI->observer().IsPlayableTile(pos)) continue;

        sc2::Color color = sc2::Colors::Red;
        if (placement_grid.IsPlacable(pos))
          color = sc2::Colors::Green;
        else if (pathing_grid.IsPathable(pos))
          color = sc2::Colors::Yellow;

        sc2::Point3D tile_pos(static_cast<float>(x), static_cast<float>(y),
                              height_map.TerrainHeight(pos));

        gAPI->debug().DrawTile(tile_pos, color);
        gAPI->debug().DrawText(height_map.TerrainHeight(pos), tile_pos, color);
      }
    }
  }

  if (m_draw_target && gZapp->getTarget() != nullptr) {
    for (int i = 0; i < 15; ++i) {
      gAPI->debug().DrawBox(
          gZapp->offset3D(gZapp->getTarget()->pos,
                          -gZapp->getTarget()->radius - i / 30.0),
          gZapp->offset3D(gZapp->getTarget()->pos,
                          gZapp->getTarget()->radius + i / 30.0),
          sc2::Colors::Purple);
    }
  }

  sc2::Units aa = gZapp->getFieldUnits();
  if (!aa.empty()) {
    for (float i = 0.0f; i < 5; ++i) {
      gAPI->debug().DrawSphere(aa.begin().operator*()->pos,
                               aa.begin().operator*()->radius + i / 10,
                               sc2::Colors::Red);
    }
  }
  //if (m_draw_target && gZapp->getTarget() != nullptr) {
  //  for (int i = 0; i < 15; ++i) {
  //    gAPI->debug().DrawBox(
  //        gZapp->offset3D(gZapp->getTarget()->pos,
  //                        -gZapp->getTarget()->radius - i / 30.0),
  //        gZapp->offset3D(gZapp->getTarget()->pos,
  //                        gZapp->getTarget()->radius + i / 30.0),
  //        sc2::Colors::Yellow);
  //  }
  //}

  gAPI->debug().DrawText("Build order:");

  std::list<Order> orders = builder_->GetOrders();
  if (orders.empty()) {
    gAPI->debug().DrawText("Empty");
  } else {
    for (const auto& i : orders) gAPI->debug().DrawText(i.name);
  }

  for (const auto& i : gHub->GetExpansions())
    gAPI->debug().DrawSphere(i.town_hall_location, 0.35f);

  gAPI->debug().SendDebug();
}

void Diagnosis::OnGameEnd() {
  gAPI->control().SaveReplay();
  gHistory.info() << "Replay saved\n";
}
