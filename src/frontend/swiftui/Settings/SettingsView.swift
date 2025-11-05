import SwiftUI

struct SettingsView: View {
    private enum Tabs: Hashable {
        case general, cpu, graphics, audio, user, system, debug
    }

    var body: some View {
        TabView {
            GeneralSettingsView()
                .tabItem {
                    Label("General", systemImage: "gear")
                }
                .tag(Tabs.general)
            CpuSettingsView()
                .tabItem {
                    Label("CPU", systemImage: "cpu")
                }
                .tag(Tabs.cpu)
            GraphicsSettingsView()
                .tabItem {
                    Label("Graphics", systemImage: "star")
                }
                .tag(Tabs.graphics)
            AudioSettingsView()
                .tabItem {
                    Label("Audio", systemImage: "speaker.wave.2")
                }
                .tag(Tabs.audio)
            UserSettingsView()
                .tabItem {
                    Label("User", systemImage: "person")
                }
                .tag(Tabs.user)
            SystemSettingsView()
                .tabItem {
                    Label("System", systemImage: "desktopcomputer")
                }
                .tag(Tabs.system)
            #if DEBUG
                DebugSettingsView()
                    .tabItem {
                        Label("Debug", systemImage: "memorychip")
                    }
                    .tag(Tabs.debug)
            #endif
        }
        .padding(20)
        // TODO: don't hardcode the size
        .frame(width: 800, height: 600)
        .onDisappear {
            hydraConfigSerialize()
        }
    }
}
