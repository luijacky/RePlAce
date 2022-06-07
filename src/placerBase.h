#ifndef __PLACER_BASE__
#define __PLACER_BASE__

#include <vector>
#include <unordered_map>
#include <memory>

namespace odb {
class dbDatabase;

class dbInst;
class dbITerm;
class dbBTerm;
class dbNet;

class dbPlacementStatus;
class dbSigType;

class dbBox;

class Rect;
}

namespace replace {

class Pin;
class Net;
class GCell;

class Logger;

class Instance {
public:
  Instance();
  Instance(odb::dbInst* inst);
  Instance(int lx, int ly, int ux, int uy);
  ~Instance();

  odb::dbInst* dbInst() const { return inst_; }

  // a cell that no need to be moved.
  bool isFixed() const;

  // a instance that need to be moved.
  bool isInstance() const;

  bool isPlaceInstance() const;

  // Dummy is virtual instance to fill in
  // empty fragmented row structures.
  // will have inst_ as nullptr
  bool isDummy() const;

  void setLocation(int x, int y);
  void setCenterLocation(int x, int y);

  void dbSetPlaced();
  void dbSetPlacementStatus(odb::dbPlacementStatus ps);
  void dbSetLocation();
  void dbSetLocation(int x, int y);
  void dbSetCenterLocation(int x, int y);

  int lx() const;
  int ly() const;
  int ux() const;
  int uy() const;
  int cx() const;
  int cy() const;
  int dx() const;
  int dy() const;

  void setExtId(int extId);
  int extId() const { return extId_; }

  void addPin(Pin* pin);
  const std::vector<Pin*> & pins() const { return pins_; }

private:
  odb::dbInst* inst_;
  std::vector<Pin*> pins_;
  int lx_;
  int ly_;
  int ux_;
  int uy_;
  int extId_;
};

class Pin {
public:
  Pin();
  Pin(odb::dbITerm* iTerm);
  Pin(odb::dbBTerm* bTerm);
  ~Pin();

  odb::dbITerm* dbITerm() const;
  odb::dbBTerm* dbBTerm() const;

  bool isITerm() const;
  bool isBTerm() const;
  bool isMinPinX() const;
  bool isMaxPinX() const;
  bool isMinPinY() const;
  bool isMaxPinY() const;

  void setITerm();
  void setBTerm();
  void setMinPinX();
  void setMinPinY();
  void setMaxPinX();
  void setMaxPinY();
  void unsetMinPinX();
  void unsetMinPinY();
  void unsetMaxPinX();
  void unsetMaxPinY();

  int cx() const;
  int cy() const;

  int offsetCx() const;
  int offsetCy() const;

  void updateLocation(const Instance* inst);

  void setInstance(Instance* inst);
  void setNet(Net* net);

  bool isPlaceInstConnected() const;

  Instance* instance() const { return inst_; }
  Net* net() const { return net_; }

private:
  void* term_;
  Instance* inst_;
  Net* net_;

  // pin center coordinate is enough
  // Pins' placed location.
  int cx_;
  int cy_;

  // offset coordinates inside instance.
  // origin point is center point of instance.
  // (e.g. (DX/2,DY/2) )
  // This will increase efficiency for bloating
  int offsetCx_;
  int offsetCy_;

  unsigned char iTermField_:1;
  unsigned char bTermField_:1;
  unsigned char minPinXField_:1;
  unsigned char minPinYField_:1;
  unsigned char maxPinXField_:1;
  unsigned char maxPinYField_:1;

  void updateCoordi(odb::dbITerm* iTerm);
  void updateCoordi(odb::dbBTerm* bTerm);
};

class Net {
public:
  Net();
  Net(odb::dbNet* net);
  ~Net();

  int lx() const;
  int ly() const;
  int ux() const;
  int uy() const;
  int cx() const;
  int cy() const;

  // HPWL: half-parameter-wire-length
  int64_t hpwl() const;

  void updateBox();

  const std::vector<Pin*> & pins() const { return pins_; }

  odb::dbNet* dbNet() const { return net_; }
  odb::dbSigType getSigType() const;

  void addPin(Pin* pin);

private:
  odb::dbNet* net_;
  std::vector<Pin*> pins_;
  int lx_;
  int ly_;
  int ux_;
  int uy_;
};

class Die {
public:
  Die();
  Die(odb::dbBox* dieBox, odb::Rect* coreRect);
  ~Die();

  void setDieBox(odb::dbBox* dieBox);
  void setCoreBox(odb::Rect* coreBox);

  int dieLx() const { return dieLx_; }
  int dieLy() const { return dieLy_; }
  int dieUx() const { return dieUx_; }
  int dieUy() const { return dieUy_; }

  int coreLx() const { return coreLx_; }
  int coreLy() const { return coreLy_; }
  int coreUx() const { return coreUx_; }
  int coreUy() const { return coreUy_; }

  int dieCx() const;
  int dieCy() const;
  int dieDx() const;
  int dieDy() const;
  int coreCx() const;
  int coreCy() const;
  int coreDx() const;
  int coreDy() const;

private:
  int dieLx_;
  int dieLy_;
  int dieUx_;
  int dieUy_;
  int coreLx_;
  int coreLy_;
  int coreUx_;
  int coreUy_;
};

class PlacerBase {
public:
  PlacerBase();
  PlacerBase(odb::dbDatabase* db, std::shared_ptr<Logger> log);
  ~PlacerBase();

  const std::vector<Instance*>& insts() const { return insts_; }
  const std::vector<Pin*>& pins() const { return pins_; }
  const std::vector<Net*>& nets() const { return nets_; }

  //
  // placeInsts : a real instance that need to be placed
  // fixedInsts : a real instance that is fixed (e.g. macros, tapcells)
  // dummyInsts : a fake instance that is for fragmented-row handling
  //
  // nonPlaceInsts : fixedInsts + dummyInsts to enable fast-iterate on Bin-init
  //
  const std::vector<Instance*>& placeInsts() const { return placeInsts_; }
  const std::vector<Instance*>& fixedInsts() const { return fixedInsts_; }
  const std::vector<Instance*>& dummyInsts() const { return dummyInsts_; }
  const std::vector<Instance*>& nonPlaceInsts() const { return nonPlaceInsts_; }

  Die& die() { return die_; }

  Instance* dbToPlace(odb::dbInst* inst) const;
  Pin* dbToPlace(odb::dbITerm* pin) const;
  Pin* dbToPlace(odb::dbBTerm* pin) const;
  Net* dbToPlace(odb::dbNet* net) const;

  int siteSizeX() const { return siteSizeX_; }
  int siteSizeY() const { return siteSizeY_; }

  int64_t hpwl() const;
  void printInfo() const;

  int64_t placeInstsArea() const { return placeInstsArea_; }
  int64_t nonPlaceInstsArea() const { return nonPlaceInstsArea_; }
  int64_t macroInstsArea() const { return macroInstsArea_; }
  int64_t stdInstsArea() const { return stdInstsArea_; }

private:
  odb::dbDatabase* db_;
  std::shared_ptr<Logger> log_;

  Die die_;

  std::vector<Instance> instStor_;
  std::vector<Pin> pinStor_;
  std::vector<Net> netStor_;

  std::vector<Instance*> insts_;
  std::vector<Pin*> pins_;
  std::vector<Net*> nets_;

  std::unordered_map<odb::dbInst*, Instance*> instMap_;
  std::unordered_map<void*, Pin*> pinMap_;
  std::unordered_map<odb::dbNet*, Net*> netMap_;

  std::vector<Instance*> placeInsts_;
  std::vector<Instance*> fixedInsts_;
  std::vector<Instance*> dummyInsts_;
  std::vector<Instance*> nonPlaceInsts_;

  int siteSizeX_;
  int siteSizeY_;

  int64_t placeInstsArea_;
  int64_t nonPlaceInstsArea_;
  
  // macroInstsArea_ + stdInstsArea_ = placeInstsArea_;
  // macroInstsArea_ should be separated
  // because of target_density tuning
  int64_t macroInstsArea_;
  int64_t stdInstsArea_;

  void init();
  void initInstsForFragmentedRow();

  void reset();
};

}

#endif
