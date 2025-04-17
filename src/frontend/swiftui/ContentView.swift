import SwiftUI

struct ContentView: View {
    @State private var selection: SidebarSelectionState? = .gamelist
    
    var body: some View {
        NavigationSplitView(sidebar: {
            SidebarView(selection: $selection)
                .frame(minWidth: 200)
        }, detail: {
            DetailView(selection: $selection)
        })
//        .toolbar {
//            ToolbarItems()
//        }
    }
}
