import SwiftUI

struct SettingsView: View {
    private enum Tabs: Hashable {
        case general, input, cpu, graphics, audio, user, system, debug
    }

    @State private var selection: Tabs = .general

    var body: some View {
        #if os(macOS)
            TabView(selection: $selection) {
                Tab("General", systemImage: "gear", value: .general) {
                    GeneralSettingsView()
                }

                Tab("Input", systemImage: "gamecontroller", value: .input) {
                    InputSettingsView()
                }

                Tab("CPU", systemImage: "cpu", value: .cpu) {
                    CpuSettingsView()
                }

                Tab("Graphics", systemImage: "star", value: .graphics) {
                    GraphicsSettingsView()
                }

                Tab("Audio", systemImage: "speaker.wave.2", value: .audio) {
                    AudioSettingsView()
                }

                Tab("User", systemImage: "person", value: .user) {
                    UserSettingsView()
                }

                Tab("System", systemImage: "desktopcomputer", value: .system) {
                    SystemSettingsView()
                }

                Tab("Debug", systemImage: "memorychip", value: .debug) {
                    DebugSettingsView()
                }
            }
            .scenePadding()
            .frame(maxWidth: 660, minHeight: 100)
            .frame(width: 660, height: 440) // TODO: Don't hardcode
            .onDisappear {
                hydraConfigSerialize()
            }
        #else
            NavigationView {
                List {
                    // No general settings

                    NavigationLink(destination: InputSettingsView()) {
                        Label("Input", systemImage: "gamecontroller")
                    }

                    NavigationLink(destination: CpuSettingsView()) {
                        Label("CPU", systemImage: "cpu")
                    }

                    NavigationLink(destination: GraphicsSettingsView()) {
                        Label("Graphics", systemImage: "star")
                    }

                    NavigationLink(destination: AudioSettingsView()) {
                        Label("Audio", systemImage: "speaker.wave.2")
                    }

                    NavigationLink(destination: UserSettingsView()) {
                        Label("User", systemImage: "person")
                    }

                    NavigationLink(destination: SystemSettingsView()) {
                        Label("System", systemImage: "desktopcomputer")
                    }

                    NavigationLink(destination: DebugSettingsView()) {
                        Label("Debug", systemImage: "memorychip")
                    }
                }
                .navigationTitle("Settings")
            }
            .onDisappear {
                hydraConfigSerialize()
            }
        #endif
    }
}
