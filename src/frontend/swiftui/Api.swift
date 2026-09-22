import SwiftUI

protocol HandleStruct: Identifiable, Hashable {
    var handle: UnsafeRawPointer { get }
}

extension HandleStruct {
    var id: UnsafeRawPointer { handle }

    func hash(into hasher: inout Hasher) {
        hasher.combine(handle)
    }

    static func == (lhs: Self, rhs: Self) -> Bool {
        lhs.handle == rhs.handle
    }
}

protocol MutableHandleStruct: Identifiable, Hashable {
    var handle: UnsafeMutableRawPointer { get }
}

extension MutableHandleStruct {
    var id: UnsafeMutableRawPointer { handle }

    func hash(into hasher: inout Hasher) {
        hasher.combine(handle)
    }

    static func == (lhs: Self, rhs: Self) -> Bool {
        lhs.handle == rhs.handle
    }
}

class HandleClass: Identifiable, Hashable {
    fileprivate var handle: UnsafeRawPointer

    fileprivate init(handle: UnsafeRawPointer) {
        self.handle = handle
    }

    static func == (lhs: HandleClass, rhs: HandleClass)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }
}

class MutableHandleClass: Identifiable, Hashable {
    fileprivate var handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    static func == (lhs: MutableHandleClass, rhs: MutableHandleClass)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }
}

// Types
// TODO: avoid copying
/*
class HydraString: Hashable, Identifiable {
    fileprivate var handle: HydraString
    private var ownsData = false

    fileprivate init(handle: HydraString) {
        self.handle = handle
    }

    convenience init(_ val: String) {
        self.init(handle: HydraString.stringToHydraString(val))
        self.ownsData = true
    }

    deinit {
        if ownsData {
            free(UnsafeMutableRawPointer(mutating: self.handle.data))
        }
    }

    static func == (lhs: HydraString, rhs: HydraString)
        -> Bool
    {
        lhs.value == rhs.value
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.value)
    }

    var id: UnsafeMutablePointer<CChar>? {
        self.handle.data
    }

    private static func stringToHydraString(_ val: String) -> HydraString {
        let data = val.data(using: String.Encoding.utf8)!
        let handle = data.withUnsafeBytes { bytes in
            let cCharPointer = bytes.bindMemory(to: CChar.self).baseAddress
            return HydraString(data: cCharPointer, size: data.count)
        }
        let ptr = malloc(handle.size)
        memcpy(ptr, handle.data, handle.size)
        return HydraString(data: ptr!.assumingMemoryBound(to: CChar.self), size: handle.size)
    }

    var value: String {
        get {
            if self.handle.data == nil && self.handle.size == 0 {
                return ""
            }

            let data = Data(bytes: self.handle.data, count: self.handle.size)
            return String(data: data, encoding: String.Encoding.utf8)!
        }
        set {
            if self.ownsData {
                free(UnsafeMutableRawPointer(mutating: self.handle.data))
            }
            self.handle = HydraString.stringToHydraString(newValue)
            self.ownsData = true
        }
    }

    static let empty = HydraString(handle: HydraString(data: nil, size: 0))

    func isEmpty() -> Bool {
        self.handle.data == nil
    }
}
*/

extension HydraU128: Equatable {
    public static func == (lhs: HydraU128, rhs: HydraU128) -> Bool {
        lhs.lo == rhs.lo && lhs.hi == rhs.hi
    }
}

extension HydraUint2: Equatable {
    public static func == (lhs: HydraUint2, rhs: HydraUint2) -> Bool {
        lhs.x == rhs.x && lhs.y == rhs.y
    }
}

extension HydraUchar3: Equatable {
    public static func == (lhs: HydraUchar3, rhs: HydraUchar3) -> Bool {
        lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z
    }
}

extension String {
    init(withHydraString str: HydraString) {
        if str.data == nil && str.size == 0 {
            self.init()
        } else {
            let data = Data(bytes: str.data, count: str.size)
            self.init(data: data, encoding: String.Encoding.utf8)!
        }
    }

    func withHydraString<T>(_ callback: (HydraString) -> T) -> T {
        let data = self.data(using: .utf8)!
        return data.withUnsafeBytes { bytes in
            let str = HydraString(
                data: bytes.bindMemory(to: CChar.self).baseAddress, size: data.count)
            return callback(str)
        }
    }
}

// String list
struct HydraStringList {
    internal let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var count: Int {
        Int(hydraStringListGetCount(self.handle))
    }

    var array: [String] {
        var arr: [String] = []
        for i in 0..<count {
            arr.append(get(at: i))
        }

        return arr
    }

    func get(at index: Int) -> String {
        return String(withHydraString: hydraStringListGet(self.handle, UInt32(index)))
    }

    func resize(to newCount: Int) {
        hydraStringListResize(self.handle, UInt32(newCount))
    }

    func set(at index: Int, value: String) {
        value.withHydraString { hydraString in
            hydraStringListSet(self.handle, UInt32(index), hydraString)
        }
    }

    func append(value: String) {
        value.withHydraString { hydraString in
            hydraStringListAppend(self.handle, hydraString)
        }
    }
}

// String view list
struct HydraStringViewList {
    internal let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var count: Int {
        Int(hydraStringViewListGetCount(self.handle))
    }

    var array: [String] {
        var arr: [String] = []
        for i in 0..<count {
            arr.append(get(at: i))
        }

        return arr
    }

    func get(at index: Int) -> String {
        return String(withHydraString: hydraStringViewListGet(self.handle, UInt32(index)))
    }

    func resize(to newCount: Int) {
        hydraStringViewListResize(self.handle, UInt32(newCount))
    }

    func set(at index: Int, value: String) {
        value.withHydraString { hydraString in
            hydraStringViewListSet(self.handle, UInt32(index), hydraString)
        }
    }

    func append(value: String) {
        value.withHydraString { hydraString in
            hydraStringViewListAppend(self.handle, hydraString)
        }
    }
}

// String to string map
struct HydraStringToStringMap {
    internal let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var count: Int {
        Int(hydraStringToStringMapGetCount(self.handle))
    }

    func getKey(at index: Int) -> String {
        return String(
            withHydraString: hydraStringToStringMapGetKey(self.handle, UInt32(index)))
    }

    func getValue(at index: Int) -> String {
        return String(
            withHydraString: hydraStringToStringMapGetValue(self.handle, UInt32(index)))
    }

    func getValue(byKey key: String) -> String {
        return key.withHydraString { hydraKey in
            String(
                withHydraString: hydraStringToStringMapGetValueByKey(
                    self.handle, hydraKey))
        }
    }

    func removeAll() {
        hydraStringToStringMapRemoveAll(self.handle)
    }

    func set(byKey key: String, value: String) {
        return key.withHydraString { hydraKey in
            value.withHydraString { hydraValue in
                hydraStringToStringMapSetByKey(self.handle, hydraKey, hydraValue)
            }
        }
    }
}

// Loader plugin
struct HydraLoaderPluginConfig {
    internal let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var path: String {
        get {
            String(
                withHydraString: hydraLoaderPluginGetPath(
                    self.handle))
        }
        set {
            newValue.withHydraString { hydraNewValue in
                hydraLoaderPluginSetPath(self.handle, hydraNewValue)
            }
        }
    }

    var options: HydraStringToStringMap {
        HydraStringToStringMap(handle: hydraLoaderPluginGetOptions(self.handle))
    }
}

struct HydraLoaderPluginConfigList {
    internal let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var count: Int {
        Int(hydraLoaderPluginListGetCount(self.handle))
    }

    func get(at index: Int) -> HydraLoaderPluginConfig {
        return HydraLoaderPluginConfig(
            handle: hydraLoaderPluginListGet(self.handle, UInt32(index)))
    }

    func resize(to newCount: Int) {
        hydraLoaderPluginListResize(self.handle, UInt32(newCount))
    }
}

// Config
func configSerialize() {
    hydraConfigSerialize()
}

func configDeserialize() {
    hydraConfigDeserialize()
}

func configGetAppDataPath() -> String {
    String(withHydraString: hydraConfigGetAppDataPath())
}

func configGetLogsPath() -> String {
    String(withHydraString: hydraConfigGetLogsPath())
}

func configGetGamePaths() -> HydraStringList {
    HydraStringList(handle: hydraConfigGetGamePaths())
}

func configGetLoaderPlugins() -> HydraLoaderPluginConfigList {
    HydraLoaderPluginConfigList(handle: hydraConfigGetLoaderPlugins())
}

func configGetPatchPaths() -> HydraStringList {
    HydraStringList(handle: hydraConfigGetPatchPaths())
}

func configGetInputBackend() -> UnsafeMutablePointer<UInt32> {
    hydraConfigGetInputBackend()
}

func configGetInputProfiles() -> HydraStringList {
    HydraStringList(handle: hydraConfigGetInputProfiles())
}

func configGetCpuBackend() -> UnsafeMutablePointer<UInt32> {
    hydraConfigGetCpuBackend()
}

func configGetGpuRenderer() -> UnsafeMutablePointer<UInt32> {
    hydraConfigGetGpuRenderer()
}

func configGetShaderBackend() -> UnsafeMutablePointer<UInt32> {
    hydraConfigGetShaderBackend()
}

func configGetDisplayResolution() -> UnsafeMutablePointer<UInt32> {
    hydraConfigGetDisplayResolution()
}

func configGetCustomDisplayResolution() -> UnsafeMutablePointer<HydraUint2> {
    hydraConfigGetCustomDisplayResolution()
}

func configGetAudioBackend() -> UnsafeMutablePointer<UInt32> {
    hydraConfigGetAudioBackend()
}

func configGetUserId() -> UnsafeMutablePointer<HydraU128> {
    hydraConfigGetUserId()
}

func configGetDeviceNickname() -> String {
    String.init(withHydraString: hydraConfigGetDeviceNickname())
}

func configSetDeviceNickname(_ value: String) {
    value.withHydraString { hydraString in
        hydraConfigSetDeviceNickname(hydraString)
    }
}

func configGetSystemLanguage() -> UnsafeMutablePointer<UInt32> {
    hydraConfigGetSystemLanguage()
}

func configGetSystemLocation() -> String {
    String.init(withHydraString: hydraConfigGetSystemLocation())
}

func configSetSystemLocation(_ value: String) {
    value.withHydraString { hydraString in
        hydraConfigSetSystemLocation(hydraString)
    }
}

func configGetFirmwarePath() -> String {
    String.init(withHydraString: hydraConfigGetFirmwarePath())
}

func configSetFirmwarePath(_ value: String) {
    value.withHydraString { hydraString in
        hydraConfigSetFirmwarePath(hydraString)
    }
}

func configGetSdCardPath() -> String {
    String.init(withHydraString: hydraConfigGetSdCardPath())
}

func configSetSdCardPath(_ value: String) {
    value.withHydraString { hydraString in
        hydraConfigSetSdCardPath(hydraString)
    }
}

func configGetSavePath() -> String {
    String.init(withHydraString: hydraConfigGetSavePath())
}

func configSetSavePath(_ value: String) {
    value.withHydraString { hydraString in
        hydraConfigSetSavePath(hydraString)
    }
}

func configGetSysmodulesPath() -> String {
    String.init(withHydraString: hydraConfigGetSysmodulesPath())
}

func configSetSysmodulesPath(_ value: String) {
    value.withHydraString { hydraString in
        hydraConfigSetSysmodulesPath(hydraString)
    }
}

func configGetHandheldMode() -> UnsafeMutablePointer<Bool> {
    hydraConfigGetHandheldMode()
}

func configGetLogOutput() -> UnsafeMutablePointer<UInt32> {
    hydraConfigGetLogOutput()
}

func configGetLogFsAccess() -> UnsafeMutablePointer<Bool> {
    hydraConfigGetLogFsAccess()
}

func configGetDebugLogging() -> UnsafeMutablePointer<Bool> {
    hydraConfigGetDebugLogging()
}

func configGetProcessArgs() -> HydraStringList {
    HydraStringList(handle: hydraConfigGetProcessArgs())
}

func configGetRecoverFromSegfault() -> UnsafeMutablePointer<Bool> {
    hydraConfigGetRecoverFromSegfault()
}

func configGetGdbEnabled() -> UnsafeMutablePointer<Bool> {
    hydraConfigGetGdbEnabled()
}

func configGetGdbPort() -> UnsafeMutablePointer<UInt16> {
    hydraConfigGetGdbPort()
}

func configGetGdbWaitForClient() -> UnsafeMutablePointer<Bool> {
    hydraConfigGetGdbWaitForClient()
}

// Filesystem
class HydraFilesystem: MutableHandleClass {
    init() {
        super.init(handle: hydraCreateFilesystem())
    }

    deinit {
        hydraFilesystemDestroy(self.handle)
    }
}

class HydraFile: MutableHandleClass {
    init(path: String) {
        super.init(handle: path.withHydraString { hydraPath in
            hydraOpenFile(hydraPath)
        })
    }

    deinit {
        hydraFileClose(self.handle)
    }
}

class HydraContentArchive: MutableHandleClass {
    private let file: HydraFile  // For ref counting

    init(file: HydraFile) {
        self.file = file
        super.init(handle: hydraCreateContentArchive(file.handle))
    }

    deinit {
        hydraContentArchiveDestroy(self.handle)
    }

    var contentType: HydraContentArchiveContentType {
        hydraContentArchiveGetContentType(self.handle)
    }
}

// Time zone manager
class HydraTimeZoneManager: MutableHandleClass {
    init(filesystem: HydraFilesystem) {
        super.init(handle: hydraCreateTimeZoneManager(filesystem.handle))
    }

    deinit {
        hydraTimeZoneManagerDestroy(self.handle)
    }

    var locationCount: Int {
        Int(hydraTimeZoneManagerGetLocationCount(self.handle))
    }

    func getLocation(at index: Int) -> String {
        String.init(withHydraString: hydraTimeZoneManagerGetLocation(self.handle, UInt32(index)))
    }
}

// Loader
enum HydraLoaderError: Error {
    case unsupported
}

enum HydraLoaderContent {
    case icon
    case exefs
    case romfs
}

class HydraLoader: MutableHandleClass {
    convenience init(path: String, pluginManager: HydraLoaderPluginManager?) throws {
        guard
            let handle = path.withHydraString({ hydraPath in
                hydraCreateLoaderFromPath(hydraPath, pluginManager?.handle)
            })
        else {
            throw HydraLoaderError.unsupported
        }
        self.init(handle: handle)
    }

    deinit {
        hydraLoaderDestroy(self.handle)
    }

    var titleId: UInt64 {
        hydraLoaderGetTitleId(self.handle)
    }

    func loadNacp() -> HydraNacp? {
        guard let handle = hydraLoaderLoadNacp(self.handle) else { return nil }
        return HydraNacp(handle: handle)
    }

    func loadIcon(width: inout UInt32, height: inout UInt32)
        -> UnsafeMutableRawPointer?
    {
        hydraLoaderLoadIcon(self.handle, &width, &height)
    }

    func hasIcon() -> Bool {
        return hydraLoaderHasIcon(self.handle)
    }

    func extractIcon(to path: String) {
        path.withHydraString { hydraPath in
            return hydraLoaderExtractIcon(self.handle, hydraPath)
        }
    }

    func hasExeFs() -> Bool {
        return hydraLoaderHasExefs(self.handle)
    }

    func extractExeFs(to path: String) {
        path.withHydraString { hydraPath in
            return hydraLoaderExtractExefs(self.handle, hydraPath)
        }
    }

    func hasRomFs() -> Bool {
        return hydraLoaderHasRomfs(self.handle)
    }

    func extractRomFs(to path: String) {
        path.withHydraString { hydraPath in
            return hydraLoaderExtractRomfs(self.handle, hydraPath)
        }
    }

    func hasContent(_ content: HydraLoaderContent) -> Bool {
        switch content {
        case .icon:
            return self.hasIcon()
        case .exefs:
            return self.hasExeFs()
        case .romfs:
            return self.hasRomFs()
        }
    }

    func extractContent(_ content: HydraLoaderContent, to path: String) {
        switch content {
        case .icon:
            self.extractIcon(to: path)
        case .exefs:
            self.extractExeFs(to: path)
        case .romfs:
            self.extractRomFs(to: path)
        }
    }
}

class HydraNcaLoader: HydraLoader {
    private let contentArchive: HydraContentArchive  // For ref counting

    init(contentArchive: HydraContentArchive) {
        self.contentArchive = contentArchive
        super.init(handle: hydraCreateNcaLoaderFromContentArchive(contentArchive.handle))
    }

    var name: String {
        String(withHydraString: hydraNcaLoaderGetName(self.handle))
    }
}

// Plugins
class HydraLoaderPluginManager: MutableHandleClass {
    init() {
        super.init(handle: hydraCreateLoaderPluginManager())
    }

    deinit {
        hydraLoaderPluginManagerDestroy(self.handle)
    }

    func refresh() {
        hydraLoaderPluginManagerRefresh(self.handle)
    }
}

enum HydraPluginError: Error {
    case unknown
}

class HydraLoaderPlugin: MutableHandleClass {
    init(path: String) throws {
        guard
            let handle =
                (path.withHydraString { hydraPath in
                    hydraCreateLoaderPlugin(hydraPath)
                })
        else {
            throw HydraPluginError.unknown
        }
        super.init(handle: handle)
    }

    deinit {
        hydraLoaderPluginDestroy(self.handle)
    }

    var name: String {
        String(withHydraString: hydraLoaderPluginGetName(self.handle))
    }

    var displayVersion: String {
        String(withHydraString: hydraLoaderPluginGetDisplayVersion(self.handle))
    }

    func getSupportedFormatCount() -> Int {
        Int(hydraLoaderPluginGetSupportedFormatCount(self.handle))
    }

    func getSupportedFormat(at index: Int) -> String {
        String(
            withHydraString: hydraLoaderPluginGetSupportedFormat(self.handle, UInt32(index)))
    }

    func getOptionConfigCount() -> Int {
        Int(hydraLoaderPluginGetOptionConfigCount(self.handle))
    }

    // HACK: cast immutable to mutable
    func getOptionConfig(at index: Int) -> HydraLoaderPluginOptionConfig {
        HydraLoaderPluginOptionConfig(
            handle: UnsafeMutableRawPointer(
                mutating: hydraLoaderPluginGetOptionConfig(self.handle, UInt32(index))))
    }
}

class HydraLoaderPluginOptionConfig: MutableHandleClass {
    fileprivate override init(handle: UnsafeMutableRawPointer) {
        super.init(handle: hydraLoaderPluginOptionConfigCopy(handle))
    }

    deinit {
        hydraLoaderPluginOptionConfigDestroy(self.handle)
    }

    var name: String {
        String(withHydraString: hydraLoaderPluginOptionConfigGetName(self.handle))
    }

    var description: String {
        String(withHydraString: hydraLoaderPluginOptionConfigGetDescription(self.handle))
    }

    var type: HydraLoaderPluginOptionType {
        hydraLoaderPluginOptionConfigGetType(self.handle)
    }

    var isRequired: Bool {
        hydraLoaderPluginOptionConfigGetIsRequired(self.handle)
    }

    // HACK: cast immutable to mutable
    var enumValueNames: HydraStringViewList {
        HydraStringViewList(
            handle: UnsafeMutableRawPointer(
                mutating: hydraLoaderPluginOptionConfigGetEnumValueNames(self.handle)))
    }

    var pathContentTypes: HydraStringViewList {
        HydraStringViewList(
            handle: UnsafeMutableRawPointer(
                mutating: hydraLoaderPluginOptionConfigGetPathContentTypes(self.handle)))
    }
}

struct HydraNacp: MutableHandleStruct {
    internal let handle: UnsafeMutableRawPointer

    init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    func getTitle(language: HydraSystemLanguage) -> HydraNacpTitle {
        return HydraNacpTitle(handle: hydraNacpGetTitle(self.handle, language))
    }

    var displayVersion: String {
        String(withHydraString: hydraNacpGetDisplayVersion(self.handle))
    }
}

struct HydraNacpTitle: HandleStruct {
    internal let handle: UnsafeRawPointer

    init(handle: UnsafeRawPointer) {
        self.handle = handle
    }

    var name: String {
        String(withHydraString: hydraNacpTitleGetName(self.handle))
    }

    var author: String {
        String(withHydraString: hydraNacpTitleGetAuthor(self.handle))
    }
}

// User manager
class HydraUserManager: MutableHandleClass {
    init() {
        super.init(handle: hydraCreateUserManager())
    }

    deinit {
        hydraUserManagerDestroy(self.handle)
    }

    func flush() {
        hydraUserManagerFlush(self.handle)
    }

    func createUser() -> HydraU128 {
        hydraUserManagerCreateUser(self.handle)
    }

    var userCount: Int {
        Int(hydraUserManagerGetUserCount(self.handle))
    }

    func getUserId(at index: Int) -> HydraU128 {
        hydraUserManagerGetUserId(self.handle, UInt32(index))
    }

    func getUser(id: HydraU128) -> HydraUser {
        HydraUser(handle: hydraUserManagerGetUser(self.handle, id))
    }

    func loadSystemAvatars(filesystem: HydraFilesystem) {
        hydraUserManagerLoadSystemAvatars(self.handle, filesystem.handle)
    }

    func loadAvatarImage(path: String, dimensions: inout UInt32) -> UnsafeRawPointer? {
        path.withHydraString { hydraPath in
            hydraUserManagerLoadAvatarImage(
                self.handle, hydraPath, &dimensions)
        }
    }

    var avatarCount: Int {
        Int(hydraUserManagerGetAvatarCount(self.handle))
    }

    func getAvatarPath(at index: Int) -> String {
        String(withHydraString: hydraUserManagerGetAvatarPath(self.handle, UInt32(index)))
    }
}

struct HydraUser: MutableHandleStruct {
    internal let handle: UnsafeMutableRawPointer

    init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var nickname: String {
        get {
            String(withHydraString: hydraUserGetNickname(self.handle))
        }
        set {
            newValue.withHydraString { hydraNewValue in
                hydraUserSetNickname(self.handle, hydraNewValue)
            }
        }
    }

    var avatarBgColor: HydraUchar3 {
        get {
            hydraUserGetAvatarBgColor(self.handle)
        }
        set {
            hydraUserSetAvatarBgColor(self.handle, newValue)
        }
    }

    var avatarPath: String {
        get {
            String(withHydraString: hydraUserGetAvatarPath(self.handle))
        }
        set {
            newValue.withHydraString { hydraNewValue in
                hydraUserSetAvatarPath(self.handle, hydraNewValue)
            }
        }
    }
}

// Emulation context
class HydraSystem: MutableHandleClass {
    init() {
        super.init(handle: hydraCreateSystem())
    }

    deinit {
        hydraSystemDestroy(self.handle)
    }

    var surface: UnsafeMutableRawPointer {
        get {
            // TODO: handle this properly
            UnsafeMutableRawPointer(bitPattern: 0)!
        }
        set {
            hydraSystemSetSurface(self.handle, newValue)
        }
    }

    func loadAndStart(loader: HydraLoader) {
        hydraSystemLoadAndStart(self.handle, loader.handle)
    }

    func requestStop() {
        hydraSystemRequestStop(self.handle)
    }

    func forceStop() {
        hydraSystemForceStop(self.handle)
    }

    func pause() {
        hydraSystemPause(self.handle)
    }

    func resume() {
        hydraSystemResume(self.handle)
    }

    func notifyOperationModeChanged() {
        hydraSystemNotifyOperationModeChanged(self.handle)
    }

    func progressFrame(width: UInt32, height: UInt32, dtAverageUpdated: inout Bool) {
        hydraSystemProgressFrame(self.handle, width, height, &dtAverageUpdated)
    }

    func isRunning() -> Bool {
        hydraSystemIsRunning(self.handle)
    }

    func getLastDeltaTimeAverage() -> Float {
        hydraSystemGetLastDeltaTimeAverage(self.handle)
    }

    func takeScreenshot() {
        hydraSystemTakeScreenshot(self.handle)
    }

    func captureGpuFrame() {
        hydraSystemCaptureGpuFrame(self.handle)
    }

    func textureCacheLock() {
        hydraSystemTextureCacheLock(self.handle)
    }

    func textureCacheUnlock() {
        hydraSystemTextureCacheUnlock(self.handle)
    }

    func textureCacheGetTextureMemoryCount() -> Int {
        Int(hydraSystemTextureCacheGetTextureMemoryCount(self.handle))
    }

    func textureCacheGetTextureMemory(at index: Int) -> HydraTextureMemory {
        HydraTextureMemory(handle: hydraSystemTextureCacheGetTextureMemory(self.handle, UInt32(index)))
    }
}

// Debugger
func debuggerManagerLock() {
    hydraDebuggerManagerLock()
}

func debuggerManagerUnlock() {
    hydraDebuggerManagerUnlock()
}

func debuggerManagerGetDebuggerCount() -> Int {
    Int(hydraDebuggerManagerGetDebuggerCount())
}

func debuggerManagerGetDebugger(at index: Int) -> HydraDebugger {
    HydraDebugger(handle: hydraDebuggerManagerGetDebugger(UInt32(index)))
}

// TODO: debugger for any process
func hydraDebuggerManagerGetDebuggerForCurrentProcess() -> HydraDebugger {
    HydraDebugger(handle: hydraDebuggerManagerGetDebuggerForProcess(nil))
}

struct HydraDebugger: MutableHandleStruct {
    internal let handle: UnsafeMutableRawPointer

    init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var name: String {
        String(withHydraString: hydraDebuggerGetName(self.handle))
    }

    func lock() {
        hydraDebuggerLock(self.handle)
    }

    func unlock() {
        hydraDebuggerUnlock(self.handle)
    }

    func registerThisThread(name: String) {
        name.withHydraString { hydraName in
            hydraDebuggerRegisterThisThread(self.handle, hydraName)
        }
    }

    func unregisterThisThread() {
        hydraDebuggerUnregisterThisThread(self.handle)
    }

    var threadCount: Int {
        Int(hydraDebuggerGetThreadCount(self.handle))
    }

    func getThread(at index: Int) -> HydraDebuggerThread {
        HydraDebuggerThread(handle: hydraDebuggerGetThread(self.handle, UInt32(index)))
    }
}

struct HydraDebuggerThread: MutableHandleStruct {
    internal let handle: UnsafeMutableRawPointer

    init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var name: String {
        String(withHydraString: hydraDebuggerThreadGetName(self.handle))
    }

    func lock() {
        hydraDebuggerThreadLock(self.handle)
    }

    func unlock() {
        hydraDebuggerThreadUnlock(self.handle)
    }

    var status: HydraDebuggerThreadStatus {
        hydraDebuggerThreadGetStatus(self.handle)
    }

    var breakReason: String {
        String(withHydraString: hydraDebuggerThreadGetBreakReason(self.handle))
    }

    var messageCount: Int {
        Int(hydraDebuggerThreadGetMessageCount(self.handle))
    }

    func getMessage(at index: Int) -> HydraDebuggerMessage {
        HydraDebuggerMessage(handle: hydraDebuggerThreadGetMessage(self.handle, UInt32(index)))
    }
}

struct HydraDebuggerMessage: HandleStruct {
    internal let handle: UnsafeRawPointer

    var logLevel: HydraLogLevel {
        hydraDebuggerMessageGetLogLevel(self.handle)
    }

    var logClass: HydraLogClass {
        hydraDebuggerMessageGetLogClass(self.handle)
    }

    var file: String {
        String(withHydraString: hydraDebuggerMessageGetFile(self.handle))
    }

    var line: UInt32 {
        hydraDebuggerMessageGetLine(self.handle)
    }

    var function: String {
        String(withHydraString: hydraDebuggerMessageGetFunction(self.handle))
    }

    var str: String {
        String(withHydraString: hydraDebuggerMessageGetString(self.handle))
    }

    var stackTrace: HydraDebuggerStackTrace {
        HydraDebuggerStackTrace(
            handle: hydraDebuggerStackTraceCopy(
                hydraDebuggerMessageGetStackTrace(self.handle)))
    }
}

class HydraDebuggerStackTrace: MutableHandleClass {
    deinit {
        hydraDebuggerStackTraceDestroy(self.handle)
    }

    var frameCount: Int {
        Int(hydraDebuggerStackTraceGetFrameCount(self.handle))
    }

    func getFrame(at index: Int) -> HydraDebuggerStackFrame {
        HydraDebuggerStackFrame(
            handle: hydraDebuggerStackTraceGetFrame(self.handle, UInt32(index)))
    }
}

struct HydraDebuggerStackFrame: HandleStruct {
    internal let handle: UnsafeRawPointer

    func resolve() -> HydraDebuggerResolvedStackFrame {
        HydraDebuggerResolvedStackFrame(handle: hydraDebuggerStackFrameResolve(self.handle))
    }
}

class HydraDebuggerResolvedStackFrame: MutableHandleClass {
    deinit {
        hydraDebuggerResolvedStackFrameDestroy(self.handle)
    }

    var module: String {
        String(withHydraString: hydraDebuggerResolvedStackFrameGetModule(self.handle))
    }

    var function: String {
        String(withHydraString: hydraDebuggerResolvedStackFrameGetFunction(self.handle))
    }

    var address: UInt64 {
        hydraDebuggerResolvedStackFrameGetAddress(self.handle)
    }
}

// Texture cache

// Texture memory
struct HydraTextureMemory: HandleStruct {
    internal let handle: UnsafeRawPointer

    var textureGroupCount: Int {
        Int(hydraTextureMemoryGetTextureGroupCount(self.handle))
    }

    func getTextureGroup(at index: Int) -> HydraTextureGroup {
        HydraTextureGroup(handle: hydraTextureMemoryGetTextureGroup(self.handle, UInt32(index)))
    }
}

// Texture group
struct HydraTextureGroup: HandleStruct {
    internal let handle: UnsafeRawPointer

    var textureStorageCount: Int {
        Int(hydraTextureGroupGetTextureStorageCount(self.handle))
    }

    func getTextureStorage(at index: Int) -> HydraTextureStorage {
        HydraTextureStorage(handle: hydraTextureGroupGetTextureStorage(self.handle, UInt32(index)))
    }
}

// Texture storage
struct HydraTextureStorage: HandleStruct {
    internal let handle: UnsafeRawPointer

    var descriptor: HydraTextureDescriptor {
        HydraTextureDescriptor(handle: hydraTextureStorageGetTextureDescriptor(self.handle))
    }
}

// Texture descriptor
extension HydraTextureType: Comparable {
    public static func < (lhs: HydraTextureType, rhs: HydraTextureType) -> Bool {
        return lhs.rawValue < rhs.rawValue
    }

    var description: String {
        switch self {
        case HYDRA_TEXTURE_TYPE_1D:
            return "1D"
        case HYDRA_TEXTURE_TYPE_1D_ARRAY:
            return "1D Array"
        case HYDRA_TEXTURE_TYPE_1D_BUFFER:
            return "1D Buffer"
        case HYDRA_TEXTURE_TYPE_2D:
            return "2D"
        case HYDRA_TEXTURE_TYPE_2D_ARRAY:
            return "2D Array"
        case HYDRA_TEXTURE_TYPE_3D:
            return "3D"
        case HYDRA_TEXTURE_TYPE_CUBE:
            return "Cube"
        case HYDRA_TEXTURE_TYPE_CUBE_ARRAY:
            return "Cube Array"
        default:
            return "Unknown \(self.rawValue)"
        }
    }
}

extension HydraTextureFormat: Comparable {
    public static func < (lhs: HydraTextureFormat, rhs: HydraTextureFormat) -> Bool {
        return lhs.rawValue < rhs.rawValue
    }

    var description: String {
        switch self {
        case HYDRA_TEXTURE_FORMAT_INVALID: return "Invalid"

        case HYDRA_TEXTURE_FORMAT_R8_UNORM: return "R8 Unorm"
        case HYDRA_TEXTURE_FORMAT_R8_SNORM: return "R8 Snorm"
        case HYDRA_TEXTURE_FORMAT_R8_UINT: return "R8 UInt"
        case HYDRA_TEXTURE_FORMAT_R8_SINT: return "R8 SInt"
        case HYDRA_TEXTURE_FORMAT_R16_FLOAT: return "R16 Float"
        case HYDRA_TEXTURE_FORMAT_R16_UNORM: return "R16 Unorm"
        case HYDRA_TEXTURE_FORMAT_R16_SNORM: return "R16 Snorm"
        case HYDRA_TEXTURE_FORMAT_R16_UINT: return "R16 UInt"
        case HYDRA_TEXTURE_FORMAT_R16_SINT: return "R16 SInt"
        case HYDRA_TEXTURE_FORMAT_R32_FLOAT: return "R32 Float"
        case HYDRA_TEXTURE_FORMAT_R32_UINT: return "R32 UInt"
        case HYDRA_TEXTURE_FORMAT_R32_SINT: return "R32 SInt"

        case HYDRA_TEXTURE_FORMAT_RG8_UNORM: return "RG8 Unorm"
        case HYDRA_TEXTURE_FORMAT_RG8_SNORM: return "RG8 Snorm"
        case HYDRA_TEXTURE_FORMAT_RG8_UINT: return "RG8 UInt"
        case HYDRA_TEXTURE_FORMAT_RG8_SINT: return "RG8 SInt"
        case HYDRA_TEXTURE_FORMAT_RG16_FLOAT: return "RG16 Float"
        case HYDRA_TEXTURE_FORMAT_RG16_UNORM: return "RG16 Unorm"
        case HYDRA_TEXTURE_FORMAT_RG16_SNORM: return "RG16 Snorm"
        case HYDRA_TEXTURE_FORMAT_RG16_UINT: return "RG16 UInt"
        case HYDRA_TEXTURE_FORMAT_RG16_SINT: return "RG16 SInt"
        case HYDRA_TEXTURE_FORMAT_RG32_FLOAT: return "RG32 Float"
        case HYDRA_TEXTURE_FORMAT_RG32_UINT: return "RG32 UInt"
        case HYDRA_TEXTURE_FORMAT_RG32_SINT: return "RG32 SInt"

        case HYDRA_TEXTURE_FORMAT_RGB32_FLOAT: return "RGB32 Float"
        case HYDRA_TEXTURE_FORMAT_RGB32_UINT: return "RGB32 UInt"
        case HYDRA_TEXTURE_FORMAT_RGB32_SINT: return "RGB32 SInt"

        case HYDRA_TEXTURE_FORMAT_RGBA8_UNORM: return "RGBA8 Unorm"
        case HYDRA_TEXTURE_FORMAT_RGBA8_SNORM: return "RGBA8 Snorm"
        case HYDRA_TEXTURE_FORMAT_RGBA8_UINT: return "RGBA8 UInt"
        case HYDRA_TEXTURE_FORMAT_RGBA8_SINT: return "RGBA8 SInt"
        case HYDRA_TEXTURE_FORMAT_RGBA16_FLOAT: return "RGBA16 Float"
        case HYDRA_TEXTURE_FORMAT_RGBA16_UNORM: return "RGBA16 Unorm"
        case HYDRA_TEXTURE_FORMAT_RGBA16_SNORM: return "RGBA16 Snorm"
        case HYDRA_TEXTURE_FORMAT_RGBA16_UINT: return "RGBA16 UInt"
        case HYDRA_TEXTURE_FORMAT_RGBA16_SINT: return "RGBA16 SInt"
        case HYDRA_TEXTURE_FORMAT_RGBA32_FLOAT: return "RGBA32 Float"
        case HYDRA_TEXTURE_FORMAT_RGBA32_UINT: return "RGBA32 UInt"
        case HYDRA_TEXTURE_FORMAT_RGBA32_SINT: return "RGBA32 SInt"

        case HYDRA_TEXTURE_FORMAT_S8_UINT: return "S8 UInt"
        case HYDRA_TEXTURE_FORMAT_Z16_UNORM: return "Z16 Unorm"
        case HYDRA_TEXTURE_FORMAT_Z24_UNORM_X8_UINT: return "Z24 Unorm X8 UInt"
        case HYDRA_TEXTURE_FORMAT_Z32_FLOAT: return "Z32 Float"
        case HYDRA_TEXTURE_FORMAT_Z24_UNORM_S8_UINT: return "Z24 Unorm S8 UInt"
        case HYDRA_TEXTURE_FORMAT_Z32_FLOAT_X24_S8_UINT: return "Z32 Float X24 S8 UInt"

        case HYDRA_TEXTURE_FORMAT_RGBX8_UNORM_SRGB: return "RGBX8 Unorm sRGB"
        case HYDRA_TEXTURE_FORMAT_RGBA8_UNORM_SRGB: return "RGBA8 Unorm sRGB"
        case HYDRA_TEXTURE_FORMAT_RGBA4_UNORM: return "RGBA4 Unorm"
        case HYDRA_TEXTURE_FORMAT_RGB5_UNORM: return "RGB5 Unorm"
        case HYDRA_TEXTURE_FORMAT_RGB5A1_UNORM: return "RGB5A1 Unorm"
        case HYDRA_TEXTURE_FORMAT_R5G6B5_UNORM: return "R5G6B5 Unorm"
        case HYDRA_TEXTURE_FORMAT_RGB10A2_UNORM: return "RGB10A2 Unorm"
        case HYDRA_TEXTURE_FORMAT_RGB10A2_UINT: return "RGB10A2 UInt"
        case HYDRA_TEXTURE_FORMAT_RG11B10_FLOAT: return "RG11B10 Float"
        case HYDRA_TEXTURE_FORMAT_E5BGR9_FLOAT: return "E5BGR9 Float"

        case HYDRA_TEXTURE_FORMAT_BC1_RGB: return "BC1 RGB"
        case HYDRA_TEXTURE_FORMAT_BC1_RGBA: return "BC1 RGBA"
        case HYDRA_TEXTURE_FORMAT_BC2_RGBA: return "BC2 RGBA"
        case HYDRA_TEXTURE_FORMAT_BC3_RGBA: return "BC3 RGBA"
        case HYDRA_TEXTURE_FORMAT_BC1_RGB_SRGB: return "BC1 RGB sRGB"
        case HYDRA_TEXTURE_FORMAT_BC1_RGBA_SRGB: return "BC1 RGBA sRGB"
        case HYDRA_TEXTURE_FORMAT_BC2_RGBA_SRGB: return "BC2 RGBA sRGB"
        case HYDRA_TEXTURE_FORMAT_BC3_RGBA_SRGB: return "BC3 RGBA sRGB"
        case HYDRA_TEXTURE_FORMAT_BC4_R_UNORM: return "BC4 R Unorm"
        case HYDRA_TEXTURE_FORMAT_BC4_R_SNORM: return "BC4 R Snorm"
        case HYDRA_TEXTURE_FORMAT_BC5_RG_UNORM: return "BC5 RG Unorm"
        case HYDRA_TEXTURE_FORMAT_BC5_RG_SNORM: return "BC5 RG Snorm"
        case HYDRA_TEXTURE_FORMAT_BC7_RGBA_UNORM: return "BC7 RGBA Unorm"
        case HYDRA_TEXTURE_FORMAT_BC7_RGBA_UNORM_SRGB: return "BC7 RGBA Unorm sRGB"
        case HYDRA_TEXTURE_FORMAT_BC6H_RGBA_SF16_FLOAT: return "BC6H SF16 Float"
        case HYDRA_TEXTURE_FORMAT_BC6H_RGBA_UF16_FLOAT: return "BC6H UF16 Float"

        case HYDRA_TEXTURE_FORMAT_RGBX8_UNORM: return "RGBX8 Unorm"
        case HYDRA_TEXTURE_FORMAT_RGBX8_SNORM: return "RGBX8 Snorm"
        case HYDRA_TEXTURE_FORMAT_RGBX8_UINT: return "RGBX8 UInt"
        case HYDRA_TEXTURE_FORMAT_RGBX8_SINT: return "RGBX8 SInt"
        case HYDRA_TEXTURE_FORMAT_RGBX16_FLOAT: return "RGBX16 Float"
        case HYDRA_TEXTURE_FORMAT_RGBX16_UNORM: return "RGBX16 Unorm"
        case HYDRA_TEXTURE_FORMAT_RGBX16_SNORM: return "RGBX16 Snorm"
        case HYDRA_TEXTURE_FORMAT_RGBX16_UINT: return "RGBX16 UInt"
        case HYDRA_TEXTURE_FORMAT_RGBX16_SINT: return "RGBX16 SInt"
        case HYDRA_TEXTURE_FORMAT_RGBX32_FLOAT: return "RGBX32 Float"
        case HYDRA_TEXTURE_FORMAT_RGBX32_UINT: return "RGBX32 UInt"
        case HYDRA_TEXTURE_FORMAT_RGBX32_SINT: return "RGBX32 SInt"

        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_4X4: return "ASTC 4x4"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_5X4: return "ASTC 5x4"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_5X5: return "ASTC 5x5"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_6X5: return "ASTC 6x5"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_6X6: return "ASTC 6x6"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X5: return "ASTC 8x5"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X6: return "ASTC 8x6"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X8: return "ASTC 8x8"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X5: return "ASTC 10x5"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X6: return "ASTC 10x6"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X8: return "ASTC 10x8"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X10: return "ASTC 10x10"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_12X10: return "ASTC 12x10"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_12X12: return "ASTC 12x12"

        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_4X4_SRGB: return "ASTC 4x4 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_5X4_SRGB: return "ASTC 5x4 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_5X5_SRGB: return "ASTC 5x5 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_6X5_SRGB: return "ASTC 6x5 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_6X6_SRGB: return "ASTC 6x6 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X5_SRGB: return "ASTC 8x5 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X6_SRGB: return "ASTC 8x6 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_8X8_SRGB: return "ASTC 8x8 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X5_SRGB: return "ASTC 10x5 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X6_SRGB: return "ASTC 10x6 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X8_SRGB: return "ASTC 10x8 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_10X10_SRGB: return "ASTC 10x10 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_12X10_SRGB: return "ASTC 12x10 sRGB"
        case HYDRA_TEXTURE_FORMAT_ASTC_RGBA_12X12_SRGB: return "ASTC 12x12 sRGB"

        case HYDRA_TEXTURE_FORMAT_B5G6R5_UNORM: return "B5G6R5 Unorm"
        case HYDRA_TEXTURE_FORMAT_BGR5_UNORM: return "BGR5 Unorm"
        case HYDRA_TEXTURE_FORMAT_BGR5A1_UNORM: return "BGR5A1 Unorm"
        case HYDRA_TEXTURE_FORMAT_A1BGR5_UNORM: return "A1BGR5 Unorm"
        case HYDRA_TEXTURE_FORMAT_BGRX8_UNORM: return "BGRX8 Unorm"
        case HYDRA_TEXTURE_FORMAT_BGRA8_UNORM: return "BGRA8 Unorm"
        case HYDRA_TEXTURE_FORMAT_BGRX8_UNORM_SRGB: return "BGRX8 Unorm sRGB"
        case HYDRA_TEXTURE_FORMAT_BGRA8_UNORM_SRGB: return "BGRA8 Unorm sRGB"

        case HYDRA_TEXTURE_FORMAT_ETC2_R_UNORM: return "ETC2 R Unorm"
        case HYDRA_TEXTURE_FORMAT_ETC2_R_SNORM: return "ETC2 R Snorm"
        case HYDRA_TEXTURE_FORMAT_ETC2_RG_UNORM: return "ETC2 RG Unorm"
        case HYDRA_TEXTURE_FORMAT_ETC2_RG_SNORM: return "ETC2 RG Snorm"
        case HYDRA_TEXTURE_FORMAT_ETC2_RGB: return "ETC2 RGB"
        case HYDRA_TEXTURE_FORMAT_PTA_ETC2_RGB: return "PTA ETC2 RGB"
        case HYDRA_TEXTURE_FORMAT_ETC2_RGBA: return "ETC2 RGBA"
        case HYDRA_TEXTURE_FORMAT_ETC2_RGB_SRGB: return "ETC2 RGB sRGB"
        case HYDRA_TEXTURE_FORMAT_PTA_ETC2_RGB_SRGB: return "PTA ETC2 RGB sRGB"
        case HYDRA_TEXTURE_FORMAT_ETC2_RGBA_SRGB: return "ETC2 RGBA sRGB"

        default:
            return "Unknown (\(self.rawValue))"
        }
    }
}

struct HydraTextureDescriptor: HandleStruct {
    internal let handle: UnsafeRawPointer

    var ptr: UInt64 {
        hydraTextureDescriptorGetPtr(self.handle)
    }

    var type: HydraTextureType {
        hydraTextureDescriptorGetType(self.handle)
    }

    var format: HydraTextureFormat {
        hydraTextureDescriptorGetFormat(self.handle)
    }

    var width: UInt32 {
        hydraTextureDescriptorGetWidth(self.handle)
    }

    var height: UInt32 {
        hydraTextureDescriptorGetHeight(self.handle)
    }

    var depth: UInt32 {
        hydraTextureDescriptorGetDepth(self.handle)
    }

    var levelCount: UInt32 {
        hydraTextureDescriptorGetLevelCount(self.handle)
    }

    var layerCount: UInt32 {
        hydraTextureDescriptorGetLayerCount(self.handle)
    }

    var blockWidthGobs: UInt32 {
        hydraTextureDescriptorGetBlockWidthGobs(self.handle)
    }

    var blockHeightGobs: UInt32 {
        hydraTextureDescriptorGetBlockHeightGobs(self.handle)
    }

    var blockDepthGobs: UInt32 {
        hydraTextureDescriptorGetBlockDepthGobs(self.handle)
    }

    var layerSize: UInt64 {
        hydraTextureDescriptorGetLayerSize(self.handle)
    }

    var size: UInt64 {
        hydraTextureDescriptorGetSize(self.handle)
    }
}
