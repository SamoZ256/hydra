import SwiftUI

struct EditablePathList: View {
    @Binding var items: [String]

    @State private var isFilePickerPresented: Bool = false

    var body: some View {
        VStack {
            StringList(items: self.$items)

            HStack {
                Spacer()

                Button(action: {
                    isFilePickerPresented.toggle()
                }) {
                    Image(systemName: "plus")
                        .foregroundColor(.gray)
                        .font(.system(size: 16))
                }
                .buttonStyle(PlainButtonStyle())
                // TODO: allow specifying allowedContentTypes
                .fileImporter(
                    isPresented: $isFilePickerPresented, allowedContentTypes: [.data],
                    allowsMultipleSelection: false
                ) { result in
                    switch result {
                    case .success(let fileURLs):
                        for fileURL in fileURLs {
                            items.append(fileURL.path())
                        }

                    case .failure(let error):
                        print(error)
                    }
                }
            }
        }
    }
}
