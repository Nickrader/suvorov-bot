// The MIT License (MIT)
//
// Copyright (c) 2017-2022 Alexander Kurbatov

#include "Building.h"

#include <Dispatcher.h>

#include "Hub.h"
#include "core/API.h"

bool Building::Build(Order* order_) {
  // Find place to build the structure
  sc2::Point3D base = gAPI->observer().StartingLocation();
  sc2::Point2D point;

  unsigned attempt = 0;
  do {
    point.x = base.x + sc2::GetRandomScalar() * 15.0f;
    point.y = base.y + sc2::GetRandomScalar() * 15.0f;
    // modified if check to allieviate congestion.
    if (++attempt > 150 || gAPI->observer().CountUnitType(
                               sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER) > 1) {
      do {
        point.x = base.x + sc2::GetRandomScalar() * 25.0f;
        point.y = base.y + sc2::GetRandomScalar() * 25.0f;

        if (++attempt > 150 ||
            gAPI->observer().CountUnitType(
                sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER) > 2) {
          do {
            point.x = base.x + sc2::GetRandomScalar() * 40.0f;
            point.y = base.y + sc2::GetRandomScalar() * 40.0f;
            if (++attempt > 150) return false;
          } while (!gAPI->query().CanBePlaced(*order_, point));
        }
      } while (!gAPI->query().CanBePlaced(*order_, point));
    }
    // return false;
  } while (!gAPI->query().CanBePlaced(*order_, point));

  return gHub->AssignBuildTask(order_, point);
}
