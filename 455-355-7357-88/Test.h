#pragma once

/*
    ###################################################################################
    Test.h
    Base class for UnitTest and Benchmark classes.
    Part of the 455-355-7357-88 (ASS-ESS-TEST-88) test framework.
    Made by PR00F88
    2024
    ###################################################################################
*/

#include <cassert>
#include <cmath>
#include <memory>   // for std::unique_ptr; requires cpp11
#include <sstream>
#include <string>
#include <vector>
#include <utility>  // std::size_t, etc.

#ifdef TEST_WITH_CCONSOLE
#include "CConsole.h"  // CConsole lib: https://github.com/proof88/Console
#endif

class Test
{
public:

    static constexpr char* frameworkVersion = "1.2";

    /**
        Convenience function for running all test cases and summarizing the results.
        The idea is the following:
        - you define your tests by creating test cases in classes derived from either the UnitTest or the Benchmark class;
        - to run these tests i.e. invoke their run() method, either you write your own code or use this function by
          passing the vector containing your derived test class instances.

        You can see examples of this:
         - Benchmarks.cpp (in this repo)
         - https://github.com/proof88/PGE/blob/master/PGE/UnitTests/UnitTests.cpp
         - https://github.com/proof88/PRooFPS-dd/blob/main/PRooFPS-dd/Tests/PRooFPS-dd-Tests.cpp

        It needs a Console reference to output the summarized results.
        The TEST_WITH_CCONSOLE macro needs to be defined to use this function.
        The Console lib is this: https://github.com/proof88/Console .

        If you want to use your own test runner and summarizer implementation, then don't define the TEST_WITH_CCONSOLE macro before including Test.h.
    */
#ifdef TEST_WITH_CCONSOLE
    static void runTests(std::vector<std::unique_ptr<Test>>& tests, CConsole& console, const char* title = "")
    {
        console.OLn("%s", title);
        console.OLn("Powered by: 455-355-7357-88 (ASS-ESS-TEST-88) Test Framework by PR00F88, version: %s", frameworkVersion);

        size_t nSucceededTests = 0;
        size_t nTotalSubTests = 0;
        size_t nTotalPassedSubTests = 0;
        for (size_t i = 0; i < tests.size(); ++i)
        {
            console.OLn("Running test %d / %d ... ", i + 1, tests.size());
            tests[i]->run();
        }

        // summarizing
        console.OLn("");
        for (size_t i = 0; i < tests.size(); ++i)
        {
            for (const auto& infoMsg : tests[i]->getInfoMessages())
            {
                console.OLn("%s", infoMsg.c_str());
            }

            if (tests[i]->isPassed())
            {
                ++nSucceededTests;
                console.SOn();
                if (tests[i]->getName().empty())
                {
                    console.OLn("Test passed: %s(%d)!", tests[i]->getFile().c_str(), tests[i]->getSubTestCount());
                }
                else if (tests[i]->getFile().empty())
                {
                    console.OLn("Test passed: %s(%d)!", tests[i]->getName().c_str(), tests[i]->getSubTestCount());
                }
                else
                {
                    console.OLn("Test passed: %s(%d) in %s!", tests[i]->getName().c_str(), tests[i]->getSubTestCount(), tests[i]->getFile().c_str());
                }
                console.SOff();
            }
            else
            {
                console.EOn();
                if (tests[i]->getName().empty())
                {
                    console.OLn("Test failed: %s", tests[i]->getFile().c_str());
                }
                else if (tests[i]->getFile().empty())
                {
                    console.OLn("Test failed: %s", tests[i]->getName().c_str());
                }
                else
                {
                    console.OLn("Test failed: %s in %s", tests[i]->getName().c_str(), tests[i]->getFile().c_str());
                }
                console.Indent();
                for (size_t j = 0; j < tests[i]->getErrorMessages().size(); ++j)
                {
                    console.OLn("%s", tests[i]->getErrorMessages()[j].c_str());
                }
                console.Outdent();
                console.EOff();
            }
            nTotalSubTests += tests[i]->getSubTestCount();
            nTotalPassedSubTests += tests[i]->getPassedSubTestCount();
        }

        console.OLn("");
        console.OLn("========================================================");
        if (nSucceededTests == tests.size())
        {
            console.SOn();
        }
        else
        {
            console.EOn();
        }
        console.OLn("Passed tests: %d / %d (SubTests: %d / %d)", nSucceededTests, tests.size(), nTotalPassedSubTests, nTotalSubTests);
        console.NOn();
        console.OLn("========================================================");
        console.OLn("");
    } // runTests()
#endif

    /**
        @param testFile The file where the test is defined.
        @param testName The name of the test. If empty, itt will be "Unnamed Test".
    */
    Test(const std::string& testFile = "", const std::string& testName = "")
    {
        reset();
        if (testName.empty() && testFile.empty())
            sTestName = "Unnamed Test";
        else
            sTestName = testName;

        if (!testFile.empty())
            sTestFile = getFilename(testFile);
    }


    virtual ~Test()
    {
        // TODO: these are not needed
        sInfoMessages.clear();
        sErrorMessages.clear();
        tSubTests.clear();
    }

    Test(const Test&) = default;
    Test& operator=(const Test&) = default;
    Test(Test&&) = default;
    Test& operator=(Test&&) = default;


    const std::string& getName() const
    {
        return sTestName;
    }


    const std::string& getFile() const
    {
        return sTestFile;
    }


    /**
        @return Informational messages after run().
    */
    const std::vector<std::string>& getInfoMessages() const
    {
        return sInfoMessages;
    }


    /**
        Adds the given info message to the informational messages.
    */
    void addToInfoMessages(const char* msg)
    {
        sInfoMessages.push_back(msg);
    }


    /**
        @return Error messages after run().
    */
    const std::vector<std::string>& getErrorMessages() const
    {
        return sErrorMessages;
    }


    /**
        Adds the given error message to the error messages.
    */
    void addToErrorMessages(const char* msg)
    {
        sErrorMessages.push_back(msg);
    }


    /**
        Adds an "Assertion failed!" error message to the messages if the given statement is false.

        @param statement The statement being checked.
        @param msg       The optional message to be concatenated to the default error message.

        @return          The value of the given statement.
    */
    bool assertTrue(bool statement, const char* msg = NULL)
    {
        if (!statement)
        {
            if (msg == NULL)
            {
                addToErrorMessages("Assertion failed!");
            }
            else
            {
                addToErrorMessages(std::string("Assertion failed: ").append(msg).c_str());
            }
        }
        return statement;
    }


    /**
        Same as assertTrue(), except it expects the given statement to be false.

        @param statement The statement being checked.
        @param msg       The optional message to be concatenated to the default error message.

        @return          True if the given statement is false, false otherwise.
    */
    bool assertFalse(bool statement, const char* msg = NULL)
    {
        return assertTrue(!statement, msg);
    }


    /**
        Adds an error message if the given checked value doesn't equal to the expected value.

        @param expected The expected value.
        @param checked  The checked value.
        @param msg      Optional error message.

        @return         True if checked equals to expected, false otherwise.
    */
    template <class T, class S>
    bool assertEquals(const T& expected, const S& checked, const char* msg = NULL)
    {
        std::string asdasd;
        if (msg != NULL)
            asdasd = toString(checked).append(" should be ").append(toString(expected)).append(", ").append(msg);

        return msg == NULL ?
            assertTrue(expected == checked,
                toString(checked).append(" should be ").append(toString(expected)).append("!").c_str()) :
            assertTrue(expected == checked, asdasd.c_str());
    }


    /**
        Specialization of assertEquals() for chars to avoid treating 0 as \0, etc.

        @param expected The expected value.
        @param checked  The checked value.
        @param msg      Optional error message.

        @return         True if checked equals to expected, false otherwise.
    */
    bool assertEquals(unsigned char expected, unsigned char checked, const char* msg = NULL)
    {
        return assertEquals(static_cast<int>(expected), static_cast<int>(checked), msg);
    }

    bool assertEquals(char expected, unsigned char checked, const char* msg = NULL)
    {
        return assertEquals(static_cast<int>(expected), static_cast<int>(checked), msg);
    }

    bool assertEquals(unsigned char expected, char checked, const char* msg = NULL)
    {
        return assertEquals(static_cast<int>(expected), static_cast<int>(checked), msg);
    }

    bool assertEquals(char expected, char checked, const char* msg = NULL)
    {
        return assertEquals(static_cast<int>(expected), static_cast<int>(checked), msg);
    }


    /**
        Specialization of assertEquals() for floats. Notice the extra epsilon parameter.

        @param expected The expected value.
        @param checked  The checked value.
        @param epsilon  The maximum allowed difference between the expected and checked value.
        @param msg      Optional error message.

        @return         True if checked value is not farther from expected value than epsilon, false otherwise.
    */
    bool assertEquals(float expected, float checked, float epsilon, const char* msg = NULL)
    {
        return msg == NULL ?
            assertTrue(std::abs(expected - checked) <= epsilon,
                toString(checked).append(" should be ").append(toString(expected)).append("!").c_str()) :
            assertTrue(std::abs(expected - checked) <= epsilon,
                toString(checked).append(" should be ").append(toString(expected)).append(", ").append(msg).c_str());
    }


    /**
        Adds an error message if the given checked value equals to the other value.

        @param comparedTo The other value comparing to.
        @param checked    The checked value.
        @param msg        Optional error message.

        @return           True if checked doesn't equal to the other value, false otherwise.
    */
    template <class T, class S>
    bool assertNotEquals(const T& comparedTo, const S& checked, const char* msg = NULL)
    {
        return msg == NULL ?
            assertTrue(comparedTo != checked,
                toString(checked).append(" should NOT be ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue(comparedTo != checked,
                toString(checked).append(" should NOT be ").append(toString(comparedTo)).append(", ").append(msg).c_str());
    }


    /**
        Specialization of assertNotEquals() for chars to avoid treating 0 as \0, etc.

        @param comparedTo The other value comparing to.
        @param checked    The checked value.
        @param msg        Optional error message.

        @return           True if checked doesn't equal to the other value, false otherwise.
    */
    bool assertNotEquals(unsigned char comparedTo, unsigned char checked, const char* msg = NULL)
    {
        return assertNotEquals((int)comparedTo, (int)checked, msg);
    }


    /**
        Specialization of assertNotEquals() for floats. Notice the extra epsilon parameter.

        @param comparedTo The other value comparing to.
        @param checked    The checked value.
        @param epsilon    The maximum disallowed difference between the checked and other value.
        @param msg        Optional error message.

        @return           True if checked value is farther from the other value than epsilon, false otherwise.
    */
    bool assertNotEquals(float comparedTo, float checked, float epsilon, const char* msg = NULL)
    {
        return msg == NULL ?
            assertTrue(std::abs(comparedTo - checked) > epsilon,
                toString(checked).append(" should NOT be ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue(std::abs(comparedTo - checked) > epsilon,
                toString(checked).append(" should NOT be ").append(toString(comparedTo)).append(", ").append(msg).c_str());
    }


    /**
        Adds an error message if the given value is not inside the given interval.

        @param minVal  The start of the interval.
        @param maxVal  The end of the interval.
        @param checked The checked value.
        @param msg     Optional error message.

        @return True if the given value is inside the given interval, false otherwise.
    */
    template <class T, class S>
    bool assertBetween(const T& minVal, const T& maxVal, const S& checked, const char* msg = NULL)
    {
        return msg == NULL ?
            assertTrue(minVal <= checked && maxVal >= checked,
                std::string("out of range: ").append(toString(minVal)).append(" <= ").append(
                    toString(checked)).append(" <= ").append(toString(maxVal)).append(" !").c_str()) :
            assertTrue(minVal <= checked && maxVal >= checked,
                std::string("out of range: ").append(toString(minVal)).append(" <= ").append(
                    toString(checked)).append(" <= ").append(toString(maxVal)).append(", ").append(msg).c_str());
    }


    /**
        Specialization of assertBetween() for floats. Notice the extra epsilon parameter.
        Adds an error message if the given value is not inside the given interval.

        @param minVal  The start of the interval.
        @param maxVal  The end of the interval.
        @param checked The checked value.
        @param epsilon The maximum allowed absolute difference between the checked value and boundaries of the interval towards outside.
        @param msg     Optional error message.

        @return True if the given value is inside the given interval, false otherwise.
    */
    bool assertBetween(float minVal, float maxVal, float checked, float epsilon, const char* msg = NULL)
    {
        bool b = ((minVal < checked) || (std::abs(minVal - checked) <= epsilon)) &&
            ((maxVal > checked) || (std::abs(maxVal - checked) <= epsilon));

        return msg == NULL ?
            assertTrue(b,
                std::string("out of range: ").append(toString(minVal)).append(" <= ").append(
                    toString(checked)).append(" <= ").append(toString(maxVal)).append(" !").c_str()) :
            assertTrue(b,
                std::string("out of range: ").append(toString(minVal)).append(" <= ").append(
                    toString(checked)).append(" <= ").append(toString(maxVal)).append(", ").append(msg).c_str());
    }


    /**
        Adds an error message if the given value is not less than the other.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is less than the other value.
    */
    template <class T, class S>
    bool assertLess(const T& checked, const S& comparedTo, const char* msg = NULL)
    {
        std::string asdasd;
        if (msg != NULL)
            asdasd = toString(checked).append(" should be < ").append(toString(comparedTo)).append(", ").append(msg);

        return msg == NULL ?
            assertTrue(checked < comparedTo,
                toString(checked).append(" should be < ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue(checked < comparedTo, asdasd.c_str());
    }


    /**
        Specialization of assertLess() for chars to avoid treating 0 as \0, etc.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is less than the other value.
    */
    bool assertLess(unsigned char checked, unsigned char comparedTo, const char* msg = NULL)
    {
        return assertLess((int)checked, (int)comparedTo, msg);
    }


    /**
        Specialization of assertLess() for floats.
        Adds an error message if the given value is not less than the other.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is less than the other value.
    */
    bool assertLess(float checked, float comparedTo, const char* msg = NULL)
    {
        // TODO: currently this is same as the template function, as for some reason we do not use any epsilon in this specialization,
        // probably we can delete this specialization in the future ...
        return msg == NULL ?
            assertTrue(checked < comparedTo,
                toString(checked).append(" should be < ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue(checked < comparedTo,
                toString(checked).append(" should be < ").append(toString(comparedTo)).append(", ").append(msg).c_str());
    }


    /**
        Adds an error message if the given value is not less than or equal to the other.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is less than or equal to the other value.
    */
    template <class T, class S>
    bool assertLequals(const T& checked, const S& comparedTo, const char* msg = NULL)
    {
        std::string asdasd;
        if (msg != NULL)
            asdasd = toString(checked).append(" should be <= ").append(toString(comparedTo)).append(", ").append(msg);

        return msg == NULL ?
            assertTrue(checked <= comparedTo,
                toString(checked).append(" should be <= ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue(checked <= comparedTo, asdasd.c_str());
    }


    /**
        Specialization of assertLequals() for chars to avoid treating 0 as \0, etc.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is less than or equal to the other value.
    */
    bool assertLequals(unsigned char checked, unsigned char comparedTo, const char* msg = NULL)
    {
        return assertLequals((int)checked, (int)comparedTo, msg);
    }


    /**
        Specialization of assertLequals() for floats. Notice the extra epsilon parameter.
        Adds an error message if the given value is not less than or equal to the other.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param epsilon     The maximum allowed difference between the checked and the other value.
        @param msg         Optional error message.

        @return            True if checked value is less than or equal to the other value.
    */
    bool assertLequals(float checked, float comparedTo, float epsilon, const char* msg = NULL)
    {
        return msg == NULL ?
            assertTrue((checked < comparedTo) || (std::abs(comparedTo - checked) <= epsilon),
                toString(checked).append(" should be <= ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue((checked < comparedTo) || (std::abs(comparedTo - checked) <= epsilon),
                toString(checked).append(" should be <= ").append(toString(comparedTo)).append(", ").append(msg).c_str());
    }


    /**
        Adds an error message if the given value is not greater than the other.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is greater than the other value.
    */
    template <class T, class S>
    bool assertGreater(const T& checked, const S& comparedTo, const char* msg = NULL)
    {
        std::string asdasd;
        if (msg != NULL)
            asdasd = toString(checked).append(" should be > ").append(toString(comparedTo)).append(", ").append(msg);

        return msg == NULL ?
            assertTrue(checked > comparedTo,
                toString(checked).append(" should be > ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue(checked > comparedTo, asdasd.c_str());
    }


    /**
        Specialization of assertGreater() for chars to avoid treating 0 as \0, etc.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is greater than the other value.
    */
    bool assertGreater(unsigned char checked, unsigned char comparedTo, const char* msg = NULL)
    {
        return assertGreater((int)checked, (int)comparedTo, msg);
    }


    /**
        Specialization of assertGreater() for floats.
        Adds an error message if the given value is not greater than the other.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is greater than the other value.
    */
    bool assertGreater(float checked, float comparedTo, const char* msg = NULL)
    {
        // TODO: currently this is same as the template function, as for some reason we do not use any epsilon in this specialization,
        // probably we can delete this specialization in the future ...
        return msg == NULL ?
            assertTrue(checked > comparedTo,
                toString(checked).append(" should be > ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue(checked > comparedTo,
                toString(checked).append(" should be > ").append(toString(comparedTo)).append(", ").append(msg).c_str());
    }


    /**
        Adds an error message if the given value is not greater than or equal to the other.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is greater than or equal to the other value.
    */
    template <class T, class S>
    bool assertGequals(const T& checked, const S& comparedTo, const char* msg = NULL)
    {
        std::string asdasd;
        if (msg != NULL)
            asdasd = toString(checked).append(" should be >= ").append(toString(comparedTo)).append(", ").append(msg);

        return msg == NULL ?
            assertTrue(checked >= comparedTo,
                toString(checked).append(" should be >= ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue(checked >= comparedTo, asdasd.c_str());
    }


    /**
        Specialization of assertGequals() for chars to avoid treating 0 as \0, etc.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param msg         Optional error message.

        @return            True if checked value is greater than or equal to the other value.
    */
    bool assertGequals(unsigned char checked, unsigned char comparedTo, const char* msg = NULL)
    {
        return assertGequals((int)checked, (int)comparedTo, msg);
    }


    /**
        Specialization of assertGequals() for floats. Notice the extra epsilon parameter.
        Adds an error message if the given value is not greater than or equal to the other.

        @param checked     The checked value.
        @param comparedTo  The other value we compare to.
        @param epsilon     The maximum allowed difference between the checked and the other value.
        @param msg         Optional error message.

        @return            True if checked value is greater than or equal to the other value.
    */
    bool assertGequals(float checked, float comparedTo, float epsilon, const char* msg = NULL)
    {
        return msg == NULL ?
            assertTrue((checked > comparedTo) || (std::abs(comparedTo - checked) <= epsilon),
                toString(checked).append(" should be >= ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue((checked > comparedTo) || (std::abs(comparedTo - checked) <= epsilon),
                toString(checked).append(" should be >= ").append(toString(comparedTo)).append(", ").append(msg).c_str());
    }


    /**
        Adds an error message if the given pointer is not a null pointer.

        @param checked The checked pointer.
        @param msg     Optional error message.

        @return True if the given pointer is null, false otherwise.
    */
    bool assertNull(const void* checked, const char* msg = NULL)
    {
        return msg == NULL ? assertTrue(checked == (void*)0, "pointer should be NULL") :
            assertTrue(checked == (void*)0, std::string("pointer should be NULL, ").append(msg).c_str());
    }


    /**
        Adds an error message if the given pointer is a null pointer.

        @param checked The checked pointer.
        @param msg     Optional error message.

        @return True if the given pointer is not null, false otherwise.
    */
    bool assertNotNull(const void* checked, const char* msg = NULL)
    {
        return msg == NULL ? assertTrue(checked != (void*)0, "pointer is NULL") :
            assertTrue(checked != (void*)0, std::string("pointer is NULL, ").append(msg).c_str());
    }


    /**
        @return True if the test has passed, false otherwise.
    */
    bool isPassed() const
    {
        return bTestRan && sErrorMessages.empty();
    }


    /**
        Executes the test. Runs the overridable testMethod() and every unit-subtests.
        The functions are called in the following order:
         - initialize();
         - if ( setUp() )
           - testMethod();
         - tearDown();
         - for all subtests:
           - if ( setUp() )
             - subtest();
           - tearDown();
         - finalize();

        @return True if the test including all subtests passed, false otherwise.
    */
    bool run()
    {
        reset();
        bTestRan = true;
        initialize();
        bool bSkipAllSubTests;
        preSetUp();
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
        postTearDown();

        // subtests start
        if (!bSkipAllSubTests)
        {
            bWeAreInSubTest = true;
            for (size_t i = 0; i < tSubTests.size(); ++i)
            {
                iCurrentSubTest = i;
                preSetUp();
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
                postTearDown();
            }
            bWeAreInSubTest = false;
        }
        // subtests ended

        finalize();
        return isPassed();
    }


    /**
        @return Returns the number of subtests in the test.
    */
    int getSubTestCount() const
    {
        return tSubTests.size();
    }


    /**
        @return Returns the number of passed subtests in the test.
    */
    int getPassedSubTestCount() const
    {
        return nSucceededSubTests;
    }


    /**
        Convenience function for logging/printing, for example, if we want to print the name of the currently running subtest.
        Being called outside from a subtest can cause either assertion failure (debug build) or thrown exception (release build).

        @return Name of currently running subtest.
                Valid already in the setUp() and tearDown() phases of the subtest.
    */
    const std::string& getCurrentSubTestName() const
    {
        assert(iCurrentSubTest < tSubTests.size());
        tSubTests[iCurrentSubTest].second;
    }


    /**
        Convenience function to know if we are running a subtest or not.

        @return True if we are in a subtest or its corresponding setUp(), tearDown() or printBenchmarkers(), false otherwise.
    */
    const bool& isSubTestRunning() const
    {
        return bWeAreInSubTest;
    }


protected:
    typedef bool (Test::* PFNUNITSUBTEST) (void);  /**< Type for a unit-subtest function pointer. */

    // ---------------------------------------------------------------------------

    /**
        This gets called before running the test, use this instead of a ctor.
        Test developer implements it in derived test class.
    */
    virtual void initialize() {};

    /**
        This may be overridden in derived test class.
        It won't be invoked if setUp() returns false!
        Test developer implements it in derived test class.
    */
    virtual bool testMethod() { return true; }; 

    /**
        This gets called before testMethod() and every subtest.
        Test developer implements it in derived test class.
    */
    virtual bool setUp() { return true; };

    /**
        This gets called after testMethod() and every subtest.
        This is invoked even if testMethod() or a subtest got skipped due to setUp() returned false!
        Test developer implements it in derived test class.
    */
    virtual void tearDown() {};

    /**
        This gets called after running all tests, use this instead of a dtor.
        Test developer implements it in derived test class.
    */
    virtual void finalize() {};

    /**
        Adds the given unit-subtest function with the given name to the test.
        Test developer can use it to add defined subtests within the derived test class.
        Such subtests must be member functions of the derived test class.
    */
    void addSubTest(const char* subTestName, PFNUNITSUBTEST subTestFunc)
    {
        if (subTestFunc == nullptr)
        {
            return;
        }

        TUNITSUBTESTFUNCNAMEPAIR newPair(subTestFunc, std::string(subTestName));
        tSubTests.push_back(newPair);
    } // addSubTest()

    /**
        Invoked by run() right before any call to setUp().
        To be implemented by a specific test type within this test framework: see class Benchmark as example.
        The test framework can implement test-type specific initialization here.
    */
    virtual void preSetUp()
    {}

    /**
        Invoked by run() right before any call to tearDown().
        To be implemented by a specific test type within this test framework: see class Benchmark as example.
        The test framework can implement test-type specific teardown here.
    */
    virtual void postTearDown()
    {}

protected:
    typedef std::pair<PFNUNITSUBTEST, std::string> TUNITSUBTESTFUNCNAMEPAIR;   /**< Subtest function pointer and subtest name pair. */

    // ---------------------------------------------------------------------------

    std::string sTestName;                             /**< Name of the test. */
    std::string sTestFile;                             /**< Testfile name. */
    std::vector<std::string> sErrorMessages;           /**< Error messages. */
    std::vector<std::string> sInfoMessages;            /**< Informational messages. */
    std::vector<TUNITSUBTESTFUNCNAMEPAIR> tSubTests;   /**< Subtests as filled by addSubTest(). */
    size_t iCurrentSubTest;                            /**< Index of currently running subtest, valid only if bWeAreInSubTest is true. */
    bool bWeAreInSubTest;                              /**< True only if a subtest is running, valid also in the subtest's corresponding setUp(), tearDown() and printBenchmarkers(). */
    int nSucceededSubTests;                            /**< Number of succeeded subtests. */
    bool bTestRan;                                     /**< Did the test attempt to run? */

    // ---------------------------------------------------------------------------

    template <class T>
    static std::string toString(const T& value)
    {
        std::stringstream str;
        str << value;
        return str.str();
    }

    static std::string toString(bool value)
    {
        return value ? "TRUE" : "FALSE";
    }

    static std::string getFilename(const std::string& path)
    {
        return path.substr(path.find_last_of('\\') + 1);
    }

    /**
        Resets the test so it gets into a rerunnable state.
    */
    void reset()
    {
        bTestRan = false;
        sErrorMessages.clear();
        sInfoMessages.clear();
        iCurrentSubTest = 0;
        bWeAreInSubTest = false;
        nSucceededSubTests = 0;
    }

}; // class Test
