//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// G4VHitsCollection
//
// Class description:
//
// This is the base class of hits collection. The user is advised to
// use G4THitsCollection template class in case his/her collection is
// transient. While, in case the collection is persistent,
// the concrete collection class can be directly derived from this
// class. Geant4 kernel will use this class methods.
//
// Author: Makoto Asai
// --------------------------------------------------------------------
#ifndef G4VHitsCollection_h
#define G4VHitsCollection_h 1

#include "globals.hh"

class G4VHit;

class G4VHitsCollection
{
 public:

  G4VHitsCollection() = default;
  G4VHitsCollection(const G4String& detName, const G4String& colNam);
  virtual ~G4VHitsCollection() = default;
  G4bool operator==(const G4VHitsCollection& right) const;

  virtual void DrawAllHits(){};
  virtual void PrintAllHits(){};

  inline const G4String& GetName() const { return collectionName; }
  inline const G4String& GetSDname() const { return SDname; }
  inline void SetColID(G4int i) { colID = i; }
  inline G4int GetColID() const { return colID; }

  // GetHit and GetSize are given a default implementation here so
  // that the template G4THitsCollection can be used, but they
  // are re-implemented G4THitsCollection.
  virtual G4VHit* GetHit(std::size_t) const { return nullptr; }
  virtual std::size_t GetSize() const { return 0; };

 protected:

  // Collection name
  G4String collectionName = "Unknown";
  G4String SDname = "Unknown";
  // Collection ID
  G4int colID = -1;
};

#endif
