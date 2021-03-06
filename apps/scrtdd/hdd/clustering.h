/***************************************************************************
 *   Copyright (C) by ETHZ/SED                                             *
 *                                                                         *
 * This program is free software: you can redistribute it and/or modify    *
 * it under the terms of the GNU Affero General Public License as published*
 * by the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU Affero General Public License for more details.                     *
 *                                                                         *
 *                                                                         *
 *   Developed by Luca Scarabello <luca.scarabello@sed.ethz.ch>            *
 ***************************************************************************/

#ifndef __HDD_CLUSTERING_H__
#define __HDD_CLUSTERING_H__

#include "catalog.h"
#include <deque>
#include <list>
#include <seiscomp3/core/baseobject.h>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace Seiscomp {
namespace HDD {

DEFINE_SMARTPOINTER(Neighbours);

// DD background catalog
struct Neighbours : public Core::BaseObject
{
  unsigned refEvId;

  std::unordered_set<unsigned> ids; // neighbouring event id

  std::unordered_map<unsigned,                       // indexed by event id
                     std::unordered_map<std::string, // indexed by station id
                                        std::set<Catalog::Phase::Type>>>
      phases;

  std::unordered_set<unsigned>::size_type numNeighbours() const
  {
    return ids.size();
  }

  std::unordered_map<std::string, std::set<Catalog::Phase::Type>>
  allPhases() const
  {
    std::unordered_map<std::string, std::set<Catalog::Phase::Type>> allPhases;
    for (const auto &kw1 : phases)
      for (const auto &kw2 : kw1.second)
        allPhases[kw2.first].insert(kw2.second.begin(), kw2.second.end());
    return allPhases;
  }

  bool has(unsigned neighbourId) const
  {
    return ids.find(neighbourId) != ids.end();
  }

  bool has(unsigned neighbourId, const std::string stationId) const
  {
    const auto &neighPhases = phases.find(neighbourId);
    if (neighPhases != phases.end())
      return neighPhases->second.find(stationId) != neighPhases->second.end();
    return false;
  }

  bool has(unsigned neighbourId,
           const std::string stationId,
           Catalog::Phase::Type type) const
  {
    const auto &neighPhases = phases.find(neighbourId);
    if (neighPhases != phases.end())
    {
      const auto &neighPhaseTypes = neighPhases->second.find(stationId);
      if (neighPhaseTypes != neighPhases->second.end())
        return neighPhaseTypes->second.find(type) !=
               neighPhaseTypes->second.end();
    }
    return false;
  }

  CatalogPtr toCatalog(const CatalogCPtr &catalog,
                       bool includeRefEv = false) const
  {
    CatalogPtr returnCat(new Catalog());
    for (unsigned neighbourId : ids)
      returnCat->add(neighbourId, *catalog, true);
    if (includeRefEv) returnCat->add(refEvId, *catalog, true);
    return returnCat;
  }
};

NeighboursPtr
selectNeighbouringEvents(const CatalogCPtr &catalog,
                         const Catalog::Event &refEv,
                         const CatalogCPtr &refEvCatalog,
                         double minPhaseWeight   = 0,
                         double minESdis         = 0,
                         double maxESdis         = -1,
                         double minEStoIEratio   = 0,
                         unsigned minDTperEvt    = 1,
                         unsigned maxDTperEvt    = 0, // 0 = no limits
                         unsigned minNumNeigh    = 1,
                         unsigned maxNumNeigh    = 0, // 0 = no limits
                         unsigned numEllipsoids  = 5,
                         double maxEllipsoidSize = 10,
                         bool keepUnmatched      = false);

std::deque<std::list<NeighboursPtr>>
selectNeighbouringEventsCatalog(const CatalogCPtr &catalog,
                                double minPhaseWeight,
                                double minESdis,
                                double maxESdis,
                                double minEStoIEratio,
                                unsigned minDTperEvt,
                                unsigned maxDTperEvt,
                                unsigned minNumNeigh,
                                unsigned maxNumNeigh,
                                unsigned numEllipsoids,
                                double maxEllipsoidSize,
                                bool keepUnmatched);

std::deque<std::list<NeighboursPtr>>
clusterizeNeighbouringEvents(const std::list<NeighboursPtr> &neighboursList);

} // namespace HDD
} // namespace Seiscomp

#endif
