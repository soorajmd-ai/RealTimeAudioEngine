#pragma once

#ifdef COREDSP_EXPORTS
#define COREDSP_API __declspec(dllexport)
#else
#define COREDSP_API __declspec(dllimport)
#endif