import SwiftUI
import UniformTypeIdentifiers

struct GameExtractButton: View {
    let game: Game
    let content: HydraLoaderContent

    @Binding var showAlert: Bool
    @Binding var alertMessage: String

    private var name: String {
        switch self.content {
        case .icon:
            return "Icon"
        case .exefs:
            return "ExeFS"
        case .romfs:
            return "RomFS"
        }
    }

    private var contentType: UTType {
        switch self.content {
        case .icon:
            return .jpeg
        case .exefs, .romfs:
            return .folder
        }
    }

    var body: some View {
        Button(self.name) {
            #if os(macOS)
                SavePanel.present(allowedContentTypes: [self.contentType], defaultFilename: nil) {
                    url in
                    guard let url = url else {
                        return
                    }

                    DispatchQueue.global(qos: .userInitiated).async {
                        #if HYDRA_DEBUGGER_ENABLED
                            hydraDebuggerManagerGetDebuggerForCurrentProcess()
                                .registerThisThread(name: "Content extract")
                        #endif
                        self.game.loader.extractContent(
                            self.content, to: url.path(percentEncoded: false))
                        self.alertMessage = "\(self.name) extracted successfully"
                        self.showAlert = true
                        #if HYDRA_DEBUGGER_ENABLED
                            hydraDebuggerManagerGetDebuggerForCurrentProcess()
                                .unregisterThisThread()
                        #endif
                    }
                }
            #else
                // TODO: implement
                self.alertMessage = "\(self.name) extraction isn't supported on iOS yet"
                self.showAlert = true
            #endif
        }
        .disabled(!self.game.loader.hasContent(self.content))
    }
}
