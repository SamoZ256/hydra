import SwiftUI

struct UserPreview: View {
    let userManager: HydraUserManager

    @State private var user: HydraUser

    var body: some View {
        HStack {
            ZStack {
                let avatarBgColor = self.user.avatarBgColor
                Rectangle()
                    .fill(
                        Color(
                            red: Double(avatarBgColor.x) / 255.0,
                            green: Double(avatarBgColor.y) / 255.0,
                            blue: Double(avatarBgColor.z) / 255.0))
                UserAvatarView(
                    userManager: self.userManager, avatarPath: self.user.avatarPath
                )
            }
            .frame(maxWidth: 128, maxHeight: 128)  // TODO: don't hardcode?
            Text(self.user.nickname)
        }
    }
}
