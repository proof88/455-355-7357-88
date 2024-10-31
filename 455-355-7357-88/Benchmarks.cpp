/*
    ###############################################
    Benchmarks.cpp
    Example of Benchmark class
    Made by PR00F88
    ################################################
*/

#include "Benchmarks.h"

#include <cassert>
#include <memory>  // for std::unique_ptr; requires cpp11
#include <thread>  // for sleep_for(); requires cpp11

//#ifndef WINPROOF88_ALLOW_CONTROLS_AND_DIALOGS
//#define WINPROOF88_ALLOW_CONTROLS_AND_DIALOGS
//#endif
//#ifndef WINPROOF88_ALLOW_MSG_USER_WINMESSAGES
//#define WINPROOF88_ALLOW_MSG_USER_WINMESSAGES
//#endif

#include "../../PFL/PFL/winproof88.h"

#include "../../Console/CConsole/src/CConsole.h"

static CConsole& getConsole()
{
    return CConsole::getConsoleInstance();
}

class ExampleBenchmarkTest :
    public Benchmark
{
public:

    ExampleBenchmarkTest() : Benchmark(__FILE__, "")
    {
    }

    ~ExampleBenchmarkTest()
    {
        finalize();
    }

    ExampleBenchmarkTest(const ExampleBenchmarkTest&) = delete;
    ExampleBenchmarkTest& operator=(const ExampleBenchmarkTest&) = delete;
    ExampleBenchmarkTest(ExampleBenchmarkTest&&) = delete;
    ExampleBenchmarkTest& operator=(ExampleBenchmarkTest&&) = delete;

protected:

    virtual void initialize() override
    {
        addSubTest("test_scope_benchmarking", (PFNUNITSUBTEST)&ExampleBenchmarkTest::test_scope_benchmarking);

        std::this_thread::sleep_for(std::chrono::seconds(1)); // sleep to avoid performance disturbance caused by Visual Studio background debug tools init
    }

private:

    bool test_scope_benchmarking()
    {
        // We are testing different sleep times from bigger to smaller, because in general sleep precisity is bigger with bigger sleeps, and
        // if we start this up in VS, its debug tools init might impact our measurements, so better do the smaller sleeps measurements later
        // when the program has already been executing for a while!
        static constexpr auto sleepTimes = PFL::std_array_of<int>(
            100, 50, 30, 25, 20, 18, 15, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

        static constexpr int iterationsPerSleepTime = 20;

        bool b = true;
        for (const int& sleepFor : sleepTimes)
        {
            const std::string scopeBmName = "sleep-" + std::to_string(sleepFor);
            const std::string scopeOhBmName = "sleep-oh-" + std::to_string(sleepFor);
            {
                ScopeBenchmarker scopeOhBm(scopeOhBmName); // "scope duration measurement overhead" measurement starts here, "oh" stands for overhead
                for (int i = 0; i < iterationsPerSleepTime; i++)
                {
                    ScopeBenchmarker scopeBm(scopeBmName); // scope duration measurement starts here
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));
                    // scope duration measurement ends here
                }
                // "scope duration measurement overhead" measurement ends here
            }

            // this is how we can access ScopeBenchmarker data after ScopeBenchmarker object is already out of scope
            const auto& scopeBmData = ScopeBenchmarker::getDataByName(scopeBmName);

            b &= assertBetween(sleepFor * iterationsPerSleepTime, 5000, scopeBmData.m_durationsTotalMillisecs); // should be assertDurationsTotalBetween
            b &= assertBetween(sleepFor, 200, scopeBmData.m_durationsMinMillisecs);
            b &= assertBetween(scopeBmData.m_durationsMinMillisecs, static_cast<long long>(200), scopeBmData.m_durationsMaxMillisecs);
            b &= assertEquals(iterationsPerSleepTime, scopeBmData.m_iterations); // should be assertIterationCountEquals
            b &= assertBetween(0, 200, scopeBmData.getAverageDurationMillisecs()); // should be assertDurationsAverageBetween

            const auto& scopeOhBmData = ScopeBenchmarker::getDataByName(scopeOhBmName);
            addToInfoMessages(
                ("  " +
                 scopeBmName +
                 ", Total Overhead: " + std::to_string(scopeOhBmData.m_durationsTotalMillisecs - scopeBmData.m_durationsTotalMillisecs)).c_str());
        }

        return b;
    }

}; // class ExampleBenchmarkTest


int WINAPI WinMain(_In_ HINSTANCE /*hInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*lpCmdLine*/, _In_ int /*nCmdShow*/)
{
    constexpr const char* const CON_TITLE = "Example benchmark test";

    getConsole().Initialize(CON_TITLE, true);
    //getConsole().SetLoggingState("4LLM0DUL3S", true);
    getConsole().SetErrorsAlwaysOn(false);
    getConsole().OLn(CON_TITLE);
    getConsole().L();
    getConsole().OLn("");

    std::vector<std::unique_ptr<Benchmark>> tests;

    tests.push_back(std::unique_ptr<Benchmark>(new ExampleBenchmarkTest));

    std::vector<std::unique_ptr<Benchmark>>::size_type nSucceededTests = 0;
    std::vector<std::unique_ptr<Benchmark>>::size_type nTotalSubTests = 0;
    std::vector<std::unique_ptr<Benchmark>>::size_type nTotalPassedSubTests = 0;
    for (std::vector<std::unique_ptr<Benchmark>>::size_type i = 0; i < tests.size(); ++i)
    {
        getConsole().OLn("Running test %d / %d ... ", i + 1, tests.size());
        tests[i]->run();
    }

    // summarizing
    getConsole().OLn("");
    for (std::vector<std::unique_ptr<Benchmark>>::size_type i = 0; i < tests.size(); ++i)
    {
        for (const auto& infoMsg : tests[i]->getInfoMessages())
        {
            getConsole().OLn("%s", infoMsg.c_str());
        }

        if (tests[i]->isPassed())
        {
            ++nSucceededTests;
            getConsole().SOn();
            if (tests[i]->getName().empty())
            {
                getConsole().OLn("Test passed: %s(%d)!", tests[i]->getFile().c_str(), tests[i]->getSubTestCount());
            }
            else if (tests[i]->getFile().empty())
            {
                getConsole().OLn("Test passed: %s(%d)!", tests[i]->getName().c_str(), tests[i]->getSubTestCount());
            }
            else
            {
                getConsole().OLn("Test passed: %s(%d) in %s!", tests[i]->getName().c_str(), tests[i]->getSubTestCount(), tests[i]->getFile().c_str());
            }
            getConsole().SOff();
        }
        else
        {
            getConsole().EOn();
            if (tests[i]->getName().empty())
            {
                getConsole().OLn("Test failed: %s", tests[i]->getFile().c_str());
            }
            else if (tests[i]->getFile().empty())
            {
                getConsole().OLn("Test failed: %s", tests[i]->getName().c_str());
            }
            else
            {
                getConsole().OLn("Test failed: %s in %s", tests[i]->getName().c_str(), tests[i]->getFile().c_str());
            }
            getConsole().Indent();
            for (std::vector<std::string>::size_type j = 0; j < tests[i]->getErrorMessages().size(); ++j)
            {
                getConsole().OLn("%s", tests[i]->getErrorMessages()[j].c_str());
            }
            getConsole().Outdent();
            getConsole().EOff();
        }
        nTotalSubTests += tests[i]->getSubTestCount();
        nTotalPassedSubTests += tests[i]->getPassedSubTestCount();
    }

    getConsole().OLn("");
    getConsole().OLn("========================================================");
    if (nSucceededTests == tests.size())
    {
        getConsole().SOn();
    }
    else
    {
        getConsole().EOn();
    }
    getConsole().OLn("Passed tests: %d / %d (SubTests: %d / %d)", nSucceededTests, tests.size(), nTotalPassedSubTests, nTotalSubTests);
    getConsole().NOn();
    getConsole().OLn("========================================================");

    system("pause");

    getConsole().Deinitialize();

    return 0;

} // WinMain()