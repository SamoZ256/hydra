import SwiftUI

struct MenuCommands: Commands {
    @Environment(\.openWindow) var openWindow

    @Binding var activeGame: Game?

    @State var firmwareApplets: [Game] = []

    var body: some Commands {
        CommandGroup(after: .newItem) {
            Button("Load from file") {
                // TODO: Implement
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
                for firmwareApplet in self.firmwareApplets {
                    hydra_loader_destroy(firmwareApplet.loader)
                }
            }
        }

        CommandMenu("Emulation") {
            Button("Stop", systemImage: "stop.fill") {
                // TODO
                //hydra_emulation_context_request_stop(self.emulationContext!)
                //hydra_emulation_context_force_stop(self.emulationContext!)
                self.activeGame = nil
            }
        }

        // Remove some items
        // CommandGroup(replacing: .help) {}
        // CommandGroup(replacing: .pasteboard) {}
        CommandGroup(replacing: .undoRedo) {}
        // CommandGroup(replacing: .systemServices) {}
    }

    func loadFirmware() {
        let firmwarePathOption = hydra_config_get_firmware_path()
        let firmwarePath = String(cString: hydra_string_option_get(firmwarePathOption))
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
        guard let file = hydra_open_file(path) else {
            return
        }
        //defer { hydra_file_close(file) }

        guard let contentArchive = hydra_create_content_archive(file) else {
            return
        }
        defer { hydra_content_archive_destroy(contentArchive) }

        if hydra_content_archive_get_content_type(contentArchive)
            != HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_PROGRAM
        {
            return
        }

        guard let loader = hydra_create_nca_loader_from_content_archive(contentArchive) else {
            return
        }

        let name = String(cString: hydra_nca_loader_get_name(loader))
        if name == "" {
            return
        }

        self.firmwareApplets.append(Game(loader: loader, name: name))
    }
}
