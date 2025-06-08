import SwiftUI

struct MenuCommands: Commands {

    var body: some Commands {
        // Adds a new command to the File menu
        CommandGroup(after: .newItem) {
            Button("Add to Library") {
                // TODO: Implement
            }
            .keyboardShortcut(KeyEquivalent("o"), modifiers: .command)
            
            Button("Boot from File") {
                // TODO: Implement
            }
            .keyboardShortcut(KeyEquivalent("b"), modifiers: .command)
        }

        // This removes/replaces menu items
        CommandGroup(replacing: CommandGroupPlacement.help) {}
        CommandGroup(replacing: CommandGroupPlacement.pasteboard) {}
        CommandGroup(replacing: CommandGroupPlacement.undoRedo) {}
//        CommandGroup(replacing: CommandGroupPlacement.systemServices) {}
    }
}
