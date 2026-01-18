import SwiftUI

struct MenuCommands: Commands {
    @Environment(\.openWindow) var openWindow

    @Binding var globalState: GlobalState

    @State private var firmwareApplets: [Game] = []

    var body: some Commands {
        CommandGroup(after: .newItem) {
            Button("Load from file") {
                // TODO: implement
                print("NOT IMPLEMENTED")
            }
            .keyboardShortcut(KeyEquivalent("l"), modifiers: .command)

            Menu("Load from firmware") {
                ForEach(self.firmwareApplets, id: \.self) { firmwareApplet in
                    Button(firmwareApplet.name) {
                        globalState.activeGame = firmwareApplet
                    }
                }
            }
            .disabled(self.firmwareApplets.count == 0)
            .onAppear {
                self.loadFirmware()
            }
            .onDisappear {
                self.firmwareApplets.removeAll()
            }
        }

        CommandGroup(after: .sidebar) {
            Button("Take Screenshot", systemImage: "camera") {
                guard let emulationContext = globalState.emulationContext else { return }
                emulationContext.takeScreenshot()
            }
            .keyboardShortcut(KeyEquivalent("t"), modifiers: .command)
        }

        CommandMenu("Emulation") {
            Button("Stop") {
                globalState.isStopping = true
            }
            Button("Switch to \(globalState.isHandheldMode ? "Console" : "Handheld") mode") {
                globalState.isHandheldMode = !globalState.isHandheldMode
                hydraConfigGetHandheldMode().pointee = globalState.isHandheldMode
                hydraConfigSerialize()

                guard let emulationContext = globalState.emulationContext else { return }
                emulationContext.notifyOperationModeChanged()
            }
            .keyboardShortcut(KeyEquivalent("o"), modifiers: .command)
        }

        CommandMenu("Debug") {
            Button("Capture GPU Frame") {
                guard let emulationContext = globalState.emulationContext else { return }
                emulationContext.captureGpuFrame()
            }
            .keyboardShortcut(KeyEquivalent("p"), modifiers: .command)
        }

        // Remove some items
        CommandGroup(replacing: .undoRedo) {}
        // CommandGroup(replacing: .pasteboard) {}
        // CommandGroup(replacing: .systemServices) {}
    }

    func loadFirmware() {
        let firmwarePath = hydraConfigGetFirmwarePath()
        if firmwarePath == "" {
            return
        }

        // TODO: don't iterate recursively?
        guard
            let enumerator = FileManager.default.enumerator(
                at: URL(fileURLWithPath: firmwarePath), includingPropertiesForKeys: nil)
        else {
            // TODO: error popup
            print("Invalid firmware directory \(firmwarePath)")
            return
        }

        while let url = enumerator.nextObject() as? URL {
            self.addFirmwareApplet(url: url)
        }
    }

    func addFirmwareApplet(url: URL) {
        let file = HydraFile(path: url.path(percentEncoded: false))
        let contentArchive = HydraContentArchive(file: file)

        if contentArchive.contentType
            != HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_PROGRAM
        {
            return
        }

        let loader = HydraNcaLoader(contentArchive: contentArchive)

        let name = loader.name
        if name == "" {
            return
        }

        self.firmwareApplets.append(
            Game(url: url, loader: loader, name: name, author: "Nintendo", version: "0"))
    }
}
