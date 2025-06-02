import SwiftUI

struct GamePreview: View {
    let game: Game
    @State private var lastClickTime: Date? = nil
    @State private var navigate = false

    var body: some View {
        // TODO: this type of NavigationLink usage is deprecated
        NavigationLink(destination: EmulationView(game: game), isActive: $navigate) {
            // TODO: display logo as well
            Text(game.path)
                .padding()
                .contentShape(Rectangle())
                .onTapGesture {
                    if let lastTime = lastClickTime, Date().timeIntervalSince(lastTime) < 0.3 {
                        navigate = true
                        lastClickTime = nil
                    } else {
                        lastClickTime = Date()
                    }
                }
        }
        .buttonStyle(PlainButtonStyle())
    }
}
