//
//  unittests.h
//  vstgui
//
//  Created by Arne Scheffler on 4/1/13.
//
//

#include "unittests.h"

#if ENABLE_UNIT_TESTS

#include <chrono>

namespace VSTGUI {
namespace UnitTest {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
TestCase::TestCase (std::string&& name, TestCaseFunction&& testCase)
: name (name)
{
	testCase (this);
}

//----------------------------------------------------------------------------------------------------
TestCase::TestCase (TestCase&& tc)
{
	name = std::move (tc.name);
	tests = std::move (tc.tests);
	setupFunction = std::move (tc.setupFunction);
	teardownFunction = std::move (tc.teardownFunction);
}

//----------------------------------------------------------------------------------------------------
void TestCase::registerTest (std::string&& name, TestFunction&& function)
{
	tests.push_back (TestPair (name, function));
}

//----------------------------------------------------------------------------------------------------
void TestCase::setSetupFunction (SetupFunction&& _setupFunction)
{
	setupFunction = std::move (_setupFunction);
}

//----------------------------------------------------------------------------------------------------
void TestCase::setTeardownFunction (TeardownFunction&& _teardownFunction)
{
	teardownFunction = std::move (_teardownFunction);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UnitTestRegistry& UnitTestRegistry::instance ()
{
	static UnitTestRegistry gUnitTestRegistry;
	return gUnitTestRegistry;
}

//----------------------------------------------------------------------------------------------------
void UnitTestRegistry::registerTestCase (TestCase&& testCase)
{
	testCases.push_back (std::move (testCase));
}

//----------------------------------------------------------------------------------------------------
void Context::print (const char* fmt, ...)
{
	va_list args;
	va_start (args, fmt);
	
	char* str = 0;
	// TODO: Windows alternative implementation needed !
#if !_WIN32
	if (vasprintf (&str, fmt, args) >= 0 && str != 0)
	{
		printRaw (str);
		free (str);
	}
#endif
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class StdOutContext : public Context
{
private:
	struct Result {
		int succeded;
		int failed;
		
		Result () : succeded (0), failed (0) {}
		
		Result& operator +=(const Result& r) { succeded += r.succeded; failed += r.failed; return *this; }
	};
public:
	StdOutContext () : intend (0) {}

	virtual void printRaw (const char* str)
	{
		testOutput += str;
		testOutput += "\n";
	}
	
	void printOutput ()
	{
		if (testOutput.empty () == false)
		{
			printf ("%s", testOutput.c_str ());
			testOutput = "";
		}
	}
	void printIntend ()
	{
		for (int i = 0; i < intend; i++) printf ("\t");
	}

	Result runTestCase (const TestCase& testCase)
	{
		Result result;
		printf ("%s\n", testCase.getName ().c_str());
		intend++;
		for (auto& it : testCase)
		{
			if (testCase.setup ())
			{
				testCase.setup () (this);
			}
			if (runTest (it.first, it.second))
			{
				result.succeded++;
			}
			else
			{
				result.failed++;
			}
			if (testCase.teardown ())
			{
				testCase.teardown () (this);
			}
		}
		intend--;
		return result;
	}
	
	bool runTest (const std::string& testName, const TestFunction& f)
	{
		std::chrono::time_point<std::chrono::system_clock> start, end;
		printIntend ();
		printf ("%s", testName.c_str());
		intend++;
		start = std::chrono::system_clock::now ();
		bool result = f (this);
		end = std::chrono::system_clock::now ();
		intend--;
		printf (" [%s] -> %lldµs\n", result ? "OK" : "Failed", std::chrono::duration_cast<std::chrono::microseconds> (end-start).count ());
		printOutput ();
		return result;
	}
	
	void run ()
	{
		Result result;
		std::chrono::time_point<std::chrono::system_clock> start, end;
		start = std::chrono::system_clock::now ();
		for (auto& it : UnitTestRegistry::instance ())
		{
			result += runTestCase (std::move (it));
		}
		end = std::chrono::system_clock::now ();
		print ("\nDone running %d tests in %lldms. [%d Failed]\n", result.succeded+result.failed, std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count (), result.failed);
		printOutput ();
	}
private:
	int intend;
	std::string testOutput;
};

static void RunTests ()
{
	StdOutContext context;
	context.run ();
}

}} // namespaces

#if __APPLE_CC__
#include <CoreFoundation/CoreFoundation.h>
namespace VSTGUI { void* gBundleRef = CFBundleGetMainBundle(); }
#endif

int main ()
{
	VSTGUI::UnitTest::RunTests ();
	
	return 0;
}

TESTCASE(Example,
		 
	int result;

	SETUP(
		result = 0;
	);
	 
	TEST(OnePlusOneIsTwo,
		result = 1+1;
		EXPECT (result == 2)
	);
	 
	TEST(ThreeMinusOneIsTwo,
		result = 3-1;
		if (result != 2)
		{
			FAIL ("result is not two")
		}
	);

);

#endif