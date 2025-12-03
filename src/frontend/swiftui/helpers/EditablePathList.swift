import SwiftUI
import UniformTypeIdentifiers

struct EditablePathList: View {
    var allowedContentTypes: [UTType]
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
                    isPresented: $isFilePickerPresented,
                    allowedContentTypes: self.allowedContentTypes,
                    allowsMultipleSelection: false
                ) { result in
                    switch result {
                    case .success(let fileURLs):
                        for fileURL in fileURLs {
                            do {
                                try registerUrl(fileURL)
                            } catch {
                                print("Failed to register URL \(fileURL.path)")
                                continue
                            }
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
