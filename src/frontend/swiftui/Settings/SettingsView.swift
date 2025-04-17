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
            GraphicsSettingsView()
                .tabItem {
                    Label("Graphics", systemImage: "star")
                }
                .tag(Tabs.graphics)
            DebugSettingsView()
                .tabItem {
                    Label("Debug", systemImage: "ant")
                }
                .tag(Tabs.debug)
        }
        .padding(20)
        .frame(width: 375, height: 150)
    }
}
