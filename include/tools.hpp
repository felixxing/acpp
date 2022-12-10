#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <fstream>
#include <sstream>
#include <chrono>
#include <random>

class Load_File
{
  private:
    std::string content;
    const char* data;

  public:
    Load_File(const std::string path);

    const std::string& get_str();
    const char*& get_data();
};

inline Load_File::Load_File(const std::string path)
{
    std::ifstream in_file(path);
    std::stringstream ss;
    ss << in_file.rdbuf();
    content = ss.str();
    data = content.data();
}

inline const std::string& Load_File::get_str()
{
    return content;
}

inline const char*& Load_File::get_data()
{
    return data;
}

class Timer
{
  private:
    std::chrono::high_resolution_clock::time_point begin_point;
    std::chrono::high_resolution_clock::time_point end_point;

  public:
    float duration_s;
    unsigned int duration_ms;

    void start();
    void finish();
};

// timer implementation
inline void Timer::start()
{
    begin_point = std::chrono::high_resolution_clock::now();
}

inline void Timer::finish()
{
    end_point = std::chrono::high_resolution_clock::now();
    auto ss = std::chrono::duration_cast<std::chrono::milliseconds>(end_point - begin_point);
    duration_ms = ss.count();
    duration_s = (float)duration_ms / 1000;
}

inline float get_random(float l, float r)
{
    static std::default_random_engine eng;
    static std::uniform_real_distribution dist6(l, r);

    return dist6(eng);
}

inline float get_random(float r)
{
    return get_random(0, r);
}

#endif // TOOLS_HPP
