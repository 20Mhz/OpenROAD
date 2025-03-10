/* Authors: Lutong Wang and Bangqi Xu */
/*
 * Copyright (c) 2019, The Regents of the University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FR_FLEXDR_H_
#define _FR_FLEXDR_H_

#include <boost/polygon/polygon.hpp>
#include <deque>
#include <memory>

#include "db/drObj/drMarker.h"
#include "db/drObj/drNet.h"
#include "dr/FlexDR_graphics.h"
#include "dr/FlexGridGraph.h"
#include "dr/FlexWavefront.h"
#include "frDesign.h"

using Rectangle = boost::polygon::rectangle_data<int>;

namespace odb {
class dbDatabase;
}
namespace ord {
class Logger;
}

namespace fr {

class frConstraint;

struct FlexDRViaData
{
  // std::pair<layer1area, layer2area>
  std::vector<std::pair<frCoord, frCoord>> halfViaEncArea;

  // via2viaMinLen[z][0], last via=down, curr via=down
  // via2viaMinLen[z][1], last via=down, curr via=up
  // via2viaMinLen[z][2], last via=up,   curr via=down
  // via2viaMinLen[z][3], last via=up,   curr via=up
  std::vector<std::pair<std::vector<frCoord>, std::vector<bool>>> via2viaMinLen;

  // via2viaMinLen[z][0], prev via=down, curr via=down, min required x dist
  // via2viaMinLen[z][1], prev via=down, curr via=down, min required y dist
  // via2viaMinLen[z][2], prev via=down, curr via=up,   min required x dist
  // via2viaMinLen[z][3], prev via=down, curr via=up,   min required y dist
  // via2viaMinLen[z][4], prev via=up,   curr via=down, min required x dist
  // via2viaMinLen[z][5], prev via=up,   curr via=down, min required y dist
  // via2viaMinLen[z][6], prev via=up,   curr via=up,   min required x dist
  // via2viaMinLen[z][7], prev via=up,   curr via=up,   min required y dist
  std::vector<std::vector<frCoord>> via2viaMinLenNew;

  // via2turnMinLen[z][0], last via=down, min required x dist
  // via2turnMinLen[z][1], last via=down, min required y dist
  // via2turnMinLen[z][2], last via=up,   min required x dist
  // via2turnMinLen[z][3], last via=up,   min required y dist
  std::vector<std::vector<frCoord>> via2turnMinLen;
};

class FlexDR
{
 public:
  // constructors
  FlexDR(frDesign* designIn, Logger* loggerIn, odb::dbDatabase* dbIn);
  ~FlexDR();
  // getters
  frTechObject* getTech() const { return design_->getTech(); }
  frDesign* getDesign() const { return design_; }
  frRegionQuery* getRegionQuery() const { return design_->getRegionQuery(); }
  // others
  int main();
  const FlexDRViaData* getViaData() const { return &via_data_; }
  void setDebug(frDebugSettings* settings);

 protected:
  frDesign* design_;
  Logger* logger_;
  odb::dbDatabase* db_;
  std::vector<std::vector<std::map<frNet*,
                                   std::set<std::pair<frPoint, frLayerNum>>,
                                   frBlockObjectComp>>>
      gcell2BoundaryPin_;

  FlexDRViaData via_data_;
  std::vector<int> numViols_;
  std::unique_ptr<FlexDRGraphics> graphics_;
  std::string debugNetName_;

  // others
  void init();
  void initFromTA();
  void initGCell2BoundaryPin();
  void getBatchInfo(int& batchStepX, int& batchStepY);

  void init_halfViaEncArea();
  void init_via2viaMinLen();
  frCoord init_via2viaMinLen_minSpc(frLayerNum lNum,
                                    frViaDef* viaDef1,
                                    frViaDef* viaDef2);
  frCoord init_via2viaMinLen_minimumcut1(frLayerNum lNum,
                                         frViaDef* viaDef1,
                                         frViaDef* viaDef2);
  bool init_via2viaMinLen_minimumcut2(frLayerNum lNum,
                                      frViaDef* viaDef1,
                                      frViaDef* viaDef2);

  void init_via2viaMinLenNew();
  frCoord init_via2viaMinLenNew_minSpc(frLayerNum lNum,
                                       frViaDef* viaDef1,
                                       frViaDef* viaDef2,
                                       bool isCurrDirY);
  frCoord init_via2viaMinLenNew_minimumcut1(frLayerNum lNum,
                                            frViaDef* viaDef1,
                                            frViaDef* viaDef2,
                                            bool isCurrDirY);
  frCoord init_via2viaMinLenNew_cutSpc(frLayerNum lNum,
                                       frViaDef* viaDef1,
                                       frViaDef* viaDef2,
                                       bool isCurrDirY);

  frCoord init_via2turnMinLen_minSpc(frLayerNum lNum,
                                     frViaDef* viaDef,
                                     bool isCurrDirY);
  frCoord init_via2turnMinLen_minStp(frLayerNum lNum,
                                     frViaDef* viaDef,
                                     bool isCurrDirY);
  void init_via2turnMinLen();

  void removeGCell2BoundaryPin();
  void checkConnectivity(int iter = -1);
  void checkConnectivity_initDRObjs(const frNet* net,
                                    std::vector<frConnFig*>& netDRObjs);
  void checkConnectivity_pin2epMap(
      const frNet* net,
      const std::vector<frConnFig*>& netDRObjs,
      std::map<frBlockObject*,
               std::set<std::pair<frPoint, frLayerNum>>,
               frBlockObjectComp>& pin2epMap);
  void checkConnectivity_pin2epMap_helper(
      const frNet* net,
      const frPoint& bp,
      frLayerNum lNum,
      std::map<frBlockObject*,
               std::set<std::pair<frPoint, frLayerNum>>,
               frBlockObjectComp>& pin2epMap,
      bool isWire);
  void checkConnectivity_nodeMap(
      const frNet* net,
      const std::vector<frConnFig*>& netDRObjs,
      std::vector<frBlockObject*>& netPins,
      const std::map<frBlockObject*,
                     std::set<std::pair<frPoint, frLayerNum>>,
                     frBlockObjectComp>& pin2epMap,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  void checkConnectivity_nodeMap_routeObjEnd(
      const frNet* net,
      const std::vector<frConnFig*>& netRouteObjs,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  void checkConnectivity_nodeMap_routeObjSplit(
      const frNet* net,
      const std::vector<frConnFig*>& netRouteObjs,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  void checkConnectivity_nodeMap_routeObjSplit_helper(
      const frPoint& crossPt,
      frCoord trackCoord,
      frCoord splitCoord,
      frLayerNum lNum,
      const std::vector<
          std::map<frCoord, std::map<frCoord, std::pair<frCoord, int>>>>&
          mergeHelper,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  void checkConnectivity_nodeMap_pin(
      const std::vector<frConnFig*>& netRouteObjs,
      std::vector<frBlockObject*>& netPins,
      const std::map<frBlockObject*,
                     std::set<std::pair<frPoint, frLayerNum>>,
                     frBlockObjectComp>& pin2epMap,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  void checkConnectivity_merge1(
      const frNet* net,
      const std::vector<frConnFig*>& netRouteObjs,
      std::vector<std::map<frCoord, std::vector<int>>>& horzPathSegs,
      std::vector<std::map<frCoord, std::vector<int>>>& vertPathSegs);
  void checkConnectivity_merge2(
      frNet* net,
      const std::vector<frConnFig*>& netRouteObjs,
      const std::vector<std::map<frCoord, std::vector<int>>>& horzPathSegs,
      const std::vector<std::map<frCoord, std::vector<int>>>& vertPathSegs,
      std::vector<std::vector<std::vector<int>>>& horzVictims,
      std::vector<std::vector<std::vector<int>>>& vertVictims,
      std::vector<std::vector<std::vector<std::pair<frCoord, frCoord>>>>&
          horzNewSegSpans,
      std::vector<std::vector<std::vector<std::pair<frCoord, frCoord>>>>&
          vertNewSegSpans);
  void checkConnectivity_merge3(
      frNet* net,
      std::vector<frConnFig*>& netRouteObjs,
      const std::vector<std::map<frCoord, std::vector<int>>>& horzPathSegs,
      const std::vector<std::map<frCoord, std::vector<int>>>& vertPathSegs,
      const std::vector<std::vector<std::vector<int>>>& horzVictims,
      const std::vector<std::vector<std::vector<int>>>& vertVictims,
      const std::vector<std::vector<std::vector<std::pair<frCoord, frCoord>>>>&
          horzNewSegSpans,
      const std::vector<std::vector<std::vector<std::pair<frCoord, frCoord>>>>&
          vertNewSegSpans);
  void checkConnectivity_addMarker(frNet* net,
                                   frLayerNum lNum,
                                   const frBox& bbox);
  void checkConnectivity_merge_perform(
      const std::vector<frConnFig*>& netRouteObjs,
      const std::vector<int>& indices,
      std::vector<int>& victims,
      std::vector<std::pair<frCoord, frCoord>>& newSegSpans,
      bool isHorz);
  void checkConnectivity_merge_perform_helper(
      const std::vector<std::pair<std::pair<frCoord, frCoord>, int>>& segSpans,
      std::vector<int>& victims,
      std::vector<std::pair<frCoord, frCoord>>& newSegSpans);
  void checkConnectivity_merge_commit(
      frNet* net,
      std::vector<frConnFig*>& netRouteObjs,
      const std::vector<int>& victims,
      frLayerNum lNum,
      frCoord trackCoord,
      const std::vector<std::pair<frCoord, frCoord>>& newSegSpans,
      bool isHorz);
  bool checkConnectivity_astar(
      frNet* net,
      std::vector<bool>& adjVisited,
      std::vector<int>& adjPrevIdx,
      const std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap,
      const vector<frConnFig*>& netDRObjs,
      const int& gCnt,
      const int& nCnt);
  void checkConnectivity_final(
      frNet* net,
      std::vector<frConnFig*>& netRouteObjs,
      std::vector<frBlockObject*>& netPins,
      const std::vector<bool>& adjVisited,
      int gCnt,
      int nCnt,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  std::map<frNet*, std::set<std::pair<frPoint, frLayerNum>>, frBlockObjectComp>
  initDR_mergeBoundaryPin(int i, int j, int size, const frBox& routeBox);
  void searchRepair(int iter,
                    int size,
                    int offset,
                    int mazeEndIter,
                    frUInt4 workerDRCCost,
                    frUInt4 workerMarkerCost,
                    int ripupMode,
                    bool followGuide);
  void end(bool writeMetrics = false);

  // utility
  void reportDRC();
};

class FlexDRWorker;
class FlexDRWorkerRegionQuery
{
 public:
  FlexDRWorkerRegionQuery(FlexDRWorker* in);
  ~FlexDRWorkerRegionQuery();
  void add(drConnFig* connFig);
  void remove(drConnFig* connFig);
  void query(const frBox& box,
             const frLayerNum layerNum,
             std::vector<drConnFig*>& result) const;
  void query(const frBox& box,
             const frLayerNum layerNum,
             std::vector<rq_box_value_t<drConnFig*>>& result) const;
  void init();
  void cleanup();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

class FlexDRMinAreaVio
{
 public:
  // constructors
  FlexDRMinAreaVio() : net_(nullptr), gapArea_(0) {}
  FlexDRMinAreaVio(drNet* netIn,
                   FlexMazeIdx bpIn,
                   FlexMazeIdx epIn,
                   frCoord gapAreaIn)
      : net_(netIn), bp_(bpIn), ep_(epIn), gapArea_(gapAreaIn)
  {
  }
  // setters
  void setDRNet(drNet* netIn) { net_ = netIn; }
  void setPoints(FlexMazeIdx bpIn, FlexMazeIdx epIn)
  {
    bp_ = bpIn;
    ep_ = epIn;
  }
  void setGapArea(frCoord gapAreaIn) { gapArea_ = gapAreaIn; }

  // getters
  drNet* getNet() const { return net_; }
  void getPoints(FlexMazeIdx& bpIn, FlexMazeIdx& epIn) const
  {
    bpIn = bp_;
    epIn = ep_;
  }
  frCoord getGapArea() const { return gapArea_; }

 protected:
  drNet* net_;
  FlexMazeIdx bp_, ep_;
  frCoord gapArea_;
};

class FlexGCWorker;
class FlexDRWorker
{
 public:
  // constructors
  FlexDRWorker(const FlexDRViaData* via_data, frDesign* design, Logger* logger)
      : design_(design),
        logger_(logger),
        graphics_(nullptr),
        via_data_(via_data),
        routeBox_(),
        extBox_(),
        drcBox_(),
        drIter_(0),
        mazeEndIter_(1),
        followGuide_(false),
        needRecheck_(false),
        skipRouting_(false),
        ripupMode_(1),
        workerDRCCost_(ROUTESHAPECOST),
        workerMarkerCost_(MARKERCOST),
        boundaryPin_(),
        pinCnt_(0),
        initNumMarkers_(0),
        apSVia_(),
        fixedObjs_(),
        planarHistoryMarkers_(),
        viaHistoryMarkers_(),
        historyMarkers_(std::vector<std::set<FlexMazeIdx>>(3)),
        nets_(),
        owner2nets_(),
        gridGraph_(design->getTech(), this),
        markers_(),
        rq_(this),
        gcWorker_(nullptr) /*, drcWorker(drIn->getDesign())*/
  {
  }
  // setters
  void setRouteBox(const frBox& boxIn) { routeBox_.set(boxIn); }
  void setExtBox(const frBox& boxIn) { extBox_.set(boxIn); }
  void setDrcBox(const frBox& boxIn) { drcBox_.set(boxIn); }
  void setGCellBox(const frBox& boxIn) { gcellBox_.set(boxIn); }
  void setDRIter(int in) { drIter_ = in; }
  void setDRIter(int in,
                 std::map<frNet*,
                          std::set<std::pair<frPoint, frLayerNum>>,
                          frBlockObjectComp>& bp)
  {
    drIter_ = in;
    boundaryPin_ = std::move(bp);
  }
  void setMazeEndIter(int in) { mazeEndIter_ = in; }
  void setRipupMode(int in) { ripupMode_ = in; }
  void setFollowGuide(bool in) { followGuide_ = in; }
  void setCost(frUInt4 drcCostIn, frUInt4 markerCostIn)
  {
    workerDRCCost_ = drcCostIn;
    workerMarkerCost_ = markerCostIn;
  }
  void setMarkers(std::vector<frMarker>& in)
  {
    markers_.clear();
    frBox box;
    for (auto& marker : in) {
      marker.getBBox(box);
      if (getDrcBox().overlaps(box)) {
        markers_.push_back(marker);
      }
    }
  }
  void setMarkers(const std::vector<std::unique_ptr<frMarker>>& in)
  {
    markers_.clear();
    frBox box;
    for (auto& uMarker : in) {
      auto& marker = *uMarker;
      marker.getBBox(box);
      if (getDrcBox().overlaps(box)) {
        markers_.push_back(marker);
      }
    }
  }
  void setMarkers(std::vector<frMarker*>& in)
  {
    markers_.clear();
    frBox box;
    for (auto& marker : in) {
      marker->getBBox(box);
      if (getDrcBox().overlaps(box)) {
        markers_.push_back(*marker);
      }
    }
  }
  void setBestMarkers() { bestMarkers_ = markers_; }
  void clearMarkers() { markers_.clear(); }
  void setInitNumMarkers(int in) { initNumMarkers_ = in; }
  void setGCWorker(FlexGCWorker* in) { gcWorker_ = in; }

  void setGraphics(FlexDRGraphics* in)
  {
    graphics_ = in;
    gridGraph_.setGraphics(in);
  }

  // getters
  frTechObject* getTech() const { return design_->getTech(); }
  void getRouteBox(frBox& boxIn) const { boxIn.set(routeBox_); }
  const frBox& getRouteBox() const { return routeBox_; }
  frBox& getRouteBox() { return routeBox_; }
  void getExtBox(frBox& boxIn) const { boxIn.set(extBox_); }
  const frBox& getExtBox() const { return extBox_; }
  frBox& getExtBox() { return extBox_; }
  const frBox& getDrcBox() const { return drcBox_; }
  frBox& getDrcBox() { return drcBox_; }
  const frBox& getGCellBox() const { return gcellBox_; }
  bool isInitDR() const { return (drIter_ == 0); }
  int getDRIter() const { return drIter_; }
  int getMazeEndIter() const { return mazeEndIter_; }
  bool isFollowGuide() const { return followGuide_; }
  int getRipupMode() const { return ripupMode_; }
  const std::vector<std::unique_ptr<drNet>>& getNets() const { return nets_; }
  std::vector<std::unique_ptr<drNet>>& getNets() { return nets_; }
  const std::vector<drNet*>* getDRNets(frNet* net) const
  {
    auto it = owner2nets_.find(net);
    if (it != owner2nets_.end()) {
      return &(it->second);
    } else {
      return nullptr;
    }
  }
  frDesign* getDesign() { return design_; }
  const std::vector<frMarker>& getMarkers() const { return markers_; }
  std::vector<frMarker>& getMarkers() { return markers_; }
  const std::vector<frMarker>& getBestMarkers() const { return bestMarkers_; }
  std::vector<frMarker>& getBestMarkers() { return bestMarkers_; }
  const FlexDRWorkerRegionQuery& getWorkerRegionQuery() const { return rq_; }
  FlexDRWorkerRegionQuery& getWorkerRegionQuery() { return rq_; }
  int getInitNumMarkers() const { return initNumMarkers_; }
  int getNumMarkers() const { return markers_.size(); }
  int getBestNumMarkers() const { return bestMarkers_.size(); }
  FlexGCWorker* getGCWorker() { return gcWorker_; }
  const FlexDRViaData* getViaData() const { return via_data_; }

  // others
  int main(frDesign* design);

  Logger* getLogger() { return logger_; }

  const vector<Point3D> getSpecialAccessAPs() const { return specialAccessAPs; }

 protected:
  typedef struct
  {
    frBlockObject* block;
    int numReroute;
    bool doRoute;
  } RouteQueueEntry;
  frDesign* design_;
  frTechObject* tech_;
  Logger* logger_;
  FlexDRGraphics* graphics_;  // owned by FlexDR
  frDebugSettings* debugSettings_;
  const FlexDRViaData* via_data_;
  frBox routeBox_;
  frBox extBox_;
  frBox drcBox_;
  frBox gcellBox_;
  int drIter_;
  int mazeEndIter_;
  bool followGuide_ : 1;
  bool needRecheck_ : 1;
  bool skipRouting_ : 1;
  int ripupMode_;
  // drNetOrderingEnum netOrderingMode;
  frUInt4 workerDRCCost_, workerMarkerCost_;
  // used in init route as gr boundary pin
  std::map<frNet*, std::set<std::pair<frPoint, frLayerNum>>, frBlockObjectComp>
      boundaryPin_;
  int pinCnt_;
  int initNumMarkers_;
  std::map<FlexMazeIdx, drAccessPattern*> apSVia_;
  std::vector<frBlockObject*> fixedObjs_;
  std::set<FlexMazeIdx> planarHistoryMarkers_;
  std::set<FlexMazeIdx> viaHistoryMarkers_;
  std::vector<std::set<FlexMazeIdx>> historyMarkers_;

  // local storage
  std::vector<std::unique_ptr<drNet>> nets_;
  std::map<frNet*, std::vector<drNet*>> owner2nets_;
  FlexGridGraph gridGraph_;
  std::vector<frMarker> markers_;
  std::vector<frMarker> bestMarkers_;
  FlexDRWorkerRegionQuery rq_;

  // persistant gc worker
  FlexGCWorker* gcWorker_;

  // init
  void init(const frDesign* design);
  void initNets(const frDesign* design);
  void initNetObjs(
      const frDesign* design,
      std::set<frNet*, frBlockObjectComp>& nets,
      std::map<frNet*,
               std::vector<std::unique_ptr<drConnFig>>,
               frBlockObjectComp>& netRouteObjs,
      std::map<frNet*,
               std::vector<std::unique_ptr<drConnFig>>,
               frBlockObjectComp>& netExtObjs,
      std::map<frNet*, std::vector<frRect>, frBlockObjectComp>& netOrigGuides);
  void initNetObjs_pathSeg(frPathSeg* pathSeg,
                           std::set<frNet*, frBlockObjectComp>& nets,
                           std::map<frNet*,
                                    std::vector<std::unique_ptr<drConnFig>>,
                                    frBlockObjectComp>& netRouteObjs,
                           std::map<frNet*,
                                    std::vector<std::unique_ptr<drConnFig>>,
                                    frBlockObjectComp>& netExtObjs);
  void initNetObjs_via(frVia* via,
                       std::set<frNet*, frBlockObjectComp>& nets,
                       std::map<frNet*,
                                std::vector<std::unique_ptr<drConnFig>>,
                                frBlockObjectComp>& netRouteObjs,
                       std::map<frNet*,
                                std::vector<std::unique_ptr<drConnFig>>,
                                frBlockObjectComp>& netExtObjs);
  void initNetObjs_patchWire(frPatchWire* pwire,
                             std::set<frNet*, frBlockObjectComp>& nets,
                             std::map<frNet*,
                                      std::vector<std::unique_ptr<drConnFig>>,
                                      frBlockObjectComp>& netRouteObjs,
                             std::map<frNet*,
                                      std::vector<std::unique_ptr<drConnFig>>,
                                      frBlockObjectComp>& netExtObjs);
  void initNets_initDR(
      const frDesign* design,
      std::set<frNet*, frBlockObjectComp>& nets,
      std::map<frNet*,
               std::vector<std::unique_ptr<drConnFig>>,
               frBlockObjectComp>& netRouteObjs,
      std::map<frNet*,
               std::vector<std::unique_ptr<drConnFig>>,
               frBlockObjectComp>& netExtObjs,
      std::map<frNet*, std::vector<frRect>, frBlockObjectComp>& netOrigGuides);
  void initNets_searchRepair(
      const frDesign* design,
      std::set<frNet*, frBlockObjectComp>& nets,
      std::map<frNet*,
               std::vector<std::unique_ptr<drConnFig>>,
               frBlockObjectComp>& netRouteObjs,
      std::map<frNet*,
               std::vector<std::unique_ptr<drConnFig>>,
               frBlockObjectComp>& netExtObjs,
      std::map<frNet*, std::vector<frRect>, frBlockObjectComp>& netOrigGuides);
  void initNets_searchRepair_pin2epMap(
      const frDesign* design,
      frNet* net,
      std::vector<std::unique_ptr<drConnFig>>& netRouteObjs,
      std::map<frBlockObject*,
               std::set<std::pair<frPoint, frLayerNum>>,
               frBlockObjectComp>& pin2epMap);

  void initNets_searchRepair_pin2epMap_helper(
      const frDesign* design,
      frNet* net,
      const frPoint& bp,
      frLayerNum lNum,
      std::map<frBlockObject*,
               std::set<std::pair<frPoint, frLayerNum>>,
               frBlockObjectComp>& pin2epMap,
      bool isPathSeg);
  void initNets_searchRepair_nodeMap(
      frNet* net,
      std::vector<std::unique_ptr<drConnFig>>& netRouteObjs,
      std::vector<frBlockObject*>& netPins,
      std::map<frBlockObject*,
               std::set<std::pair<frPoint, frLayerNum>>,
               frBlockObjectComp>& pin2epMap,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);

  void initNets_searchRepair_nodeMap_routeObjEnd(
      frNet* net,
      std::vector<std::unique_ptr<drConnFig>>& netRouteObjs,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  void initNets_searchRepair_nodeMap_routeObjSplit(
      frNet* net,
      std::vector<std::unique_ptr<drConnFig>>& netRouteObjs,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  void initNets_searchRepair_nodeMap_routeObjSplit_helper(
      const frPoint& crossPt,
      frCoord trackCoord,
      frCoord splitCoord,
      frLayerNum lNum,
      std::vector<
          std::map<frCoord, std::map<frCoord, std::pair<frCoord, int>>>>&
          mergeHelper,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  void initNets_searchRepair_nodeMap_pin(
      frNet* net,
      std::vector<std::unique_ptr<drConnFig>>& netRouteObjs,
      std::vector<frBlockObject*>& netPins,
      std::map<frBlockObject*,
               std::set<std::pair<frPoint, frLayerNum>>,
               frBlockObjectComp>& pin2epMap,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap);
  void initNets_searchRepair_connComp(
      frNet* net,
      std::map<std::pair<frPoint, frLayerNum>, std::set<int>>& nodeMap,
      std::vector<int>& compIdx);

  void initNet(const frDesign* design,
               frNet* net,
               std::vector<std::unique_ptr<drConnFig>>& routeObjs,
               std::vector<std::unique_ptr<drConnFig>>& extObjs,
               std::vector<frRect>& origGuides,
               std::vector<frBlockObject*>& terms);
  void initNet_term_new(const frDesign* design,
                        drNet* dNet,
                        std::vector<frBlockObject*>& terms);
  void initNet_termGenAp_new(const frDesign* design, drPin* dPin);
  bool isRestrictedRouting(frLayerNum lNum);
  void initNet_addNet(std::unique_ptr<drNet> in);
  void getTrackLocs(bool isHorzTracks,
                    frLayerNum currLayerNum,
                    frCoord low,
                    frCoord high,
                    std::set<frCoord>& trackLocs);
  bool findAPTracks(frLayerNum startLayerNum,
                    frLayerNum endLayerNum,
                    Rectangle& pinRect,
                    std::set<frCoord>& xLocs,
                    std::set<frCoord>& yLocs);
  void initNet_boundary(drNet* net,
                        std::vector<std::unique_ptr<drConnFig>>& extObjs);
  void initNets_regionQuery();
  void initNets_numPinsIn();
  void initNets_boundaryArea();

  void initGridGraph(const frDesign* design);
  void initTrackCoords(
      std::map<frCoord, std::map<frLayerNum, frTrackPattern*>>& xMap,
      std::map<frCoord, std::map<frLayerNum, frTrackPattern*>>& yMap);
  void initTrackCoords_route(
      drNet* net,
      std::map<frCoord, std::map<frLayerNum, frTrackPattern*>>& xMap,
      std::map<frCoord, std::map<frLayerNum, frTrackPattern*>>& yMap);
  void initTrackCoords_pin(
      drNet* net,
      std::map<frCoord, std::map<frLayerNum, frTrackPattern*>>& xMap,
      std::map<frCoord, std::map<frLayerNum, frTrackPattern*>>& yMap);
  void initMazeIdx();
  void initMazeIdx_connFig(drConnFig* connFig);
  void initMazeIdx_ap(drAccessPattern* ap);
  void initMazeCost(const frDesign* design);
  void initMazeCost_connFig();
  void initMazeCost_planarTerm(const frDesign* design);
  void initMazeCost_pin(drNet* net, bool isAddPathCost);
  void initMazeCost_fixedObj(const frDesign* design);
  void initMazeCost_terms(const std::set<frBlockObject*>& objs,
                          bool isAddPathCost,
                          bool isSkipVia = false);
  void initMazeCost_ap();  // disable maze edge
  void initMazeCost_marker_route_queue(const frMarker& marker);
  void initMazeCost_marker_route_queue_addHistoryCost(const frMarker& marker);

  // void initMazeCost_via();
  void initMazeCost_via_helper(drNet* net, bool isAddPathCost);
  void initMazeCost_minCut_helper(drNet* net, bool isAddPathCost);
  void initMazeCost_guide_helper(drNet* net, bool isAdd);
  void initMazeCost_ap_helper(drNet* net, bool isAddPathCost);
  void initMazeCost_ap_planarGrid_helper(const FlexMazeIdx& mi,
                                         const frDirEnum& dir,
                                         frCoord bloatLen,
                                         bool isAddPathCost);
  void initMazeCost_boundary_helper(drNet* net, bool isAddPathCost);

  // DRC
  void initFixedObjs(const frDesign* design);
  void initMarkers(const frDesign* design);

  // route_queue
  void route_queue(FlexGCWorker& gcWorker);
  void route_queue_main(std::queue<RouteQueueEntry>& rerouteQueue);
  void route_queue_resetRipup();
  void route_queue_markerCostDecay();
  void route_queue_addMarkerCost(
      const std::vector<std::unique_ptr<frMarker>>& markers);
  void route_queue_addMarkerCost();
  void route_queue_init_queue(std::queue<RouteQueueEntry>& rerouteQueue);
  void route_queue_update_from_marker(
      frMarker* marker,
      std::set<frBlockObject*>& uniqueVictims,
      std::set<frBlockObject*>& uniqueAggressors,
      std::vector<RouteQueueEntry>& checks,
      std::vector<RouteQueueEntry>& routes);
  void route_queue_update_queue(const std::vector<RouteQueueEntry>& checks,
                                const std::vector<RouteQueueEntry>& routes,
                                std::queue<RouteQueueEntry>& rerouteQueue);
  void route_queue_update_queue(
      const std::vector<std::unique_ptr<frMarker>>& markers,
      std::queue<RouteQueueEntry>& rerouteQueue);
  bool canRipup(drNet* n);
  // route
  void addPathCost(drConnFig* connFig);
  void subPathCost(drConnFig* connFig);
  void modPathCost(drConnFig* connFig, int type);
  // minSpc

  void modMinSpacingCostPlanar(const frBox& box,
                               frMIdx z,
                               int type,
                               bool isBlockage = false,
                               frNonDefaultRule* ndr = nullptr);
  void modCornerToCornerSpacing(const frBox& box, frMIdx z, int type);
  void modMinSpacingCostVia(const frBox& box,
                            frMIdx z,
                            int type,
                            bool isUpperVia,
                            bool isCurrPs,
                            bool isBlockage = false,
                            frNonDefaultRule* ndr = nullptr);

  void modCornerToCornerSpacing_helper(const frBox& box, frMIdx z, int type);

  void modMinSpacingCostVia_eol(const frBox& box,
                                const frBox& tmpBx,
                                int type,
                                bool isUpperVia,
                                frMIdx i,
                                frMIdx j,
                                frMIdx z);
  void modMinSpacingCostVia_eol_helper(const frBox& box,
                                       const frBox& testBox,
                                       int type,
                                       bool isUpperVia,
                                       frMIdx i,
                                       frMIdx j,
                                       frMIdx z);
  // eolSpc
  void modEolSpacingCost_helper(const frBox& testbox,
                                frMIdx z,
                                int type,
                                int eolType);
  void modEolSpacingRulesCost(const frBox& box,
                              frMIdx z,
                              int type,
                              bool isSkipVia = false);
  void modEolSpacingCost(const frBox& box,
                         frMIdx z,
                         int type,
                         frConstraint* con,
                         bool isSkipVia = false);
  // cutSpc
  void modCutSpacingCost(const frBox& box,
                         frMIdx z,
                         int type,
                         bool isBlockage = false);
  void modInterLayerCutSpacingCost(const frBox& box,
                                   frMIdx z,
                                   int type,
                                   bool isUpperVia,
                                   bool isBlockage = false);
  void modLef58InterLayerCutSpacingCost(const frBox& box,
                                        frMIdx z,
                                        int type,
                                        bool isUpperVia,
                                        bool isBlockage = false);
  // adjCut
  void modAdjCutSpacingCost_fixedObj(const frDesign* design,
                                     const frBox& box,
                                     frVia* origVia);
  void modMinimumcutCostVia(const frBox& box,
                            frMIdx z,
                            int type,
                            bool isUpperVia);
  void modViaForbiddenThrough(const FlexMazeIdx& bi,
                              const FlexMazeIdx& ei,
                              int type);
  void modBlockedPlanar(const frBox& box, frMIdx z, bool setBlock);
  void modBlockedVia(const frBox& box, frMIdx z, bool setBlock);

  bool mazeIterInit_sortRerouteNets(int mazeIter,
                                    std::vector<drNet*>& rerouteNets);

  void mazeNetInit(drNet* net);
  void mazeNetEnd(drNet* net);
  bool routeNet(drNet* net);
  void routeNet_prep(drNet* net,
                     std::set<drPin*, frBlockObjectComp>& pins,
                     std::map<FlexMazeIdx, std::set<drPin*, frBlockObjectComp>>&
                         mazeIdx2unConnPins,
                     std::set<FlexMazeIdx>& apMazeIdx,
                     std::set<FlexMazeIdx>& realPinAPMazeIdx,
                     std::map<FlexMazeIdx, frBox3D*>& mazeIdx2TaperBox,
                     list<pair<drPin*, frBox3D>>& pinTaperBoxes);
  void routeNet_prepAreaMap(drNet* net,
                            std::map<FlexMazeIdx, frCoord>& areaMap);
  void routeNet_setSrc(
      std::set<drPin*, frBlockObjectComp>& unConnPins,
      std::map<FlexMazeIdx, std::set<drPin*, frBlockObjectComp>>&
          mazeIdx2unConnPins,
      std::vector<FlexMazeIdx>& connComps,
      FlexMazeIdx& ccMazeIdx1,
      FlexMazeIdx& ccMazeIdx2,
      frPoint& centerPt);
  void mazePinInit();
  drPin* routeNet_getNextDst(
      FlexMazeIdx& ccMazeIdx1,
      FlexMazeIdx& ccMazeIdx2,
      std::map<FlexMazeIdx, std::set<drPin*, frBlockObjectComp>>&
          mazeIdx2unConnPins,
      list<pair<drPin*, frBox3D>>& pinTaperBoxes);
  void routeNet_postAstarUpdate(
      std::vector<FlexMazeIdx>& path,
      std::vector<FlexMazeIdx>& connComps,
      std::set<drPin*, frBlockObjectComp>& unConnPins,
      std::map<FlexMazeIdx, std::set<drPin*, frBlockObjectComp>>&
          mazeIdx2unConnPins,
      bool isFirstConn);
  void routeNet_postAstarWritePath(
      drNet* net,
      std::vector<FlexMazeIdx>& points,
      const std::set<FlexMazeIdx>& realPinApMazeIdx,
      std::map<FlexMazeIdx, frBox3D*>& mazeIdx2Taperbox,
      const set<FlexMazeIdx>& apMazeIdx);
  void setNDRStyle(drNet* net,
                   frSegStyle& currStyle,
                   frMIdx startX,
                   frMIdx endX,
                   frMIdx startY,
                   frMIdx endY,
                   frMIdx z,
                   FlexMazeIdx* prev,
                   FlexMazeIdx* next);
  bool isInsideTaperBox(frMIdx x,
                        frMIdx y,
                        frMIdx startZ,
                        frMIdx endZ,
                        map<FlexMazeIdx, frBox3D*>& mazeIdx2TaperBox);
  bool splitPathSeg(frMIdx& midX,
                    frMIdx& midY,
                    bool& taperFirstPiece,
                    frMIdx startX,
                    frMIdx startY,
                    frMIdx endX,
                    frMIdx endY,
                    frMIdx z,
                    frBox3D* srcBox,
                    frBox3D* dstBox,
                    drNet* net);
  void processPathSeg(frMIdx startX,
                      frMIdx startY,
                      frMIdx endX,
                      frMIdx endY,
                      frMIdx z,
                      const set<FlexMazeIdx>& realApMazeIdx,
                      drNet* net,
                      bool vertical,
                      bool taper,
                      int i,
                      vector<FlexMazeIdx>& points,
                      const set<FlexMazeIdx>& apMazeIdx);
  void checkPathSegStyle(drPathSeg* ps, bool isBegin, frSegStyle& style);
  void checkViaConnectivityToAP(drVia* ps, bool isBottom, frNet* net);
  bool hasAccessPoint(const frPoint& pt, frLayerNum lNum, frNet* net);
  void routeNet_postAstarPatchMinAreaVio(
      drNet* net,
      const std::vector<FlexMazeIdx>& path,
      const std::map<FlexMazeIdx, frCoord>& areaMap);
  frCoord getHalfViaEncArea(frMIdx z, bool isLayer1, drNet* net);
  void routeNet_postAstarAddPatchMetal(drNet* net,
                                       const FlexMazeIdx& bpIdx,
                                       const FlexMazeIdx& epIdx,
                                       frCoord gapArea,
                                       frCoord patchWidth,
                                       bool bpPatchStyle = true,
                                       bool epPatchStyle = false);
  int routeNet_postAstarAddPathMetal_isClean(const FlexMazeIdx& bpIdx,
                                             bool isPatchHorz,
                                             bool isPatchLeft,
                                             frCoord patchLength);
  void routeNet_postAstarAddPatchMetal_addPWire(drNet* net,
                                                const FlexMazeIdx& bpIdx,
                                                bool isPatchHorz,
                                                bool isPatchLeft,
                                                frCoord patchLength,
                                                frCoord patchWidth);
  void routeNet_postRouteAddPathCost(drNet* net);
  void routeNet_postRouteAddPatchMetalCost(drNet* net);

  // end
  void cleanup();
  void end(frDesign* design);
  void endGetModNets(std::set<frNet*, frBlockObjectComp>& modNets);
  void endRemoveNets(frDesign* design,
                     std::set<frNet*, frBlockObjectComp>& modNets,
                     std::map<frNet*,
                              std::set<std::pair<frPoint, frLayerNum>>,
                              frBlockObjectComp>& boundPts);
  void endRemoveNets_pathSeg(
      frDesign* design,
      frPathSeg* pathSeg,
      std::set<std::pair<frPoint, frLayerNum>>& boundPts);
  void endRemoveNets_via(frDesign* design, frVia* via);
  void endRemoveNets_patchWire(frDesign* design, frPatchWire* pwire);
  void endAddNets(frDesign* design,
                  std::map<frNet*,
                           std::set<std::pair<frPoint, frLayerNum>>,
                           frBlockObjectComp>& boundPts);
  void endAddNets_pathSeg(frDesign* design, drPathSeg* pathSeg);
  void endAddNets_via(frDesign* design, drVia* via);
  void endAddNets_patchWire(frDesign* design, drPatchWire* pwire);
  void endAddNets_merge(frDesign* design,
                        frNet* net,
                        std::set<std::pair<frPoint, frLayerNum>>& boundPts);

  void endRemoveMarkers(frDesign* design);
  void endAddMarkers(frDesign* design);

  friend class FlexDR;
  friend class FlexGC;

 private:
  // on-the-fly access points that require adding access edges in the grid graph
  vector<Point3D> specialAccessAPs;
};

}  // namespace fr

#endif
