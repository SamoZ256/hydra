import SwiftUI

struct GeneralSettingsView: View {
    @State private var gamePaths: [String] = []
    @State private var patchPaths: [String] = []
    @State private var sdCardPath: String = ""
    @State private var savePath: String = ""

    var body: some View {
        VStack {
            Text("Game paths")
            EditablePathList(items: self.$gamePaths)
            Text("Patch paths")
            EditablePathList(items: self.$patchPaths)

            // TODO: make these editable
            Text("SD card path: \(self.sdCardPath)")
            Text("Save path: \(self.savePath)")
        }
        .onAppear {
            load()
        }
        .onDisappear {
            save()
        }
    }

    func load() {
        let gamePathsOption = hydra_config_get_game_paths()
        self.gamePaths = []
        for i in 0..<hydra_string_array_option_get_count(gamePathsOption) {
            if let gamePathRaw = hydra_string_array_option_get(gamePathsOption, UInt32(i)) {
                let gamePath = String(cString: gamePathRaw)
                self.gamePaths.append(gamePath)
            }
        }

        let patchPathsOption = hydra_config_get_patch_paths()
        self.patchPaths = []
        for i in 0..<hydra_string_array_option_get_count(patchPathsOption) {
            if let patchPathRaw = hydra_string_array_option_get(patchPathsOption, UInt32(i)) {
                let patchPath = String(cString: patchPathRaw)
                self.patchPaths.append(patchPath)
            }
        }

        let sdCardPathOption = hydra_config_get_sd_card_path()
        self.sdCardPath = String(cString: hydra_string_option_get(sdCardPathOption))

        let savePathOption = hydra_config_get_save_path()
        self.savePath = String(cString: hydra_string_option_get(savePathOption))
    }

    func save() {
        // TODO: simplify this?
        let gamePathsOption = hydra_config_get_game_paths()
        for i in 0..<self.gamePaths.count {
            hydra_string_array_option_set(
                gamePathsOption, UInt32(i), self.gamePaths[i].cString(using: .ascii))
        }
        // HACK: need to do it this way in order to avoid "failed to produce diagnostic for expression"
        var i = self.gamePaths.count
        while i < hydra_string_array_option_get_count(gamePathsOption) {
            hydra_string_array_option_remove(gamePathsOption, UInt32(i))
            i += 1
        }

        let patchPathsOption = hydra_config_get_patch_paths()
        for i in 0..<self.patchPaths.count {
            hydra_string_array_option_set(
                patchPathsOption, UInt32(i), self.patchPaths[i].cString(using: .ascii))
        }
        i = self.patchPaths.count
        while i < hydra_string_array_option_get_count(patchPathsOption) {
            hydra_string_array_option_remove(patchPathsOption, UInt32(i))
            i += 1
        }

        let sdCardPathOption = hydra_config_get_sd_card_path()
        hydra_string_option_set(sdCardPathOption, self.sdCardPath.cString(using: .ascii))

        let savePathOption = hydra_config_get_save_path()
        hydra_string_option_set(savePathOption, self.savePath.cString(using: .ascii))
    }
}
