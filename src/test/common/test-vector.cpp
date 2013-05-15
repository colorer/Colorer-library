
#define COLORER_FEATURE_ASSERT TRUE

#include<common/Logging.h>
#include<common/Vector.h>
#include<common/SortedVector.h>

int main()
{
  Vector<int> v;

  v.addElement(4);
  v.addElement(5);
  v.addElement(6);

  assert(v.elementAt(0) == 4);
  assert(v.elementAt(2) == 6);

  v.removeElement(5);

  assert(v.elementAt(1) == 6);

  v.insertElementAt(100, 2);

  assert(v.elementAt(2) == 100);

  printf("passed: Vector\n");

  
  
  SortedVector<int> sv;

  sv.addElement(7);
  assert(sv.elementAt(0) == 7);

  sv.addElement(6);
  assert(sv.elementAt(0) == 6);
  assert(sv.elementAt(1) == 7);

  sv.addElement(9);
  assert(sv.elementAt(2) == 9);

  sv.addElement(3);
  assert(sv.elementAt(0) == 3);
  assert(sv.elementAt(3) == 9);

  sv.addElement(1);

  sv.removeElement(6);

  assert(sv.size() == 4);
  assert(sv.elementAt(3) == 9);
  assert(sv.indexOf(9) == 3);
  assert(sv.indexOf(1) == 0);

  int numbers = 100;
  while(numbers--){
    sv.addElement(numbers%2 ? -numbers : numbers);
  }

  int maxel = -1000000;
  for (int idx = 0; idx < sv.size(); idx++) {
    //printf("%d, ", sv.elementAt(idx));
    if (sv.elementAt(idx) < maxel) assert(!"sv order failed");
    maxel = sv.elementAt(idx);
  }

  assert(sv.indexOf(10000) == -1);
  assert(sv.indexOf(-10000) == -1);
  
  sv.clear();

  assert(sv.size() == 0);

  sv.addElement(0);
  sv.addElement(2);
  sv.addElement(2);
  sv.addElement(2);

  assert(sv.size() == 4);
  // undefined assert(sv.indexOf(2) == 1);

  assert(sv.elementAt(0) == 0);
  assert(sv.elementAt(3) == 2);


  printf("passed: SortedVector\n");

}
