#include <iostream>
#include <unistd.h>
#include "FATData.h"

using namespace std;

int main()
{
  FATData* testFAT;
  testFAT = new FATData("fat.ima");

  cout << "Calling FATvol\n";
  testFAT->fatvol();
  cout << "Calling FATls\n";
  testFAT->fatls();
  cout << endl;
  cout << "Calling fatout\n";
  testFAT->fatout();
  cout << endl;
  delete testFAT;
}


