/*
 * Copyright (c) 2014-2015 United States Government as represented by
 * the National Aeronautics and Space Administration.  No copyright
 * is claimed in the United States under Title 17, U.S.Code. All Other
 * Rights Reserved.
 */
/*
 * LossData.h
 *
 * [CAM] (time_in,time_out) is the time interval of loss of separation. Every point in the open interval represents
 * a time where the aircraft are in violation. Whether or not the bounds of the interval are violation points or not
 * depends on the detector, e.g., for CD3D the points time_in and time_out are not necessarily violations points,
 * for WCV_tvar the points time_in and time_out violation times. Furthermore, time_in and time_out are always
 * between the lookahead time interval [B,T], where 0 <= B < T. It is always the case that if time_in < time_out
 * is true, then there is a conflict.
 */

#ifndef SRC_LOSSDATA_H_
#define SRC_LOSSDATA_H_

#include <string>
namespace larcfm {

class LossData {
public:
//  LossData();

  double  time_in;                       // relative time to loss of separation
  double  time_out;                      // relative time to the exit from loss of separation

  virtual std::string toString() const ;

  LossData(double time_in, double time_out);

  LossData();

  bool conflict() const;

  bool conflict(double thr) const;

  double getTimeIn() const;

  double getTimeOut() const;

  virtual ~LossData() {};

};

} /* namespace larcfm */

#endif /* SRC_LOSSDATA_H_ */
