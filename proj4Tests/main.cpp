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
  delete testFAT;
}


