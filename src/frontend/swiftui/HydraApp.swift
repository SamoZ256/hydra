import SwiftUI

@main
struct MyApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
                .frame(minWidth: 1280, minHeight: 720)
        }
        .windowResizability(.contentSize)
        .commands {
            MenuCommands()
        }
    }
}
