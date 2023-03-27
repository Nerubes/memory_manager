#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "src/doctest.h"
#include "mm.h"
#include <vector>

class TestClass {
 public:
  TestClass() = default;

  TestClass(int n, std::string name, time_t time)
      : n(n), name(std::move(name)), time(time) {};

  TestClass& operator=(const TestClass& other) = default;

  TestClass(const TestClass& other) = default;

  bool operator==(const TestClass& other) const {
    return ((n == other.n) && (name == other.name) && (time == other.time));
  }

  bool operator!=(const TestClass& other) const { return !(*this == other); }

 private:
  int n = 0;
  std::string name;
  time_t time = time_t();
};

using namespace lab618;

TEST_CASE("Testing MM with Deleting elements on destruct") {
  int size = 100;
  CMemoryManager mm = CMemoryManager<TestClass>(size, true);
  int amount = 1000;
  for (int i = 0; i < amount; ++i) {
    TestClass *obj = mm.newObject();
    CHECK(*obj == TestClass());
    if (i % 2) {
      CHECK(mm.deleteObject(obj) == true);
    }
  }
  TestClass* wrong_obj = new TestClass();
  CHECK(mm.deleteObject(wrong_obj) == false);
  CHECK_NOTHROW(mm.clear());
}

TEST_CASE("Testing MM without Deleting elements on destruct") {
  int size = 100;
  CMemoryManager mm = CMemoryManager<TestClass>(size, false);
  std::vector<TestClass *> pointers;
  int amount = 1000;
  TestClass *wrong_obj = new TestClass();
  for (int i = 0; i < amount; ++i) {
    TestClass *obj = mm.newObject();
    CHECK(*obj == TestClass());
    pointers.push_back(obj);
  }
  CHECK(mm.deleteObject(wrong_obj) == false);
  for (auto& obj : pointers) {
    CHECK(mm.deleteObject(obj) == true);
  }
  for (auto& obj : pointers) {
    CHECK(mm.deleteObject(obj) == false);
  }
  delete wrong_obj;
}

TEST_CASE("Testing MM without Deleting elements on destruct (not actually deleting elements)") {
  int size = 100;
  CMemoryManager mm = CMemoryManager<TestClass>(size, false);
  std::vector<TestClass*> pointers;
  int amount = 1000;
  for (int i = 0; i < amount; ++i) {
    TestClass *obj = mm.newObject();
    CHECK(*obj == TestClass());
    pointers.push_back(obj);
  }
  CHECK_THROWS(mm.clear());
  for (auto& obj : pointers) {
    CHECK(mm.deleteObject(obj) == true);
  }
}

TEST_CASE("Testing clear") {
  int size = 100;
  CMemoryManager mm = CMemoryManager<TestClass>(size, true);
  std::vector<TestClass*> pointers;
  int amount = 1000;
  for (int i = 0; i < amount; ++i) {
    TestClass *obj = mm.newObject();
    pointers.push_back(obj);
    CHECK(*obj == TestClass());
  }
  for (int i = 0; i < amount; ++i) {
    if (i % 2) {
      CHECK(mm.deleteObject(pointers[i]) == true);
    }
  }
  mm.clear();
  for (int i = 0; i < amount; ++i) {
    CHECK(mm.deleteObject(pointers[i]) == false);
  }
}
