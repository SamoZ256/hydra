import SwiftUI

struct EditableStringList: View {
    @Binding var items: [String]

    @State private var newItemText: String = ""

    var body: some View {
        VStack {
            StringList(items: self.$items)

            HStack {
                // TODO: make the text configurable
                TextField("Enter new item", text: self.$newItemText)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                    .padding(.leading)

                Button(action: addItem) {
                    Text("Add")
                }
                .disabled(newItemText.isEmpty)
                .padding(.trailing)
            }
            .padding(.top)
        }
    }

    func addItem() {
        if !newItemText.isEmpty {
            items.append(newItemText)
            newItemText = ""
        }
    }
}
