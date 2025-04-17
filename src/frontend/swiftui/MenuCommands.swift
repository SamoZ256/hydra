import SwiftUI

struct MenuCommands: Commands {

    var body: some Commands {
        // Adds a new command to the File menu
        CommandGroup(after: .newItem) {
            Button("Boot Game") {

            }
            .keyboardShortcut(KeyEquivalent("b"), modifiers: /*@START_MENU_TOKEN@*/.command/*@END_MENU_TOKEN@*/)
        }

        // This is an example of removing menu items, in this case from the help menu
        CommandGroup(replacing: CommandGroupPlacement.help) {}

        // Add the show/hide sidebar command and shortcut to the menu
        SidebarCommands()
    }
}
