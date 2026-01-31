import SwiftUI

// Types
// TODO: avoid copying
/*
class HydraString: Hashable, Identifiable {
    fileprivate var handle: hydra_string
    private var ownsData = false

    fileprivate init(handle: hydra_string) {
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

    private static func stringToHydraString(_ val: String) -> hydra_string {
        let data = val.data(using: String.Encoding.utf8)!
        let handle = data.withUnsafeBytes { bytes in
            let cCharPointer = bytes.bindMemory(to: CChar.self).baseAddress
            return hydra_string(data: cCharPointer, size: data.count)
        }
        let ptr = malloc(handle.size)
        memcpy(ptr, handle.data, handle.size)
        return hydra_string(data: ptr!.assumingMemoryBound(to: CChar.self), size: handle.size)
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

    static let empty = HydraString(handle: hydra_string(data: nil, size: 0))

    func isEmpty() -> Bool {
        self.handle.data == nil
    }
}
*/

extension hydra_u128: Equatable {
    public static func == (lhs: hydra_u128, rhs: hydra_u128) -> Bool {
        lhs.lo == rhs.lo && lhs.hi == rhs.hi
    }
}

extension hydra_uint2: Equatable {
    public static func == (lhs: hydra_uint2, rhs: hydra_uint2) -> Bool {
        lhs.x == rhs.x && lhs.y == rhs.y
    }
}

extension hydra_uchar3: Equatable {
    public static func == (lhs: hydra_uchar3, rhs: hydra_uchar3) -> Bool {
        lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z
    }
}

extension String {
    init(withHydraString str: hydra_string) {
        if str.data == nil && str.size == 0 {
            self.init()
        } else {
            let data = Data(bytes: str.data, count: str.size)
            self.init(data: data, encoding: String.Encoding.utf8)!
        }
    }

    func withHydraString<T>(_ callback: (hydra_string) -> T) -> T {
        let data = self.data(using: .utf8)!
        return data.withUnsafeBytes { bytes in
            let str = hydra_string(
                data: bytes.bindMemory(to: CChar.self).baseAddress, size: data.count)
            return callback(str)
        }
    }
}

// String list
struct HydraStringList {
    private let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var count: Int {
        Int(hydra_string_list_get_count(self.handle))
    }

    var array: [String] {
        var arr: [String] = []
        for i in 0..<count {
            arr.append(get(at: i))
        }

        return arr
    }

    func get(at index: Int) -> String {
        return String(withHydraString: hydra_string_list_get(self.handle, UInt32(index)))
    }

    func resize(to newCount: Int) {
        hydra_string_list_resize(self.handle, UInt32(newCount))
    }

    func set(at index: Int, value: String) {
        value.withHydraString { hydraString in
            hydra_string_list_set(self.handle, UInt32(index), hydraString)
        }
    }

    func append(value: String) {
        value.withHydraString { hydraString in
            hydra_string_list_append(self.handle, hydraString)
        }
    }
}

// String view list
struct HydraStringViewList {
    private let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var count: Int {
        Int(hydra_string_view_list_get_count(self.handle))
    }

    var array: [String] {
        var arr: [String] = []
        for i in 0..<count {
            arr.append(get(at: i))
        }

        return arr
    }

    func get(at index: Int) -> String {
        return String(withHydraString: hydra_string_view_list_get(self.handle, UInt32(index)))
    }

    func resize(to newCount: Int) {
        hydra_string_view_list_resize(self.handle, UInt32(newCount))
    }

    func set(at index: Int, value: String) {
        value.withHydraString { hydraString in
            hydra_string_view_list_set(self.handle, UInt32(index), hydraString)
        }
    }

    func append(value: String) {
        value.withHydraString { hydraString in
            hydra_string_view_list_append(self.handle, hydraString)
        }
    }
}

// String to string map
struct HydraStringToStringMap {
    private let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var count: Int {
        Int(hydra_string_to_string_map_get_count(self.handle))
    }

    func getKey(at index: Int) -> String {
        return String(
            withHydraString: hydra_string_to_string_map_get_key(self.handle, UInt32(index)))
    }

    func getValue(at index: Int) -> String {
        return String(
            withHydraString: hydra_string_to_string_map_get_value(self.handle, UInt32(index)))
    }

    func getValue(byKey key: String) -> String {
        return key.withHydraString { hydraKey in
            String(
                withHydraString: hydra_string_to_string_map_get_value_by_key(
                    self.handle, hydraKey))
        }
    }

    func removeAll() {
        hydra_string_to_string_map_remove_all(self.handle)
    }

    func set(byKey key: String, value: String) {
        return key.withHydraString { hydraKey in
            value.withHydraString { hydraValue in
                hydra_string_to_string_map_set_by_key(self.handle, hydraKey, hydraValue)
            }
        }
    }
}

// Loader plugin
struct HydraLoaderPluginConfig {
    private let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var path: String {
        get {
            String(
                withHydraString: hydra_loader_plugin_get_path(
                    self.handle))
        }
        set {
            newValue.withHydraString { hydraNewValue in
                hydra_loader_plugin_set_path(self.handle, hydraNewValue)
            }
        }
    }

    var options: HydraStringToStringMap {
        HydraStringToStringMap(handle: hydra_loader_plugin_get_options(self.handle))
    }
}

struct HydraLoaderPluginConfigList {
    private let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var count: Int {
        Int(hydra_loader_plugin_list_get_count(self.handle))
    }

    func get(at index: Int) -> HydraLoaderPluginConfig {
        return HydraLoaderPluginConfig(
            handle: hydra_loader_plugin_list_get(self.handle, UInt32(index)))
    }

    func resize(to newCount: Int) {
        hydra_loader_plugin_list_resize(self.handle, UInt32(newCount))
    }
}

// Config
func hydraConfigSerialize() {
    hydra_config_serialize()
}

func hydraConfigDeserialize() {
    hydra_config_deserialize()
}

func hydraConfigGetAppDataPath() -> String {
    String(withHydraString: hydra_config_get_app_data_path())
}

func hydraConfigGetGamePaths() -> HydraStringList {
    HydraStringList(handle: hydra_config_get_game_paths())
}

func hydraConfigGetLoaderPlugins() -> HydraLoaderPluginConfigList {
    HydraLoaderPluginConfigList(handle: hydra_config_get_loader_plugins())
}

func hydraConfigGetPatchPaths() -> HydraStringList {
    HydraStringList(handle: hydra_config_get_patch_paths())
}

func hydraConfigGetInputProfiles() -> HydraStringList {
    HydraStringList(handle: hydra_config_get_input_profiles())
}

func hydraConfigGetCpuBackend() -> UnsafeMutablePointer<UInt32> {
    hydra_config_get_cpu_backend()
}

func hydraConfigGetGpuRenderer() -> UnsafeMutablePointer<UInt32> {
    hydra_config_get_gpu_renderer()
}

func hydraConfigGetShaderBackend() -> UnsafeMutablePointer<UInt32> {
    hydra_config_get_shader_backend()
}

func hydraConfigGetDisplayResolution() -> UnsafeMutablePointer<UInt32> {
    hydra_config_get_display_resolution()
}

func hydraConfigGetCustomDisplayResolution() -> UnsafeMutablePointer<hydra_uint2> {
    hydra_config_get_custom_display_resolution()
}

func hydraConfigGetAudioBackend() -> UnsafeMutablePointer<UInt32> {
    hydra_config_get_audio_backend()
}

func hydraConfigGetUserId() -> UnsafeMutablePointer<hydra_u128> {
    hydra_config_get_user_id()
}

func hydraConfigGetFirmwarePath() -> String {
    String.init(withHydraString: hydra_config_get_firmware_path())
}

func hydraConfigSetFirmwarePath(_ value: String) {
    value.withHydraString { hydraString in
        hydra_config_set_firmware_path(hydraString)
    }
}

func hydraConfigGetSdCardPath() -> String {
    String.init(withHydraString: hydra_config_get_sd_card_path())
}

func hydraConfigSetSdCardPath(_ value: String) {
    value.withHydraString { hydraString in
        hydra_config_set_sd_card_path(hydraString)
    }
}

func hydraConfigGetSavePath() -> String {
    String.init(withHydraString: hydra_config_get_save_path())
}

func hydraConfigSetSavePath(_ value: String) {
    value.withHydraString { hydraString in
        hydra_config_set_save_path(hydraString)
    }
}

func hydraConfigGetSysmodulesPath() -> String {
    String.init(withHydraString: hydra_config_get_sysmodules_path())
}

func hydraConfigSetSysmodulesPath(_ value: String) {
    value.withHydraString { hydraString in
        hydra_config_set_sysmodules_path(hydraString)
    }
}

func hydraConfigGetHandheldMode() -> UnsafeMutablePointer<Bool> {
    hydra_config_get_handheld_mode()
}

func hydraConfigGetLogOutput() -> UnsafeMutablePointer<UInt32> {
    hydra_config_get_log_output()
}

func hydraConfigGetLogFsAccess() -> UnsafeMutablePointer<Bool> {
    hydra_config_get_log_fs_access()
}

func hydraConfigGetDebugLogging() -> UnsafeMutablePointer<Bool> {
    hydra_config_get_debug_logging()
}

func hydraConfigGetProcessArgs() -> HydraStringList {
    HydraStringList(handle: hydra_config_get_process_args())
}

func hydraConfigGetRecoverFromSegfault() -> UnsafeMutablePointer<Bool> {
    hydra_config_get_recover_from_segfault()
}

func hydraConfigGetGdbEnabled() -> UnsafeMutablePointer<Bool> {
    hydra_config_get_gdb_enabled()
}

func hydraConfigGetGdbPort() -> UnsafeMutablePointer<UInt16> {
    hydra_config_get_gdb_port()
}

func hydraConfigGetGdbWaitForClient() -> UnsafeMutablePointer<Bool> {
    hydra_config_get_gdb_wait_for_client()
}

// Loader plugins
func hydraLoaderPluginManagerRefresh() {
    hydra_loader_plugin_manager_refresh()
}

enum HydraPluginError: Error {
    case unknown
}

class HydraLoaderPlugin {
    private let handle: UnsafeMutableRawPointer

    init(path: String) throws {
        guard
            let handle =
                (path.withHydraString { hydraPath in
                    hydra_create_loader_plugin(hydraPath)
                })
        else {
            throw HydraPluginError.unknown
        }
        self.handle = handle
    }

    deinit {
        hydra_loader_plugin_destroy(self.handle)
    }

    var name: String {
        String(withHydraString: hydra_loader_plugin_get_name(self.handle))
    }

    var displayVersion: String {
        String(withHydraString: hydra_loader_plugin_get_display_version(self.handle))
    }

    func getSupportedFormatCount() -> Int {
        Int(hydra_loader_plugin_get_supported_format_count(self.handle))
    }

    func getSupportedFormat(at index: Int) -> String {
        String(
            withHydraString: hydra_loader_plugin_get_supported_format(self.handle, UInt32(index)))
    }

    func getOptionConfigCount() -> Int {
        Int(hydra_loader_plugin_get_option_config_count(self.handle))
    }

    // HACK: cast immutable to mutable
    func getOptionConfig(at index: Int) -> HydraLoaderPluginOptionConfig {
        HydraLoaderPluginOptionConfig(
            handle: UnsafeMutableRawPointer(
                mutating: hydra_loader_plugin_get_option_config(self.handle, UInt32(index))))
    }
}

class HydraLoaderPluginOptionConfig: Hashable, Identifiable {
    private let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = hydra_loader_plugin_option_config_copy(handle)
    }

    deinit {
        hydra_loader_plugin_option_config_destroy(self.handle)
    }

    static func == (lhs: HydraLoaderPluginOptionConfig, rhs: HydraLoaderPluginOptionConfig)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    var name: String {
        String(withHydraString: hydra_loader_plugin_option_config_get_name(self.handle))
    }

    var description: String {
        String(withHydraString: hydra_loader_plugin_option_config_get_description(self.handle))
    }

    var type: HydraLoaderPluginOptionType {
        hydra_loader_plugin_option_config_get_type(self.handle)
    }

    var isRequired: Bool {
        hydra_loader_plugin_option_config_get_is_required(self.handle)
    }

    // HACK: cast immutable to mutable
    var enumValueNames: HydraStringViewList {
        HydraStringViewList(
            handle: UnsafeMutableRawPointer(
                mutating: hydra_loader_plugin_option_config_get_enum_value_names(self.handle)))
    }

    var pathContentTypes: HydraStringViewList {
        HydraStringViewList(
            handle: UnsafeMutableRawPointer(
                mutating: hydra_loader_plugin_option_config_get_path_content_types(self.handle)))
    }
}

// Filesystem
class HydraFilesystem {
    fileprivate let handle: UnsafeMutableRawPointer

    init() {
        self.handle = hydra_create_filesystem()
    }

    deinit {
        hydra_filesystem_destroy(self.handle)
    }

    static func == (lhs: HydraFilesystem, rhs: HydraFilesystem)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    func tryInstallFirmware() {
        hydra_try_install_firmware_to_filesystem(self.handle)
    }
}

class HydraFile: Hashable, Identifiable {
    fileprivate let handle: UnsafeMutableRawPointer

    init(path: String) {
        self.handle = path.withHydraString { hydraPath in
            hydra_open_file(hydraPath)
        }
    }

    deinit {
        hydra_file_close(self.handle)
    }

    static func == (lhs: HydraFile, rhs: HydraFile)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }
}

class HydraContentArchive: Hashable, Identifiable {
    private let file: HydraFile  // For ref counting

    fileprivate let handle: UnsafeMutableRawPointer

    init(file: HydraFile) {
        self.file = file
        self.handle = hydra_create_content_archive(file.handle)
    }

    deinit {
        hydra_content_archive_destroy(self.handle)
    }

    static func == (lhs: HydraContentArchive, rhs: HydraContentArchive)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    var contentType: HydraContentArchiveContentType {
        hydra_content_archive_get_content_type(self.handle)
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

class HydraLoader: Hashable, Identifiable {
    fileprivate let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    convenience init(path: String) throws {
        guard
            let handle = path.withHydraString({ hydraPath in
                hydra_create_loader_from_path(hydraPath)
            })
        else {
            throw HydraLoaderError.unsupported
        }
        self.init(handle: handle)
    }

    deinit {
        hydra_loader_destroy(self.handle)
    }

    static func == (lhs: HydraLoader, rhs: HydraLoader)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    var titleId: UInt64 {
        hydra_loader_get_title_id(self.handle)
    }

    func loadNacp() -> HydraNacp? {
        guard let handle = hydra_loader_load_nacp(self.handle) else { return nil }
        return HydraNacp(handle: handle)
    }

    func loadIcon(width: inout UInt32, height: inout UInt32)
        -> UnsafeMutableRawPointer?
    {
        hydra_loader_load_icon(self.handle, &width, &height)
    }

    func hasIcon() -> Bool {
        return hydra_loader_has_icon(self.handle)
    }

    func extractIcon(to path: String) {
        path.withHydraString { hydraPath in
            return hydra_loader_extract_icon(self.handle, hydraPath)
        }
    }

    func hasExeFs() -> Bool {
        return hydra_loader_has_exefs(self.handle)
    }

    func extractExeFs(to path: String) {
        path.withHydraString { hydraPath in
            return hydra_loader_extract_exefs(self.handle, hydraPath)
        }
    }

    func hasRomFs() -> Bool {
        return hydra_loader_has_romfs(self.handle)
    }

    func extractRomFs(to path: String) {
        path.withHydraString { hydraPath in
            return hydra_loader_extract_romfs(self.handle, hydraPath)
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
        super.init(handle: hydra_create_nca_loader_from_content_archive(contentArchive.handle))
    }

    var name: String {
        String(withHydraString: hydra_nca_loader_get_name(self.handle))
    }
}

class HydraNacp: Hashable, Identifiable {
    private let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    static func == (lhs: HydraNacp, rhs: HydraNacp)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    var title: HydraNacpTitle {
        HydraNacpTitle(handle: hydra_nacp_get_title(self.handle))
    }

    var displayVersion: String {
        String(withHydraString: hydra_nacp_get_display_version(self.handle))
    }
}

class HydraNacpTitle: Hashable, Identifiable {
    private let handle: UnsafeRawPointer

    fileprivate init(handle: UnsafeRawPointer) {
        self.handle = handle
    }

    static func == (lhs: HydraNacpTitle, rhs: HydraNacpTitle)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    var name: String {
        String(withHydraString: hydra_nacp_title_get_name(self.handle))
    }

    var author: String {
        String(withHydraString: hydra_nacp_title_get_author(self.handle))
    }
}

// User manager
class HydraUserManager: Hashable, Identifiable {
    private let handle: UnsafeMutableRawPointer

    init() {
        self.handle = hydra_create_user_manager()
    }

    deinit {
        hydra_user_manager_destroy(self.handle)
    }

    static func == (lhs: HydraUserManager, rhs: HydraUserManager)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    func flush() {
        hydra_user_manager_flush(self.handle)
    }

    func createUser() -> hydra_u128 {
        hydra_user_manager_create_user(self.handle)
    }

    var userCount: Int {
        Int(hydra_user_manager_get_user_count(self.handle))
    }

    func getUserId(at index: Int) -> hydra_u128 {
        hydra_user_manager_get_user_id(self.handle, UInt32(index))
    }

    func getUser(id: hydra_u128) -> HydraUser {
        HydraUser(handle: hydra_user_manager_get_user(self.handle, id))
    }

    func loadSystemAvatars(filesystem: HydraFilesystem) {
        hydra_user_manager_load_system_avatars(self.handle, filesystem.handle)
    }

    func loadAvatarImage(path: String, dimensions: inout UInt32) -> UnsafeRawPointer? {
        path.withHydraString { hydraPath in
            hydra_user_manager_load_avatar_image(
                self.handle, hydraPath, &dimensions)
        }
    }

    var avatarCount: Int {
        Int(hydra_user_manager_get_avatar_count(self.handle))
    }

    func getAvatarPath(at index: Int) -> String {
        String(withHydraString: hydra_user_manager_get_avatar_path(self.handle, UInt32(index)))
    }
}

struct HydraUser: Hashable, Identifiable {
    private let handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var id: UnsafeMutableRawPointer {
        self.handle
    }

    var nickname: String {
        get {
            String(withHydraString: hydra_user_get_nickname(self.handle))
        }
        set {
            newValue.withHydraString { hydraNewValue in
                hydra_user_set_nickname(self.handle, hydraNewValue)
            }
        }
    }

    var avatarBgColor: hydra_uchar3 {
        get {
            hydra_user_get_avatar_bg_color(self.handle)
        }
        set {
            hydra_user_set_avatar_bg_color(self.handle, newValue)
        }
    }

    var avatarPath: String {
        get {
            String(withHydraString: hydra_user_get_avatar_path(self.handle))
        }
        set {
            newValue.withHydraString { hydraNewValue in
                hydra_user_set_avatar_path(self.handle, hydraNewValue)
            }
        }
    }
}

// Emulation context
class HydraEmulationContext: Hashable, Identifiable {
    private let handle: UnsafeMutableRawPointer

    init() {
        self.handle = hydra_create_emulation_context()
    }

    deinit {
        hydra_emulation_context_destroy(self.handle)
    }

    static func == (lhs: HydraEmulationContext, rhs: HydraEmulationContext)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    var surface: UnsafeMutableRawPointer {
        get {
            // TODO: handle this properly
            UnsafeMutableRawPointer(bitPattern: 0)!
        }
        set {
            hydra_emulation_context_set_surface(self.handle, newValue)
        }
    }

    func loadAndStart(loader: HydraLoader) {
        hydra_emulation_context_load_and_start(self.handle, loader.handle)
    }

    func requestStop() {
        hydra_emulation_context_request_stop(self.handle)
    }

    func forceStop() {
        hydra_emulation_context_force_stop(self.handle)
    }

    func pause() {
        hydra_emulation_context_pause(self.handle)
    }

    func resume() {
        hydra_emulation_context_resume(self.handle)
    }

    func notifyOperationModeChanged() {
        hydra_emulation_context_notify_operation_mode_changed(self.handle)
    }

    func progressFrame(width: UInt32, height: UInt32, dtAverageUpdated: inout Bool) {
        hydra_emulation_context_progress_frame(self.handle, width, height, &dtAverageUpdated)
    }

    func isRunning() -> Bool {
        hydra_emulation_context_is_running(self.handle)
    }

    func getLastDeltaTimeAverage() -> Float {
        hydra_emulation_context_get_last_delta_time_average(self.handle)
    }

    func takeScreenshot() {
        hydra_emulation_context_take_screenshot(self.handle)
    }

    func captureGpuFrame() {
        hydra_emulation_context_capture_gpu_frame(self.handle)
    }
}

// Debugger
func hydraDebuggerManagerLock() {
    hydra_debugger_manager_lock()
}

func hydraDebuggerManagerUnlock() {
    hydra_debugger_manager_unlock()
}

func hydraDebuggerManagerGetDebuggerCount() -> Int {
    Int(hydra_debugger_manager_get_debugger_count())
}

func hydraDebuggerManagerGetDebugger(at index: Int) -> HydraDebugger {
    HydraDebugger(handle: hydra_debugger_manager_get_debugger(UInt32(index)))
}

// TODO: debugger for any process
func hydraDebuggerManagerGetDebuggerForCurrentProcess() -> HydraDebugger {
    HydraDebugger(handle: hydra_debugger_manager_get_debugger_for_process(nil))
}

struct HydraDebugger: Hashable, Identifiable {
    private var handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var id: UnsafeMutableRawPointer {
        self.handle
    }

    var name: String {
        String(withHydraString: hydra_debugger_get_name(self.handle))
    }

    func lock() {
        hydra_debugger_lock(self.handle)
    }

    func unlock() {
        hydra_debugger_unlock(self.handle)
    }

    func registerThisThread(name: String) {
        name.withHydraString { hydraName in
            hydra_debugger_register_this_thread(self.handle, hydraName)
        }
    }

    func unregisterThisThread() {
        hydra_debugger_unregister_this_thread(self.handle)
    }

    var threadCount: Int {
        Int(hydra_debugger_get_thread_count(self.handle))
    }

    func getThread(at index: Int) -> HydraDebuggerThread {
        HydraDebuggerThread(handle: hydra_debugger_get_thread(self.handle, UInt32(index)))
    }
}

struct HydraDebuggerThread: Hashable, Identifiable {
    private var handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    var id: UnsafeMutableRawPointer {
        self.handle
    }

    var name: String {
        String(withHydraString: hydra_debugger_thread_get_name(self.handle))
    }

    func lock() {
        hydra_debugger_thread_lock(self.handle)
    }

    func unlock() {
        hydra_debugger_thread_unlock(self.handle)
    }

    var status: HydraDebuggerThreadStatus {
        hydra_debugger_thread_get_status(self.handle)
    }

    var breakReason: String {
        String(withHydraString: hydra_debugger_thread_get_break_reason(self.handle))
    }

    var messageCount: Int {
        Int(hydra_debugger_thread_get_message_count(self.handle))
    }

    func getMessage(at index: Int) -> HydraDebuggerMessage {
        HydraDebuggerMessage(handle: hydra_debugger_thread_get_message(self.handle, UInt32(index)))
    }
}

struct HydraDebuggerMessage: Hashable, Identifiable {
    private var handle: UnsafeRawPointer

    fileprivate init(handle: UnsafeRawPointer) {
        self.handle = handle
    }

    var id: UnsafeRawPointer {
        self.handle
    }

    var logLevel: HydraLogLevel {
        hydra_debugger_message_get_log_level(self.handle)
    }

    var logClass: HydraLogClass {
        hydra_debugger_message_get_log_class(self.handle)
    }

    var file: String {
        String(withHydraString: hydra_debugger_message_get_file(self.handle))
    }

    var line: UInt32 {
        hydra_debugger_message_get_line(self.handle)
    }

    var function: String {
        String(withHydraString: hydra_debugger_message_get_function(self.handle))
    }

    var str: String {
        String(withHydraString: hydra_debugger_message_get_string(self.handle))
    }

    var stackTrace: HydraDebuggerStackTrace {
        HydraDebuggerStackTrace(
            handle: hydra_debugger_stack_trace_copy(
                hydra_debugger_message_get_stack_trace(self.handle)))
    }
}

class HydraDebuggerStackTrace: Hashable, Identifiable {
    private var handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    deinit {
        hydra_debugger_stack_trace_destroy(self.handle)
    }

    static func == (lhs: HydraDebuggerStackTrace, rhs: HydraDebuggerStackTrace)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    var frameCount: Int {
        Int(hydra_debugger_stack_trace_get_frame_count(self.handle))
    }

    func getFrame(at index: Int) -> HydraDebuggerStackFrame {
        HydraDebuggerStackFrame(
            handle: hydra_debugger_stack_trace_get_frame(self.handle, UInt32(index)))
    }
}

struct HydraDebuggerStackFrame: Hashable, Identifiable {
    private var handle: UnsafeRawPointer

    fileprivate init(handle: UnsafeRawPointer) {
        self.handle = handle
    }

    var id: UnsafeRawPointer {
        self.handle
    }

    func resolve() -> HydraDebuggerResolvedStackFrame {
        HydraDebuggerResolvedStackFrame(handle: hydra_debugger_stack_frame_resolve(self.handle))
    }
}

class HydraDebuggerResolvedStackFrame: Hashable, Identifiable {
    private var handle: UnsafeMutableRawPointer

    fileprivate init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }

    deinit {
        hydra_debugger_resolved_stack_frame_destroy(self.handle)
    }

    static func == (lhs: HydraDebuggerResolvedStackFrame, rhs: HydraDebuggerResolvedStackFrame)
        -> Bool
    {
        lhs.handle == rhs.handle
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(self.handle)
    }

    var module: String {
        String(withHydraString: hydra_debugger_resolved_stack_frame_get_module(self.handle))
    }

    var function: String {
        String(withHydraString: hydra_debugger_resolved_stack_frame_get_function(self.handle))
    }

    var address: UInt64 {
        hydra_debugger_resolved_stack_frame_get_address(self.handle)
    }
}
