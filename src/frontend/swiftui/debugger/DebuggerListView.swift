import SwiftUI

struct DebuggerListView: View {
    @Binding var activeDebugger: HydraDebugger?

    @State private var debuggers: [HydraDebugger] = []

    var body: some View {
        List {
            ForEach(self.debuggers.indices, id: \.self) { index in
                let debugger = self.debuggers[index]
                ClickableListItem(onClick: {
                    self.activeDebugger = debugger
                }) {
                    Text(debugger.name)
                }
            }
        }
        .onAppear {
            load()
        }
    }

    func load() {
        debuggerManagerLock()

        // Debuggers
        self.debuggers.removeAll()
        for i in 0..<debuggerManagerGetDebuggerCount() {
            // TODO: name
            self.debuggers.append(
                debuggerManagerGetDebugger(at: i))
        }

        debuggerManagerUnlock()
    }
}
