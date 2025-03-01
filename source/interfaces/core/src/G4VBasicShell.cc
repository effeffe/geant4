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
//
//

#include "G4VBasicShell.hh"

#include "G4StateManager.hh"
#include "G4UIcommand.hh"
#include "G4UIcommandStatus.hh"
#include "G4UIcommandTree.hh"
#include "G4UImanager.hh"

#include <sstream>
#include <vector>

G4VBasicShell::G4VBasicShell() : currentDirectory("/") {}

G4VBasicShell::~G4VBasicShell() = default;

G4String G4VBasicShell::ModifyToFullPathCommand(const char* aCommandLine) const
{
  const G4String& rawCommandLine = aCommandLine;
  if (rawCommandLine.empty() || rawCommandLine[0] == '\0') return rawCommandLine;
  const G4String& commandLine = G4StrUtil::strip_copy(rawCommandLine);
  G4String commandString;
  G4String parameterString;
  size_t i = commandLine.find(' ');
  if (i != std::string::npos) {
    commandString = commandLine.substr(0, i);
    parameterString = " ";
    parameterString += commandLine.substr(i + 1, commandLine.length() - (i + 1));
  }
  else {
    commandString = commandLine;
  }

  G4String fullPathCommandLine = ModifyPath(commandString) + parameterString;
  return fullPathCommandLine;
}

G4String G4VBasicShell::GetCurrentWorkingDirectory() const { return currentDirectory; }

G4bool G4VBasicShell::ChangeDirectory(const char* newDir)
{
  const G4String& newPrefix = G4StrUtil::strip_copy(newDir);

  G4String newDirectory = ModifyPath(newPrefix);
  if (newDirectory.back() != '/') {
    newDirectory += "/";
  }
  if (FindDirectory(newDirectory.c_str()) == nullptr) {
    return false;
  }
  currentDirectory = std::move(newDirectory);
  return true;
}

G4UIcommandTree* G4VBasicShell::FindDirectory(const char* dirName) const
{
  G4String theDir = G4StrUtil::strip_copy(dirName);

  G4String targetDir = ModifyPath(theDir);
  if (targetDir.back() != '/') {
    targetDir += "/";
  }
  G4UIcommandTree* comTree = G4UImanager::GetUIpointer()->GetTree();
  if (targetDir == "/") {
    return comTree;
  }
  size_t idx = 1;
  while (idx < targetDir.length() - 1) {
    size_t i = targetDir.find('/', idx);
    comTree = comTree->GetTree(targetDir.substr(0, i + 1).c_str());
    if (comTree == nullptr) {
      return nullptr;
    }
    idx = i + 1;
  }
  return comTree;
}

G4UIcommand* G4VBasicShell::FindCommand(const char* commandName) const
{
  const G4String& commandLine = G4StrUtil::strip_copy(commandName);

  G4String commandString;
  std::size_t i = commandLine.find(' ');
  if (i != std::string::npos) {
    commandString = commandLine.substr(0, i);
  }
  else {
    commandString = commandLine;
  }

  const G4String& targetCom = ModifyPath(commandString);
  return G4UImanager::GetUIpointer()->GetTree()->FindPath(targetCom);
}

G4String G4VBasicShell::ModifyPath(const G4String& tempPath) const
{
  if (tempPath.length() == 0) return tempPath;

  G4String newPath = "";

  // temporal full path
  if (tempPath[0] == '/')
    newPath = tempPath;
  else
    newPath = currentDirectory + tempPath;

  // body of path...
  while (true) {
    size_t idx = newPath.find("/./");
    if (idx == G4String::npos) break;
    newPath.erase(idx, 2);
  }

  while (true) {
    size_t idx = newPath.find("/../");
    if (idx == G4String::npos) break;
    if (idx == 0) {
      newPath.erase(1, 3);
      continue;
    }
    size_t idx2 = newPath.find_last_of('/', idx - 1);
    if (idx2 != G4String::npos) newPath.erase(idx2, idx - idx2 + 3);
  }

  // end of path...
  if (newPath.size() >= 3) {
    if (newPath.substr(newPath.size() - 3, 3) == "/..") {
      if (newPath.size() == 3) {
        newPath = "/";
      }
      else {
        size_t idx = newPath.find_last_of('/', newPath.size() - 4);
        if (idx != G4String::npos) newPath.erase(idx + 1);
      }
    }
  }

  if (newPath.size() >= 2) {
    if (newPath.substr(newPath.size() - 2, 2) == "/.") newPath.erase(newPath.size() - 1, 1);
  }

  // truncate "/////" to "/"
  while (true) {
    size_t idx = newPath.find("//");
    if (idx == G4String::npos) break;
    newPath.erase(idx, 1);
  }

  return newPath;
}
////////////////////////////////////////////
// Method used for command completion //////
////////////////////////////////////////////
G4String G4VBasicShell::Complete(const G4String& commandName)
{
  const G4String& rawCommandLine = commandName;
  const G4String& commandLine = G4StrUtil::strip_copy(rawCommandLine);

  std::size_t i = commandLine.find(' ');
  if (i != std::string::npos)
    return rawCommandLine;  // Already entering parameters,
                            // assume command path is correct.
  G4String commandString = commandLine;
  G4String targetCom = ModifyPath(commandString);
  G4UIcommandTree* tree = G4UImanager::GetUIpointer()->GetTree();
  G4String value = FindMatchingPath(tree, targetCom);
  if (value.empty()) return rawCommandLine;
  return value;
}

G4String G4VBasicShell::FindMatchingPath(G4UIcommandTree* aTree, const G4String& aCommandPath)
{
  return aTree->CompleteCommandPath(aCommandPath);
}

////////////////////////////////////////////
// Method involving an interactive G4cout //
////////////////////////////////////////////
/***************************************************************************/
void G4VBasicShell::ExecuteCommand(const G4String& aCommand)
/***************************************************************************/
// Should be put in G4VBasicShell.
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
{
  if (aCommand.length() < 2) return;
  G4UImanager* UI = G4UImanager::GetUIpointer();
  if (UI == nullptr) return;
  G4int commandStatus = UI->ApplyCommand(aCommand);
  switch (commandStatus) {
    case fCommandSucceeded:
      break;
    case fCommandNotFound:
      G4cerr << "command not found: "
             << "\"" << aCommand << "\"" << G4endl;
      break;
    case fIllegalApplicationState:
      G4cerr << "illegal application state -- command refused:"
             << "\"" << aCommand << "\"" << G4endl;
      break;
    case fParameterOutOfRange:
    case fParameterUnreadable:
    case fParameterOutOfCandidates:
    default:
      G4cerr << "command refused (" << commandStatus << "):"
             << "\"" << aCommand << "\"" << G4endl;
  }
}
/***************************************************************************/
void G4VBasicShell::ApplyShellCommand(
  const G4String& a_string, G4bool& exitSession, G4bool& exitPause)
/***************************************************************************/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
{
  G4UImanager* UI = G4UImanager::GetUIpointer();
  if (UI == nullptr) return;

  G4String command = G4StrUtil::lstrip_copy(a_string);

  if (command[0] == '#') {
    G4cout << command << G4endl;
  }
  else if (command == "ls" || command.substr(0, 3) == "ls ") {
    ListDirectory(command);
  }
  else if (command == "pwd") {
    G4cout << "Current Working Directory : " << GetCurrentWorkingDirectory() << G4endl;
  }
  else if (command == "cd" || command.substr(0, 3) == "cd ") {
    ChangeDirectoryCommand(command);
  }
  else if (command == "help" || command.substr(0, 5) == "help ") {
    TerminalHelp(command);
  }
  else if (command[0] == '?') {
    ShowCurrent(command);
  }
  else if (command == "hist" || command == "history") {
    G4int nh = UI->GetNumberOfHistory();
    for (G4int i = 0; i < nh; i++) {
      G4cout << i << ": " << UI->GetPreviousCommand(i) << G4endl;
    }
  }
  else if (command[0] == '!') {
    G4String ss = command.substr(1, command.length() - 1);
    G4int vl;
    const char* tt = ss;
    std::istringstream is(tt);
    is >> vl;
    G4int nh = UI->GetNumberOfHistory();
    if (vl >= 0 && vl < nh) {
      G4String prev = UI->GetPreviousCommand(vl);
      G4cout << prev << G4endl;
      ExecuteCommand(ModifyToFullPathCommand(prev));
    }
    else {
      G4cerr << "history " << vl << " is not found." << G4endl;
    }
  }
  else if (command == "exit") {
    if (!exitPause) {  // In a secondary loop.
      G4cout << "You are now processing RUN." << G4endl;
      G4cout << "Please abort it using \"/run/abort\" command first" << G4endl;
      G4cout << " and use \"continue\" command until the application" << G4endl;
      G4cout << " becomes to Idle." << G4endl;
    }
    else {
      exitSession = true;
    }
  }
  else if (command == "cont" || command == "continue") {
    exitPause = true;
  }
  else {
    ExecuteCommand(ModifyToFullPathCommand(a_string));
  }
}

void G4VBasicShell::ShowCurrent(const G4String& newCommand) const
{
  G4UImanager* UI = G4UImanager::GetUIpointer();
  if (UI == nullptr) return;
  G4String comString = newCommand.substr(1, newCommand.length() - 1);
  G4String theCommand = ModifyToFullPathCommand(comString);
  G4String curV = UI->GetCurrentValues(theCommand);
  if (! curV.empty()) {
    G4cout << "Current value(s) of the parameter(s) : " << curV << G4endl;
  }
}

void G4VBasicShell::ChangeDirectoryCommand(const G4String& newCommand)
{
  G4String prefix;
  if (newCommand.length() <= 3) {
    prefix = "/";
  }
  else {
    const G4String& aNewPrefix = newCommand.substr(3, newCommand.length() - 3);
    prefix = G4StrUtil::strip_copy(aNewPrefix);
  }
  if (! ChangeDirectory(prefix)) {
    G4cout << "directory <" << prefix << "> not found." << G4endl;
  }
}

void G4VBasicShell::ListDirectory(const G4String& newCommand) const
{
  G4String targetDir;
  if (newCommand.length() <= 3) {
    targetDir = GetCurrentWorkingDirectory();
  }
  else {
    const G4String& newPrefix = newCommand.substr(3, newCommand.length() - 3);
    targetDir = G4StrUtil::strip_copy(newPrefix);
  }
  G4UIcommandTree* commandTree = FindDirectory(targetDir);
  if (commandTree == nullptr) {
    G4cout << "Directory <" << targetDir << "> is not found." << G4endl;
  }
  else {
    commandTree->ListCurrent();
  }
}
void G4VBasicShell::TerminalHelp(const G4String& newCommand)
{
  G4UImanager* UI = G4UImanager::GetUIpointer();
  if (UI == nullptr) return;
  G4UIcommandTree* treeTop = UI->GetTree();
  size_t i = newCommand.find(' ');
  if (i != std::string::npos) {
    G4String newValue = newCommand.substr(i + 1, newCommand.length() - (i + 1));
    G4StrUtil::strip(newValue);
    G4String targetCom = ModifyToFullPathCommand(newValue);
    G4UIcommand* theCommand = treeTop->FindPath(targetCom);
    if (theCommand != nullptr) {
      theCommand->List();
      return;
    }

    G4cout << "Command <" << newValue << " is not found." << G4endl;
    return;
  }

  G4UIcommandTree* floor[10];
  floor[0] = treeTop;
  size_t iFloor = 0;
  size_t prefixIndex = 1;
  G4String prefix = GetCurrentWorkingDirectory();
  while (prefixIndex < prefix.length() - 1) {
    size_t ii = prefix.find('/', prefixIndex);
    floor[iFloor + 1] = floor[iFloor]->GetTree(G4String(prefix.substr(0, ii + 1)));
    prefixIndex = ii + 1;
    iFloor++;
  }
  floor[iFloor]->ListCurrentWithNum();
  // 1998 Oct 2 non-number input
  while (true) {
    // G4cout << G4endl << "Type the number ( 0:end, -n:n level back ) : "<<std::flush;
    G4cout << G4endl << "Type the number ( 0:end, -n:n level back ) : " << G4endl;
    G4int j;
    if (! GetHelpChoice(j)) {
      G4cout << G4endl << "Not a number, once more" << G4endl;
      continue;
    }
    if (j < 0) {
      if (iFloor < (size_t)-j)
        iFloor = 0;
      else
        iFloor += j;
      // iFloor += j;
      // if( iFloor < 0 ) iFloor = 0;
      floor[iFloor]->ListCurrentWithNum();
      continue;
    }
    if (j == 0) {
      break;
    }
    if (j > 0) {
      G4int n_tree = floor[iFloor]->GetTreeEntry();
      if (j > n_tree) {
        if (j <= n_tree + floor[iFloor]->GetCommandEntry()) {
          floor[iFloor]->GetCommand(j - n_tree)->List();
        }
      }
      else {
        floor[iFloor + 1] = floor[iFloor]->GetTree(j);
        iFloor++;
        floor[iFloor]->ListCurrentWithNum();
      }
    }
  }
  G4cout << "Exit from HELP." << G4endl << G4endl;
  // G4cout << G4endl;
  ExitHelp();
}
