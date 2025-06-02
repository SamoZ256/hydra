import SwiftUI

struct StringList: View {
    @Binding var items: [String]

    var body: some View {
        List {
            ForEach(items, id: \.self) { item in
                HStack {
                    Text(item)
                    Spacer()
                    Button(action: {
                        if let index = items.firstIndex(of: item) {
                            items.remove(at: index)
                        }
                    }) {
                        Image(systemName: "trash")
                            .foregroundColor(.red)
                    }
                    .buttonStyle(BorderlessButtonStyle())
                }
            }
        }
        .listStyle(PlainListStyle())
    }
}
