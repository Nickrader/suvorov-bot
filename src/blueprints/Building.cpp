// The MIT License (MIT)
//
// Copyright (c) 2017-2022 Alexander Kurbatov

#include "Building.h"

#include "Hub.h"
#include "core/API.h"
#include <strategies/terran/ZappBrannigan.h>
#include <Dispatcher.h>

bool Building::Build(Order* order_) {
  // Find place to build the structure
  sc2::Point3D base = gAPI->observer().StartingLocation();
  sc2::Point2D point;

  unsigned attempt = 0;
  do {
    point.x = base.x + sc2::GetRandomScalar() * 15.0f;
    point.y = base.y + sc2::GetRandomScalar() * 15.0f;
    if (++attempt > 150) {
      do {
        point.x = base.x + sc2::GetRandomScalar() * 25.0f;
        point.y = base.y + sc2::GetRandomScalar() * 25.0f;

        if (++attempt > 150) return false;
      } while (!gAPI->query().CanBePlaced(*order_, point));
    }
    // return false;
  } while (!gAPI->query().CanBePlaced(*order_, point));

  return gHub->AssignBuildTask(order_, point);
}

// could use pointer to ZappBrannigan::number_of_townhalls as flow control, to bigger
// build area, if nat or third taken, much more room to build.
// right now my pointer is worthless, debugger shows it points to nothing.
//
// ptr is never instantiated like gHub or gAPI
// m_plugins takes the riff raff, but can't access it without looping ???
