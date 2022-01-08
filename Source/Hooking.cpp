//Hooking.cpp
#pragma once
#include "stdafx.h"


using namespace Memory;

ScriptThread*(*GetActiveThread)() = nullptr;
HMODULE _hmoduleDLL;
HANDLE mainFiber;
DWORD wakeAt;

std::vector<LPVOID>		Hooking::m_hooks;
uint64_t*				Hooking::m_frameCount;
fpIsDLCPresent			Hooking::is_DLC_present;
static eGameState* 											m_gameState;
static uint64_t												m_worldPtr;
static BlipList*											m_blipList;
static Hooking::NativeRegistrationNew**						m_registrationTable;
static std::unordered_map<uint64_t, Hooking::NativeHandler>	m_handlerCache;
static std::vector<LPVOID>									m_hookedNative;
static __int64**                                            m_globalPtr;

const int EVENT_COUNT = 78;
static std::vector<void*> EventPtr;
static char EventRestore[EVENT_COUNT] = {};


/* Start Hooking */
void Hooking::Start(HMODULE hmoduleDLL)
{
	_hmoduleDLL = hmoduleDLL;
	Log::Init(hmoduleDLL);
	FindPatterns();
	if (!InitializeHooks()) Cleanup();
}
BOOL Hooking::InitializeHooks()
{
	BOOL returnVal = TRUE;

	if (!iHook.Initialize()) {

		Log::Error("Failed to initialize InputHook");
		returnVal = FALSE;
	}

	if (MH_Initialize() != MH_OK) {
		Log::Error("MinHook failed to initialize");
		returnVal = FALSE;
	}

	if (!HookNatives()) {

		Log::Error("Failed to initialize NativeHooks");
		returnVal = FALSE;
	}

	return returnVal;
}

template <typename T>
bool Native(DWORD64 hash, LPVOID hookFunction, T** trampoline)
{
	if (*reinterpret_cast<LPVOID*>(trampoline) != NULL)
		return true;
	auto originalFunction = Hooking::GetNativeHandler(hash);
	if (originalFunction != 0) {
		MH_STATUS createHookStatus = MH_CreateHook(originalFunction, hookFunction, reinterpret_cast<LPVOID*>(trampoline));
		if (((createHookStatus == MH_OK) || (createHookStatus == MH_ERROR_ALREADY_CREATED)) && (MH_EnableHook(originalFunction) == MH_OK))
		{
			m_hookedNative.push_back((LPVOID)originalFunction);
			DEBUGMSG("Hooked Native 0x%#p", hash);
			return true;
		}
	}

	return false;
}

uint64_t CMetaData::m_begin = 0;
uint64_t CMetaData::m_end = 0;
DWORD CMetaData::m_size = 0;

uint64_t CMetaData::begin()
{
	return m_begin;
}
uint64_t CMetaData::end()
{
	return m_end;
}
DWORD CMetaData::size()
{
	return m_size;
}

void CMetaData::init()
{
	if (m_begin && m_size)
		return;

	m_begin = (uint64_t)GetModuleHandleA(nullptr);
	const IMAGE_DOS_HEADER*	headerDos = (const IMAGE_DOS_HEADER*)m_begin;
	const IMAGE_NT_HEADERS*	headerNt = (const IMAGE_NT_HEADERS64*)((const BYTE*)headerDos + headerDos->e_lfanew);
	m_size = headerNt->OptionalHeader.SizeOfCode;
	m_end = m_begin + m_size;
	return;
}
fpIsDLCPresent	OG_IS_DLC_PRESENT = nullptr;
BOOL __cdecl HK_IS_DLC_PRESENT()
{
	static uint64_t	last = 0;
	uint64_t		cur = *Hooking::m_frameCount;
	if (last != cur)
	{
		last = cur;
		Hooking::onTickInit();
	}
	return OG_IS_DLC_PRESENT();
}
Hooking::NativeHandler ORIG_WAIT = NULL;
void* __cdecl MY_WAIT(NativeContext *cxt)
{
	static int lastThread = 0;
	int threadId = SCRIPT::GET_ID_OF_THIS_THREAD();
	if (!lastThread)
	{
		char* name = SCRIPT::_GET_NAME_OF_THREAD(threadId);
		if (strcmp(name, "main_persistent") == 0)
		{
			lastThread = threadId;
			Log::Msg("Hooked Script NAME: %s ID: %i", name, threadId);
		}
	}
	if (threadId == lastThread) Hooking::onTickInit();
	ORIG_WAIT(cxt);
	return cxt;
}

Hooking::NativeHandler  ORIG_NETWORK_SESSION_KICK_PLAYER = NULL;
void *__cdecl MY_NETWORK_SESSION_KICK_PLAYER(NativeContext* cxt)
{
	Player player = cxt->GetArgument<int>(0);
	if (player == PLAYER::PLAYER_ID())
	{
		ORIG_NETWORK_SESSION_KICK_PLAYER(cxt);
	}
	return nullptr;
}


Hooking::NativeHandler  ORIG_NETWORK_ADD_PED_TO_SYNCHRONISED_SCENE = NULL;
void *__cdecl MY_NETWORK_ADD_PED_TO_SYNCHRONISED_SCENE(NativeContext* cxt)
{
	Player player = cxt->GetArgument<int>(0);
	if (player == PLAYER::PLAYER_ID())
	{
		ORIG_NETWORK_ADD_PED_TO_SYNCHRONISED_SCENE(cxt);
	}
	return nullptr;
}

Hooking::NativeHandler  ORIG_ATTACH_ENTITY_TO_ENTITY = NULL;
void *__cdecl MY_ATTACH_ENTITY_TO_ENTITY(NativeContext* cxt)
{
	Player player = cxt->GetArgument<int>(0);
	if (player == PLAYER::PLAYER_ID())
	{
		ORIG_ATTACH_ENTITY_TO_ENTITY(cxt);
	}
	return nullptr;
}

Hooking::NativeHandler ORIG_CLEAR_PED_TASKS_IMMEDIATELY = NULL;
void *__cdecl MY_CLEAR_PED_TASKS_IMMEDIATELY(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != PLAYER::PLAYER_PED_ID())
	{
		ORIG_CLEAR_PED_TASKS_IMMEDIATELY(cxt);
	}

	return nullptr;
}

Hooking::NativeHandler ORIG_CLEAR_PED_TASKS = NULL;
void *__cdecl MY_CLEAR_PED_TASKS(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != PLAYER::PLAYER_PED_ID())
	{
		ORIG_CLEAR_PED_TASKS(cxt);
	}

	return nullptr;
}

Hooking::NativeHandler ORIG_CLEAR_PED_SECONDARY_TASK = NULL;
void *__cdecl MY_CLEAR_PED_SECONDARY_TASK(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != PLAYER::PLAYER_PED_ID())
	{
		ORIG_CLEAR_PED_SECONDARY_TASK(cxt);
	}
	return nullptr;
}

Hooking::NativeHandler ORIG_CLONE_PED = NULL;
void *__cdecl MY_CLONE_PED(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != PLAYER::PLAYER_PED_ID())
	{
		ORIG_CLONE_PED(cxt);
	}
	return nullptr;
}


bool Hooking::HookNatives()
{
	MH_STATUS status = MH_CreateHook(Hooking::is_DLC_present, HK_IS_DLC_PRESENT, (void**)&OG_IS_DLC_PRESENT);
	if ((status != MH_OK && status != MH_ERROR_ALREADY_CREATED) || MH_EnableHook(Hooking::is_DLC_present) != MH_OK)
		return false;
	Hooking::m_hooks.push_back(Hooking::is_DLC_present);
	return true;
}

void __stdcall ScriptFunction(LPVOID lpParameter)
{
	try
	{
		ScriptMain();

	}
	catch (...)
	{
		Log::Fatal("Failed scriptFiber");
	}
}

void Hooking::onTickInit()
{
	if (mainFiber == nullptr)
		mainFiber = ConvertThreadToFiber(nullptr);

	if (timeGetTime() < wakeAt)
		return;

	static HANDLE scriptFiber;
	if (scriptFiber)
		SwitchToFiber(scriptFiber);
	else
		scriptFiber = CreateFiber(NULL, ScriptFunction, nullptr);
}

void Hooking::FailPatterns(const char* name)
{
	char buf[4096];
	sprintf_s(buf, "finding %s", name);
	Log::Error(buf);
	Cleanup();
}

/*
//CPatternResult
*/

CPatternResult::CPatternResult(void* pVoid) :
	m_pVoid(pVoid)
{}
CPatternResult::CPatternResult(void* pVoid, void* pBegin, void* pEnd) :
	m_pVoid(pVoid),
	m_pBegin(pBegin),
	m_pEnd(pEnd)
{}
CPatternResult::~CPatternResult() {}

/*
//CPattern Public
*/

CPattern::CPattern(char* szByte, char* szMask) :
	m_szByte(szByte),
	m_szMask(szMask),
	m_bSet(false)
{}
CPattern::~CPattern() {}

CPattern&	CPattern::find(int i, uint64_t startAddress)
{
	match(i, startAddress, false);
	if (m_result.size() <= i)
		m_result.push_back(nullptr);
	return *this;
}

CPattern&	CPattern::virtual_find(int i, uint64_t startAddress)
{
	match(i, startAddress, true);
	if (m_result.size() <= i)
		m_result.push_back(nullptr);
	return *this;
}

CPatternResult	CPattern::get(int i)
{
	if (m_result.size() > i)
		return m_result[i];
	return nullptr;
}

/*
//CPattern Private
*/
bool	CPattern::match(int i, uint64_t startAddress, bool virt)
{
	if (m_bSet)
		return false;
	uint64_t	begin = 0;
	uint64_t	end = 0;
	if (!virt)
	{
		CMetaData::init();
		begin = CMetaData::begin() + startAddress;
		end = CMetaData::end();
		if (begin >= end)
			return false;
	}
	int		j = 0;
	do
	{
		if (virt)
			begin = virtual_find_pattern(startAddress, (BYTE*)m_szByte, m_szMask) + 1;
		else
			begin = find_pattern(begin, end, (BYTE*)m_szByte, m_szMask) + 1;
		if (begin == NULL)
			break;
		j++;
	} while (j < i);

	m_bSet = true;
	return true;
}

bool	CPattern::byte_compare(const BYTE* pData, const BYTE* btMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++btMask)
		if (*szMask == 'x' && *pData != *btMask)
			break;
	if ((*szMask) != 0)
		return false;
	return true;
}


uint64_t	CPattern::find_pattern(uint64_t address, uint64_t end, BYTE *btMask, char *szMask)
{
	size_t len = strlen(szMask) + 1;
	for (uint64_t i = 0; i < (end - address - len); i++)
	{
		BYTE*	ptr = (BYTE*)(address + i);
		if (byte_compare(ptr, btMask, szMask))
		{
			m_result.push_back(CPatternResult((void*)(address + i)));
			return address + i;
		}
	}
	return NULL;
}

uint64_t	CPattern::virtual_find_pattern(uint64_t address, BYTE *btMask, char *szMask)
{
	MEMORY_BASIC_INFORMATION mbi;
	char*	pStart = nullptr;
	char*	pEnd = nullptr;
	char*	res = nullptr;
	size_t	maskLen = strlen(szMask);

	while (res == nullptr && sizeof(mbi) == VirtualQuery(pEnd, &mbi, sizeof(mbi)))
	{
		pStart = pEnd;
		pEnd += mbi.RegionSize;
		if (mbi.Protect != PAGE_READWRITE || mbi.State != MEM_COMMIT)
			continue;

		for (int i = 0; pStart < pEnd - maskLen && res == nullptr; ++pStart)
		{
			if (byte_compare((BYTE*)pStart, btMask, szMask))
			{
				m_result.push_back(CPatternResult((void*)pStart, mbi.BaseAddress, pEnd));
				res = pStart;
			}
		}

		mbi = {};
	}
	return (uint64_t)res;
}

void	failPat(const char* name)
{
	Log::Fatal("Failed to find %s pattern.", name);
	exit(0);
}

template <typename T>
void	setPat
(
	const char*	name,
	char*		pat,
	char*		mask,
	T**			out,
	bool		rel,
	int			offset = 0,
	int			deref = 0,
	int			skip = 0
)
{
	T*	ptr = nullptr;

	CPattern pattern(pat, mask);
	pattern.find(1 + skip);
	if (rel)
		ptr = pattern.get(skip).get_rel<T>(offset);
	else
		ptr = pattern.get(skip).get<T>(offset);

	while (true)
	{
		if (ptr == nullptr)
			failPat(name);

		if (deref <= 0)
			break;
		ptr = *(T**)ptr;
		--deref;
	}

	*out = ptr;
	return;
}

template <typename T>
void	setFn
(
	const char*	name,
	char*		pat,
	char*		mask,
	T*			out,
	int			skip = 0
)
{
	char*	ptr = nullptr;

	CPattern pattern(pat, mask);
	pattern.find(1 + skip);
	ptr = pattern.get(skip).get<char>(0);

	if (ptr == nullptr)
		failPat(name);

	*out = (T)ptr;
	return;
}

void Hooking::FindPatterns()
{
	HANDLE steam = GetModuleHandleA("steam_api64.dll");

	auto p_activeThread = pattern("E8 ? ? ? ? 48 8B 88 10 01 00 00");
	auto p_blipList = pattern("4C 8D 05 ? ? ? ? 0F B7 C1");
	auto p_fixVector3Result = pattern("83 79 18 00 48 8B D1 74 4A FF 4A 18");
	auto p_gameLegals = pattern("72 1F E8 ? ? ? ? 8B 0D");
	auto p_gameLogos = pattern("70 6C 61 74 66 6F 72 6D 3A");
	auto p_gameState = pattern("83 3D ? ? ? ? ? 75 17 8B 43 20 25");
	auto p_modelCheck = pattern("48 85 C0 0F 84 ? ? ? ? 8B 48 50");
	auto p_modelSpawn = pattern("48 8B C8 FF 52 30 84 C0 74 05 48");
	auto p_nativeTable = pattern("76 32 48 8B 53 40 48 8D 0D");
	auto p_worldPtr = pattern("48 8B 05 ? ? ? ? 45 ? ? ? ? 48 8B 48 08 48 85 C9 74 07");
	auto p_globalPtr = pattern("4C 8D 05 ? ? ? ? 4D 8B 08 4D 85 C9 74 11");
	auto p_eventHook = pattern("48 83 EC 28 E8 ? ? ? ? 48 8B 0D ? ? ? ? 4C 8D 0D ? ? ? ? 4C 8D 05 ? ? ? ? BA 03");

	setPat<uint64_t>("frame count",
		"\x8B\x15\x00\x00\x00\x00\x41\xFF\xCF",
		"xx????xxx",
		&Hooking::m_frameCount,
		true,
		2);

	setFn<fpIsDLCPresent>("is_DLC_present",
		"\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x81\xF9\x00\x00\x00\x00",
		"xxxx?xxxxxxx????",
		&Hooking::is_DLC_present);

	char * c_location = nullptr;
	void * v_location = nullptr;

	Sleep(5000);

	Log::Msg("Getting Game State...");
	c_location = p_gameState.count(1).get(0).get<char>(2);
	c_location == nullptr ? FailPatterns("gameState") : m_gameState = reinterpret_cast<decltype(m_gameState)>(c_location + *(int32_t*)c_location + 5);

	Log::Msg("Getting vector3 result fixer func...");
	v_location = p_fixVector3Result.count(1).get(0).get<void>(0);
	if (v_location != nullptr) scrNativeCallContext::SetVectorResults = (void(*)(scrNativeCallContext*))(v_location);

	Log::Msg("Getting native registration table...");
	c_location = p_nativeTable.count(1).get(0).get<char>(9);
	c_location == nullptr ? FailPatterns("native registration table") : m_registrationTable = reinterpret_cast<decltype(m_registrationTable)>(c_location + *(int32_t*)c_location + 4);

	Log::Msg("Getting World Pointer...");
	c_location = p_worldPtr.count(1).get(0).get<char>(0);
	c_location == nullptr ? FailPatterns("world Pointer") : m_worldPtr = reinterpret_cast<uint64_t>(c_location) + *reinterpret_cast<int*>(reinterpret_cast<uint64_t>(c_location) + 3) + 7;

	Log::Msg("Getting Blip List...");
	c_location = p_blipList.count(1).get(0).get<char>(0);
	c_location == nullptr ? FailPatterns("blip List") : m_blipList = (BlipList*)(c_location + *reinterpret_cast<int*>(c_location + 3) + 7);


	Log::Msg("Bypassing Object restrictions..");

	CPattern pattern_modelCheck("\x48\x85\xC0\x0F\x84\x00\x00\x00\x00\x8B\x48\x50", "xxxxx????xxx");
	auto ptr = pattern_modelCheck.find(0).get(0).get<char>(0);
	ptr == nullptr ? Log::Error("[Pattern] Failed to find modelCheck.") : Memory::nop(ptr, 24);

	CPattern pattern_modelSpawn("\x48\x8B\xC8\xFF\x52\x30\x84\xC0\x74\x05\x48", "xxxxxxxxxxx");
	ptr = pattern_modelSpawn.find(0).get(0).get<char>(8);
	ptr == nullptr ? Log::Error("[Pattern] Failed to find modelSpawn.") : Memory::nop(ptr, 2);

	Log::Msg("Getting active script thread...");
	c_location = p_activeThread.count(1).get(0).get<char>(1);
	c_location == nullptr ? FailPatterns("Active Script Thread") : GetActiveThread = reinterpret_cast<decltype(GetActiveThread)>(c_location + *(int32_t*)c_location + 4);

	Log::Msg("Getting World Pointer...");
	c_location = p_globalPtr.count(1).get(0).get<char>(0);
	__int64 patternAddr = NULL;
	c_location == nullptr ? FailPatterns("globalTable") : patternAddr = reinterpret_cast<decltype(patternAddr)>(c_location);
	m_globalPtr = (__int64**)(patternAddr + *(int*)(patternAddr + 3) + 7);

	Log::Msg("Getting Event Hook...");
	if ((c_location = p_eventHook.count(1).get(0).get<char>(0)))
	{
		int i = 0, j = 0, matches = 0, found = 0;
		char* pattern = "\x4C\x8D\x05";
		while (found != EVENT_COUNT)
		{
			if (c_location[i] == pattern[j])
			{
				if (++matches == 3)
				{
					EventPtr.push_back((void*)(reinterpret_cast<uint64_t>(c_location + i - j) + *reinterpret_cast<int*>(c_location + i + 1) + 7));
					found++;
					j = matches = 0;
				}
				j++;
			}
			else
			{
				matches = j = 0;
			}
			i++;
		}
	}

	Log::Msg("Initializing natives...");
	CrossMapping::initNativeMap();

	Log::Msg("Checking if GTA V is ready...");
	while (*m_gameState != GameStatePlaying) {
		Sleep(200);
	}
	Log::Msg("GTA V ready!");
}

static Hooking::NativeHandler _Handler(uint64_t origHash)
{
	uint64_t newHash = CrossMapping::MapNative(origHash);
	if (newHash == 0)
	{
		return nullptr;
	}

	Hooking::NativeRegistrationNew * table = m_registrationTable[newHash & 0xFF];

	for (; table; table = table->getNextRegistration())
	{
		for (uint32_t i = 0; i < table->getNumEntries(); i++)
		{
			if (newHash == table->getHash(i))
			{
				return table->handlers[i];
			}
		}
	}
	return nullptr;
}

Hooking::NativeHandler Hooking::GetNativeHandler(uint64_t origHash)
{
	auto& handler = m_handlerCache[origHash];

	if (handler == nullptr)
	{
		handler = _Handler(origHash);
	}

	return handler;
}

eGameState Hooking::GetGameState()
{
	return *m_gameState;
}

uint64_t Hooking::getWorldPtr()
{
	return m_worldPtr;
}
void WAIT(DWORD ms)
{
	wakeAt = timeGetTime() + ms;
	SwitchToFiber(mainFiber);
}

void Hooking::Cleanup()
{
	Log::Msg("Cleaning up hooks");
	for (int i = 0; i < m_hooks.size(); i++)
		if (MH_DisableHook(m_hooks[i]) != MH_OK && MH_RemoveHook(m_hooks[i]) != MH_OK)
			Log::Error("Failed to unhook %p", (void*)m_hooks[i]);
	MH_Uninitialize();

	FreeLibraryAndExitThread(static_cast<HMODULE>(_hmoduleDLL), 1);
}

void Hooking::defuseEvent(RockstarEvent e, bool toggle)
{
	static const unsigned char retn = 0xC3;
	char* p = (char*)EventPtr[e];
	if (toggle)
	{
		if (EventRestore[e] == 0)
			EventRestore[e] = p[0];
		*p = retn;
	}
	else
	{
		if (EventRestore[e] != 0)
			*p = EventRestore[e];
	}
}

__int64** Hooking::getGlobalPtr()
{
	return m_globalPtr;
}