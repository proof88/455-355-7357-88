#pragma once

/*
    ###################################################################################
    ScopeBenchmarker.h
    Basic header-only scope benchmarker class to conveniently stopwatch a scope.
    Part of the 455-355-7357-88 (ASS-ESS-TEST-88) test framework.
    Made by PR00F88
    2024
    ###################################################################################
*/

#include <chrono>    // seconds, milliseconds, now(), etc.; requires cpp11
#include <cstdint>   // intmax_t
#include <limits>    // LLONG_MAX
#include <map>
#include <string>

#include "PFL.h"  // for PFL::StringHash

/**
* Class for handling the global container of scope benchmarkers.
* The actual class is derived from this.
* With this code segregation, Benchmark class can use these static functions without specifying template argument
* that is requred for the derived ScopeBenchmarker class.
*/
class ScopeBenchmarkerDataStore
{
public:

    struct BmData
    {
        std::string m_name;
        long long m_durationsTotal = 0;        /** Time unit (sec, millisec, etc.) is the actual template parameter DurationType when instantiating ScopeBenchmarker. */
        long long m_durationsMin = LLONG_MAX;  /** Time unit (sec, millisec, etc.) is the actual template parameter DurationType when instantiating ScopeBenchmarker. */
        long long m_durationsMax = 0;          /** Time unit (sec, millisec, etc.) is the actual template parameter DurationType when instantiating ScopeBenchmarker. */
        long long m_iterations = 0;            /** Number of entering the scope (code block) measured by this benchmarker. */
        // using intmax_t because std::ratio also uses it for numerator and denominator
        intmax_t m_ratioDenominator = 0;       /** Denominator of the std::ratio of DurationType passed to ScopeBenchmarker.
                                                   We need this for printing unit of measure.
                                                   0 is obviously invalid value, only non-0 values can be formatted to valid string. */

        static const char* getUnitString(const intmax_t& ratioDenominator)
        {
            switch (ratioDenominator)
            {
            case 1: return "s";
            case 1000: return "ms";
            case 1000000: return "us";
            case 1000000000: return "ns";
            default: return "";
            }
        }

        const char* getUnitString() const
        {
            return getUnitString(m_ratioDenominator);
        }

        /**
        * @return Simply the total duration divided by the number of iterations (entering the measured scope).
        *         Time unit (sec, millisec, etc.) is the actual template parameter DurationType when instantiating ScopeBenchmarker.
        */
        float getAverageDuration() const
        {
            return m_iterations == 0 ?
                0.f :
                m_durationsTotal / static_cast<float>(m_iterations);
        }

        void reset()
        {
            m_durationsTotal = 0;
            m_durationsMin = LLONG_MAX;
            m_durationsMax = 0;
            m_iterations = 0;
        }
    };

    /**
    * @return All globally stored benchmark data.
    */
    static std::map<PFL::StringHash, BmData>& getAllData()
    {
        // originally s_scopeBenchmarkersData was static member, but that way you cannot create header-only stuff, so
        // there are different tricks for that and this local static variable is the most convenient!
        static std::map<PFL::StringHash, BmData> s_scopeBenchmarkersData;
        return s_scopeBenchmarkersData;
    }

    /**
    * @param  hash Benchmarker name hash to search for.
    * @return Returns measurement-specific data by the given benchmarker name hash.
    *         If such does not exist yet, a new entry with given name will be created and returned with default values.
    */
    static BmData& getDataByNameHash(const PFL::StringHash& hash)
    {
        return getAllData()[hash];
    }

    /**
    * @param  hash Benchmarker name to search for.
    * @return Returns measurement-specific data by the given benchmarker name.
    *         If such does not exist yet, a new entry with given name will be created and returned with default values.
    */
    static BmData& getDataByName(const std::string& name)
    {
        return getDataByNameHash(PFL::calcHash(name));
    }

    /**
    * Resets all measurement-specific data in stored benchmarkers.
    * Does not delete any of the stored benchmarkers.
    */
    static void resetAll()
    {
        for (auto& bmData : getAllData())
        {
            bmData.second.reset();
        }
    }

    /**
    * Deletes all stored benchmarkers.
    */
    static void clear()
    {
        getAllData().clear();
    }
};


/**
* Basic header-only scope benchmarker class to conveniently stopwatch a scope (code block).
*
* The template parameter DurationType should be the time measurement unit, e.g. std::chrono::milliseconds.
* Note that DurationType shall be fine-grained enough to properly measure min/max/avg/total values.
* For example, if you specify std::chrono::seconds, but your measured code always finishes within milliseconds,
* it won't be properly measured, so you should use std::chrono::milliseconds or std::chrono::macroseconds.
* 
* Improvement idea: durations should be always measured in macro- or nanoseconds, and then those values should be
* converted to DurationType upon evaluating the results, this way the user could specify arbitrary DurationType, the
* measurements would stay precise.
* 
* For example usage, see Benchmarks.cpp.
*/
template <typename DurationType>
class ScopeBenchmarker : public ScopeBenchmarkerDataStore
{
public:

    static_assert(std::chrono::_Is_duration_v<DurationType>, "DurationType template argument must be of std::duration!");
    ScopeBenchmarker(const std::string& name)
    {
        if (name.empty())
        {
            throw std::runtime_error("ScopeBenchmarker ctor: name cannot be empty!");
        }

        m_nameHash = PFL::calcHash(name);
        auto& bmData = getDataByNameHash(m_nameHash);

        bmData.m_iterations++;
        assert(bmData.m_iterations > 0);
        if (bmData.m_iterations <= 0)
        {
            throw std::runtime_error("ScopeBenchmarker ctor: m_iterations overflew!");
        }

        // We could debate what is faster from performance perspective:
        // - copy string everytime there (brute-force)
        // - copy string only if target is empty (hash already provides uniqueness so we are sure if target is non-empty, it already equals to our input name)
        // I should profile this but not now!
        bmData.m_name = name;
        bmData.m_ratioDenominator = DurationType::period::den;
        m_timeStartScope = std::chrono::steady_clock::now();
    }

    ~ScopeBenchmarker()
    {
        const auto thisDurationCount = std::chrono::duration_cast<DurationType>(std::chrono::steady_clock::now() - m_timeStartScope).count();
        auto& bmData = getDataByNameHash(m_nameHash);

        bmData.m_durationsTotal += thisDurationCount;

        assert(bmData.m_durationsTotal >= 0);
        
        // dtor cannot throw
        //if (bmData.m_durationsTotal < 0)
        //{
        //    throw std::runtime_error("ScopeBenchmarker dtor: m_durationsTotal overflew!");
        //}

        if (thisDurationCount < bmData.m_durationsMin)
        {
            bmData.m_durationsMin = thisDurationCount;
        }
        if (thisDurationCount > bmData.m_durationsMax)
        {
            bmData.m_durationsMax = thisDurationCount;
        }
    }

    ScopeBenchmarker(const ScopeBenchmarker&) = default;
    ScopeBenchmarker& operator=(const ScopeBenchmarker&) = default;
    ScopeBenchmarker(ScopeBenchmarker&&) = default;
    ScopeBenchmarker& operator=(ScopeBenchmarker&&) = default;

private:

    PFL::StringHash m_nameHash;                                              /**< Key to ScopeBenchmarkerDataStore::getAllData(). */
    std::chrono::time_point<std::chrono::steady_clock> m_timeStartScope;     /**< Timestamp of scope beginning. */
};
