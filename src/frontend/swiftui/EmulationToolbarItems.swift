import SwiftUI

struct EmulationToolbarItems: ToolbarContent {
    @EnvironmentObject var globalState: GlobalState

    @State private var isRunning: Bool = false
    @State private var isFramerateUnlocked = false

    var body: some ToolbarContent {
        #if os(macOS)
            ToolbarItemGroup(placement: .principal) {
                if isRunning {
                    Button("Pause", systemImage: "pause") {
                        guard let emulationContext = globalState.emulationContext else { return }
                        emulationContext.pause()
                        isRunning = false
                    }
                } else {
                    Button("Resume", systemImage: "play") {
                        guard let emulationContext = globalState.emulationContext else { return }
                        emulationContext.resume()
                        isRunning = true
                    }
                }
                Button("Stop", systemImage: "stop") {
                    globalState.isStopping = true
                }
                .onAppear {
                    isRunning = globalState.emulationContext!.isRunning()
                }
            }
            
            ToolbarItemGroup(placement: .confirmationAction) {
                if isFramerateUnlocked {
                    Button{
                        isFramerateUnlocked.toggle()
                    } label: {
                        ZStack {
                            Image(systemName: "square.stack.3d.down.forward")
                            Image(systemName: "lock.open")
                                .symbolVariant(.none)
                                .font(.footnote)
                                .offset(x: 10, y: 5)
                        }
                    }
                    .help("Lock Framerate to 60fps")
                } else {
                    Button{
                        isFramerateUnlocked.toggle()
                    } label: {
                        ZStack {
                            Image(systemName: "square.stack.3d.down.forward")
                            Image(systemName: "lock")
                                .symbolVariant(.none)
                                .font(.footnote)
                                .offset(x: 10, y: 5)
                        }
                    }
                    .help("Unlock Framerate")
                }
            }
            
            // This compiler check is only needed when compiling on a macOS version earlier than 26
            #if compiler(>=6.2.3)
                if #available(macOS 26.0, *) {
                    ToolbarSpacer(.fixed)
                }
            #endif
            
            if #available(macOS 26.0, *) {
                ToolbarItemGroup(placement: .confirmationAction) {
                    Button("Console Mode", systemImage: "inset.filled.tv") {
                        globalState.isHandheldMode.toggle()
                    }
                    .disabled(!globalState.isHandheldMode)
                    .help("Change to Console mode")
                    
                    Button("Handheld Mode", systemImage: "formfitting.gamecontroller.fill") {
                        globalState.isHandheldMode.toggle()
                    }
                    .disabled(globalState.isHandheldMode)
                    .help("Change to Handheld mode")
                }
            } else {
                ToolbarItemGroup(placement: .confirmationAction) {
                    if globalState.isHandheldMode {
                        Button("Console Mode", systemImage: "inset.filled.tv") {
                            globalState.isHandheldMode.toggle()
                        }
                        .help("Change to Console mode")
                    } else {
                       Button("Handheld Mode", systemImage: "formfitting.gamecontroller.fill") {
                           globalState.isHandheldMode.toggle()
                       }
                       .help("Change to Handheld mode")
                    }
                }
            }
        #else
            // TODO: options
            ToolbarItemGroup(placement: .principal) {}
        #endif
    }
}
