import SwiftUI

struct SettingsView: View {
    private enum Tabs: Hashable {
        case general, graphics, debug
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
                .tag(Tabs.graphics)
            GraphicsSettingsView()
                .tabItem {
                    Label("Graphics", systemImage: "star")
                }
                .tag(Tabs.graphics)
            // TODO: user settings
            DebugSettingsView()
                .tabItem {
                    Label("Debug", systemImage: "memorychip")
                }
                .tag(Tabs.debug)
        }
        .padding(20)
        // TODO: don't hardcode the size
        .frame(width: 375, height: /*150*/ 400)
        .onDisappear {
            hydra_config_serialize()
        }
    }
}
