#pragma once

/*
    ###################################################################################
    UnitTest.h
    Basic header-only Unit Test class.
    Part of the 455-355-7357-88 (ASS-ESS-TEST-88) test suite.
    Made by PR00F88
    2014
    ###################################################################################
*/

#include "Test.h"

/**
    A note/statement about using & bitwise operator on bool operands.
    In some unit tests I do "bitwise and" operation with multiple bool operands.
    I intentionally do this to make sure all operands are evaluated: if an assertion fails, I would still like to see the results of
    all the other assertions too.
    
    In this comment I'm justifying the safety of using "bitwise and" operation with bool operands.
    
    This is C++. In C++, bool true is implicitly converted to 1, bool false is implicitly converted to 0.
    The standard says "If the source type is bool, the value false is converted to zero and the value true is converted to one".
    Hence the "bitwise and" operations in my tests are operating with well-defined values (0,1).
    
    The standard also says that "A zero value, null pointer value, or null member pointer value is converted to false; any other value is converted to true".
    Hence assigning the result of "bitwise and" operation to a bool will have well-defined result in my unit tests.
    
    So this will compile and run as expected and as I intended:
    
      bool bResult = assertTrue(someFunction()) & assertTrue(someOtherFunction());
    
    Visual Studio is marking it as a potential problem: "Using bitwise '&' when logical '&&' was probably intended".
    This is a very useful warning. To get rid of it i.e. to assure Visual Studio I know what I'm doing, I'm changing such conditions to this:
    
      bool bResult = (assertTrue(someFunction()) & assertTrue(someOtherFunction())) != 0;
    
    So that I will have a bResult telling me if the "bitwise and" operation resulted in zero or not.
    
    Note that when using "bitwise and" instead of "logical and", we cannot be sure about the evaluation order of operands.
    Thus neither the assertions nor the evaluated expressions within the assertions should have any side-effect of affecting
    each other's results, the results should be identical in arbitrary evaluated order.
    This responsibility is obviously on the unit test developer.
*/

/**
    What I have learned from TDD training:
    - as implementation needs to be more generic than specific, unit test cases need to be more specific than generic;
    - unit tests should express usage of the tested code;
    - name of the unit test cases should be well readable, contain proposals and assertions, e.g.:
      instead of testObject3DCtor(), name should be testThatObjectIsInitiallyEmpty().
	  Jon Jagger recommends snake case naming convention for these long names, e.g. test_that_object_is_initially_empty();
    - unit tests don't need to go down to function-level. I can choose the unit, but still need to avoid heavyweight testing;
    - unit test is not neccessarily small but FAST test (ALL tests execution time <30 seconds);
    - since we are continuously creating tests and doing the implementation, test execution speed is really crucial, fast feedback is needed anytime on-demand;
    - unit tests should offer high code coverage, including error handling paths as well, so both success and failure scenarios;
    - unit tests are automated;
    - unit tests need to be deterministic (no intermittent outcome);
    - unit tests are independent of each other, can be run in arbitrary order;
    - unit tests shouldn't touch database, filesystem, network, etc., instead test doubles (stubs/spies/mocks/fakes/dummies) should be used;
    - if database, filesystem, network, etc. is used so the test is a little bit slower, it can be unit test also, but may be marked as "slow", so
      a different set of tests can be run on-demand (fast tests), and then sometimes all tests including the "slow" tests.
*/

/**
    This class should be the base of every unit tester class.
    It should be used in the following way:
    1. - Create the actual unit test class by deriving from this class (mandatory);
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


    Example unit test class:

        class ColorTest :
            public UnitTest
        {
        public:

            ColorTest() :
                UnitTest( __FILE__ )
            {
                addSubTest("testCtor", (PFNUNITSUBTEST) &ColorTest::testCtor);
                addSubTest("testGetRed", (PFNUNITSUBTEST) &ColorTest::testGetRed);
                ...
            }

        private:

            bool testCtor()
            {
                const Color clr(1,2,3,4);
                return assertEquals(1, (int) clr.getRed(), "red") &
                    assertEquals(2, (int) clr.getGreen(), "green") &
                    assertEquals(3, (int) clr.getBlue(), "blue") &
                    assertEquals(4, (int) clr.getAlpha(), "alpha");        
            }

            bool testGetRed()
            {
                const Color clr(1,2,3,4);
                return assertEquals(1, clr.getRed());
            }

            ...

        };
*/

class UnitTest : public Test
{
public:

    /**
        @param testFile The file where the test is defined.
        @param testName The name of the test. If empty, itt will be "Unnamed Test".
    */
    UnitTest(const std::string& testFile = "", const std::string& testName = "") :
        Test(testFile, testName)
    {} 

}; // class UnitTest
