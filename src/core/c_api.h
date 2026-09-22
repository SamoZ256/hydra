#ifndef HYDRA_C_API
#define HYDRA_C_API

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// NOLINTBEGIN(cppcoreguidelines-use-enum-class)

// Types
typedef struct {
    uint64_t lo;
    uint64_t hi;
} HydraU128;

typedef struct {
    uint32_t x;
    uint32_t y;
} HydraUint2;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} HydraUchar3;

typedef struct {
    const char* data;
    size_t size;
} HydraString;

// Enums
typedef enum : uint32_t {
    HYDRA_INPUT_BACKEND_APPLE_GAME_CONTROLLER,
    HYDRA_INPUT_BACKEND_SDL,
} HydraInputBackend;

typedef enum : uint32_t {
    HYDRA_CPU_BACKEND_APPLE_HYPERVISOR,
    HYDRA_CPU_BACKEND_DYNARMIC,
} HydraCpuBackend;

typedef enum : uint32_t {
    HYDRA_GPU_RENDERER_NULL,
    HYDRA_GPU_RENDERER_METAL,
} HydraGpuRenderer;

typedef enum : uint32_t {
    HYDRA_SHADER_BACKEND_MSL,
    HYDRA_SHADER_BACKEND_AIR,
} HydraShaderBackend;

typedef enum : uint32_t {
    HYDRA_RESOLUTION_AUTO,
    HYDRA_RESOLUTION_720P,
    HYDRA_RESOLUTION_1080P,
    HYDRA_RESOLUTION_1440P,
    HYDRA_RESOLUTION_2160P,
    HYDRA_RESOLUTION_4320P,
    HYDRA_RESOLUTION_AUTO_EXACT,
    HYDRA_RESOLUTION_CUSTOM,
} HydraResolution;

typedef enum : uint32_t {
    HYDRA_AUDIO_BACKEND_NULL,
    HYDRA_AUDIO_BACKEND_CUBEB,
} HydraAudioBackend;

typedef enum : uint32_t {
    HYDRA_SYSTEM_LANGUAGE_AMERICAN_ENGLISH,
    HYDRA_SYSTEM_LANGUAGE_BRITISH_ENGLISH,
    HYDRA_SYSTEM_LANGUAGE_JAPANESE,
    HYDRA_SYSTEM_LANGUAGE_FRENCH,
    HYDRA_SYSTEM_LANGUAGE_GERMAN,
    HYDRA_SYSTEM_LANGUAGE_LATIN_AMERICAN_SPANISH,
    HYDRA_SYSTEM_LANGUAGE_SPANISH,
    HYDRA_SYSTEM_LANGUAGE_ITALIAN,
    HYDRA_SYSTEM_LANGUAGE_DUTCH,
    HYDRA_SYSTEM_LANGUAGE_CANADIAN_FRENCH,
    HYDRA_SYSTEM_LANGUAGE_PORUGUESE,
    HYDRA_SYSTEM_LANGUAGE_RUSSIAN,
    HYDRA_SYSTEM_LANGUAGE_KOREAN,
    HYDRA_SYSTEM_LANGUAGE_TRADITIONAL_CHINESE,
    HYDRA_SYSTEM_LANGUAGE_SIMPLIFIED_CHINESE,
    HYDRA_SYSTEM_LANGUAGE_BRAZILIAN_PORTUGUESE,
    HYDRA_SYSTEM_LANGUAGE_POLISH,
    HYDRA_SYSTEM_LANGUAGE_THAI,
} HydraSystemLanguage;

typedef enum : uint32_t {
    HYDRA_LOG_OUTPUT_NONE,
    HYDRA_LOG_OUTPUT_STD_OUT,
    HYDRA_LOG_OUTPUT_FILE,
} HydraLogOutput;

typedef enum : uint32_t {
    HYDRA_LOG_LEVEL_DEBUG,
    HYDRA_LOG_LEVEL_INFO,
    HYDRA_LOG_LEVEL_STUB,
    HYDRA_LOG_LEVEL_WARNING,
    HYDRA_LOG_LEVEL_ERROR,
    HYDRA_LOG_LEVEL_FATAL,
} HydraLogLevel;

typedef enum : uint32_t {
    HYDRA_LOG_CLASS_COMMON,
    HYDRA_LOG_CLASS_MMU,
    HYDRA_LOG_CLASS_CPU,
    HYDRA_LOG_CLASS_GPU,
    HYDRA_LOG_CLASS_ENGINES,
    HYDRA_LOG_CLASS_MACRO,
    HYDRA_LOG_CLASS_SHADER_DECOMPILER,
    HYDRA_LOG_CLASS_METAL_RENDERER,
    HYDRA_LOG_CLASS_SDL3_WINDOW,
    HYDRA_LOG_CLASS_HORIZON,
    HYDRA_LOG_CLASS_KERNEL,
    HYDRA_LOG_CLASS_FILESYSTEM,
    HYDRA_LOG_CLASS_LOADER,
    HYDRA_LOG_CLASS_SERVICES,
    HYDRA_LOG_CLASS_APPLETS,
    HYDRA_LOG_CLASS_CUBEB,
    HYDRA_LOG_CLASS_HYPERVISOR,
    HYDRA_LOG_CLASS_DYNARMIC,
    HYDRA_LOG_CLASS_INPUT,
    HYDRA_LOG_CLASS_OTHER,
} HydraLogClass;

typedef enum : uint32_t {
    HYDRA_DEBUGGER_THREAD_STATUS_RUNNING,
    HYDRA_DEBUGGER_THREAD_STATUS_BREAK,
} HydraDebuggerThreadStatus;

typedef enum : uint32_t {
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_PROGRAM = 0,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_META = 1,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_CONTROL = 2,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_MANUAL = 3,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_DATA = 4,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_PUBLIC_DATA = 5,
} HydraContentArchiveContentType;

// String list
__attribute__((returns_nonnull)) void* hydraCreateStringList();
void hydraStringListDestroy(void* list);
uint32_t hydraStringListGetCount(const void* list);
HydraString hydraStringListGet(const void* list, uint32_t index);
void hydraStringListResize(void* list, uint32_t size);
void hydraStringListSet(void* list, uint32_t index, HydraString value);
void hydraStringListAppend(void* list, HydraString value);

// String view list
uint32_t hydraStringViewListGetCount(const void* list);
HydraString hydraStringViewListGet(const void* list, uint32_t index);
void hydraStringViewListResize(void* list, uint32_t size);
void hydraStringViewListSet(void* list, uint32_t index, HydraString value);
void hydraStringViewListAppend(void* list, HydraString value);

// String to string map
__attribute__((returns_nonnull)) void* hydraCreateStringToStringMap();
void hydraStringToStringMapDestroy(void* map);
uint32_t hydraStringToStringMapGetCount(const void* map);
HydraString hydraStringToStringMapGetKey(const void* map, uint32_t index);
HydraString hydraStringToStringMapGetValue(const void* map, uint32_t index);
HydraString hydraStringToStringMapGetValueByKey(const void* map,
                                                 HydraString key);
void hydraStringToStringMapRemoveAll(void* map);
void hydraStringToStringMapSetByKey(void* map, HydraString key,
                                    HydraString value);

// Loader plugin
HydraString hydraLoaderPluginGetPath(const void* plugin);
void hydraLoaderPluginSetPath(void* plugin, HydraString path);
void* hydraLoaderPluginGetOptions(void* plugin);

uint32_t hydraLoaderPluginListGetCount(const void* list);
void* hydraLoaderPluginListGet(void* list, uint32_t index);
void hydraLoaderPluginListResize(void* list, uint32_t size);

// Config
void hydraConfigSerialize();
void hydraConfigDeserialize();

HydraString hydraConfigGetAppDataPath();
HydraString hydraConfigGetLogsPath();

void* hydraConfigGetGamePaths();
void* hydraConfigGetLoaderPlugins();
void* hydraConfigGetPatchPaths();
uint32_t* hydraConfigGetInputBackend();
void* hydraConfigGetInputProfiles();
uint32_t* hydraConfigGetCpuBackend();
uint32_t* hydraConfigGetGpuRenderer();
uint32_t* hydraConfigGetShaderBackend();
uint32_t* hydraConfigGetDisplayResolution();
HydraUint2* hydraConfigGetCustomDisplayResolution();
uint32_t* hydraConfigGetAudioBackend();
HydraU128* hydraConfigGetUserId();
HydraString hydraConfigGetDeviceNickname();
void hydraConfigSetDeviceNickname(HydraString value);
uint32_t* hydraConfigGetSystemLanguage();
HydraString hydraConfigGetSystemLocation();
void hydraConfigSetSystemLocation(HydraString value);
HydraString hydraConfigGetFirmwarePath();
void hydraConfigSetFirmwarePath(HydraString value);
HydraString hydraConfigGetSdCardPath();
void hydraConfigSetSdCardPath(HydraString value);
HydraString hydraConfigGetSavePath();
void hydraConfigSetSavePath(HydraString value);
HydraString hydraConfigGetSysmodulesPath();
void hydraConfigSetSysmodulesPath(HydraString value);
bool* hydraConfigGetHandheldMode();
uint32_t* hydraConfigGetLogOutput();
bool* hydraConfigGetLogFsAccess();
bool* hydraConfigGetDebugLogging();
void* hydraConfigGetProcessArgs();
bool* hydraConfigGetRecoverFromSegfault();
bool* hydraConfigGetGdbEnabled();
uint16_t* hydraConfigGetGdbPort();
bool* hydraConfigGetGdbWaitForClient();

// Option config
typedef enum HydraLoaderPluginOptionType : uint32_t {
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_BOOLEAN = 0,
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_INTEGER = 1,
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_ENUMERATION = 2,
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_STRING = 3,
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_PATH = 4,
} HydraLoaderPluginOptionType;

void* hydraLoaderPluginOptionConfigCopy(const void* config);
void hydraLoaderPluginOptionConfigDestroy(void* config);
HydraString hydraLoaderPluginOptionConfigGetName(const void* config);
HydraString hydraLoaderPluginOptionConfigGetDescription(const void* config);
HydraLoaderPluginOptionType
hydraLoaderPluginOptionConfigGetType(const void* config);
bool hydraLoaderPluginOptionConfigGetIsRequired(const void* config);
__attribute__((returns_nonnull)) const void*
hydraLoaderPluginOptionConfigGetEnumValueNames(const void* config);
__attribute__((returns_nonnull)) const void*
hydraLoaderPluginOptionConfigGetPathContentTypes(const void* config);

// Filesystem
void* hydraCreateFilesystem();
void hydraFilesystemDestroy(void* fs);

void* hydraOpenFile(HydraString path);
void hydraFileClose(void* file);

void* hydraCreateContentArchive(void* file);
void hydraContentArchiveDestroy(void* content_archive);
HydraContentArchiveContentType
hydraContentArchiveGetContentType(void* content_archive);

// Time zone manager
void* hydraCreateTimeZoneManager(void* filesystem);
void hydraTimeZoneManagerDestroy(void* manager);
uint32_t hydraTimeZoneManagerGetLocationCount(void* manager);
HydraString hydraTimeZoneManagerGetLocation(void* manager, uint32_t index);

// Loader
void* hydraCreateLoaderFromPath(HydraString path, void* plugin_manager);
void hydraLoaderDestroy(void* loader);
uint64_t hydraLoaderGetTitleId(void* loader);
void* hydraLoaderLoadNacp(void* loader);
void* hydraLoaderLoadIcon(void* loader, uint32_t* width, uint32_t* height);
bool hydraLoaderHasIcon(const void* loader);
void hydraLoaderExtractIcon(const void* loader, HydraString path);
bool hydraLoaderHasExefs(const void* loader);
void hydraLoaderExtractExefs(const void* loader, HydraString path);
bool hydraLoaderHasRomfs(const void* loader);
void hydraLoaderExtractRomfs(const void* loader, HydraString path);

void* hydraCreateNcaLoaderFromContentArchive(void* content_archive);
HydraString hydraNcaLoaderGetName(void* nca_loader);

// Plugins

// Manager
void* hydraCreateLoaderPluginManager();
void hydraLoaderPluginManagerDestroy(void* manager);
void hydraLoaderPluginManagerRefresh(void* manager);

// Plugin
void* hydraCreateLoaderPlugin(HydraString path);
void hydraLoaderPluginDestroy(void* plugin);
HydraString hydraLoaderPluginGetName(const void* plugin);
HydraString hydraLoaderPluginGetDisplayVersion(const void* plugin);
uint32_t hydraLoaderPluginGetSupportedFormatCount(const void* plugin);
HydraString hydraLoaderPluginGetSupportedFormat(const void* plugin,
                                                 uint32_t index);
uint32_t hydraLoaderPluginGetOptionConfigCount(const void* plugin);
const void* hydraLoaderPluginGetOptionConfig(const void* plugin,
                                             uint32_t index);

// NACP
void hydraNacpDestroy(void* nacp);
const void* hydraNacpGetTitle(void* nacp, HydraSystemLanguage lang);
HydraString hydraNacpGetDisplayVersion(void* nacp);

// NACP title
HydraString hydraNacpTitleGetName(const void* title);
HydraString hydraNacpTitleGetAuthor(const void* title);

// User manager
void* hydraCreateUserManager();
void hydraUserManagerDestroy(void* user_manager);
void hydraUserManagerFlush(void* user_manager);
HydraU128 hydraUserManagerCreateUser(void* user_manager);
uint32_t hydraUserManagerGetUserCount(void* user_manager);
HydraU128 hydraUserManagerGetUserId(void* user_manager, uint32_t index);
void* hydraUserManagerGetUser(void* user_manager, HydraU128 user_id);
void hydraUserManagerLoadSystemAvatars(void* user_manager, void* fs);
const void* hydraUserManagerLoadAvatarImage(void* user_manager,
                                            HydraString path,
                                            uint32_t* out_dimensions);
uint32_t hydraUserManagerGetAvatarCount(void* user_manager);
HydraString hydraUserManagerGetAvatarPath(void* user_manager, uint32_t index);

HydraString hydraUserGetNickname(void* user);
void hydraUserSetNickname(void* user, HydraString nickname);
HydraUchar3 hydraUserGetAvatarBgColor(void* user);
void hydraUserSetAvatarBgColor(void* user, HydraUchar3 color);
HydraString hydraUserGetAvatarPath(void* user);
void hydraUserSetAvatarPath(void* user, HydraString path);

// System
void* hydraCreateSystem();
void hydraSystemDestroy(void* system);

void hydraSystemSetSurface(void* system, void* surface);

void hydraSystemLoadAndStart(void* system, void* loader);
void hydraSystemRequestStop(void* system);
void hydraSystemForceStop(void* system);

void hydraSystemPause(void* system);
void hydraSystemResume(void* system);

void hydraSystemNotifyOperationModeChanged(void* system);

void hydraSystemProgressFrame(void* system, uint32_t width, uint32_t height,
                              bool* out_dt_average_updated);

bool hydraSystemIsRunning(void* system);
float hydraSystemGetLastDeltaTimeAverage(void* system);

void hydraSystemTakeScreenshot(void* system);
void hydraSystemCaptureGpuFrame(void* system);

void hydraSystemTextureCacheLock(void* system);
void hydraSystemTextureCacheUnlock(void* system);
uint32_t hydraSystemTextureCacheGetTextureMemoryCount(void* system);
const void* hydraSystemTextureCacheGetTextureMemory(void* system,
                                                    uint32_t index);

// Input
// TODO

// Debugger

// Debugger manager
void hydraDebuggerManagerLock();
void hydraDebuggerManagerUnlock();
uint64_t hydraDebuggerManagerGetDebuggerCount();
void* hydraDebuggerManagerGetDebugger(uint32_t index);
void* hydraDebuggerManagerGetDebuggerForProcess(void* process);

// Debugger
HydraString hydraDebuggerGetName(void* debugger);
void hydraDebuggerLock(void* debugger);
void hydraDebuggerUnlock(void* debugger);
void hydraDebuggerRegisterThisThread(void* debugger, HydraString name);
void hydraDebuggerUnregisterThisThread(void* debugger);
uint64_t hydraDebuggerGetThreadCount(void* debugger);
void* hydraDebuggerGetThread(void* debugger, uint32_t index);

// Thread
HydraString hydraDebuggerThreadGetName(void* thread);
void hydraDebuggerThreadLock(void* thread);
void hydraDebuggerThreadUnlock(void* thread);
HydraDebuggerThreadStatus hydraDebuggerThreadGetStatus(void* thread);
HydraString hydraDebuggerThreadGetBreakReason(void* thread);
uint64_t hydraDebuggerThreadGetMessageCount(void* thread);
const void* hydraDebuggerThreadGetMessage(void* thread, uint32_t index);

// Message
HydraLogLevel hydraDebuggerMessageGetLogLevel(const void* msg);
HydraLogClass hydraDebuggerMessageGetLogClass(const void* msg);
HydraString hydraDebuggerMessageGetFile(const void* msg);
uint32_t hydraDebuggerMessageGetLine(const void* msg);
HydraString hydraDebuggerMessageGetFunction(const void* msg);
HydraString hydraDebuggerMessageGetString(const void* msg);
const void* hydraDebuggerMessageGetStackTrace(const void* msg);

// Stack trace
void* hydraDebuggerStackTraceCopy(const void* stack_trace);
void hydraDebuggerStackTraceDestroy(void* stack_trace);
uint32_t hydraDebuggerStackTraceGetFrameCount(const void* stack_trace);
const void* hydraDebuggerStackTraceGetFrame(const void* stack_trace,
                                            uint32_t index);

// Stack frame
void* hydraDebuggerStackFrameResolve(const void* stack_frame);

// Resolved stack frame
void hydraDebuggerResolvedStackFrameDestroy(void* resolved_stack_frame);
HydraString
hydraDebuggerResolvedStackFrameGetModule(const void* resolved_stack_frame);
HydraString
hydraDebuggerResolvedStackFrameGetFunction(const void* resolved_stack_frame);
uint64_t
hydraDebuggerResolvedStackFrameGetAddress(const void* resolved_stack_frame);

// Texture cache

// Texture memory
uint32_t hydraTextureMemoryGetTextureGroupCount(const void* mem);
const void* hydraTextureMemoryGetTextureGroup(const void* mem, uint32_t index);

// Texture group
uint32_t hydraTextureGroupGetTextureStorageCount(const void* group);
const void* hydraTextureGroupGetTextureStorage(const void* group,
                                               uint32_t index);

// Texture storage
const void* hydraTextureStorageGetTextureDescriptor(const void* storage);

// Texture descriptor
typedef enum HydraTextureType : uint32_t {
    HYDRA_TEXTURE_TYPE_1D = 0,
    HYDRA_TEXTURE_TYPE_1D_ARRAY,
    HYDRA_TEXTURE_TYPE_1D_BUFFER,
    HYDRA_TEXTURE_TYPE_2D,
    HYDRA_TEXTURE_TYPE_2D_ARRAY,
    HYDRA_TEXTURE_TYPE_3D,
    HYDRA_TEXTURE_TYPE_CUBE,
    HYDRA_TEXTURE_TYPE_CUBE_ARRAY,
} HydraTextureType;

typedef enum HydraTextureFormat : uint32_t {
    HYDRA_TEXTURE_FORMAT_INVALID = 0,

    HYDRA_TEXTURE_FORMAT_R8_UNORM,
    HYDRA_TEXTURE_FORMAT_R8_SNORM,
    HYDRA_TEXTURE_FORMAT_R8_UINT,
    HYDRA_TEXTURE_FORMAT_R8_SINT,
    HYDRA_TEXTURE_FORMAT_R16_FLOAT,
    HYDRA_TEXTURE_FORMAT_R16_UNORM,
    HYDRA_TEXTURE_FORMAT_R16_SNORM,
    HYDRA_TEXTURE_FORMAT_R16_UINT,
    HYDRA_TEXTURE_FORMAT_R16_SINT,
    HYDRA_TEXTURE_FORMAT_R32_FLOAT,
    HYDRA_TEXTURE_FORMAT_R32_UINT,
    HYDRA_TEXTURE_FORMAT_R32_SINT,

    HYDRA_TEXTURE_FORMAT_RG8_UNORM,
    HYDRA_TEXTURE_FORMAT_RG8_SNORM,
    HYDRA_TEXTURE_FORMAT_RG8_UINT,
    HYDRA_TEXTURE_FORMAT_RG8_SINT,
    HYDRA_TEXTURE_FORMAT_RG16_FLOAT,
    HYDRA_TEXTURE_FORMAT_RG16_UNORM,
    HYDRA_TEXTURE_FORMAT_RG16_SNORM,
    HYDRA_TEXTURE_FORMAT_RG16_UINT,
    HYDRA_TEXTURE_FORMAT_RG16_SINT,
    HYDRA_TEXTURE_FORMAT_RG32_FLOAT,
    HYDRA_TEXTURE_FORMAT_RG32_UINT,
    HYDRA_TEXTURE_FORMAT_RG32_SINT,

    HYDRA_TEXTURE_FORMAT_RGB32_FLOAT,
    HYDRA_TEXTURE_FORMAT_RGB32_UINT,
    HYDRA_TEXTURE_FORMAT_RGB32_SINT,

    HYDRA_TEXTURE_FORMAT_RGBA8_UNORM,
    HYDRA_TEXTURE_FORMAT_RGBA8_SNORM,
    HYDRA_TEXTURE_FORMAT_RGBA8_UINT,
    HYDRA_TEXTURE_FORMAT_RGBA8_SINT,
    HYDRA_TEXTURE_FORMAT_RGBA16_FLOAT,
    HYDRA_TEXTURE_FORMAT_RGBA16_UNORM,
    HYDRA_TEXTURE_FORMAT_RGBA16_SNORM,
    HYDRA_TEXTURE_FORMAT_RGBA16_UINT,
    HYDRA_TEXTURE_FORMAT_RGBA16_SINT,
    HYDRA_TEXTURE_FORMAT_RGBA32_FLOAT,
    HYDRA_TEXTURE_FORMAT_RGBA32_UINT,
    HYDRA_TEXTURE_FORMAT_RGBA32_SINT,

    HYDRA_TEXTURE_FORMAT_S8_UINT,
    HYDRA_TEXTURE_FORMAT_Z16_UNORM,
    HYDRA_TEXTURE_FORMAT_Z24_UNORM_X8_UINT,
    HYDRA_TEXTURE_FORMAT_Z32_FLOAT,
    HYDRA_TEXTURE_FORMAT_Z24_UNORM_S8_UINT,
    HYDRA_TEXTURE_FORMAT_Z32_FLOAT_X24_S8_UINT,

    HYDRA_TEXTURE_FORMAT_RGBX8_UNORM_SRGB,
    HYDRA_TEXTURE_FORMAT_RGBA8_UNORM_SRGB,

    HYDRA_TEXTURE_FORMAT_RGBA4_UNORM,
    HYDRA_TEXTURE_FORMAT_RGB5_UNORM,
    HYDRA_TEXTURE_FORMAT_RGB5A1_UNORM,
    HYDRA_TEXTURE_FORMAT_R5G6B5_UNORM,
    HYDRA_TEXTURE_FORMAT_RGB10A2_UNORM,
    HYDRA_TEXTURE_FORMAT_RGB10A2_UINT,
    HYDRA_TEXTURE_FORMAT_RG11B10_FLOAT,
    HYDRA_TEXTURE_FORMAT_E5BGR9_FLOAT,

    HYDRA_TEXTURE_FORMAT_BC1_RGB,
    HYDRA_TEXTURE_FORMAT_BC1_RGBA,
    HYDRA_TEXTURE_FORMAT_BC2_RGBA,
    HYDRA_TEXTURE_FORMAT_BC3_RGBA,
    HYDRA_TEXTURE_FORMAT_BC1_RGB_SRGB,
    HYDRA_TEXTURE_FORMAT_BC1_RGBA_SRGB,
    HYDRA_TEXTURE_FORMAT_BC2_RGBA_SRGB,
    HYDRA_TEXTURE_FORMAT_BC3_RGBA_SRGB,
    HYDRA_TEXTURE_FORMAT_BC4_R_UNORM,
    HYDRA_TEXTURE_FORMAT_BC4_R_SNORM,
    HYDRA_TEXTURE_FORMAT_BC5_RG_UNORM,
    HYDRA_TEXTURE_FORMAT_BC5_RG_SNORM,
    HYDRA_TEXTURE_FORMAT_BC7_RGBA_UNORM,
    HYDRA_TEXTURE_FORMAT_BC7_RGBA_UNORM_SRGB,
    HYDRA_TEXTURE_FORMAT_BC6H_RGBA_SF16_FLOAT,
    HYDRA_TEXTURE_FORMAT_BC6H_RGBA_UF16_FLOAT,

    HYDRA_TEXTURE_FORMAT_RGBX8_UNORM,
    HYDRA_TEXTURE_FORMAT_RGBX8_SNORM,
    HYDRA_TEXTURE_FORMAT_RGBX8_UINT,
    HYDRA_TEXTURE_FORMAT_RGBX8_SINT,
    HYDRA_TEXTURE_FORMAT_RGBX16_FLOAT,
    HYDRA_TEXTURE_FORMAT_RGBX16_UNORM,
    HYDRA_TEXTURE_FORMAT_RGBX16_SNORM,
    HYDRA_TEXTURE_FORMAT_RGBX16_UINT,
    HYDRA_TEXTURE_FORMAT_RGBX16_SINT,
    HYDRA_TEXTURE_FORMAT_RGBX32_FLOAT,
    HYDRA_TEXTURE_FORMAT_RGBX32_UINT,
    HYDRA_TEXTURE_FORMAT_RGBX32_SINT,

    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_4X4,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_5X4,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_5X5,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_6X5,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_6X6,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X5,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X6,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X8,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X5,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X6,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X8,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X10,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_12X10,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_12X12,

    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_4X4_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_5X4_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_5X5_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_6X5_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_6X6_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X5_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X6_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X8_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X5_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X6_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X8_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X10_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_12X10_SRGB,
    HYDRA_TEXTURE_FORMAT_ASTC_RGBA_12X12_SRGB,

    HYDRA_TEXTURE_FORMAT_B5G6R5_UNORM,
    HYDRA_TEXTURE_FORMAT_BGR5_UNORM,
    HYDRA_TEXTURE_FORMAT_BGR5A1_UNORM,
    HYDRA_TEXTURE_FORMAT_A1BGR5_UNORM,
    HYDRA_TEXTURE_FORMAT_BGRX8_UNORM,
    HYDRA_TEXTURE_FORMAT_BGRA8_UNORM,
    HYDRA_TEXTURE_FORMAT_BGRX8_UNORM_SRGB,
    HYDRA_TEXTURE_FORMAT_BGRA8_UNORM_SRGB,

    HYDRA_TEXTURE_FORMAT_ETC2_R_UNORM,
    HYDRA_TEXTURE_FORMAT_ETC2_R_SNORM,
    HYDRA_TEXTURE_FORMAT_ETC2_RG_UNORM,
    HYDRA_TEXTURE_FORMAT_ETC2_RG_SNORM,
    HYDRA_TEXTURE_FORMAT_ETC2_RGB,
    HYDRA_TEXTURE_FORMAT_PTA_ETC2_RGB,
    HYDRA_TEXTURE_FORMAT_ETC2_RGBA,
    HYDRA_TEXTURE_FORMAT_ETC2_RGB_SRGB,
    HYDRA_TEXTURE_FORMAT_PTA_ETC2_RGB_SRGB,
    HYDRA_TEXTURE_FORMAT_ETC2_RGBA_SRGB,
} HydraTextureFormat;

uint64_t hydraTextureDescriptorGetPtr(const void* descriptor);
HydraTextureType hydraTextureDescriptorGetType(const void* descriptor);
HydraTextureFormat hydraTextureDescriptorGetFormat(const void* descriptor);
uint32_t hydraTextureDescriptorGetWidth(const void* descriptor);
uint32_t hydraTextureDescriptorGetHeight(const void* descriptor);
uint32_t hydraTextureDescriptorGetDepth(const void* descriptor);
uint32_t hydraTextureDescriptorGetLevelCount(const void* descriptor);
uint32_t hydraTextureDescriptorGetLayerCount(const void* descriptor);
uint32_t hydraTextureDescriptorGetBlockWidthGobs(const void* descriptor);
uint32_t hydraTextureDescriptorGetBlockHeightGobs(const void* descriptor);
uint32_t hydraTextureDescriptorGetBlockDepthGobs(const void* descriptor);
uint64_t hydraTextureDescriptorGetLayerSize(const void* descriptor);
uint64_t hydraTextureDescriptorGetSize(const void* descriptor);

// NOLINTEND(cppcoreguidelines-use-enum-class)

#ifdef __cplusplus
}
#endif

#endif
