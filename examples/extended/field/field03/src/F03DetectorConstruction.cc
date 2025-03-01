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
/// \file field/field03/src/F03DetectorConstruction.cc
/// \brief Implementation of the F03DetectorConstruction class
//
//
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "F03DetectorConstruction.hh"

#include "F03CalorimeterSD.hh"
#include "F03DetectorMessenger.hh"

#include "G4AutoDelete.hh"
#include "G4GeometryManager.hh"
#include "G4FieldBuilder.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalConstants.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SolidStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4UniformMagField.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

F03DetectorConstruction::F03DetectorConstruction()
{
  fDetectorMessenger = new F03DetectorMessenger(this);

  // create field builder
  // this will create commands for field parameters 
  G4FieldBuilder* fieldBuilder = G4FieldBuilder::Instance();
  // fieldBuilder->SetVerboseLevel(2);

  auto globalFieldParameters = fieldBuilder->GetFieldParameters();
  auto localFieldParameters = fieldBuilder->CreateFieldParameters("Radiator");

  // set default min step 0.25 mm
  globalFieldParameters->SetMinimumStep(0.25 * mm);
  localFieldParameters->SetMinimumStep(0.25 * mm);

  // create materials
  DefineMaterials();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

F03DetectorConstruction::~F03DetectorConstruction()
{
  delete fDetectorMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* F03DetectorConstruction::Construct()
{
  return ConstructCalorimeter();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::DefineMaterials()
{
  // This function illustrates the possible ways to define materials

  G4String name, symbol;  // a=mass of a mole;
  G4double a, z, density;  // z=mean number of protons;
  G4int nel;
  G4int ncomponents;
  G4double fractionmass, pressure, temperature;

  //
  // define Elements
  //

  a = 1.01 * g / mole;
  auto elH = new G4Element(name = "Hydrogen", symbol = "H", z = 1., a);

  a = 12.01 * g / mole;
  auto elC = new G4Element(name = "Carbon", symbol = "C", z = 6., a);

  a = 14.01 * g / mole;
  auto elN = new G4Element(name = "Nitrogen", symbol = "N", z = 7., a);

  a = 16.00 * g / mole;
  auto elO = new G4Element(name = "Oxygen", symbol = "O", z = 8., a);

  a = 39.948 * g / mole;
  auto elAr = new G4Element(name = "Argon", symbol = "Ar", z = 18., a);

  //
  // define simple materials
  //

  // Mylar

  density = 1.39 * g / cm3;
  auto mylar = new G4Material(name = "Mylar", density, nel = 3);
  mylar->AddElement(elO, 2);
  mylar->AddElement(elC, 5);
  mylar->AddElement(elH, 4);

  // Polypropelene

  auto CH2 = new G4Material("Polypropelene", 0.91 * g / cm3, 2);
  CH2->AddElement(elH, 2);
  CH2->AddElement(elC, 1);

  // Krypton as detector gas, STP

  density = 3.700 * mg / cm3;
  a = 83.80 * g / mole;
  auto Kr = new G4Material(name = "Kr", z = 36., a, density);

  // Dry air (average composition)

  density = 1.7836 * mg / cm3;  // STP
  auto argon = new G4Material(name = "Argon", density, ncomponents = 1);
  argon->AddElement(elAr, 1);

  density = 1.25053 * mg / cm3;  // STP
  auto nitrogen = new G4Material(name = "N2", density, ncomponents = 1);
  nitrogen->AddElement(elN, 2);

  density = 1.4289 * mg / cm3;  // STP
  auto oxygen = new G4Material(name = "O2", density, ncomponents = 1);
  oxygen->AddElement(elO, 2);

  density = 1.2928 * mg / cm3;  // STP
  density *= 1.0e-8;  // pumped vacuum
  temperature = STP_Temperature;
  pressure = 1.0e-8 * STP_Pressure;

  auto air =
    new G4Material(name = "Air", density, ncomponents = 3, kStateGas, temperature, pressure);
  air->AddMaterial(nitrogen, fractionmass = 0.7557);
  air->AddMaterial(oxygen, fractionmass = 0.2315);
  air->AddMaterial(argon, fractionmass = 0.0128);

  // Xenon as detector gas, STP

  density = 5.858 * mg / cm3;
  a = 131.29 * g / mole;
  auto Xe = new G4Material(name = "Xenon", z = 54., a, density);

  // Carbon dioxide, STP

  density = 1.842 * mg / cm3;
  auto CarbonDioxide = new G4Material(name = "CO2", density, nel = 2);
  CarbonDioxide->AddElement(elC, 1);
  CarbonDioxide->AddElement(elO, 2);

  // 80% Xe + 20% CO2, STP

  density = 5.0818 * mg / cm3;
  auto Xe20CO2 = new G4Material(name = "Xe20CO2", density, ncomponents = 2);
  Xe20CO2->AddMaterial(Xe, fractionmass = 0.922);
  Xe20CO2->AddMaterial(CarbonDioxide, fractionmass = 0.078);

  // 80% Kr + 20% CO2, STP

  density = 3.601 * mg / cm3;
  auto Kr20CO2 = new G4Material(name = "Kr20CO2", density, ncomponents = 2);
  Kr20CO2->AddMaterial(Kr, fractionmass = 0.89);
  Kr20CO2->AddMaterial(CarbonDioxide, fractionmass = 0.11);

  G4cout << *(G4Material::GetMaterialTable()) << G4endl;

  // default materials of the calorimeter and TR radiator

  fRadiatorMat = air;  // CH2 ;   // mylar;

  fAbsorberMaterial = air;  //  Kr20CO2;   // XeCO2CF4;

  fWorldMaterial = air;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* F03DetectorConstruction::ConstructCalorimeter()
{
  // Cleanup old geometry

  if (fPhysiWorld) {
    G4GeometryManager::GetInstance()->OpenGeometry();
    G4PhysicalVolumeStore::GetInstance()->Clean();
    G4LogicalVolumeStore::GetInstance()->Clean();
    G4SolidStore::GetInstance()->Clean();
  }

  // complete the Calor parameters definition and Print

  ComputeCalorParameters();
  PrintCalorParameters();

  G4bool checkOverlaps = true;

  fSolidWorld = new G4Tubs("World",  // its name
                           0., fWorldSizeR, fWorldSizeZ / 2., 0., twopi);  // its size

  fLogicWorld = new G4LogicalVolume(fSolidWorld,  // its solid
                                    fWorldMaterial,  // its material
                                    "World");  // its name

  fPhysiWorld = new G4PVPlacement(nullptr,  // no rotation
                                  G4ThreeVector(),  // at (0,0,0)
                                  "World",  // its name
                                  fLogicWorld,  // its logical volume
                                  nullptr,  // its mother  volume
                                  false,  // no boolean op.
                                  0,  // copy number
                                  checkOverlaps);  // checkOverlaps

  // TR radiator envelope
  G4double radThick = fFoilNumber * (fRadThickness + fGasGap) + fDetGap;
  G4double zRad = fZAbsorber - 0.5 * (radThick + fAbsorberThickness);

  G4cout << "zRad = " << zRad / mm << " mm" << G4endl;
  G4cout << "radThick = " << radThick / mm << " mm" << G4endl;
  G4cout << "fFoilNumber = " << fFoilNumber << G4endl;
  G4cout << "fRadiatorMat = " << fRadiatorMat->GetName() << G4endl;
  G4cout << "WorldMaterial = " << fWorldMaterial->GetName() << G4endl;

  fSolidRadiator = new G4Tubs("Radiator", 0.0, fAbsorberRadius, 0.5 * radThick, 0.0, twopi);

  fLogicRadiator = new G4LogicalVolume(fSolidRadiator, fWorldMaterial, "Radiator");

  fPhysiRadiator = new G4PVPlacement(nullptr, G4ThreeVector(0, 0, zRad), "Radiator", fLogicRadiator,
                                     fPhysiWorld, false, 0, checkOverlaps);

  fSolidRadSlice = new G4Tubs("RadSlice", 0.0, fAbsorberRadius, 0.5 * fRadThickness, 0.0, twopi);

  fLogicRadSlice = new G4LogicalVolume(fSolidRadSlice, fRadiatorMat, "RadSlice");

  // Radiator slice
  G4double radSliceThick = fRadThickness + fGasGap;
  G4double zStart = 0.5 * (-radThick + radSliceThick) + fDetGap;
  // start on the board of radiator enevelope + det gap

  for (G4int j = 0; j < fFoilNumber; j++) {
    G4double zSlice = zStart + j * radSliceThick;
    G4cout << zSlice / mm << " mm"
           << "\t";

    fPhysiRadSlice = new G4PVPlacement(nullptr, G4ThreeVector(0., 0., zSlice), "RadSlice",
                                       fLogicRadSlice, fPhysiRadiator, false, j, checkOverlaps);
  }
  G4cout << G4endl;

  // Absorber

  fSolidAbsorber =
    new G4Tubs("Absorber", 1.0 * mm, fAbsorberRadius, fAbsorberThickness / 2., 0.0, twopi);

  fLogicAbsorber = new G4LogicalVolume(fSolidAbsorber, fAbsorberMaterial, "Absorber");

  fPhysiAbsorber = new G4PVPlacement(nullptr, G4ThreeVector(0., 0., fZAbsorber), "Absorber",
                                     fLogicAbsorber, fPhysiWorld, false, 0, checkOverlaps);

  return fPhysiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::PrintCalorParameters()
{
  G4cout << "\n The  WORLD   is made of " << fWorldSizeZ / mm << "mm of "
         << fWorldMaterial->GetName();
  G4cout << ", the transverse size (R) of the world is " << fWorldSizeR / mm << " mm. " << G4endl;
  G4cout << " The ABSORBER is made of " << fAbsorberThickness / mm << "mm of "
         << fAbsorberMaterial->GetName();
  G4cout << ", the transverse size (R) is " << fAbsorberRadius / mm << " mm. " << G4endl;
  G4cout << " Z position of the (middle of the) absorber " << fZAbsorber / mm << "  mm." << G4endl;
  G4cout << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::SetAbsorberMaterial(G4String materialChoice)
{
  // get the pointer to the material table
  const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();

  // search the material by its name
  G4Material* material;
  for (size_t j = 0; j < theMaterialTable->size(); j++) {
    material = (*theMaterialTable)[j];
    if (material->GetName() == materialChoice) {
      fAbsorberMaterial = material;
      fLogicAbsorber->SetMaterial(material);
      G4RunManager::GetRunManager()->PhysicsHasBeenModified();
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::SetWorldMaterial(G4String materialChoice)
{
  // get the pointer to the material table
  const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();

  // search the material by its name
  G4Material* material;
  for (size_t j = 0; j < theMaterialTable->size(); j++) {
    material = (*theMaterialTable)[j];
    if (material->GetName() == materialChoice) {
      fWorldMaterial = material;
      fLogicWorld->SetMaterial(material);
      G4RunManager::GetRunManager()->PhysicsHasBeenModified();
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::SetAbsorberThickness(G4double val)
{
  // change Absorber thickness and recompute the calorimeter parameters
  fAbsorberThickness = val;
  ComputeCalorParameters();
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::SetAbsorberRadius(G4double val)
{
  // change the transverse size and recompute the calorimeter parameters
  fAbsorberRadius = val;
  ComputeCalorParameters();
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::SetWorldSizeZ(G4double val)
{
  fWorldSizeZ = val;
  ComputeCalorParameters();
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::SetWorldSizeR(G4double val)
{
  fWorldSizeR = val;
  ComputeCalorParameters();
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::SetAbsorberZpos(G4double val)
{
  fZAbsorber = val;
  ComputeCalorParameters();
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::SetFieldValue(G4ThreeVector value)
{
  fFieldVector = value;

  G4UniformMagField* magField = nullptr;
  if (fFieldVector != G4ThreeVector(0.,0.,0.)) {
    magField = new G4UniformMagField(fFieldVector);
  }

  // Set field to the field builder
  auto fieldBuilder = G4FieldBuilder::Instance();
  fieldBuilder->SetGlobalField(magField);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void F03DetectorConstruction::SetLocalFieldValue(G4ThreeVector value)
{
  fLocalFieldVector = value;

  G4UniformMagField* magField = nullptr;
  if (fLocalFieldVector != G4ThreeVector(0.,0.,0.)) {
    magField = new G4UniformMagField(fLocalFieldVector);
  }

  // Set field to the field builder
  auto fieldBuilder = G4FieldBuilder::Instance();
  fieldBuilder->SetLocalField(magField, fLogicRadiator);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void F03DetectorConstruction::ConstructSDandField()
{
  // Sensitive Detectors: Absorber

  if (!fCalorimeterSD.Get()) {
    auto calorimeterSD = new F03CalorimeterSD("CalorSD", this);
    fCalorimeterSD.Put(calorimeterSD);
  }
  G4SDManager::GetSDMpointer()->AddNewDetector(fCalorimeterSD.Get());
  SetSensitiveDetector(fLogicAbsorber, fCalorimeterSD.Get());

  // Create detector fields
  SetFieldValue(fFieldVector);
  SetLocalFieldValue(fLocalFieldVector);

  // Construct all Geant4 field objects
  auto fieldBuilder = G4FieldBuilder::Instance();
  fieldBuilder->ConstructFieldSetup();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
