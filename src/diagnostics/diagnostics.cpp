// C/C++ headers
#include <cstring>
#include <sstream>
#include <stdexcept>

// Athena++ headers
#include "diagnostics.hpp"
#include "../mesh/mesh.hpp"
#include "../coordinates/coordinates.hpp"

Diagnostics::Diagnostics(MeshBlock *pmb, ParameterInput *pin):
  myname("HEAD"), type("NONE"), grid("NONE"), prev(NULL), next(NULL), 
  ncycle(0), pmy_block_(pmb)
{
  std::stringstream msg;
  char cstr[80];
  std::string varnames = pin->GetOrAddString("problem", "diagnostics", "");
  std::strcpy(cstr, varnames.c_str());
  char *p = std::strtok(cstr, " ,");
  while (p != NULL) {
    std::string name(p);
    if (name == "div")  // 1.
      AddDiagnostics(Divergence(pmb));
    else if (name == "curl")  // 2.
      AddDiagnostics(Curl(pmb));
    else if (name == "mean")  // 3.
      AddDiagnostics(HydroMean(pmb));
    else if (name == "tempa") // 4.
      AddDiagnostics(TemperatureAnomaly(pmb));
    else if (name == "presa") // 5.
      AddDiagnostics(PressureAnomaly(pmb));
    else if (name == "eddyflux") // 6.
      AddDiagnostics(EddyFlux(pmb));
    else if (name == "flux") // 7.
      AddDiagnostics(TotalFlux(pmb));
    else if (name == "div_h") // 8.
      AddDiagnostics(HorizontalDivergence(pmb));
    else if (name == "b") // 9.
      AddDiagnostics(Buoyancy(pmb));
    else {
      msg << "### FATAL ERROR in function Diagnostics::Diagnostics"
          << std::endl << "Diagnostic variable " << name << " not defined";
      throw std::runtime_error(msg.str().c_str());
    }
    p = std::strtok(NULL, " ,");
  }

  if (NGHOST < 2) {
    msg << "### FATAL ERROR in function Diagnostics::Diagnostics"
        << std::endl << "Most diagnostic variables require at least 2 ghost cells";
    throw std::runtime_error(msg.str().c_str());
  }
}

Diagnostics::Diagnostics(MeshBlock *pmb, std::string name):
  myname(name), prev(NULL), next(NULL), ncycle(0), pmy_block_(pmb)
{
  ncells1_ = pmb->block_size.nx1 + 2*(NGHOST);
  ncells2_ = 1; 
  ncells3_ = 1;
  if (pmb->block_size.nx2 > 1) ncells2_ = pmb->block_size.nx2 + 2*(NGHOST);
  if (pmb->block_size.nx3 > 1) ncells3_ = pmb->block_size.nx3 + 2*(NGHOST);

  x1edge_.NewAthenaArray(ncells1_+1);
  x1edge_p1_.NewAthenaArray(ncells1_);
  x2edge_.NewAthenaArray(ncells1_+1);
  x2edge_p1_.NewAthenaArray(ncells1_);
  x3edge_.NewAthenaArray(ncells1_+1);
  x3edge_p1_.NewAthenaArray(ncells1_);

  x1area_.NewAthenaArray(ncells1_+1);
  x2area_.NewAthenaArray(ncells1_);
  x2area_p1_.NewAthenaArray(ncells1_);
  x3area_.NewAthenaArray(ncells1_);
  x3area_p1_.NewAthenaArray(ncells1_);

  vol_.NewAthenaArray(ncells1_);
}

Diagnostics::~Diagnostics() {
  if (prev != NULL) prev->next = next;
  if (next != NULL) next->prev = prev;

  x1edge_.DeleteAthenaArray();
  x1edge_p1_.DeleteAthenaArray();
  x2edge_.DeleteAthenaArray();
  x2edge_p1_.DeleteAthenaArray();
  x3edge_.DeleteAthenaArray();
  x3edge_p1_.DeleteAthenaArray();

  x1area_.DeleteAthenaArray();
  x2area_.DeleteAthenaArray();
  x2area_p1_.DeleteAthenaArray();
  x3area_.DeleteAthenaArray();
  x3area_p1_.DeleteAthenaArray();

  vol_.DeleteAthenaArray();
}

Diagnostics* Diagnostics::operator[](std::string name)
{
  Diagnostics *p = this;
  while (p != NULL) {
    if (p->myname == name)
      return p;
    p = p->next;
  }
  return p;
}
