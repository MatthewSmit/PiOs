#pragma once

typedef signed char int8_t;
static_assert(sizeof(int8_t) == 1);

typedef unsigned char uint8_t;
static_assert(sizeof(uint8_t) == 1);

typedef signed int int32_t;
static_assert(sizeof(int32_t) == 4);

typedef unsigned int uint32_t;
static_assert(sizeof(uint32_t) == 4);

typedef signed long long int64_t;
static_assert(sizeof(int64_t) == 8);

typedef unsigned long long uint64_t;
static_assert(sizeof(uint64_t) == 8);

typedef unsigned long long size_t;
static_assert(sizeof(size_t) == 8);