#pragma once

#ifdef DEBUG
#define O_LOG(...) LOG("OpenGL", __VA_ARGS__)
#define O_WARN(...) WARN("OpenGL", __VA_ARGS__)
#define O_ERROR(...) ERROR("OpenGL", __VA_ARGS__)
#else
#define O_LOG(x) 
#define O_WARN(x) 
#define O_ERROR(x) 
#endif
