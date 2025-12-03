import SwiftUI

struct UserEditorView: View {
    let userManager: HydraUserManager
    let user: HydraUser

    @State private var showAvatarEditor = false

    @State private var nickname: String = ""
    @State private var avatarBgColor: hydra_uchar3 = hydra_uchar3(x: 0, y: 0, z: 0)
    @State private var avatarPath: String = ""

    var body: some View {
        VStack {
            ZStack {
                Rectangle()
                    .fill(
                        Color(
                            red: Double(self.avatarBgColor.x) / 255.0,
                            green: Double(self.avatarBgColor.y) / 255.0,
                            blue: Double(self.avatarBgColor.z) / 255.0))
                UserAvatarView(
                    userManager: self.userManager, avatarPath: self.avatarPath
                )

                // Edit button
                Button(action: {
                    self.showAvatarEditor = true
                }) {
                    Image(systemName: "pencil")
                        .foregroundColor(.white)
                        .padding()
                }
            }
            .frame(maxWidth: 128, maxHeight: 128)  // TODO: don't hardcode?
            TextField(
                "Nickname",
                text: self.$nickname
            )
            .disableAutocorrection(true)
            .onSubmit {
                var user = self.user
                user.nickname = self.nickname
            }
        }
        .sheet(isPresented: self.$showAvatarEditor) {
            // Title bar
            HStack {
                Text("Select an Image")
                    .font(.headline)
                Spacer()
                Button("Cancel") {
                    self.showAvatarEditor = false
                }
                .keyboardShortcut(.cancelAction)
            }
            .padding()
            // TODO: uncomment?
            //.background(Color(nsColor: .controlBackgroundColor))

            Divider()

            // Editor view
            UserAvatarEditorView(
                userManager: self.userManager, avatarBgColor: self.$avatarBgColor,
                avatarPath: self.$avatarPath
            )
            .onChange(of: self.avatarBgColor) { _, newValue in
                var user = self.user
                user.avatarBgColor = newValue
            }
            .onChange(of: self.avatarPath) { _, newValue in
                var user = self.user
                user.avatarPath = newValue
            }
        }
        .onAppear {
            self.nickname = self.user.nickname
            self.avatarBgColor = self.user.avatarBgColor
            self.avatarPath = self.user.avatarPath
        }
    }
}
