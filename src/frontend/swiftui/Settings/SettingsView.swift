import SwiftUI

struct SettingsView: View {
    private enum Tabs: Hashable {
        case general, cpu, graphics, audio, user, system, debug
    }
    
    @State private var selection: Tabs = .general
  
    var body: some View {
        #if os(macOS)
            TabView {
                Tab("General", systemImage: "gear", value: .general) {
                    GeneralSettingsView()
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
            .frame(maxWidth: 500, minHeight: 100)
            .onDisappear {
                hydraConfigSerialize()
            }
        #else
            NavigationView {
                List {
                    // No general settings
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
