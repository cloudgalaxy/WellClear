/*
 * Copyright (c) 2015 United States Government as represented by
 * the National Aeronautics and Space Administration.  No copyright
 * is claimed in the United States under Title 17, U.S.Code. All Other
 * Rights Reserved.
 */
#include "WCV_tvar.h"
#include "Vect3.h"
#include "Velocity.h"
#include "WCV_Vertical.h"
#include "Horizontal.h"
#include "WCVTable.h"
#include "ParameterData.h"
#include "ConflictData.h"
#include "LossData.h"
#include "format.h"
#include "string_util.h"
#include <cfloat>

namespace larcfm {

bool WCV_tvar::pvsCheck = false;

WCVTable WCV_tvar::getWCVTable() {
  return table;
}

void WCV_tvar::setWCVTable(const WCVTable& tab) {
  table.copyValues(tab);
}

double WCV_tvar::getDTHR() const {
  return table.getDTHR();
}
double WCV_tvar::getDTHR(const std::string& u) const {
  return table.getDTHR(u);
}

double WCV_tvar::getZTHR() const {
  return table.getZTHR();
}
double WCV_tvar::getZTHR(const std::string& u) const {
  return table.getZTHR(u);
}

double WCV_tvar::getTTHR() const {
  return table.getTTHR();
}
double WCV_tvar::getTTHR(const std::string& u) const {
  return table.getTTHR(u);
}

double WCV_tvar::getTCOA() const {
  return table.getTCOA();
}
double WCV_tvar::getTCOA(const std::string& u) const {
  return table.getTCOA(u);
}

void WCV_tvar::setDTHR(double val) {
  table.setDTHR(val);
}
void WCV_tvar::setDTHR(double val, const std::string& u) {
  table.setDTHR(val, u);
}

void WCV_tvar::setZTHR(double val) {
  table.setZTHR(val);
}
void WCV_tvar::setZTHR(double val, const std::string& u) {
  table.setZTHR(val,u);
}

void WCV_tvar::setTTHR(double val) {
  table.setTTHR(val);
}
void WCV_tvar::setTTHR(double val, const std::string& u) {
  table.setTTHR(val,u);
}

void WCV_tvar::setTCOA(double val) {
  table.setTCOA(val);
}
void WCV_tvar::setTCOA(double val, const std::string& u) {
  table.setTCOA(val,u);
}

bool WCV_tvar::horizontal_WCV(const Vect2& s, const Vect2& v) const {
  if (s.norm() <= table.getDTHR()) return true;
  if (Horizontal::dcpa(s,v) <= table.getDTHR()) {
    double tvar = horizontal_tvar(s,v);
    return 0  <= tvar && tvar <= table.getTTHR();
  }
  return false;
}

bool WCV_tvar::violation(const Vect3& so, const Velocity& vo, const Vect3& si, const Velocity& vi) const {
  Vect2 so2 = so.vect2();
  Vect2 si2 = si.vect2();
  Vect2 s2 = so2.Sub(si2);
  Vect2 vo2 = vo.vect2();
  Vect2 vi2 = vi.vect2();
  Vect2 v2 = vo2.Sub(vi2);
  return horizontal_WCV(s2,v2) &&
      WCV_Vertical::vertical_WCV(table.getZTHR(),table.getTCOA(),so.z-si.z,vo.z-vi.z);
}

bool WCV_tvar::conflict(const Vect3& so, const Velocity& vo, const Vect3& si, const Velocity& vi, double B, double T) const {
  return WCV3D(so,vo,si,vi,B,T).conflict();
}


ConflictData WCV_tvar::conflictDetection(const Vect3& so, const Velocity& vo, const Vect3& si, const Velocity& vi, double B, double T) const {
  LossData ret = WCV3D(so,vo,si,vi,B,T);
  double t_tca = (ret.time_in + ret.time_out)/2;
  double dist_tca = so.linear(vo, t_tca).Sub(si.linear(vi, t_tca)).cyl_norm(table.getDTHR(),table.getZTHR());
  return ConflictData(ret, t_tca,dist_tca);
}

LossData WCV_tvar::WCV3D(const Vect3& so, const Velocity& vo, const Vect3& si, const Velocity& vi, double B, double T) const {
  return WCV_interval(so,vo,si,vi,B,T);
}

LossData WCV_tvar::WCV_interval(const Vect3& so, const Velocity& vo, const Vect3& si, const Velocity& vi, double B, double T) const {
  if (T <= B) {
    T = DBL_MAX;
  }
  double time_in = T;
  double time_out = B;

  print_PVS_input(so,vo,si,vi,B,T);

  Vect2 so2 = so.vect2();
  Vect2 si2 = si.vect2();
  Vect2 s2 = so2.Sub(si2);
  Vect2 vo2 = vo.vect2();
  Vect2 vi2 = vi.vect2();
  Vect2 v2 = vo2.Sub(vi2);
  double sz = so.z-si.z;
  double vz = vo.z-vi.z;

  WCV_Vertical wcvz;
  wcvz.vertical_WCV_interval(table.getZTHR(),table.getTCOA(),B,T,sz,vz);

  if (wcvz.time_in > wcvz.time_out) {
    print_PVS_output(time_in, time_out, "case 1");
    return LossData(time_in,time_out);
  }
  Vect2 step = v2.ScalAdd(wcvz.time_in,s2);
  if (Util::almost_equals(wcvz.time_in,wcvz.time_out)) { // [CAM] Changed from == to almost_equals to mitigate numerical problems
    print_PVS_output(time_in, time_out, "case 2");
    if (horizontal_WCV(step,v2)) {
      time_in = wcvz.time_in;
      time_out = wcvz.time_out;
    }
    return LossData(time_in,time_out);
  }
  LossData ld = horizontal_WCV_interval(wcvz.time_out-wcvz.time_in,step,v2);
  time_in = ld.time_in + wcvz.time_in;
  time_out = ld.time_out + wcvz.time_in;
  print_PVS_output(time_in, time_out, "case 3");
  return LossData(time_in,time_out);
}

void WCV_tvar::print_PVS_input(const Vect3& so, const Velocity& vo, const Vect3& si, const Velocity& vi, double B, double T) const {
  //  Thread.dumpStack();
  if (pvsCheck) {
    fpln ("(# DTHR := "+Fm8(table.getDTHR())+", ZTHR := "+Fm8(table.getZTHR())+", TTHR := "+Fm8(table.getTTHR())+", TCOA := "+Fm8(table.getTCOA())
        +", B := "+Fm8(B)+", T := "+Fm8(T)+" #)");
    fpln("(# x := "+Fm8(so.x)+", y := "+Fm8(so.y)+", z := "+Fm8(so.z)+" #) % so");
    fpln("(# x := "+Fm8(vo.x)+", y := "+Fm8(vo.y)+", z := "+Fm8(vo.z)+" #) % vo");
    fpln("(# x := "+Fm8(si.x)+", y := "+Fm8(si.y)+", z := "+Fm8(si.z)+" #) % si");
    fpln("(# x := "+Fm8(vi.x)+", y := "+Fm8(vi.y)+", z := "+Fm8(vi.z)+" #) % vi");
  }
}

void WCV_tvar::print_PVS_output(double time_in, double time_out, const std::string& comment) const {
  if (pvsCheck) {
    fpln("("+Fm8(time_in)+","+Fm8(time_out)+") % "+getSimpleClassName()+" "+id+" time in/out "+comment);
  }
}

std::string WCV_tvar::toString() const {
  return (id == "" ? "" : id+" = ")+getSimpleClassName()+": {"+table.toString()+"}";
}

ParameterData WCV_tvar::getParameters() const {
  ParameterData p;
  updateParameterData(p);
  return p;
}

void WCV_tvar::updateParameterData(ParameterData& p) const {
  table.updateParameterData(p);
  p.set("id",id);
}

void WCV_tvar::setParameters(const ParameterData& p) {
  table.setParameters(p);
  if (p.contains("id")) {
    id = p.getString("id");
  }
}

std::string WCV_tvar::getIdentifier() const {
  return id;
}

void WCV_tvar::setIdentifier(const std::string& s) {
  id = s;
}

bool WCV_tvar::equals(Detection3D *obj) const {
  if (!larcfm::equals(getCanonicalClassName(), obj->getCanonicalClassName())) return false;
  if (!table.equals(((WCV_tvar*)obj)->table)) return false;
  if (!larcfm::equals(id, ((WCV_tvar*)obj)->id)) return false;
  return true;
}

}

