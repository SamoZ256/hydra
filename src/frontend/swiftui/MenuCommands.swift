import SwiftUI

struct MenuCommands: Commands {

    var body: some Commands {
        // Adds a new command to the File menu
        /*
        CommandGroup(after: .newItem) {
            Button("Boot Game") {

            }
            .keyboardShortcut(KeyEquivalent("b"), modifiers: .command)
        }
        */

        // This removes/replaces menu items
        CommandGroup(replacing: CommandGroupPlacement.help) {}
        CommandGroup(replacing: CommandGroupPlacement.pasteboard) {}
        CommandGroup(replacing: CommandGroupPlacement.systemServices) {}
        CommandGroup(replacing: CommandGroupPlacement.undoRedo) {}

        // Add the show/hide sidebar command and shortcut to the menu
        SidebarCommands()
    }
}
