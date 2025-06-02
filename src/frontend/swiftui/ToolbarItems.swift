import SwiftUI

struct ToolbarItems: ToolbarContent {
    @State private var isFilePickerPresented: Bool = false

    var body: some ToolbarContent {
        ToolbarItemGroup(placement: .automatic) {
            Button("Add Game Path", systemImage: "plus") {
                isFilePickerPresented.toggle()
            }
            // TODO: change allowedContentTypes to nsp etc
            .fileImporter(
                isPresented: $isFilePickerPresented, allowedContentTypes: [.data],
                allowsMultipleSelection: false
            ) { result in
                switch result {
                case .success(let fileurls):
                    print(fileurls.count)

                    for fileurl in fileurls {
                        print(fileurl.path)
                    }

                case .failure(let error):
                    print(error)
                }
            }
        }
    }

}
