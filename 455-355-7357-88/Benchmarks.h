#pragma once

/*
    ###################################################################################
    Benchmarks.h
    Basic header-only Benchmark Test class.
    Part of the 455-355-7357-88 (ASS-ESS-TEST-88) test suite.
    Made by PR00F88
    2024
    ###################################################################################
*/

#include "Test.h"
#include "ScopeBenchmarker.h"

/**
    This class should be the base of every benchmark tester class.
    It should be used in the following way:
    1. - Create the actual benchmark test class by deriving from this class (mandatory);
    2. - Override initialize() (optional):
         it will be called by run() only once before running any test;
    3. - Override setUp() (optional):
         it will be called by run() right before any test (after initialize());
    4. - Override testMethod() (optional):
         write the actual test code into it; note that it won't be invoked if setUp() failed;
    5. - Override tearDown() (optional):
         it will be called by run() right after any test, also if no test was invoked due to setUp() failed;
    6. - Override finalize() (optional):
         it will be called by run() only once after all tests finished (either successfully or unsuccessfully);
    7. - You can create unit-subtests (optional):
         use addSubTest() in initialize() or the class constructor to add your subtest methods to the test;
    8. - call run() to run the test(s).
         If the test fails, use getMessage() to get the cause of the failure.
         Any call to addToErrorMessages() adds message into the string array returned by getMessage().

    General rule: although addToErrorMessages() can be used manually to save error messages, it is recommended to use
    the assertXXX() functions instead that automatically call addToErrorMessages() with actual values in case of assertion failure.

    If you decide to make unit-subtests, you should bear in mind the following:
    - addSubTest() expects a PFNUNITSUBTEST pointer, so the signature of your unit-subtests should match described by PFNUNITSUBTEST:
      bool unitSubTest(void);
    - a unit-subtest should return true on pass and false on fail;
    - it is ok to use multiple assertions in a single subtest but using the optional message parameters of the assertion methods is highly recommended.
*/

class Benchmark : public Test
{
public:

    /**
        @param testFile The file where the test is defined.
        @param testName The name of the test. If empty, itt will be "Unnamed Test".
    */
    Benchmark(const std::string& testFile = "", const std::string& testName = "") :
        Test(testFile, testName)
    {
    }


    /**
        Executes the test. Runs the overridable testMethod() and every unit-subtests.
        run() calls functions in the following order:
         - initialize();
         - if ( setUp() )
           - TestMethod();
         - tearDown();
         - for all subtests:
           - if ( setUp() )
             - subtest();
           - tearDown();
         - finalize();

        @return True if the test passed, false otherwise.
    */
    virtual bool run() override
    {
        reset();
        bTestRan = true;
        initialize();
        bool bSkipAllSubTests;
        initBenchmarkers();
        if (setUp())
        {
            bSkipAllSubTests = false;
            if (!testMethod())
            {
                addToErrorMessages(std::string("  <").append(sTestFile).append("> failed!").c_str());
            }
        }
        else
        {
            bSkipAllSubTests = true;
            addToErrorMessages(std::string("  <").append(sTestFile).append("> setUp() failed!").c_str());
        }
        tearDown();
        printBenchmarkers();

        // subtests start
        if (!bSkipAllSubTests)
        {
            bWeAreInSubTest = true;
            for (size_t i = 0; i < tSubTests.size(); ++i)
            {
                iCurrentSubTest = i;
                initBenchmarkers();
                if (setUp())
                {
                    PFNUNITSUBTEST func = tSubTests[i].first;
                    if ((this->*func)())
                        ++nSucceededSubTests;
                    else
                        addToErrorMessages(std::string("  <").append(tSubTests[i].second).append("> failed!").c_str());
                }
                else
                {
                    addToErrorMessages(std::string("  <").append(tSubTests[i].second).append("> SKIPPED due to setUp() failed!").c_str());
                }
                tearDown();
                printBenchmarkers();
            }
            bWeAreInSubTest = false;
        }
        // subtests ended

        finalize();
        return isPassed();
    }

private:

    void initBenchmarkers()
    {
        ScopeBenchmarker::clear(); // since ScopeBenchmarker works with static data, make sure previous tests did not leave something there
    }

    void printBenchmarkers()
    {
        if (ScopeBenchmarker::getAllData().empty())
        {
            return;
        }

        if (isSubTestRunning())
        {
            addToInfoMessages((std::string("  <").append(sTestFile + "::" + tSubTests[iCurrentSubTest].second).append("> Scope Benchmarkers:")).c_str());
        }
        else
        {
            addToInfoMessages((std::string("  <").append(sTestFile).append("> Scope Benchmarkers:")).c_str());
        }

        for (const auto& bmData : ScopeBenchmarker::getAllData())
        {
            addToInfoMessages(
                ("    " +
                    bmData.second.m_name +
                    " Iterations: " + std::to_string(bmData.second.m_iterations) +
                    ", Min/Max/Avg Duration: " +
                    std::to_string(bmData.second.m_durationsMinMillisecs) + "/" +
                    std::to_string(bmData.second.m_durationsMaxMillisecs) + "/" +
                    std::to_string(bmData.second.getAverageDurationMillisecs()) +
                    ", Total Duration: " + std::to_string(bmData.second.m_durationsTotalMillisecs)).c_str());
        }
        addToInfoMessages("");

        ScopeBenchmarker::clear(); // since ScopeBenchmarker works with static data, make sure we dont leave anything there
    }

}; // class Benchmark
