import SwiftUI

struct MenuCommands: Commands {
    @Environment(\.openWindow) var openWindow

    @Binding var activeGame: Game?
    @Binding var emulationContext: HydraEmulationContext?

    @State private var firmwareApplets: [Game] = []
    @State private var handheldMode = false

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
                        self.activeGame = firmwareApplet
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
                guard let emulationContext = self.emulationContext else { return }
                emulationContext.takeScreenshot()
            }
            .keyboardShortcut(KeyEquivalent("t"), modifiers: .command)
        }

        CommandMenu("Emulation") {
            Button("Stop") {
                guard let emulationContext = self.emulationContext else { return }
                emulationContext.requestStop()
                // TODO: wait a bit?
                emulationContext.forceStop()
                self.activeGame = nil
            }
            Button("Switch to \(self.handheldMode ? "Console" : "Handheld") mode") {
                self.handheldMode = !self.handheldMode
                var handheldModeOption = hydraConfigGetHandheldMode()
                handheldModeOption.value = self.handheldMode

                guard let emulationContext = self.emulationContext else { return }
                emulationContext.notifyOperationModeChanged()
            }
            .keyboardShortcut(KeyEquivalent("o"), modifiers: .command)
            .onAppear {
                let handheldModeOption = hydraConfigGetHandheldMode()
                self.handheldMode = handheldModeOption.value
            }
        }

        CommandMenu("Debug") {
            Button("Capture GPU Frame") {
                guard let emulationContext = self.emulationContext else { return }
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
        let firmwarePathOption = hydraConfigGetFirmwarePath()
        let firmwarePath = firmwarePathOption.value
        if firmwarePath == "" {
            return
        }

        // TODO: do all of this with URLs
        let fileManager = FileManager.default

        // TODO: don't iterate recursively?
        guard let enumerator = fileManager.enumerator(atPath: firmwarePath) else {
            // TODO: error popup
            print("Invalid firmware directory \(firmwarePath)")
            return
        }

        while let filename = enumerator.nextObject() as? String {
            let path = "\(firmwarePath)/\(filename)"
            self.addFirmwareApplet(path: path)
        }
    }

    func addFirmwareApplet(path: String) {
        let file = HydraFile(path: path)
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

        self.firmwareApplets.append(Game(loader: loader, name: name, author: "Nintendo"))
    }
}
