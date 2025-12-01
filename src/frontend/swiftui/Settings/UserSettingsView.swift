import SwiftUI

class UserSettingsState: ObservableObject {
    var filesystem = HydraFilesystem()
    var userManager = HydraUserManager()

    init() {
        self.filesystem.tryInstallFirmware()
        self.userManager.loadSystemAvatars(filesystem: self.filesystem)
    }
}

struct UserSettingsView: View {
    // TODO: this never gets deallocated, is it a problem?
    @StateObject private var state = UserSettingsState()

    @State private var userIDs: [hydra_u128] = []
    @State private var selectedUserIndex: Int = 0

    var body: some View {
        VStack {
            if !self.userIDs.isEmpty {
                NavigationSplitView {
                    List(self.userIDs.indices, id: \.self, selection: self.$selectedUserIndex) {
                        index in
                        NavigationLink(value: index) {
                            let userID = self.userIDs[index]
                            UserPreview(
                                userManager: self.state.userManager,
                                user: self.state.userManager.getUser(id: userID)
                            )
                        }
                    }
                    .navigationTitle("Users")
                    .toolbar(removing: .sidebarToggle)
                } detail: {
                    let userID = self.userIDs[self.selectedUserIndex]
                    // TODO: react to changes
                    UserEditorView(
                        userManager: self.state.userManager,
                        user: self.state.userManager.getUser(id: userID)
                    )
                }
            }
        }
        .onAppear {
            for index in 0..<self.state.userManager.userCount {
                let userID = self.state.userManager.getUserId(at: index)
                self.userIDs.append(userID)
            }
        }
        .onDisappear {
            self.userIDs.removeAll()
            self.state.userManager.flush()
        }
    }
}

