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
        for i in 0..<hydra_string_array_option_get_count(gamePathsOption) {
            if let gamePathRaw = hydra_string_array_option_get(gamePathsOption, UInt32(i)) {
                let gamePath = String(cString: gamePathRaw)
                self.gamePaths.append(gamePath)
            }
        }

        let patchPathsOption = hydra_config_get_patch_paths()
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
        // TODO: save game and patch paths

        let sdCardPathOption = hydra_config_get_sd_card_path()
        hydra_string_option_set(sdCardPathOption, self.sdCardPath.cString(using: .ascii))

        let savePathOption = hydra_config_get_save_path()
        hydra_string_option_set(savePathOption, self.savePath.cString(using: .ascii))
    }
}
