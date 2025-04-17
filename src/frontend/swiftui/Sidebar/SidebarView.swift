import SwiftUI

struct SidebarView: View {
    @Binding var selection: SidebarSelectionState?

    var body: some View {
        List(selection: $selection) {
            ForEach(SidebarSelectionState.allCases) { selection in
                Label(selection.displayName, systemImage: selection.iconName)
                    .tag(selection)
                    .foregroundColor(.primary)
            }
        }
    }
}
