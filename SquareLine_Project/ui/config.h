#pragma once


//Number of screens
#define NUM_SCREENS 4

//Test mode 
extern bool g_test_mode;
inline bool is_test_mode()
{
    return g_test_mode;
}

inline void set_test_mode(bool enabled)
{
    g_test_mode = enabled;
}

inline void toggle_test_mode()
{
    g_test_mode = !g_test_mode;
}