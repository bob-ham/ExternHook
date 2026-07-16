#pragma once
#include <map>
#include "process.h"
#include "hook.h"
#include "mmap.h"



struct HookLibrary
{
	Process* process = nullptr;
	std::map<void*, Hook> registry;
	Mapper::MapResult lastMapResult = { 0, 0, false };
};

static HookLibrary g_Lib;