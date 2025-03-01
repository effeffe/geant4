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
////////////////////////////////////////////////////////////////////////////////
//  Class:   G4IonInverseIonisation
//  Author:         L. Desorgher
//  Organisation:   SpaceIT GmbH
//
//  Adjoint/reverse discrete ionisation for ions
////////////////////////////////////////////////////////////////////////////////

#ifndef G4IonInverseIonisation_h
#define G4IonInverseIonisation_h 1

#include "globals.hh"
#include "G4VAdjointReverseReaction.hh"

class G4AdjointIonIonisationModel;

class G4IonInverseIonisation : public G4VAdjointReverseReaction
{
 public:
  explicit G4IonInverseIonisation(G4bool whichScatCase, const G4String& process_name,
                                  G4AdjointIonIonisationModel* aEmAdjointModel);
  ~G4IonInverseIonisation() override = default;

  void ProcessDescription(std::ostream&) const override;
  void DumpInfo() const override { ProcessDescription(G4cout); }

  G4IonInverseIonisation(G4IonInverseIonisation&) = delete;
  G4IonInverseIonisation& operator=(const G4IonInverseIonisation& right) = delete;
};

#endif
