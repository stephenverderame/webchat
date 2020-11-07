#include "pch.h"
#include "CppUnitTest.h"
#include "../WebLib2/StringMap.h"
#include <vector>
#include "../WebLib2/StreamView.h"
#include <fstream>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestTest
{
	//Right click references, add the project to test
	//To run tests, go to Test, test explorer
	TEST_CLASS(UnitTestTest)
	{
	public:
		
		TEST_METHOD(TestStringMap)
		{
			Assert::AreEqual(1, 1);
			void* tv = nullptr;
			Assert::IsNull(tv); //Assertations are in the Assert class
			StringMap<std::string, std::string> sm;
			sm.put("Hello", "Test");
			Assert::AreEqual(sm["Hello"].second.c_str(), "Test");

			sm.remove("Hello");
			Assert::AreEqual(sm.size(), 0U);

			std::ifstream in("C:/Users/SEV/source/repos/stephenverderame/webchat/webchat/UnitTestTest/test.txt");
			StreamView sv;
			std::vector<std::string> test;
			StringMap<StreamView, size_t> testMap;
			size_t i = 0;
			std::string line;
			std::stringstream ss;
			std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>>();
			while(std::getline(in, line) && i < 2000) {
				for (char c : line)
					buffer->push_back(c);
				test.push_back(line);
				testMap.put(StreamView(buffer, buffer->size() - line.size(), buffer->size()), i++);
			}
			std::vector<int> removed;
			for(int i = 0; i < 1000; ++i) {
				size_t index = rand() % test.size();
				Assert::IsTrue(testMap[test[index]].first == test[index].c_str() && testMap[test[index]].second == index);
			}
			for(int i = 0; i < 500; ++i) {
				int r = 0;
				while(std::find(removed.begin(), removed.end(), (r = rand() % test.size())) != removed.end());
				removed.push_back(r);
				Assert::IsTrue(testMap.remove(test[r]));
			}
			for(int i : removed) {
				Assert::IsFalse(testMap.find(test[i]));
			}
			auto li = testMap.keyList();
			for(auto& k : li) {
				Assert::IsTrue(testMap.find(k));
			}
			Assert::AreEqual(li.size(), test.size() - removed.size());
			testMap[StreamView("notawordmanidk", 14)].second = 5U;
			Assert::IsTrue(testMap[StreamView("notawordmanidk", 14)].second == 5);

			i = 0;
			for(auto& p : testMap) {
				++i;
			}
			Assert::AreEqual(i, testMap.size());
		}
	};
}
