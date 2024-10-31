# 455-355-7357-88

Also known as ASS-ESS-TEST-88 framework.

## Intro

A very basic header-only framework for unit testing and benchmarking in C++.  
It is very basic, and does not support mocking and other fancy stuff. Basically it is just a bunch of assert functions with convenient logging the results.  
And even the benchmarking part is very limited, basically a scope time measurement class.

Depends on the following libraries:
 - [PFL](https://github.com/proof88/PFL)
 - [CConsole](https://github.com/proof88/Console).

## Recommendation

I don't recommend you to use this framework!  
I use it because I've been already using it for 10 years, and it would take a lot of time to migrate my existing unit tests to a different framework.  
If you want unit testing and performance measurement, find a better and more sophisticated library, for example:
 - [gtest](https://github.com/google/googletest)
 - [Google Benchmark](https://github.com/google/benchmark).

## Examples

You can find various example uses of my UnitTest class, [for example in the repository of PGE](https://github.com/proof88/PGE/blob/master/PGE/UnitTests/).  
The Visual Studio project file is included.  
However, if you want to see example of integration in other projects, you may be interested in the Visual Studio solution including other projects as well in [PGE-misc](https://github.com/proof88/PGE-misc) repo.

## Story

Around 2012, even before getting the "junior software developer" title, I decided to create a simple unit tester class in C++ because I wanted to mimic a Java unit tester framework I gained experience with.  
Today, I would immediately use an existing and better framework for that, but at that time, I wanted to create my own solution.  
I'm still using my framework in my own projects so I decided to finally move it into this separate repo.  

**Origin of the repo name:**  
The name of the repo is the [1337/leet](https://en.wikipedia.org/wiki/Leet) -style of "ASS-ESS-TEST", because I realized that the word "assess" looks cool with leet-style, then I appended the word "test" also in leet-style to it, and finally appended "88" because usually I also put that in my user name.
