// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef MEAN_VERTEX_CALIBRATOR_H_
#define MEAN_VERTEX_CALIBRATOR_H_

#include "DetectorsCalibration/TimeSlotCalibration.h"
#include "DetectorsCalibration/TimeSlot.h"
#include "DetectorsCalibration/MeanVertexData.h"
#include "DataFormatsCalibration/MeanVertexObject.h"
#include "CCDB/CcdbObjectInfo.h"
#include "MathUtils/detail/Bracket.h"
#include <array>
#include <deque>

namespace o2
{
namespace calibration
{

class MeanVertexCalibrator final : public o2::calibration::TimeSlotCalibration<o2::dataformats::PrimaryVertex, o2::calibration::MeanVertexData>
{
  using PVertex = o2::dataformats::PrimaryVertex;
  using MeanVertexData = o2::calibration::MeanVertexData;
  using Slot = o2::calibration::TimeSlot<MeanVertexData>;
  using MVObject = o2::dataformats::MeanVertexObject;
  using MVObjectVector = std::vector<MVObject>;
  using CcdbObjectInfo = o2::ccdb::CcdbObjectInfo;
  using CcdbObjectInfoVector = std::vector<CcdbObjectInfo>;

 public:
  struct HistoParams {
    int nBins = 0.;
    float binWidth = 0.;
    float minRange = 0.;
    float maxRange = 0.;
  };

  MeanVertexCalibrator() = default;
  ~MeanVertexCalibrator() final = default;

  bool hasEnoughData(const Slot& slot) const final;
  void initOutput() final;
  void finalizeSlot(Slot& slot) final;
  Slot& emplaceNewSlot(bool front, TFType tstart, TFType tend) final;

  void doSimpleMovingAverage(std::deque<float>& dq, float& sma);
  void doSimpleMovingAverage(std::deque<MVObject>& dq, MVObject& sma);

  const MVObjectVector& getMeanVertexObjectVector() const { return mMeanVertexVector; }
  const CcdbObjectInfoVector& getMeanVertexObjectInfoVector() const { return mInfoVector; }
  CcdbObjectInfoVector& getMeanVertexObjectInfoVector() { return mInfoVector; }

  void useVerboseMode(bool flag) { mVerbose = flag; }
  bool getVerboseMode() const { return mVerbose; }

  bool fitMeanVertex(o2::calibration::MeanVertexData* c, o2::dataformats::MeanVertexObject& mvo);
  void fitMeanVertexCoord(int icoord, const float* array, const HistoParams& hpar, o2::dataformats::MeanVertexObject& mvo);
  HistoParams binVector(std::vector<float>& vectOut, const std::vector<float>& vectIn, o2::calibration::MeanVertexData* c, int dim);
  void printVector(const std::vector<float>& vect, const HistoParams& hpar);
  void printVector(const float* vect, const HistoParams& hpar);

 private:
  CcdbObjectInfoVector mInfoVector;                                    // vector of CCDB Infos , each element is filled with the CCDB description
                                                                       // of the accompanying LHCPhase
  MVObjectVector mMeanVertexVector;                                    // vector of Mean Vertex Objects, each element is filled in "process"
                                                                       // when we finalize one slot (multiple can be finalized during the same
                                                                       // "process", which is why we have a vector. Each element is to be considered
                                                                       // the output of the device, and will go to the CCDB. It is the simple
                                                                       // moving average
  std::deque<MVObject> mTmpMVobjDq;                                    // This is the deque of MeanVertex objecs that will be used for the
                                                                       // simple moving average
  MVObject mSMAMVobj;                                                  // object containing the Simple Moving Average to be put to CCDB
  std::deque<long> mTmpMVobjDqTimeStart;                               // This is the deque of MeanVertex objecs that will be used for the
                                                                       // simple moving average, start time of used TFs
  std::deque<o2::math_utils::detail::Bracket<long>> mTmpMVobjDqTime;   // This is the deque for the start and end time of the
                                                                       // slots used for the SMA
  bool mVerbose = false;                                               // Whether to log in verbose mode

  ClassDefOverride(MeanVertexCalibrator, 1);
};

} // end namespace calibration
} // end namespace o2

#endif /* TOF_LHCPHASE_CALIBRATION_H_ */
