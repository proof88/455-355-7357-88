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
#include <limits>
#include <map>
#include <string>
#include <utility>

#include "PFL.h"  // for PFL::StringHash

/**
* Basic header-only scope benchmarker class to conveniently stopwatch a scope.
* 
* For example usage, see Benchmarks.cpp.
*/
class ScopeBenchmarker
{
public:

    struct BmData
    {
        std::string m_name;
        long long m_durationsTotalMillisecs = 0;
        long long m_durationsMinMillisecs = LLONG_MAX;
        long long m_durationsMaxMillisecs = 0;
        long long m_iterations = 0;

        float getAverageDurationMillisecs() const
        {
            return m_iterations == 0 ?
                0.f :
                m_durationsTotalMillisecs / static_cast<float>(m_iterations);
        }

        void reset()
        {
            m_durationsTotalMillisecs = 0;
            m_durationsMinMillisecs = LLONG_MAX;
            m_durationsMaxMillisecs = 0;
            m_iterations = 0;
        }
    };

    static std::map<PFL::StringHash, BmData>& getAllData()
    {
        // originally s_scopeBenchmarkersData was static member, but that way you cannot create header-only stuff, so
        // there are different tricks for that and this local static variable is the most convenient!
        static std::map<PFL::StringHash, BmData> s_scopeBenchmarkersData;
        return s_scopeBenchmarkersData;
    }

    static BmData& getDataByNameHash(const PFL::StringHash& hash)
    {
        return getAllData()[hash];
    }

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

    ScopeBenchmarker(const std::string& name)
    {
        if (name.empty())
        {
            throw std::runtime_error("ScopeBenchmarker ctor: name cannot be empty!");
        }

        m_nameHash = PFL::calcHash(name);
        auto& bmData = getDataByNameHash(m_nameHash);
        bmData.m_iterations++;
        // We could debate what is faster from performance perspective:
        // - copy string everytime there (brute-force)
        // - copy string only if target is empty (hash already provides uniqueness so we are sure if target is non-empty, it already equals to our input name)
        // I should profile this but not now!
        bmData.m_name = name;
        m_timeStartScope = std::chrono::steady_clock::now();
    }

    ~ScopeBenchmarker()
    {
        const auto thisDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_timeStartScope).count();
        auto& bmData = getDataByNameHash(m_nameHash);
        bmData.m_durationsTotalMillisecs += thisDuration;
        if (thisDuration < bmData.m_durationsMinMillisecs)
        {
            bmData.m_durationsMinMillisecs = thisDuration;
        }
        if (thisDuration > bmData.m_durationsMaxMillisecs)
        {
            bmData.m_durationsMaxMillisecs = thisDuration;
        }
    }

    ScopeBenchmarker(const ScopeBenchmarker&) = default;
    ScopeBenchmarker& operator=(const ScopeBenchmarker&) = default;
    ScopeBenchmarker(ScopeBenchmarker&&) = default;
    ScopeBenchmarker& operator=(ScopeBenchmarker&&) = default;

private:

    PFL::StringHash m_nameHash;                                              /**< Key to s_scopeBenchmarkersData. */
    std::chrono::time_point<std::chrono::steady_clock> m_timeStartScope;     /**< Timestamp of scope beginning. */
};
