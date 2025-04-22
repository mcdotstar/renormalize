#pragma once
#include <mcpl.h>

#ifdef WIN32
// Export symbols if compile flags "RENORMALIZE_SHARED" and "RENORMALIZE_EXPORT" are set on Windows.
    #ifdef RENORMALIZE_SHARED
        #ifdef RENORMALIZE_EXPORT
            #define RL_API __declspec(dllexport)
        #else
            #define RL_API __declspec(dllimport)
        #endif
    #else
        // Disable definition if linking statically.
        #define RL_API
    #endif
#else
// Disable definition for non-Win32 systems.
#define RL_API
#endif

#ifdef __cplusplus
extern "C" {
#include <cstdint>
#include <cstddef>
#endif

RL_API const char * renormalize_key();

RL_API uint64_t get_renormalize_particle_count(mcpl_file_t mcpl_file);

RL_API int set_renormalize_particle_count(mcpl_outfile_t file, uint64_t count);

RL_API int renormalize_merge_files(const char * output, const char ** input, size_t count);

#ifdef __cplusplus
}
#endif