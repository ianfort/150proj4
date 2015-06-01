#include <iostream>
#include <unistd.h>
#include "FATData.h"

using namespace std;

int main()
{
  string myStr;
  bool success;
  FATData* testFAT;
  testFAT = new FATData("fat.ima");

  cout << "Calling FATvol\n";
  testFAT->fatvol();
  cout << "Calling FATls\n";
  testFAT->fatls();
  cout << endl;

  success = testFAT->readFromFile(string("MACHINE .CPP"), 1000, &myStr);

  cout << "success: " << success << ". myStr: " << endl;
  cout << myStr << endl;
  
/*
  cout << "Calling fatout\n";
  testFAT->fatout();
  cout << endl;
*/
  delete testFAT;
}


