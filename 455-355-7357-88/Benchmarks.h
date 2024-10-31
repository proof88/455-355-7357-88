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

#include <cassert>
#include <math.h>   // TODO: change it to cmath
#include <chrono>   // seconds, milliseconds, now(), etc.; requires cpp11
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <utility>  // std::size_t, etc.

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

class Benchmark
{
public:

    /**
        @param testFile The file where the test is defined.
        @param testName The name of the test. If empty, itt will be "Unnamed Test".
    */
    Benchmark(const std::string& testFile = "", const std::string& testName = "")
    {
        reset();
        if (testName.empty() && testFile.empty())
            sTestName = "Unnamed Test";
        else
            sTestName = testName;

        if (!testFile.empty())
            sTestFile = getFilename(testFile);
    }


    virtual ~Benchmark()
    {
        // these are not needed
        sInfoMessages.clear();
        sErrorMessages.clear();
        tSubTests.clear();
    }


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
            assertTrue(abs(expected - checked) <= epsilon,
                toString(checked).append(" should be ").append(toString(expected)).append("!").c_str()) :
            assertTrue(abs(expected - checked) <= epsilon,
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
            assertTrue(abs(comparedTo - checked) > epsilon,
                toString(checked).append(" should NOT be ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue(abs(comparedTo - checked) > epsilon,
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
        bool b = ((minVal < checked) || (abs(minVal - checked) <= epsilon)) &&
            ((maxVal > checked) || (abs(maxVal - checked) <= epsilon));

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
            assertTrue((checked < comparedTo) || (abs(comparedTo - checked) <= epsilon),
                toString(checked).append(" should be <= ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue((checked < comparedTo) || (abs(comparedTo - checked) <= epsilon),
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
            assertTrue((checked > comparedTo) || (abs(comparedTo - checked) <= epsilon),
                toString(checked).append(" should be >= ").append(toString(comparedTo)).append("!").c_str()) :
            assertTrue((checked > comparedTo) || (abs(comparedTo - checked) <= epsilon),
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
    bool run()
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
        if (!bSkipAllSubTests)
        {
            for (std::vector<TUNITSUBTESTFUNCNAMEPAIR>::size_type i = 0; i < tSubTests.size(); ++i)
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
        }
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


protected:
    typedef bool (Benchmark::* PFNUNITSUBTEST) (void);  /**< Type for a unit-subtest function pointer. */

    // ---------------------------------------------------------------------------

    virtual void initialize() {}; /**< This gets called before running the test, use this instead of a ctor. */

    virtual bool testMethod() { return true; }; /**< This may be overridden in actual unit tests. It won't be invoked if setUp() returned false! */

    virtual bool setUp() { return true; };   /**< This gets called before testMethod() and every subtest. */

    virtual void tearDown() {};   /**< This gets called after testMethod() and every subtest.
                                       This is invoked even if testMethod() or a subtest got skipped due to setUp() returned false! */

    virtual void finalize() {};   /**< This gets called after running the whole tests, use this instead of a dtor. */

    /**
        Adds the given unit-subtest function with the given name to the test.
    */
    void addSubTest(const char* subTestName, PFNUNITSUBTEST subTestFunc)
    {
        if (subTestFunc == NULL)
            return;

        TUNITSUBTESTFUNCNAMEPAIR newPair(subTestFunc, std::string(subTestName));
        tSubTests.push_back(newPair);
    } // addSubTest()

private:
    typedef std::pair<PFNUNITSUBTEST, std::string> TUNITSUBTESTFUNCNAMEPAIR;   /**< Unit-subtest function pointer and subtest name pair. */

    // ---------------------------------------------------------------------------

    static Benchmark utInstance;  // TODO: why this is here? Not even used?

    std::string sTestName;                             /**< Name of the test. */
    std::string sTestFile;                             /**< Testfile name. */
    std::vector<std::string> sErrorMessages;           /**< Error messages. */
    std::vector<std::string> sInfoMessages;            /**< Informational messages. */
    std::vector<TUNITSUBTESTFUNCNAMEPAIR> tSubTests;   /**< Unit-subtests, not mandatory. */
    size_t iCurrentSubTest;                            /**< Index of currently running unit-subtest, valid only if tSubTests is NOT empty and a subtest is running. */
    int nSucceededSubTests;                            /**< Number of succeeded unit-subtests. */
    bool bTestRan;                                     /**< Did the test run? */

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

        addToInfoMessages("Scope Benchmarkers:");
        for (const auto& bmData : ScopeBenchmarker::getAllData())
        {
            addToInfoMessages(
                ("  " +
                    bmData.second.m_name +
                    " Iterations: " + std::to_string(bmData.second.m_iterations) +
                    ", Min/Max/Avg Duration: " +
                    std::to_string(bmData.second.m_durationsMinMillisecs) + "/" +
                    std::to_string(bmData.second.m_durationsMaxMillisecs) + "/" +
                    std::to_string(bmData.second.getAverageDurationMillisecs()) +
                    ", Total Duration: " + std::to_string(bmData.second.m_durationsTotalMillisecs)).c_str());
        }

        ScopeBenchmarker::clear(); // since ScopeBenchmarker works with static data, make sure we dont leave anything there
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
        nSucceededSubTests = 0;
    }

}; // class Benchmarks
