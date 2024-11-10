/*
    ###############################################
    BenchmarksExample.cpp
    Example for Benchmark class.
    You don't need this cpp file in your project, this is just an example for using my Benchmark class.
    Made by PR00F88
    2024
    ################################################
*/

// need to define this macro so we can use Test::runTests() with Console lib
#ifndef TEST_WITH_CCONSOLE
#define TEST_WITH_CCONSOLE
#endif
#include "Benchmarks.h"

#include <cassert>
#include <memory>  // for std::unique_ptr; requires cpp11
#include <thread>  // for sleep_for(); requires cpp11

#include "winproof88.h"  // part of PFL lib: https://github.com/proof88/PFL

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
        // well, I just added only 1 subtest, which means I should rather implement the test by overriding testMethod(), but
        // let's treat this as an example on how to add subtest to a test class!
        addSubTest("test_scope_benchmarking", (PFNUNITSUBTEST)&ExampleBenchmarkTest::test_scope_benchmarking);

        // sleep to avoid performance disturbance caused by Visual Studio background debug tools init after start debugging
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
                ScopeBenchmarker<std::chrono::milliseconds> scopeOhBm(scopeOhBmName); // "scope duration measurement overhead" measurement starts here, "oh" stands for overhead
                for (int i = 0; i < iterationsPerSleepTime; i++)
                {
                    ScopeBenchmarker<std::chrono::milliseconds> scopeBm(scopeBmName); // scope duration measurement starts here
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));
                    // scope duration measurement ends here
                }
                // "scope duration measurement overhead" measurement ends here
            }

            // this is how we can access ScopeBenchmarker data after ScopeBenchmarker object is already out of scope
            const auto& scopeBmData = ScopeBenchmarkerDataStore::getDataByName(scopeBmName);

            b &= assertBetween(sleepFor * iterationsPerSleepTime, 5000, scopeBmData.m_durationsTotal); // should be assertDurationsTotalBetween
            b &= assertBetween(sleepFor, 200, scopeBmData.m_durationsMin);
            b &= assertBetween(scopeBmData.m_durationsMin, static_cast<long long>(200), scopeBmData.m_durationsMax);
            b &= assertEquals(iterationsPerSleepTime, scopeBmData.m_iterations); // should be assertIterationCountEquals
            b &= assertBetween(0, 200, scopeBmData.getAverageDuration()); // should be assertDurationsAverageBetween

            const auto& scopeOhBmData = ScopeBenchmarkerDataStore::getDataByName(scopeOhBmName);
            addToInfoMessages(
                ("  " +
                 scopeBmName +
                 ", Total Overhead: " + std::to_string(scopeOhBmData.m_durationsTotal - scopeBmData.m_durationsTotal)).c_str());
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

    getConsole().OLn("");
    // Expecting NDEBUG to be reliable: https://man7.org/linux/man-pages/man3/assert.3.html
#ifdef NDEBUG
    const char* const szBuildType = "Release";
#else
    const char* const szBuildType = "Debug";
#endif 
    getConsole().OLn("%s. Build Type: %s, Timestamp: %s @ %s", CON_TITLE, szBuildType, __DATE__, __TIME__);

    std::vector<std::unique_ptr<Test>> tests;
    tests.push_back(std::unique_ptr<Test>(new ExampleBenchmarkTest));

    Test::runTests(tests, getConsole(), "Running Performance Tests ...");
    system("pause");

    getConsole().Deinitialize();

    return 0;

} // WinMain()